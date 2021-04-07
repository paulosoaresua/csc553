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

static int LOOP_FREQ = 10;

static void append_child_instructions(tnode *child, tnode *parent);
static void append_instruction(inode *instruction, tnode *node);
static void append_instructions(inode *instructions, tnode *node);
static void generate_binary_expr_code(symtabnode *func_header, tnode *node,
                                      enum InstructionType type, int lr_type,
                                      int outer_scope_freq);
static void generate_bool_expr_code(symtabnode *func_header, tnode *node,
                                    inode *label_then, inode *label_else,
                                    int outer_scope_freq);
static enum InstructionType get_boolean_comp_type(SyntaxNodeType node_type);
static void generate_function_args_code(symtabnode *func_header,
                                        tnode *call_node, int outer_scope_freq);
static void collect_var_cost(inode *instruction, int freq);

void process_function_header(symtabnode *func_header, tnode *body) {
  append_instructions(global_head, body);
  // This syntax tree node is the first node after a function if defined.
  // Therefore, we need to create an Enter instruction for it.
  inode *instruction = create_instruction(OP_Enter, func_header, NULL, NULL);
  append_instruction(instruction, body);
  func_header->entered = true;

  // Clear globals
  global_head = NULL;
  global_tail = NULL;
}

void process_allocations(symtabnode *function_ptr) {
  function_ptr->byte_size = fill_local_allocations();
}

