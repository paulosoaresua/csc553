/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_translation.h"
#include "instruction.h"

void to_mips(tnode *node) {
  print_mips_println();

  print_mips_function_header("main");
  printf("main: j _main \n");

  inode *curr_instruction = node->code_head;
  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Enter: {
      symtabnode* function_ptr = SRC1(curr_instruction);
      print_mips_function_header(function_ptr->name);
      printf("_%s:               \n", function_ptr->name);
      printf("  la $sp, -8($sp)  \n");
      printf("  sw $fp, 4($sp)   \n");
      printf("  sw $ra, 0($sp)   \n");
      printf("  la $fp, 0($sp)   \n");
      printf("  la $sp, -%d($sp) \n", function_ptr->byte_size);
      break;
    }

    case OP_Assign_Int:
      // TODO - fix for numbers bigger than 16 bits
      printf("\n");
      printf("  # OP_Assign_Int \n");
      printf("  li $t0, %d       \n", curr_instruction->val.const_int);
      printf("  sw $t0, -%d($fp) \n", curr_instruction->dest->fp_offset);
      break;

    case OP_Assign_Char:
      // Load from the globals
      break;

    case OP_Assign:
      printf("\n");
      printf("  # OP_Assign      \n");
      printf("  lw $t0, -%d($fp) \n", SRC1(curr_instruction)->fp_offset);
      printf("  sw $t0, -%d($fp) \n", DEST(curr_instruction)->fp_offset);
      break;

    case OP_Param:
      printf("\n");
      printf("  # OP_Param       \n");
      printf("  lw $t0, -%d($fp) \n", SRC1(curr_instruction)->fp_offset);
      printf("  la $sp, -4($sp)  \n");
      printf("  sw $t0, 0($sp)   \n");
      break;

    case OP_Call: {
      symtabnode * function_ptr = SRC1(curr_instruction);
      printf("\n");
      printf("  # OP_Call       \n");
      printf("  jal _%s         \n", function_ptr->name);
      printf("  la $sp, %d($sp) \n", 4 * function_ptr->num_formals);
      break;
    }

    case OP_Return: {
      // TODO - handle return of values for milestone 2
      printf("\n");
      printf("  # OP_Return    \n");
      printf("  la $sp, 0($fp) \n");
      printf("  lw $ra, 0($sp) \n");
      printf("  lw $fp, 4($sp) \n");
      printf("  la $sp, 8($sp) \n");
      printf("  jr $ra         \n");
      break;
    }

    default:
      break;

    // TODO - milestone 2
//    case OP_Retrieve: {
//      printf("la $sp, 0($fp) # deallocate locals \n");
//      printf("lw $ra, 0($sp) # restore return address \n");
//      printf("lw $fp, 4($sp) # restore frame pointer \n");
//      printf("la $sp, 8($sp) # restore stack pointer \n");
//      printf("jr $ra # return \n");
//      break;
//    }
    }

    curr_instruction = curr_instruction->next;
  }
}

void print_mips_println() {
  print_mips_function_header("println");
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

void print_mips_function_header(char* function_name) {
  printf("\n");
  printf("# -------------------------- \n");
  printf("# FUNCTION %s                \n", function_name);
  printf("# -------------------------- \n");
}