/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

inode *create_label_instruction() {

  return NULL;
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
  instruction->dest = dest;

  return instruction;
}

inode *create_const_int_instruction(int int_val, symtabnode *dest){
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Assign_Int;
  instruction->val.const_int = int_val;
  instruction->dest = dest;

  return instruction;
}

inode *create_const_char_instruction(char* char_val, symtabnode *dest){
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Assign_Char;
  instruction->val.const_char = char_val;
  instruction->dest = dest;

  return instruction;
}