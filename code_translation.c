/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_translation.h"
#include "code_optimization.h"
#include "instruction.h"
#include "symbol-table.h"

static int NUM_RESERVED_REG = 2;

static void print_println();
static void print_function_header(char *function_name);
static void load_int_to_register(int integer, char *reg);
static void load_from_memory(symtabnode *addr, char *reg, int dest_type);
static void store_at_memory(symtabnode *addr, char *reg);
static void copy_from_register(char *reg_src, char *reg_dest);
static char *get_operation_name(enum InstructionType type);
static char get_mem_op_type(int type);
static char *get_register_name(int reg);
static bool is_var_in_memory(symtabnode *var);
static int find_register(symtabnode *var, int default_reg);
static void load_reg_allocated_variables_from_memory(inode *instruction);
static void save_reg_allocated_variables_in_memory(inode *instruction);
static void reg_to_char(char *reg);
static void save_registers_at_function_enter(symtabnode *function_ptr);
static void restore_callee_saved_registers(symtabnode* function_ptr);

void print_pre_defined_instructions() {
  print_println();
  print_function_header("~ ENTRY POINT ~");
  printf("main: j _main \n");
}

void print_instructions(tnode *node) {
  inode *last_instruction = NULL;
  inode *curr_instruction = node->code_head;

  while (curr_instruction) {
    if (curr_instruction->dead) {
      curr_instruction = curr_instruction->next;
      continue;
    }

    switch (curr_instruction->op_type) {
    case OP_Global: {
      if (!last_instruction || last_instruction->op_type != OP_Global) {
        printf("\n");
        printf("# -------------------------- \n");
        printf("# GLOBALS                    \n");
        printf("# -------------------------- \n");
        printf(".data \n");
      }

      int byte_size;

      switch (SRC1(curr_instruction)->type) {
      case t_Int:
        byte_size = 4;
        break;
      case t_Char:
        byte_size = 1;
        break;
      case t_Array: {
        int num_elements = SRC1(curr_instruction)->num_elts;
        if (SRC1(curr_instruction)->elt_type == t_Char) {
          byte_size = num_elements;
        } else {
          byte_size = 4 * num_elements;
        }
        break;
      }
      default:
        break;
      }

      printf("_%s:.space %d \n", SRC1(curr_instruction)->name, byte_size);
      if (byte_size % 4 != 0) {
        printf(".align 2 \n");
      }
      break;
    }
    case OP_Enter: {
      if (last_instruction && last_instruction->op_type == OP_Global) {
        printf("\n.text \n");
      }

      symtabnode *function_ptr = SRC1(curr_instruction);
      print_function_header(function_ptr->name);
      printf("_%s:              \n", function_ptr->name);
      save_registers_at_function_enter(function_ptr);
      printf("  la $fp, 0($sp)  \n");
      printf("  la $sp, %d($sp) \n", -function_ptr->byte_size);
      break;
    }

    case OP_Assign: {
      printf("\n");
      printf("  # OP_Assign      \n");

      int src_reg = find_register(SRC1(curr_instruction), 0);
      int dest_reg = find_register(curr_instruction->dest, 0);
      char *src_reg_name = get_register_name(src_reg);
      char *dest_reg_name = get_register_name(dest_reg);

      if (is_var_in_memory(SRC1(curr_instruction))) {
        if (is_var_in_memory(curr_instruction->dest)) {
          load_from_memory(SRC1(curr_instruction), src_reg_name,
                           SRC1(curr_instruction)->type);
        } else {
          // Copy directly to the register of the target variable
          load_from_memory(SRC1(curr_instruction), dest_reg_name,
                           SRC1(curr_instruction)->type);
          if (curr_instruction->dest->type == t_Char &&
              SRC1(curr_instruction)->type == t_Int) {
            reg_to_char(dest_reg_name);
          }
        }
      }

      if (curr_instruction->dest->type == t_Addr) {
        // The LHS is an array memory location, therefore we load the word in
        // curr_instruction->dest instead of its address. And the type of the
        // value to be stored in the array location is determined by the type
        // of the elements in the array.
        load_from_memory(curr_instruction->dest, "$t1", t_Word);
        char mem_op_type = get_mem_op_type(curr_instruction->dest->elt_type);
        printf("  s%c %s, 0($t1) \n", mem_op_type, src_reg_name);
      } else {
        if (!is_var_in_memory(SRC1(curr_instruction)) ||
            is_var_in_memory(curr_instruction->dest)) {
          if (is_var_in_memory(curr_instruction->dest)) {
            store_at_memory(curr_instruction->dest, src_reg_name);
          } else {
            if (src_reg == dest_reg) {
              printf("  # move %s, %s \n", dest_reg_name, src_reg_name);
            } else {
              // Copy from one register to the other
              copy_from_register(src_reg_name, dest_reg_name);
            }
            if (curr_instruction->dest->type == t_Char &&
                SRC1(curr_instruction)->type == t_Int) {
              reg_to_char(dest_reg_name);
            }
          }
        }
      }
      break;
    }
    case OP_Param: {
      printf("\n");
      printf("  # OP_Param       \n");

      int reg = find_register(SRC1(curr_instruction), 0);
      char *reg_name = get_register_name(reg);

      if (SRC1(curr_instruction)->formal &&
          SRC1(curr_instruction)->type == t_Array) {
        // When a function passes one of its formal to another, just copy
        // the whole word if it stores a memory address
        load_from_memory(SRC1(curr_instruction), reg_name, t_Word);
        printf("  la $sp, -4($sp)  \n");
        printf("  sw $t0, 0($sp)   \n");
      } else {
        if (is_var_in_memory(SRC1(curr_instruction))) {
          load_from_memory(SRC1(curr_instruction), reg_name,
                           SRC1(curr_instruction)->type);
        }
      }
      printf("  la $sp, -4($sp)  \n");
      printf("  sw %s, 0($sp)    \n", reg_name);

      break;
    }

    case OP_Call: {
      save_reg_allocated_variables_in_memory(curr_instruction);
      symtabnode *function_ptr = SRC1(curr_instruction);
      printf("\n");
      printf("  # OP_Call       \n");
      printf("  jal _%s         \n", function_ptr->name);
      printf("  la $sp, %d($sp) \n", 4 * function_ptr->num_formals);
      load_reg_allocated_variables_from_memory(curr_instruction);
      break;
    }

    case OP_Leave:
      printf("\n");
      printf("  # OP_Leave    \n");
      restore_callee_saved_registers(SRC1(curr_instruction));
      break;

    case OP_Return:
      printf("\n");
      printf("  # OP_Return    \n");
      if (SRC1(curr_instruction)) {
        if (is_var_in_memory(SRC1(curr_instruction))) {
          load_from_memory(SRC1(curr_instruction), "$v0",
                           SRC1(curr_instruction)->type);
        } else {
          int reg = find_register(SRC1(curr_instruction), 0);
          copy_from_register(get_register_name(reg), "$v0");
        }
      }
      printf("  la $sp, 0($fp) \n");
      printf("  lw $ra, 0($sp) \n");
      printf("  lw $fp, 4($sp) \n");
      printf("  la $sp, 8($sp) \n");
      printf("  jr $ra         \n");
      break;

    case OP_Retrieve:
      printf("\n");
      printf("  # OP_Retrieve    \n");
      if (is_var_in_memory(curr_instruction->dest)) {
        store_at_memory(curr_instruction->dest, "$v0");
      } else {
        int dest_reg = find_register(curr_instruction->dest, 0);
        copy_from_register("$v0", get_register_name(dest_reg));
      }
      break;

    case OP_UMinus:
      printf("\n");
      printf("  # OP_UMinus    \n");
      int src_reg = find_register(SRC1(curr_instruction), 0);
      int dest_reg = find_register(curr_instruction->dest, 0);
      char *src_reg_name = get_register_name(src_reg);
      char *dest_reg_name = get_register_name(dest_reg);

      if (is_var_in_memory(SRC1(curr_instruction))) {
        load_from_memory(SRC1(curr_instruction), src_reg_name,
                         SRC1(curr_instruction)->type);
      }

      printf("  neg %s, %s \n", dest_reg_name, src_reg_name);
      if (is_var_in_memory(curr_instruction->dest)) {
        store_at_memory(curr_instruction->dest, dest_reg_name);
      } else if (curr_instruction->dest->type == t_Char) {
        reg_to_char(dest_reg_name);
      }

      break;

    case OP_BinaryArithmetic: {
      printf("\n");
      printf("  # OP_BinaryArithmetic    \n");
      int src1_reg = find_register(SRC1(curr_instruction), 0);
      int src2_reg = find_register(SRC2(curr_instruction), 1);
      int dest_reg = find_register(curr_instruction->dest, 0);
      char *src1_reg_name = get_register_name(src1_reg);
      char *src2_reg_name = get_register_name(src2_reg);
      char *dest_reg_name = get_register_name(dest_reg);
      if (is_var_in_memory(SRC1(curr_instruction))) {
        load_from_memory(SRC1(curr_instruction), src1_reg_name,
                         SRC1(curr_instruction)->type);
      }
      if (is_var_in_memory(SRC1(curr_instruction))) {
        load_from_memory(SRC2(curr_instruction), src2_reg_name,
                         SRC2(curr_instruction)->type);
      }

      char *op_name = get_operation_name(curr_instruction->type);
      printf("  %s %s, %s, %s \n", op_name, dest_reg_name, src1_reg_name,
             src2_reg_name);
      if (is_var_in_memory(curr_instruction->dest)) {
        store_at_memory(curr_instruction->dest, dest_reg_name);
      } else if (curr_instruction->dest->type == t_Char) {
        reg_to_char(dest_reg_name);
      }
      break;
    }

    case OP_Label:
      printf("\n");
      printf("  # OP_Label \n");
      printf("  _%s:       \n", curr_instruction->label);
      break;

    case OP_If: {
      printf("\n");
      printf("  # OP_If \n");
      int src1_reg = find_register(SRC1(curr_instruction), 0);
      int src2_reg = find_register(SRC2(curr_instruction), 1);
      char *src1_reg_name = get_register_name(src1_reg);
      char *src2_reg_name = get_register_name(src2_reg);
      if (is_var_in_memory(SRC1(curr_instruction))) {
        load_from_memory(SRC1(curr_instruction), src1_reg_name,
                         SRC1(curr_instruction)->type);
      }
      if (is_var_in_memory(SRC2(curr_instruction))) {
        load_from_memory(SRC2(curr_instruction), src2_reg_name,
                         SRC2(curr_instruction)->type);
      }
      char *op_name = get_operation_name(curr_instruction->type);
      printf("  b%s %s, %s, _%s \n", op_name, src1_reg_name, src2_reg_name,
             curr_instruction->jump_to->label);
      break;
    }

    case OP_Goto:
      printf("\n");
      printf("  # OP_Goto \n");
      printf("  j _%s     \n", curr_instruction->jump_to->label);
      break;

    case OP_Index_Array: {
      printf("\n");
      printf("  # OP_Index_Array \n");
      int src_reg = find_register(SRC1(curr_instruction), 0);
      int dest_reg = find_register(curr_instruction->dest, 0);
      char *src_reg_name = get_register_name(src_reg);
      char *dest_reg_name = get_register_name(dest_reg);

      if (is_var_in_memory(SRC1(curr_instruction))) {
        load_from_memory(SRC1(curr_instruction), src_reg_name,
                         SRC1(curr_instruction)->type);
      }
      // Load address of the first position of the array into $t1
      if (SRC2(curr_instruction)->formal) {
        // If it's a formal, the address of the first position of the array
        // will be stored in the stack, therefore we read the content instead
        // of the address of the formal in the stack;
        load_from_memory(SRC2(curr_instruction), "$t1", t_Word);
      } else {
        load_from_memory(SRC2(curr_instruction), "$t1", t_Addr);
      }
      // Find the correct memory address of the index
      if (SRC2(curr_instruction)->elt_type == t_Int) {
        printf("  sll $t0, %s, 2  \n", src_reg_name);
      }
      printf("  add %s, $t0, $t1 \n", dest_reg_name);
      if (is_var_in_memory(curr_instruction->dest)) {
        store_at_memory(curr_instruction->dest, dest_reg_name);
      }
      break;
    }
    case OP_Deref: {
      printf("\n");
      printf("  # OP_Deref \n");
      int dest_reg = find_register(curr_instruction->dest, 0);
      char *dest_reg_name = get_register_name(dest_reg);

      load_from_memory(SRC1(curr_instruction), "$t0", t_Word);
      char mem_op_type = get_mem_op_type(curr_instruction->dest->type);
      printf("  l%c %s, 0($t0) \n", mem_op_type, dest_reg_name);
      if (is_var_in_memory(curr_instruction->dest)) {
        store_at_memory(curr_instruction->dest, dest_reg_name);
      }
      break;
    }
    default:
      break;
    }

    last_instruction = curr_instruction;
    curr_instruction = curr_instruction->next;
  }
}

