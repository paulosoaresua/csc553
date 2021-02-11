/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

inode *create_label_instruction() {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->label = malloc(16* sizeof(char));
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
                                    symtabnode *src2, char *label,
                                    enum InstructionType type) {
  inode *instruction = create_expr_instruction(op_type, src1, src2, NULL, type);
  instruction->label  = label;
  return instruction;
}

inode *create_jump_instruction(char *label) {
  inode *instruction = create_instruction(OP_Goto, NULL, NULL, NULL);
  instruction->label = label;
  return instruction;
}