void generate_function_code(symtabnode *func_header, tnode *node, int lr_type,
                            int outer_scope_freq) {
  inode *instruction;
  symtabnode *tmp;

  if (!node) {
    // Nothing to be done if the node is NULL
    return;
  }

  switch (node->ntype) {
  case Assg:
    generate_function_code(func_header, stAssg_Lhs(node), L_VALUE,
                           outer_scope_freq);
    append_child_instructions(stAssg_Lhs(node), node);

    generate_function_code(func_header, stAssg_Rhs(node), R_VALUE,
                           outer_scope_freq);
    append_child_instructions(stAssg_Rhs(node), node);

    if (stAssg_Lhs(node)->ntype == ArraySubscript) {
      instruction = create_instruction(OP_Assign, stAssg_Rhs(node)->place, NULL,
                                       stAssg_Lhs(node)->loc);
    } else {
      instruction = create_instruction(OP_Assign, stAssg_Rhs(node)->place, NULL,
                                       stAssg_Lhs(node)->place);
    }
    collect_var_cost(instruction, outer_scope_freq);
    append_instruction(instruction, node);

    // No longer needed after assigned to a variable
    if (stAssg_Rhs(node)->place->is_temporary) {
      free_temporary(stAssg_Rhs(node)->place);
    }

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
      node->place = create_temporary(t_Int);
      tmp = create_constant_variable(t_Int, node->val.iconst);
      instruction = create_instruction(OP_Assign, tmp, NULL, node->place);
      collect_var_cost(instruction, outer_scope_freq);
      append_instruction(instruction, node);
    }
    break;

  case Charcon:
    if (lr_type == L_VALUE) {
      fprintf(stderr, "A constant char cannot be used as an l-value.\n");
      return;
    } else {
      node->place = create_temporary(t_Char);
      tmp = create_constant_variable(t_Char, node->val.iconst);
      instruction = create_instruction(OP_Assign, tmp, NULL, node->place);
      collect_var_cost(instruction, outer_scope_freq);
      append_instruction(instruction, node);
    }
    break;

  case Stringcon:
    node->place = create_constant_string(stStringcon(node));
    break;

  case FunCall: {
    // Expand the parameters
    generate_function_args_code(func_header, node, outer_scope_freq);
    append_child_instructions(stFunCall_Args(node), node);

    // Create PARAM instructions
    inode *params_instructions = NULL;
    for (tnode *param = stFunCall_Args(node); param != NULL;
         param = stList_Rest(param)) {
      if (stList_Head(param)->place) {
        instruction =
            create_instruction(OP_Param, stList_Head(param)->place, NULL, NULL);

        if (stList_Head(param)->place->is_temporary) {
          free_temporary(stList_Head(param)->place);
        }
      } else {
        instruction =
            create_instruction(OP_Param, stList_Head(param)->loc, NULL, NULL);

        if (stList_Head(param)->loc->is_temporary) {
          free_temporary(stList_Head(param)->loc);
        }
      }
      collect_var_cost(instruction, outer_scope_freq);
      // Actuals from the right to the left
      instruction->next = params_instructions;
      params_instructions = instruction;
    }
    append_instructions(params_instructions, node);

    symtabnode *function_ptr = stFunCall_Fun(node);
    instruction = create_instruction(OP_Call, function_ptr, NULL, NULL);
    append_instruction(instruction, node);

    if (function_ptr->ret_type != t_None) {
      node->place = create_temporary(function_ptr->ret_type);
      instruction = create_instruction(OP_Retrieve, NULL, NULL, node->place);
      collect_var_cost(instruction, outer_scope_freq);
      append_instruction(instruction, node);
    }
    break;
  }

  case STnodeList: {
    for (tnode *arg = node; arg != NULL; arg = stList_Rest(arg)) {
      generate_function_code(func_header, stList_Head(arg), lr_type,
                             outer_scope_freq);
      append_child_instructions(stList_Head(arg), node);
    }
    break;
  }

  case Return:
    if (stReturn(node)) {
      generate_function_code(func_header, stReturn(node), R_VALUE,
                             outer_scope_freq);
      append_child_instructions(stReturn(node), node);

      node->place = create_temporary(func_header->ret_type);
      instruction = create_instruction(OP_Assign, stReturn(node)->place, NULL,
                                       node->place);
      collect_var_cost(instruction, outer_scope_freq);
      append_instruction(instruction, node);
    }
    instruction = create_instruction(OP_Return, node->place, NULL, NULL);
    collect_var_cost(instruction, outer_scope_freq);
    append_instruction(instruction, node);
    break;

  case UnaryMinus:
    generate_function_code(func_header, stUnop_Op(node), R_VALUE,
                           outer_scope_freq);
    append_child_instructions(stUnop_Op(node), node);
    node->place = create_temporary(node->etype);
    instruction = create_instruction(OP_UMinus, stUnop_Op(node)->place, NULL,
                                     node->place);
    collect_var_cost(instruction, outer_scope_freq);
    append_instruction(instruction, node);

    if (stUnop_Op(node)->place->is_temporary) {
      free_temporary(stUnop_Op(node)->place);
    }
    break;

  case Plus:
    generate_binary_expr_code(func_header, node, IT_Plus, R_VALUE,
                              outer_scope_freq);
    break;

  case BinaryMinus:
    generate_binary_expr_code(func_header, node, IT_BinaryMinus, R_VALUE,
                              outer_scope_freq);
    break;

  case Mult:
    generate_binary_expr_code(func_header, node, IT_Mult, R_VALUE,
                              outer_scope_freq);
    break;

  case Div:
    generate_binary_expr_code(func_header, node, IT_Div, R_VALUE,
                              outer_scope_freq);
    break;

  case If: {
    inode *label_then = create_label_instruction();
    inode *label_else = create_label_instruction();
    inode *label_after = create_label_instruction();

    // Boolean expression
    if (stIf_Else(node)) {
      generate_bool_expr_code(func_header, stIf_Test(node), label_then,
                              label_else, outer_scope_freq);
    } else {
      generate_bool_expr_code(func_header, stIf_Test(node), label_then,
                              label_after, outer_scope_freq);
    }
    append_child_instructions(stIf_Test(node), node);

    // Then block
    append_instruction(label_then, node);
    generate_function_code(func_header, stIf_Then(node), lr_type,
                           outer_scope_freq);
    append_child_instructions(stIf_Then(node), node);

    if (stIf_Else(node)) {
      // Jump to after the IF statement
      instruction = create_jump_instruction(label_after);
      append_instruction(instruction, node);

      // Else block
      append_instruction(label_else, node);
      generate_function_code(func_header, stIf_Else(node), lr_type,
                             outer_scope_freq);
      append_child_instructions(stIf_Else(node), node);
    }

    // Label AFTER
    append_instruction(label_after, node);
    break;
  }

  case While: {
    inode *label_body = create_label_instruction();
    inode *label_eval = create_label_instruction();
    inode *label_after = create_label_instruction();

    // Jump to eval
    instruction = create_jump_instruction(label_eval);
    append_instruction(instruction, node);

    // Body
    generate_function_code(func_header, stWhile_Body(node), lr_type,
                           LOOP_FREQ * outer_scope_freq);
    append_instruction(label_body, node);
    append_child_instructions(stWhile_Body(node), node);

    // Eval
    generate_bool_expr_code(func_header, stWhile_Test(node), label_body,
                            label_after, LOOP_FREQ * outer_scope_freq);
    append_instruction(label_eval, node);
    append_child_instructions(stWhile_Test(node), node);

    // After the WHILE
    append_instruction(label_after, node);
    break;
  }

  case For: {
    inode *label_body = create_label_instruction();
    inode *label_eval = create_label_instruction();
    inode *label_after = create_label_instruction();

    // Initialization
    generate_function_code(func_header, stFor_Init(node), lr_type,
                           outer_scope_freq);
    append_child_instructions(stFor_Init(node), node);

    // Jump to eval
    instruction = create_jump_instruction(label_eval);
    append_instruction(instruction, node);

    // Body
    generate_function_code(func_header, stFor_Body(node), lr_type,
                           LOOP_FREQ * outer_scope_freq);
    append_instruction(label_body, node);
    append_child_instructions(stFor_Body(node), node);

    // Update
    generate_function_code(func_header, stFor_Update(node), lr_type,
                           LOOP_FREQ * outer_scope_freq);
    append_child_instructions(stFor_Update(node), node);

    // Eval
    append_instruction(label_eval, node);
    if (stFor_Test(node)) {
      generate_bool_expr_code(func_header, stFor_Test(node), label_body,
                              label_after, LOOP_FREQ * outer_scope_freq);
      append_child_instructions(stFor_Test(node), node);
    } else {
      // No condition. Runs forever until stopped internally by a return.
      instruction = create_jump_instruction(label_body);
      append_instruction(instruction, node);
    }

    // After the FOR
    append_instruction(label_after, node);
    break;
  }

  case ArraySubscript: {
    // Evaluate the node's index as an r-value
    generate_function_code(func_header, stArraySubscript_Subscript(node),
                           R_VALUE, outer_scope_freq);
    append_child_instructions(stArraySubscript_Subscript(node), node);

    if (stArraySubscript_Subscript(node)->place->is_temporary) {
      free_temporary(stArraySubscript_Subscript(node)->place);
    }

    // This stores the memory address of the first position of the array
    symtabnode *array_node = stArraySubscript_Array(node);

    // Calculate the memory address of the correct index of the array and
    // store that in the tmp variable.
    symtabnode *tmp = create_temporary(t_Addr);
    tmp->elt_type = array_node->elt_type;
    instruction = create_instruction(OP_Index_Array,
                                     stArraySubscript_Subscript(node)->place,
                                     array_node, tmp);
    collect_var_cost(instruction, outer_scope_freq);
    append_instruction(instruction, node);

    if (lr_type == L_VALUE) {
      node->loc = tmp;
    } else {
      node->place = create_temporary(array_node->elt_type);
      instruction = create_instruction(OP_Deref, tmp, NULL, node->place);
      collect_var_cost(instruction, outer_scope_freq);
      append_instruction(instruction, node);
    }

    break;
  }

  default:
    fprintf(stderr, "Unknown syntax tree node type %d\n", node->ntype);
    break;
  }
}

