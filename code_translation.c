/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_translation.h"
#include "instruction.h"

void print_globals() {
  print_println();
  print_function_header("~ ENTRY POINT ~");
  printf("main: j _main \n");
}

void print_function(tnode *node) {
  inode *curr_instruction = node->code_head;

  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Enter: {
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
      printf("  sw $t0, %d($fp) \n", curr_instruction->dest->fp_offset);
      break;

    case OP_Assign_Char:
      printf("\n");
      printf("  # OP_Assign_Char \n");
      printf("  li $t0, %d       \n", curr_instruction->val.const_int);
      printf("  sb $t0, %d($fp) \n", curr_instruction->dest->fp_offset);
      break;

    case OP_Assign_Str:
      // TODO - milestone 2
      // Load from the globals
      break;

    case OP_Assign:
      printf("\n");
      printf("  # OP_Assign      \n");
      char word_or_byte;
      if (DEST(curr_instruction)->type == t_Char) {
        word_or_byte = 'b';
      } else {
        word_or_byte = 'w';
      }
      printf("  l%c $t0, %d($fp) \n", word_or_byte,
             SRC1(curr_instruction)->fp_offset);
      printf("  s%c $t0, %d($fp) \n", word_or_byte,
             DEST(curr_instruction)->fp_offset);

      break;

    case OP_Param:
      printf("\n");
      printf("  # OP_Param       \n");
      printf("  lw $t0, %d($fp) \n", SRC1(curr_instruction)->fp_offset);
      printf("  la $sp, -4($sp)  \n");
      printf("  sw $t0, 0($sp)   \n");
      break;

    case OP_Call: {
      symtabnode *function_ptr = SRC1(curr_instruction);
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

void print_function_header(char *function_name) {
  printf("\n");
  printf("# -------------------------- \n");
  printf("# FUNCTION %s                \n", function_name);
  printf("# -------------------------- \n");
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