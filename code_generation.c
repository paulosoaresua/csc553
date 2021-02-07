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

inode *global_head = NULL;
inode *global_tail = NULL;

static void append_child_instructions(tnode *child, tnode *parent);
static void append_instruction(inode *instruction, tnode *node);
static void append_instructions(inode *instructions, tnode *node);
static void generate_binary_arithmetic_expr_code(tnode *node, ExprType type,
                                                 int lr_type);

void process_function_header(symtabnode *func_header, tnode *body) {
  append_instructions(global_head, body);
  // This syntax tree node is the first node after a function if defined.
  // Therefore, we need to create an Enter instruction for it.
  inode *instruction = create_instruction(I_Enter, func_header, NULL, NULL);
  append_instruction(instruction, body);

  // Clear globals
  global_head = NULL;
  global_tail = NULL;
}

void process_allocations(symtabnode *function_ptr) {
  function_ptr->byte_size = fill_local_allocations();
}

void generate_function_code(tnode *node, int lr_type) {
  inode *instruction;
  symtabnode *tmp;

  if (!node) {
    // Nothing to be done if the node is NULL
    return;
  }

  switch (node->ntype) {
  case Assg:
    generate_function_code(stAssg_Lhs(node), L_VALUE);
    append_child_instructions(stAssg_Lhs(node), node);

    generate_function_code(stAssg_Rhs(node), R_VALUE);
    append_child_instructions(stAssg_Rhs(node), node);

    instruction = create_instruction(I_Assign, stAssg_Rhs(node)->place, NULL,
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
    //      fprintf(stderr, "Constant characters cannot be used as an
    //      l-value.\n");
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
    generate_function_code(stFunCall_Args(node), lr_type);
    append_child_instructions(stFunCall_Args(node), node);

    // Create PARAM instructions
    inode *params_instructions = NULL;
    for (tnode *param = stFunCall_Args(node); param != NULL;
         param = stList_Rest(param)) {
      instruction =
          create_instruction(I_Param, stList_Head(param)->place, NULL, NULL);
      // Actuals from the right to the left
      instruction->next = params_instructions;
      params_instructions = instruction;
    }
    append_instructions(params_instructions, node);

    symtabnode *function_ptr = stFunCall_Fun(node);
    instruction = create_instruction(I_Call, function_ptr, NULL, NULL);
    append_instruction(instruction, node);

    if (function_ptr->ret_type != t_None) {
      tmp = create_temporary(function_ptr->ret_type);
      node->place = tmp;
      instruction = create_instruction(I_Retrieve, NULL, NULL, tmp);
      append_instruction(instruction, node);
    }
    break;
  }

  case STnodeList: {
    for (tnode *arg = node; arg != NULL; arg = stList_Rest(arg)) {
      generate_function_code(stList_Head(arg), lr_type);
      append_child_instructions(stList_Head(arg), node);
    }
    break;
  }

  case Return:
    if(stReturn(node)) {
      generate_function_code(stReturn(node), R_VALUE);
      append_child_instructions(stReturn(node), node);
      node->place = stReturn(node)->place;
    }
    instruction = create_instruction(I_Return, NULL, NULL, node->place);
    append_instruction(instruction, node);
    break;

  case UnaryMinus:
    generate_function_code(stUnop_Op(node), lr_type);
    append_child_instructions(stUnop_Op(node), node);
    tmp = create_temporary(node->etype);
    node->place = tmp;
    instruction =
        create_instruction(I_UMinus, stUnop_Op(node)->place, NULL, tmp);
    append_instruction(instruction, node);
    break;

  case Plus:
    generate_binary_arithmetic_expr_code(node, IT_Plus, lr_type);
    break;

  case BinaryMinus:
    generate_binary_arithmetic_expr_code(node, IT_BinaryMinus, lr_type);
    break;

  case Mult:
    generate_binary_arithmetic_expr_code(node, IT_Mult, lr_type);
    break;

  case Div:
    generate_binary_arithmetic_expr_code(node, IT_Div, lr_type);
    break;

  default:
    break;
  }
}

static void generate_binary_arithmetic_expr_code(tnode *node, ExprType type,
                                                 int lr_type) {
  generate_function_code(stBinop_Op1(node), lr_type);
  append_child_instructions(stBinop_Op1(node), node);
  generate_function_code(stBinop_Op2(node), lr_type);
  append_child_instructions(stBinop_Op2(node), node);
  symtabnode *tmp = create_temporary(node->etype);
  node->place = tmp;
  inode *instruction = create_expr_instruction(
      I_BinaryArithmetic, stBinop_Op1(node)->place, stBinop_Op2(node)->place,
      tmp, type);
  append_instruction(instruction, node);
}

void collect_global(char *id_name, int type, int scope) {
  if (scope == Global) {
    inode *instruction = create_global_decl_instruction(id_name, type);
    if (!global_head) {
      global_head = instruction;
      global_tail = global_head;
    } else {
      global_tail->next = instruction;
      global_tail = global_tail->next;
    }
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
  while (instruction) {
    append_instruction(instruction, node);
    instruction = instruction->next;
  }
}