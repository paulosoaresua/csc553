//
// Created by Paulo Soares on 1/29/21.
//

#include <stdio.h>
#include "instruction.h"
#include "protos.h"
#include "syntax-tree.h"

void append_instructions(tnode *child, tnode *parent);
void append_instruction(inode *instruction, tnode *node);
void print_println_definition();

void generate_code(tnode *node) {
  inode *instruction;

  switch (node->ntype) {
  case Intcon: {
    instruction = create_const_int_instruction(node->val.iconst);
    append_instruction(instruction, node);
    break;
  }
  case FunCall: {
    generate_code(stFunCall_Args(node));
    append_instructions(stList_Head(stFunCall_Args(node)), node);
    symtabnode *function_ptr = stFunCall_Fun(node);
    instruction = create_instruction(OP_Call, function_ptr, NULL, NULL);
    append_instruction(instruction, node);
    break;
  }
  case STnodeList: {
    // Function arguments are processed here
    for (tnode *arg = node; arg != NULL; arg = stList_Rest(arg)) {
      generate_code(stList_Head(arg));
      append_instructions(stList_Head(arg), node);
    }
    break;
  }
  }
}

/**
 * Appends a list of the instructions from the child node to the parent node.
 * @param child: child node in the syntax tree
 * @param parent: parent of the child node in the syntax tree
 */
void append_instructions(tnode *child, tnode *parent) {
  if (!parent->code_head) {
    parent->code_head = child->code_head;
    parent->code_tail = child->code_tail;
  } else {
    parent->code_tail->next = child->code_head;
    parent->code_tail = child->code_tail;
  }
}

/**
 * Append a instruction to a node's list of instructions.
 *
 * @param instruction: instruction
 * @param node: node in the syntax tree
 */
void append_instruction(inode *instruction, tnode *node) {
  if (!node->code_head) {
    node->code_head = instruction;
    node->code_tail = instruction;
  } else {
    node->code_tail->next = instruction;
    node->code_tail = node->code_tail->next;
  }
}

void print_code(tnode *root) {
  printf("main: j _main \n");
  printf("_main: j _main \n");
  print_println_definition();

  inode *curr_instruction = root->code_head;
  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Const_Int: {
      printf("# const int    \n");
      printf("  li $3, %d      \n", curr_instruction->val.const_int);
      printf("  la $sp -4($sp) \n");
      printf("  sw $3 0($sp)   \n");
      break;
    }
//    case OP_Enter: {
//      int n = 0; // curr_instruction->src1->... get size in bytes
//      printf("la $sp, -8($sp) # allocate space for old $fp and $ra");
//      printf("sw $fp, 4($sp)  # save old $fp");
//      printf("sw $ra, 0($sp)  # save return address");
//      printf("la $fp, 0($sp)  # set up frame pointer");
//      printf("la $sp, −%d($sp) # allocate stack frame", n);
//      break;
//    }
    case OP_Call: {
      char* func_name = curr_instruction->val.op_members.src1->name;
      printf("  jal _%s # Jump to function \n", func_name);
      printf("  la $sp %d($sp) # Pop the actual from the stack \n", 4);
      break;
    }
    }
    curr_instruction = curr_instruction->next;
  }
}

void print_println_definition() {
  printf("# func println        \n");
  printf("  .align 2            \n");
  printf("  .data               \n");
  printf("  nl: .asciiz \"\\n\" \n");
  printf("  .align 2            \n");
  printf("  .text               \n");
  printf("  _println:           \n");
  printf("    li $v0, 1         \n");
  printf("    lw $a0, 0($sp)    \n");
  printf("    syscall           \n");
  printf("    li $v0, 4         \n");
  printf("    la $a0, nl        \n");
  printf("    syscall           \n");
  printf("    jr $ra            \n");
}

void generate_statement_code(tnode *statement_node) {
  switch (statement_node->ntype) {
  case For:
    break;
  case While:
    break;
  case If:
    break;
  case Equals:
    break;
    //...
  }
}

void generate_expression_code(tnode *expression_node) {
  switch (expression_node->ntype) {
  case Plus:
    break;
  case Mult:
    break;
  case BinaryMinus:
    break;
  case Div:
    break;
    //...
  }
}

symtabnode *create_temporary(int type, int scope) {
  symtabnode *st_node = SymTabInsert("_tmp", scope);
  st_node->type = type;

  return st_node;
}