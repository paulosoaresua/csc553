/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"
#include "protos.h"
#include "syntax-tree.h"
#include <stdio.h>

#define L_VALUE 0
#define R_VALUE 1

static void append_child_instructions(tnode *child, tnode *parent);
static void append_instruction(inode *instruction, tnode *node);
static void append_instructions(inode *instructions, tnode *node);

void process_function_header(symtabnode *func_header, tnode *body) {
  // This syntax tree node is the first node after a function if defined.
  // Therefore, we need to create an Enter instruction for it.
  inode *instruction = create_instruction(OP_Enter, func_header, NULL, NULL);
  body->code_head = instruction;
  body->code_tail = instruction;
}

void process_allocations(symtabnode *function_ptr) {
  function_ptr->byte_size = fill_local_allocations();
}

void generate_code(tnode *node, int lr_type) {
  inode *instruction;
  symtabnode *tmp;

  if (!node) {
    // Nothing to be done if the node is NULL
    return;
  }

  switch (node->ntype) {
  case Assg:
    generate_code(stAssg_Lhs(node), L_VALUE);
    append_child_instructions(stAssg_Lhs(node), node);

    generate_code(stAssg_Rhs(node), R_VALUE);
    append_child_instructions(stAssg_Rhs(node), node);

    instruction = create_instruction(OP_Assign, stAssg_Rhs(node)->place, NULL,
                                     stAssg_Lhs(node)->place);
    append_instruction(instruction, node);
    break;

  case Var:
    // No code is needed for the variable. We just inform to the syntax tree
    // node about its location in the symbol table.
    node->place = stVar(node);
    break;

  case Intcon:
    if (lr_type == L_VALUE) {
      fprintf(stderr, "A constant integer cannot be used as an l-value.\n");
      return;
    } else {
      tmp = create_temporary(node->etype);
      node->place = tmp;
      instruction = create_const_int_instruction(node->val.iconst, tmp);
      append_instruction(instruction, node);
    }
    break;

  case Charcon:
    if (lr_type == L_VALUE) {
      fprintf(stderr, "A constant char cannot be used as an l-value.\n");
      return;
    } else {
      tmp = create_temporary(node->etype);
      node->place = tmp;
      instruction = create_const_char_instruction(node->val.iconst, tmp);
      append_instruction(instruction, node);
    }
    break;

  case Stringcon:
    // TODO - milestone 2
//    if (lr_type == L_VALUE) {
//      fprintf(stderr, "Constant characters cannot be used as an l-value.\n");
//    } else {
//      // Create a global label for the string
//      instruction = create_string_instruction(node->val.strconst);
//      // Save the label to be referenced later
//      instruction = create_const_char_instruction(instruction->label);
//      append_instruction(instruction, node);
//    }
    break;

  case FunCall: {
    // Expand the parameters
    generate_code(stFunCall_Args(node), lr_type);
    append_child_instructions(stFunCall_Args(node), node);

    // Create PARAM instructions
    inode* params_instructions = NULL;
    for (tnode *param = stFunCall_Args(node); param != NULL;
         param = stList_Rest(param)) {
      instruction =
          create_instruction(OP_Param, stList_Head(param)->place, NULL, NULL);
      // Actuals from the right to the left
      instruction->next = params_instructions;
      params_instructions = instruction;
    }
    append_instructions(params_instructions, node);

    symtabnode *function_ptr = stFunCall_Fun(node);
    instruction = create_instruction(OP_Call, function_ptr, NULL, NULL);
    append_instruction(instruction, node);

    // TODO -  Second milestone
    // if (function_ptr->ret_type != t_None) {
    //   node->place = tmp;
    //   instruction = create_instruction(OP_Retrieve, NULL, NULL,
    //   function_ptr);
    //   append_instruction(instruction, node);
    // }

    break;
  }

  case STnodeList: {
    for (tnode *arg = node; arg != NULL; arg = stList_Rest(arg)) {
      generate_code(stList_Head(arg), lr_type);
      append_child_instructions(stList_Head(arg), node);
    }
    break;
  }

  case Return:
    // TODO - second milestone
    // tmp = create_temporary(stReturn(node)->etype);
    // node->place = tmp;
    instruction = create_instruction(OP_Return, NULL, NULL, NULL);
    append_instruction(instruction, node);
    break;

  default:
    break;
  }
}

/**
 * Appends a list of the instructions from the child node to the parent node.
 * @param child: child node in the syntax tree
 * @param parent: parent of the child node in the syntax tree
 */
static void append_child_instructions(tnode *child, tnode *parent) {
  if (!child || !child->code_head) {
    return;
  }

  if (!parent->code_head) {
    parent->code_head = child->code_head;
    parent->code_tail = child->code_tail;
  } else {
    parent->code_tail->next = child->code_head;
    parent->code_tail = child->code_tail;
  }
}

/**
 * Append an instruction to a node's list of instructions.
 *
 * @param instruction: instruction
 * @param node: node in the syntax tree
 */
static void append_instruction(inode *instruction, tnode *node) {
  if (!node->code_head || !node->code_tail) {
    node->code_head = instruction;
    node->code_tail = instruction;
  } else {
    node->code_tail->next = instruction;
    node->code_tail = node->code_tail->next;
  }
}

/**
 * Append instructions to a node's list of instructions.
 *
 * @param instruction: instruction
 * @param node: node in the syntax tree
 */
static void append_instructions(inode *instructions, tnode *node) {
  inode *instruction = instructions;
  while(instruction) {
    append_instruction(instruction, node);
    instruction = instruction->next;
  }
}