/**
 * Prints MIPS assembly code for the predefined function println.
 */
void print_println() {
  print_function_header("println");
  printf(".align 2            \n");
  printf(".data               \n");
  printf("nl: .asciiz \"\\n\" \n");
  printf(".align 2            \n");
  printf(".text               \n");
  printf("_println:           \n");
  printf("  li $v0, 1         \n");
  printf("  lw $a0, 0($sp)    \n");
  printf("  syscall           \n");
  printf("  li $v0, 4         \n");
  printf("  la $a0, nl        \n");
  printf("  syscall           \n");
  printf("  jr $ra            \n");
}

/**
 * Prints a comment with the function name before the function definition.
 *
 * @param function_name: name of the function
 */
void print_function_header(char *function_name) {
  printf("\n");
  printf("# -------------------------- \n");
  printf("# FUNCTION %s                \n", function_name);
  printf("# -------------------------- \n");
}

/**
 * Loads a constant integer to a register.
 *
 * @param integer: constant integer
 * @param reg: register
 */
void load_int_to_register(int integer, char *reg) {
  int high = (integer >> 16);
  int low = (integer & 0xffff);
  if (high == 0) {
    printf("  li %s, %d \n", reg, integer);
  } else {
    printf("  lui %s, %d \n", reg, high);
    printf("  ori %s, %d \n", reg, low);
  }
}

