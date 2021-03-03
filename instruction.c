/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

inode *create_label_instruction() {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->label = malloc(16 * sizeof(char));
  sprintf(instruction->label, "L%d", label_counter++);
  instruction->op_type = OP_Label;

  return instruction;
}

inode *create_instruction(enum OpType i_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest) {

  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = i_type;
  instruction->val.op_members.src1 = src1;
  instruction->val.op_members.src2 = src2;
  instruction->dest = dest;

  return instruction;
}

inode *create_expr_instruction(enum OpType i_type, symtabnode *src1,
                               symtabnode *src2, symtabnode *dest,
                               enum InstructionType type) {
  inode *instruction = create_instruction(i_type, src1, src2, dest);
  instruction->type = type;

  return instruction;
}

inode *create_const_int_instruction(int int_val, symtabnode *dest) {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Assign_Int;
  instruction->val.const_int = int_val;
  instruction->dest = dest;

  return instruction;
}

inode *create_const_char_instruction(int char_val, symtabnode *dest) {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Assign_Char;
  instruction->val.const_int = char_val;
  instruction->dest = dest;

  return instruction;
}

inode *create_global_decl_instruction(symtabnode *var) {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Global;
  instruction->val.op_members.src1 = var;

  return instruction;
}

inode *create_cond_jump_instruction(enum OpType op_type, symtabnode *src1,
                                    symtabnode *src2, inode* destiny_instruction,
                                    enum InstructionType type) {
  inode *instruction = create_expr_instruction(op_type, src1, src2, NULL, type);
  instruction->jump_to = destiny_instruction;
  return instruction;
}

inode *create_jump_instruction(inode* destiny_instruction) {
  inode *instruction = create_instruction(OP_Goto, NULL, NULL, NULL);
  instruction->jump_to = destiny_instruction;
  return instruction;
}

void print_instruction(inode *instruction) {
  switch (instruction->op_type) {
  case OP_Global: {
    printf("GLOBAL %s \n", SRC1(instruction)->name);
    break;
  }
  case OP_Enter: {
    printf("ENTER %s \n", SRC1(instruction)->name);
    break;
  }

  case OP_Assign_Int:
  case OP_Assign_Char:
  case OP_Assign:
    printf("ASSIGN %s \n", instruction->dest->name);
    break;

  case OP_Param: {
    printf("PARAM %s \n", SRC1(instruction)->name);
    break;
  }

  case OP_Call: {
    printf("CALL %s \n", SRC1(instruction)->name);
    break;
  }

  case OP_Return:
    printf("RETURN \n");
    break;

  case OP_Retrieve:
    printf("RETRIEVE %s \n", instruction->dest->name);
    break;

  case OP_UMinus:
    printf("UMINUS %s \n", SRC1(instruction)->name);
    break;

  case OP_BinaryArithmetic:
    printf("BINARY_OPERATION %s ? %s \n", SRC1(instruction)->name,
           SRC2(instruction)->name);
    break;

  case OP_Label:
    printf("LABEL %s \n", instruction->label);
    break;

  case OP_If:
    printf("COND_JUMP %s \n", instruction->label);
    break;

  case OP_Goto:
    printf("JUMP %s \n", instruction->label);
    break;

  case OP_Index_Array:
    printf("ARRAY_INDEX %s \n", SRC1(instruction)->name);
    break;

  case OP_Deref:
    printf("DEREF \n");
    break;

  default:
    break;
  }
}

/*********************************************************************
 *                                                                   *
 *                           for optimization                        *
 *                                                                   *
 *********************************************************************/

void invert_boolean_operator(inode* instruction) {
  switch (instruction->type) {
  case IT_EQ:
    instruction->type = IT_NE;
    break;
  case IT_NE:
    instruction->type = IT_EQ;
    break;
  case IT_LE:
    instruction->type = IT_GT;
    break;
  case IT_GT:
    instruction->type = IT_LE;
    break;
  case IT_GE:
    instruction->type = IT_LT;
    break;
  case IT_LT:
    instruction->type = IT_GE;
    break;
  default:
    break;
  }
}

void fill_backward_connections(inode* instructions_head) {
  inode* last_node = NULL;
  inode* curr_instruction = instructions_head;
  while(curr_instruction) {
    curr_instruction->previous = last_node;
    last_node = curr_instruction;
    curr_instruction = curr_instruction->next;
  }
}

bnode *create_block(inode *leader, bool preserve_old) {
  if(leader->block) {
    return leader->block;
  }

  bnode *block = zalloc(sizeof(bnode));
  block->first_instruction = leader;

  return block;
}

void add_child_block(bnode *child, bnode *parent) {
  // Add child to the parent's child list
  blist_node *child_node = zalloc(sizeof(blist_node));
  child_node->block = child;
  child_node->next = parent->children;
  parent->children = child_node;

  // Add parent to the child's parent list
  blist_node *parent_node = zalloc(sizeof(blist_node));
  parent_node->block = parent;
  parent_node->next = child->parents;
  child->parents = parent_node;
}