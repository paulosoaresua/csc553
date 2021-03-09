/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

inode *create_instruction(enum OpType i_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest) {

  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = i_type;
  instruction->val.op_members.src1 = src1;
  instruction->val.op_members.src2 = src2;
  instruction->dest = dest;
  instruction->dead = false;

  return instruction;
}

inode *create_label_instruction() {
  inode *instruction = create_instruction(OP_Label, NULL, NULL, NULL);
  instruction->label = malloc(16 * sizeof(char));
  sprintf(instruction->label, "L%d", label_counter++);

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
  inode *instruction = create_instruction(OP_Assign_Int, NULL, NULL, dest);
  instruction->val.const_int = int_val;

  return instruction;
}

inode *create_const_char_instruction(int char_val, symtabnode *dest) {
  inode *instruction = create_instruction(OP_Assign_Int, NULL, NULL, dest);
  instruction->val.const_int = char_val;

  return instruction;
}

inode *create_global_decl_instruction(symtabnode *var) {
  inode *instruction = create_instruction(OP_Global, var, NULL, NULL);

  return instruction;
}

inode *create_cond_jump_instruction(symtabnode *src1, symtabnode *src2,
                                    inode *destiny_instruction,
                                    enum InstructionType type) {
  inode *instruction = create_expr_instruction(OP_If, src1, src2, NULL, type);
  instruction->jump_to = destiny_instruction;
  return instruction;
}

inode *create_jump_instruction(inode *destiny_instruction) {
  inode *instruction = create_instruction(OP_Goto, NULL, NULL, NULL);
  instruction->jump_to = destiny_instruction;
  return instruction;
}

void print_instruction(inode *instruction, FILE *file) {
  switch (instruction->op_type) {
  case OP_Global: {
    fprintf(file, "GLOBAL %s", SRC1(instruction)->name);
    break;
  }
  case OP_Enter: {
    fprintf(file, "ENTER %s", SRC1(instruction)->name);
    break;
  }

  case OP_Assign_Int:
  case OP_Assign_Char:
    fprintf(file, "ASSIGN_CONST %s = %d", instruction->dest->name,
            instruction->val.const_int);
    break;

  case OP_Assign:
    fprintf(file, "ASSIGN %s = %s", instruction->dest->name,
            SRC1(instruction)->name);
    break;

  case OP_Param: {
    fprintf(file, "PARAM %s", SRC1(instruction)->name);
    break;
  }

  case OP_Call: {
    fprintf(file, "CALL %s", SRC1(instruction)->name);
    break;
  }

  case OP_Return:
    fprintf(file, "RETURN");
    break;

  case OP_Retrieve:
    fprintf(file, "RETRIEVE %s", instruction->dest->name);
    break;

  case OP_UMinus:
    fprintf(file, "UMINUS %s", SRC1(instruction)->name);
    break;

  case OP_BinaryArithmetic:
    fprintf(file, "BINARY_OPERATION %s = %s ? %s", instruction->dest->name,
            SRC1(instruction)->name, SRC2(instruction)->name);
    break;

  case OP_Label:
    fprintf(file, "LABEL %s", instruction->label);
    break;

  case OP_If:
    fprintf(file, "COND_JUMP %s ? %s", SRC1(instruction)->name,
            SRC2(instruction)->name);
    break;

  case OP_Goto:
    fprintf(file, "JUMP %s", instruction->jump_to->label);
    break;

  case OP_Index_Array:
    fprintf(file, "ARRAY_INDEX %s -> %s", SRC1(instruction)->name,
            instruction->dest->name);
    break;

  case OP_Deref:
    fprintf(file, "DEREF %s -> %s", SRC1(instruction)->name,
            instruction->dest->name);
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

void invert_boolean_operator(inode *instruction) {
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

void fill_backward_connections(inode *instructions_head) {
  inode *last_node = NULL;
  inode *curr_instruction = instructions_head;
  while (curr_instruction) {
    curr_instruction->previous = last_node;
    last_node = curr_instruction;
    curr_instruction = curr_instruction->next;
  }
}

bool redefines_variable(inode *instruction) {
  return instruction->dest != NULL;
}

bool is_rhs_variable(inode *instruction) {
  return SRC1(instruction) && instruction->op_type != OP_Assign_Int &&
         instruction->op_type != OP_Assign_Char &&
         instruction->op_type != OP_Call && instruction->op_type != OP_Enter &&
         instruction->op_type != OP_Global;
}
