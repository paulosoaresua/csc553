/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

inode *create_label_instruction() {
//  inode* instruction = zalloc(sizeof(*instruction));
//  instruction->op_type = OP_Label;
//  instruction->val.label = src1;
//
//  return instruction;
}

inode *create_instruction(OperationType op_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest) {

  inode* instruction = zalloc(sizeof(*instruction));
  instruction->op_type = op_type;
  instruction->val.op_members.src1 = src1;
  instruction->val.op_members.src2 = src2;
  instruction->val.op_members.dest = dest;

  return instruction;
}

inode *create_const_int_instruction(int constant) {
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Const_Int;
  instruction->val.const_int = constant;

  return instruction;
}