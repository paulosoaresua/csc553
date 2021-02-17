/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_translation.h"
#include "instruction.h"
#include "symbol-table.h"

static void print_println();
static void print_function_header(char *function_name);
static void load_to_register(symtabnode *addr, char *reg, int dest_type);
static void store_at_memory(symtabnode *addr, char *reg);
static char *get_operation_name(enum InstructionType type);
static char get_mem_op_type(int type);

void print_pre_defined_instructions() {
  print_println();
  print_function_header("~ ENTRY POINT ~");
  printf("main: j _main \n");
}

void print_function(tnode *node) {
  inode *last_instruction = NULL;
  inode *curr_instruction = node->code_head;

  while (curr_instruction) {
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
      printf("  la $sp, -8($sp) \n");
      printf("  sw $fp, 4($sp)  \n");
      printf("  sw $ra, 0($sp)  \n");
      printf("  la $fp, 0($sp)  \n");
      printf("  la $sp, %d($sp) \n", -function_ptr->byte_size);
      break;
    }

    case OP_Assign_Int:
      printf("\n");
      printf("  # OP_Assign_Int \n");
      int n = curr_instruction->val.const_int;
      int high = (n >> 16);
      int low = (n & 0xffff);
      if (high == 0) {
        printf("  li $t0, %d \n", n);
      } else {
        printf("  lui $t0, %d \n", high);
        printf("  ori $t0, %d \n", low);
      }
      store_at_memory(curr_instruction->dest, "$t0");
      break;

    case OP_Assign_Char:
      printf("\n");
      printf("  # OP_Assign_Char \n");
      printf("  li $t0, %d       \n", curr_instruction->val.const_int);
      store_at_memory(curr_instruction->dest, "$t0");
      break;

    case OP_Assign:
      printf("\n");
      printf("  # OP_Assign      \n");
      if (curr_instruction->dest->type == t_Addr) {
        // The LHS is an array memory location, therefore we load the word in
        // curr_instruction->dest instead of its address. And the type of the
        // value to be stored in the array location is determined by the type
        // of the elements in the array.
        load_to_register(SRC1(curr_instruction), "$t0",
                         SRC1(curr_instruction)->type);
        load_to_register(curr_instruction->dest, "$t1", t_Word);
        char mem_op_type = get_mem_op_type(curr_instruction->dest->elt_type);
        printf("  s%c $t0, 0($t1) \n", mem_op_type);
      } else {
        load_to_register(SRC1(curr_instruction), "$t0",
                         SRC1(curr_instruction)->type);
        store_at_memory(curr_instruction->dest, "$t0");
      }
      break;

    case OP_Param: {
      printf("\n");
      printf("  # OP_Param       \n");
      load_to_register(SRC1(curr_instruction), "$t0",
                       SRC1(curr_instruction)->type);
      printf("  la $sp, -4($sp)  \n");
      printf("  sw $t0, 0($sp)   \n");

      break;
    }

    case OP_Call: {
      symtabnode *function_ptr = SRC1(curr_instruction);
      printf("\n");
      printf("  # OP_Call       \n");
      printf("  jal _%s         \n", function_ptr->name);
      printf("  la $sp, %d($sp) \n", 4 * function_ptr->num_formals);
      break;
    }

    case OP_Return:
      printf("\n");
      printf("  # OP_Return    \n");
      if (curr_instruction->dest) {
        load_to_register(curr_instruction->dest, "$v0",
                         curr_instruction->dest->type);
      }
      printf("  la $sp, 0($fp) \n");
      printf("  lw $ra, 0($sp) \n");
      printf("  lw $fp, 4($sp) \n");
      printf("  la $sp, 8($sp) \n");
      printf("  jr $ra         \n");
      break;

    case OP_Retrieve:
      store_at_memory(curr_instruction->dest, "$v0");
      break;

    case OP_UMinus:
      printf("\n");
      printf("  # OP_UMinus    \n");
      // Load the content from src1 into $t0
      load_to_register(SRC1(curr_instruction), "$t0",
                       SRC1(curr_instruction)->type);
      printf("  neg $t1, $t0 \n");
      store_at_memory(curr_instruction->dest, "$t1");
      break;

    case OP_BinaryArithmetic: {
      printf("\n");
      printf("  # OP_BinaryArithmetic    \n");
      printf("  #%s, %s \n", SRC1(curr_instruction)->name,
             SRC2(curr_instruction)->name);
      load_to_register(SRC1(curr_instruction), "$t0",
                       SRC1(curr_instruction)->type);
      load_to_register(SRC2(curr_instruction), "$t1",
                       SRC2(curr_instruction)->type);
      char *op_name = get_operation_name(curr_instruction->type);
      printf("  %s $t2, $t0, $t1 \n", op_name);
      store_at_memory(curr_instruction->dest, "$t2");
      break;
    }

    case OP_Label:
      printf("\n");
      printf("  # OP_Label \n");
      printf("  _%s:       \n", curr_instruction->label);
      break;

    case OP_If:
      printf("\n");
      printf("  # OP_If \n");
      load_to_register(SRC1(curr_instruction), "$t0",
                       SRC1(curr_instruction)->type);
      load_to_register(SRC2(curr_instruction), "$t1",
                       SRC2(curr_instruction)->type);
      char *op_name = get_operation_name(curr_instruction->type);
      printf("  b%s $t0, $t1, _%s \n", op_name, curr_instruction->label);
      break;

    case OP_Goto:
      printf("\n");
      printf("  # OP_Goto \n");
      printf("  j _%s     \n", curr_instruction->label);
      break;

    case OP_Index_Array:
      printf("\n");
      printf("  # OP_Index_Array \n");
      load_to_register(SRC1(curr_instruction), "$t0", t_Int);
      // Load address of the first position of the array into $t1
      if (SRC2(curr_instruction)->formal) {
        // If it's a formal, the address of the first position of the array
        // will be stored in the stack, therefore we read the content instead of
        // the address of the formal in the stack;
        load_to_register(SRC2(curr_instruction), "$t1", t_Word);
      } else {
        load_to_register(SRC2(curr_instruction), "$t1", t_Addr);
      }
      // Find the correct memory address of the index
      if (SRC2(curr_instruction)->elt_type == t_Int) {
        printf("  sll $t0, $t0, 2  \n");
      }
      printf("  add $t1, $t0, $t1 \n");
      store_at_memory(curr_instruction->dest, "$t1");
      break;

    case OP_Deref:
      printf("\n");
      printf("  # OP_Deref \n");
      load_to_register(SRC1(curr_instruction), "$t0", t_Word);
      char mem_op_type = get_mem_op_type(curr_instruction->dest->type);
      printf("  l%c $t1, 0($t0) \n", mem_op_type);
      store_at_memory(curr_instruction->dest, "$t1");
      break;

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
static void print_println() {
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
static void print_function_header(char *function_name) {
  printf("\n");
  printf("# -------------------------- \n");
  printf("# FUNCTION %s                \n", function_name);
  printf("# -------------------------- \n");
}

static void load_to_register(symtabnode *addr, char *reg, int dest_type) {
  char load_op_type = get_mem_op_type(dest_type);

  if (addr->scope == Global) {
    printf("  l%c %s, _%s \n", load_op_type, reg, addr->name);
  } else {
    printf("  l%c %s, %d($fp) \n", load_op_type, reg, addr->fp_offset);
  }
}

static void store_at_memory(symtabnode *addr, char *reg) {
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

static char *get_operation_name(enum InstructionType type) {
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

static char get_mem_op_type(int type) {
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