void load_from_memory(symtabnode *addr, char *reg, int dest_type) {
  if (addr->is_constant) {
    load_int_to_register(addr->const_val, reg);
  } else {
    char load_op_type = get_mem_op_type(dest_type);
    if (addr->scope == Global) {
      printf("  l%c %s, _%s \n", load_op_type, reg, addr->name);
    } else {
      printf("  l%c %s, %d($fp) \n", load_op_type, reg, addr->fp_offset);
    }
  }
}

void store_at_memory(symtabnode *addr, char *reg) {
  char mem_op_type;
  if (addr->type == t_Addr) {
    mem_op_type = get_mem_op_type(t_Word);
  } else {
    mem_op_type = get_mem_op_type(addr->type);
  }

  if (addr->scope == Global) {
    printf("  s%c %s, _%s \n", mem_op_type, reg, addr->name);
  } else {
    printf("  s%c %s, %d($fp) \n", mem_op_type, reg, addr->fp_offset);
  }
}

void copy_from_register(char *reg_src, char *reg_dest) {
  printf("  move %s, %s \n", reg_dest, reg_src);
}

char *get_operation_name(enum InstructionType type) {
  switch (type) {
  case IT_Plus:
    return "add";
  case IT_BinaryMinus:
    return "sub";
  case IT_Mult:
    return "mul";
  case IT_Div:
    return "div";
  case IT_LE:
    return "le";
  case IT_EQ:
    return "eq";
  case IT_LT:
    return "lt";
  case IT_GT:
    return "gt";
  case IT_NE:
    return "ne";
  case IT_GE:
    return "ge";
  default:
    fprintf(stderr, "Invalid operation type %d.\n", type);
    return "";
  }
}

