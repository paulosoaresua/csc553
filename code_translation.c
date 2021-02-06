/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_translation.h"
#include "instruction.h"
#include "symbol-table.h"

static void print_println();
static void print_function_header(char *function_name);
static char get_word_or_byte(int type);
static void load_to_register(symtabnode *addr, int reg, int dest_type);
static void store_at_memory(symtabnode *addr, int reg);
static char* get_operation_name(ExprType type);

void print_pre_defined_instructions() {
  print_println();
  print_function_header("~ ENTRY POINT ~");
  printf("main: j _main \n");
}

void print_function(tnode *node) {
  inode *last_instruction = NULL;
  inode *curr_instruction = node->code_head;

  while (curr_instruction) {
    switch (curr_instruction->i_type) {
    case I_Global: {
      if (!last_instruction || last_instruction->i_type != I_Global) {
        printf("\n");
        printf("# -------------------------- \n");
        printf("# GLOBALS                    \n");
        printf("# -------------------------- \n");
        printf(".data \n");
      }

      switch (curr_instruction->type) {
      case IT_Int:
        printf("%s:.space 4 \n", curr_instruction->label);
        break;
      case IT_Char:
        printf("%s:.space 1 \n", curr_instruction->label);
        printf(".align 2 \n");
        break;
      default:
        break;
      }

      break;
    }
    case I_Enter: {
      if (last_instruction && last_instruction->i_type == I_Global) {
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

    case I_Assign_Int:
      printf("\n");
      printf("  # I_Assign_Int \n");
      int n = curr_instruction->val.const_int;
      int high = (n >> 16);
      int low = (n & 0xffff);
      if (high == 0) {
        printf("  li $t0, %d \n", n);
      } else {
        printf("  lui $t0, %d \n", high);
        printf("  ori $t0, %d \n", low);
      }
      printf("  sw $t0, %d($fp) \n", curr_instruction->dest->fp_offset);
      break;

    case I_Assign_Char:
      printf("\n");
      printf("  # I_Assign_Char \n");
      printf("  li $t0, %d       \n", curr_instruction->val.const_int);
      printf("  sb $t0, %d($fp) \n", curr_instruction->dest->fp_offset);
      break;

    case I_Assign_Str:
      // TODO - milestone 2
      // Load from the globals
      break;

    case I_Assign:
      printf("\n");
      printf("  # I_Assign      \n");
      load_to_register(SRC1(curr_instruction), 0, DEST(curr_instruction)->type);
      store_at_memory(DEST(curr_instruction), 0);
      break;

    case I_Param: {
      printf("\n");
      printf("  # I_Param       \n");
      load_to_register(SRC1(curr_instruction), 0, SRC1(curr_instruction)->type);
      printf("  la $sp, -4($sp)  \n");
      printf("  sw $t0, 0($sp)   \n");
      break;
    }

    case I_Call: {
      symtabnode *function_ptr = SRC1(curr_instruction);
      printf("\n");
      printf("  # I_Call       \n");
      printf("  jal _%s         \n", function_ptr->name);
      printf("  la $sp, %d($sp) \n", 4 * function_ptr->num_formals);
      break;
    }

    case I_Return:
      // TODO - handle return of values for milestone 2
      printf("\n");
      printf("  # I_Return    \n");
      printf("  la $sp, 0($fp) \n");
      printf("  lw $ra, 0($sp) \n");
      printf("  lw $fp, 4($sp) \n");
      printf("  la $sp, 8($sp) \n");
      printf("  jr $ra         \n");
      break;

    case I_UMinus:
      printf("\n");
      printf("  # I_UMinus    \n");
      // Load the content from src1 into $t0
      load_to_register(SRC1(curr_instruction), 0, SRC1(curr_instruction)->type);
      printf("  neg $t1, $t0 \n");
      store_at_memory(DEST(curr_instruction), 1);
      break;

    case I_BinaryArithmetic: {
      printf("\n");
      printf("  # I_BinaryArithmetic    \n");
      load_to_register(SRC1(curr_instruction), 0, SRC1(curr_instruction)->type);
      load_to_register(SRC2(curr_instruction), 1, SRC2(curr_instruction)->type);
      char* op_name = get_operation_name(curr_instruction->type);
      printf("  %s $t2, $t0, $t1 \n", op_name);
      store_at_memory(DEST(curr_instruction), 2);
      break;
    }


    default:
      break;

      // TODO - milestone 2
      //    case I_Retrieve: {
      //      printf("la $sp, 0($fp) # deallocate locals \n");
      //      printf("lw $ra, 0($sp) # restore return address \n");
      //      printf("lw $fp, 4($sp) # restore frame pointer \n");
      //      printf("la $sp, 8($sp) # restore stack pointer \n");
      //      printf("jr $ra # return \n");
      //      break;
      //    }
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

static char get_word_or_byte(int type) {
  char word_or_byte;
  if (type == t_Char) {
    word_or_byte = 'b';
  } else {
    word_or_byte = 'w';
  }

  return word_or_byte;
}

static void load_to_register(symtabnode *addr, int reg, int dest_type) {
  char word_or_byte = get_word_or_byte(dest_type);

  if (addr->scope == Global) {
    printf("  l%c $t%d, %s \n", word_or_byte, reg, addr->name);
  } else {
    printf("  l%c $t%d, %d($fp) \n", word_or_byte, reg, addr->fp_offset);
  }
}

static void store_at_memory(symtabnode *addr, int reg) {
  if (addr->scope == Global) {
    printf("  sw $t%d, %s \n", reg, addr->name);
  } else {
    printf("  sw $t%d, %d($fp) \n", reg, addr->fp_offset);
  }
}

static char* get_operation_name(ExprType type) {
  switch (type) {
  case IT_Plus:
    return "add";
  case IT_BinaryMinus:
    return "sub";
  case IT_Mult:
    return "mul";
  case IT_Div:
    return "div";
  default:
    fprintf(stderr, "Invalid operation type %d.\n", type);
    return "";
  }
}

void print_strings() {
  inode *instruction = get_string_instruction_head();
  if (instruction) {
    printf("\n");
    printf("# -------------------------- \n");
    printf("# STRINGS                    \n");
    printf("# -------------------------- \n");
    printf(".data \n");

    while (instruction) {
      printf("%s: .asciiz \"%s\" \n", instruction->label,
             instruction->val.const_char);
      instruction = instruction->next;
    }
  }
}