void generate_function_args_code(symtabnode *func_header, tnode *call_node,
                                 int outer_scope_freq) {
  symtabnode *formal = stFunCall_Fun(call_node)->formals;
  tnode *arg_node = stFunCall_Args(call_node);
  for (tnode *arg = arg_node; arg != NULL; arg = stList_Rest(arg)) {
    if (formal->type == t_Array) {
      generate_function_code(func_header, stList_Head(arg), L_VALUE,
                             outer_scope_freq);
    } else {
      generate_function_code(func_header, stList_Head(arg), R_VALUE,
                             outer_scope_freq);
    }
    append_child_instructions(stList_Head(arg), arg_node);
    formal = formal->next;
  }
}

void generate_binary_expr_code(symtabnode *func_header, tnode *node,
                               enum InstructionType type, int lr_type,
                               int outer_scope_freq) {
  generate_function_code(func_header, stBinop_Op1(node), lr_type,
                         outer_scope_freq);
  append_child_instructions(stBinop_Op1(node), node);
  generate_function_code(func_header, stBinop_Op2(node), lr_type,
                         outer_scope_freq);
  append_child_instructions(stBinop_Op2(node), node);
  node->place = create_temporary(node->etype);
  inode *instruction =
      create_expr_instruction(OP_BinaryArithmetic, stBinop_Op1(node)->place,
                              stBinop_Op2(node)->place, node->place, type);
  collect_var_cost(instruction, outer_scope_freq);

  if (stBinop_Op1(node)->place->is_temporary) {
    free_temporary(stBinop_Op1(node)->place);
  }
  if (stBinop_Op2(node)->place->is_temporary) {
    free_temporary(stBinop_Op2(node)->place);
  }

  append_instruction(instruction, node);
}