char get_mem_op_type(int type) {
  char mem_op_type;

  if (type == t_Char || type == t_Bool) {
    mem_op_type = 'b';
  } else if (type == t_Int || type == t_Word) {
    mem_op_type = 'w';
  } else {
    mem_op_type = 'a';
  }

  return mem_op_type;
}

void print_strings() {
  symtabnode *str_node = get_string_list_head();
  if (str_node) {
    printf("\n");
    printf("# -------------------------- \n");
    printf("# STRINGS                    \n");
    printf("# -------------------------- \n");
    printf(".data \n");

    while (str_node) {
      printf("_%s: .asciiz \"%s\" \n", str_node->name, str_node->const_str);
      printf(".align 2 \n");
      str_node = str_node->next;
    }
  }
}

char *get_register_name(int reg) {
  char *name;
  name = zalloc(4 * sizeof(char));
  if (reg < 10) { // $t0 - $t9
    sprintf(name, "$t%d", reg);
  } else { //$s0 - $s7
    sprintf(name, "$s%d", reg - 10);
  }

  return name;
}

bool is_var_in_memory(symtabnode *var) {
  return !var->live_range_node || var->live_range_node->reg == -1;
}

int find_register(symtabnode *var, int default_reg) {
  int reg = default_reg;
  gnode *live_range = var->live_range_node;
  if (live_range && live_range->reg >= 0) {
    reg = live_range->reg + NUM_RESERVED_REG;
  }

  return reg;
}