void generate_bool_expr_code(symtabnode *func_header, tnode *node,
                             inode *label_true, inode *label_false,
                             int outer_scope_freq) {

  inode *instruction;

  switch (node->ntype) {
  case LogicalAnd: {
    inode *label_next = create_label_instruction();
    generate_bool_expr_code(func_header, stBinop_Op1(node), label_next,
                            label_false, outer_scope_freq);
    generate_bool_expr_code(func_header, stBinop_Op2(node), label_true,
                            label_false, outer_scope_freq);

    append_child_instructions(stBinop_Op1(node), node);
    append_instruction(label_next, node);
    append_child_instructions(stBinop_Op2(node), node);
    break;
  }
  case LogicalOr: {
    inode *label_next = create_label_instruction();
    generate_bool_expr_code(func_header, stBinop_Op1(node), label_true,
                            label_next, outer_scope_freq);
    generate_bool_expr_code(func_header, stBinop_Op2(node), label_true,
                            label_false, outer_scope_freq);

    append_child_instructions(stBinop_Op1(node), node);
    append_instruction(label_next, node);
    append_child_instructions(stBinop_Op2(node), node);
    break;
  }
  case LogicalNot: {
    generate_bool_expr_code(func_header, stUnop_Op(node), label_false,
                            label_true, outer_scope_freq);
    append_child_instructions(stUnop_Op(node), node);
    break;
  }
  case Leq:
  case Equals:
  case Lt:
  case Gt:
  case Neq:
  case Geq: {
    generate_function_code(func_header, stBinop_Op1(node), R_VALUE, outer_scope_freq);
    generate_function_code(func_header, stBinop_Op2(node), R_VALUE, outer_scope_freq);

    append_child_instructions(stBinop_Op1(node), node);
    append_child_instructions(stBinop_Op2(node), node);

    enum InstructionType type = get_boolean_comp_type(node->ntype);
    instruction = create_cond_jump_instruction(
        stBinop_Op1(node)->place, stBinop_Op2(node)->place, label_true, type);
    collect_var_cost(instruction, outer_scope_freq);
    append_instruction(instruction, node);

    instruction = create_jump_instruction(label_false);
    append_instruction(instruction, node);
    break;
  }

  default:
    fprintf(stderr, "Unknown syntax tree node type %d\n", node->ntype);
    break;
  }
}

enum InstructionType get_boolean_comp_type(SyntaxNodeType node_type) {
  switch (node_type) {
  case Leq:
    return IT_LE;
  case Equals:
    return IT_EQ;
  case Lt:
    return IT_LT;
  case Gt:
    return IT_GT;
  case Neq:
    return IT_NE;
  case Geq:
    return IT_GE;
  default:
    fprintf(stderr,
            "Invalid syntax node type for boolean expression "
            "evaluation. %d.\n",
            node_type);
    return -1;
  }
}

void collect_global(symtabnode *var) {
  if (var->scope == Global) {
    inode *instruction = create_global_decl_instruction(var);
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
void append_child_instructions(tnode *child, tnode *parent) {
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
void append_instruction(inode *instruction, tnode *node) {
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
void append_instructions(inode *instructions, tnode *node) {
  inode *instruction = instructions;
  while (instruction) {
    append_instruction(instruction, node);
    instruction = instruction->next;
  }
}

void collect_var_cost(inode *instruction, int freq) {
  if (SRC1(instruction)) {
    SRC1(instruction)->cost += freq;
  }

  if (SRC2(instruction)) {
    SRC2(instruction)->cost += freq;
  }

  if (instruction->dest) {
    instruction->dest->cost += freq;
  }
}