void load_reg_allocated_variables_from_memory(inode *instruction) {
  if (!is_set_undefined(instruction->live_at_call)) {
    bool some_load = true;
    if (!is_set_empty(instruction->live_at_call)) {
      printf("\n  # Load registers \n");
      some_load = false;
    }
    set tmp = clone_set(instruction->live_at_call);
    int i = 0;
    while (!is_set_empty(tmp)) {
      if (does_elto_belong_to_set(i, tmp)) {
        symtabnode *var = get_variable_by_id(i);
        if (!is_var_in_memory(var)) {
          if (!SRC1(instruction)->entered ||
              does_elto_belong_to_set(var->live_range_node->reg,
                                      SRC1(instruction)->registers_used)) {
            if(var->live_range_node->reg < 8) { // One of the $t registers
              int reg = find_register(var, 0);
              load_from_memory(var, get_register_name(reg), var->type);
              some_load = true;
            }
          }
        }
        remove_from_set(i, tmp);
      }
      i = i + 1;
    }
    if (!some_load) {
      printf("  # > Nothing to load \n");
    }
  }
}

/**
 * Before calling a function, save registers used to allocate variables into
 * the memory
 */
void save_reg_allocated_variables_in_memory(inode *instruction) {
  if (!is_set_undefined(instruction->live_at_call)) {
    bool some_storage = true;
    if (!is_set_empty(instruction->live_at_call)) {
      printf("\n  # Store registers \n");
      some_storage = false;
    }
    set tmp = clone_set(instruction->live_at_call);
    int i = 0;
    while (!is_set_empty(tmp)) {
      if (does_elto_belong_to_set(i, tmp)) {
        symtabnode *var = get_variable_by_id(i);
        if (!is_var_in_memory(var)) {
          if (!SRC1(instruction)->entered ||
              does_elto_belong_to_set(var->live_range_node->reg,
                                      SRC1(instruction)->registers_used)) {
            if(var->live_range_node->reg < 8) { // One of the $t registers
              // We only save to memory if the register where the variable is
              // allocated is used inside the function being called or if the
              // function has not been parsed yet.
              int reg = find_register(var, 0);
              store_at_memory(var, get_register_name(reg));
              some_storage = true;
            }
          }
        }
        remove_from_set(i, tmp);
      }
      i = i + 1;
    }
    if (!some_storage) {
      printf("  # > Nothing to store \n");
    }
  }
}

void reg_to_char(char *reg) {
  printf("\n  # Conversion to char with sign-extension \n");
  printf("  sll %s, %s, 24 \n", reg, reg);
  printf("  sra %s, %s, 24 \n", reg, reg);
}

void save_registers_at_function_enter(symtabnode *function_ptr) {
  int num_callee_saved_registers = 0;

  // s0 - s7
  for(int reg = 8; reg < function_ptr->registers_used.max_size; reg++) {
    if(does_elto_belong_to_set(reg, function_ptr->registers_used)) {
      num_callee_saved_registers++;
    }
  }
  // fp + ra + callee-saved
  int bytes_in_memory = 8 + 4 * num_callee_saved_registers;
  printf("  la $sp, -%d($sp) \n", bytes_in_memory);

  // Store registers in memory
  int pos = 0;
  for(int reg = 8; reg < function_ptr->registers_used.max_size; reg++) {
    if(does_elto_belong_to_set(reg, function_ptr->registers_used)) {
      char* reg_name = get_register_name(reg + 2); // Index starts in $t2
      printf("  sw %s, %d($sp)  \n", reg_name, pos);
      pos += 4;
    }
  }
  printf("  sw $fp, %d($sp)  \n", pos + 4);
  printf("  sw $ra, %d($sp)  \n", pos);
}

void restore_callee_saved_registers(symtabnode* function_ptr) {
  int pos = 0;
  for(int reg = 8; reg < function_ptr->registers_used.max_size; reg++) {
    if(does_elto_belong_to_set(reg, function_ptr->registers_used)) {
      char* reg_name = get_register_name(reg + 2); // Index starts in $t2
      if (pos == 0) {
        printf("  la $sp, 0($fp)  \n");
      }
      printf("  lw %s, %d($sp)  \n", reg_name, pos);
      pos += 4;
    }
  }
  if (pos > 0) {
    // Move the frame pointer to where the return register is in the stack
    printf("  la $fp, %d($fp) \n", pos);
  } else {
    printf("#  No callee-saved registers to restore");
  }
}
