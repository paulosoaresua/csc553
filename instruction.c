/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

// Global variable that stores all the string instructions created
static inode* string_instructions_head = NULL;

inode *create_label_instruction() {

  return NULL;
//  inode* instruction = zalloc(sizeof(*instruction));
//  instruction->i_type = OP_Label;
//  instruction->val.label = src1;
//
//  return instruction;
}

inode *create_instruction(InstructionType i_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest) {

  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = i_type;
  instruction->val.op_members.src1 = src1;
  instruction->val.op_members.src2 = src2;
  instruction->dest = dest;

  return instruction;
}

inode *create_expr_instruction(InstructionType i_type, symtabnode *src1,
                               symtabnode *src2, symtabnode *dest,
                               ExprType type) {
  inode* instruction = create_instruction(i_type, src1, src2, dest);
  instruction->type = type;

  return instruction;
}

inode *create_const_int_instruction(int int_val, symtabnode *dest){
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = I_Assign_Int;
  instruction->val.const_int = int_val;
  instruction->dest = dest;

  return instruction;
}

inode *create_const_char_instruction(int char_val, symtabnode *dest){
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = I_Assign_Char;
  instruction->val.const_int = char_val;
  instruction->dest = dest;

  return instruction;
}

inode *create_const_string_instruction(char* str_label){
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = I_Assign_Str;
  instruction->label = str_label;

  return instruction;
}

inode* create_string_instruction(char* str) {
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = I_String;
  instruction->val.const_char = malloc(strlen(str) * sizeof(char));
  instruction->val.const_char = strcpy(instruction->val.const_char, str);

  // Unique identifier for the string label
  instruction->label = malloc(16 * sizeof(char));
  sprintf(instruction->label, "__Str%d", string_counter++);

  save_string_instruction(instruction);

  return instruction;
}

void save_string_instruction(inode* instruction) {
  if (string_instructions_head) {
    string_instructions_head->next = instruction;
  } else {
    string_instructions_head = instruction;
  }
}

inode* get_string_instruction_head() {
  return string_instructions_head;
}

inode *create_global_decl_instruction(char* id_name, int data_type) {
  inode* instruction = zalloc(sizeof(*instruction));
  instruction->i_type = I_Global;
  instruction->label = id_name;

  switch (data_type) {
  case t_Int:
    instruction->type = IT_Int;
    break;
  case t_Char:
    instruction->type = IT_Char;
    break;
  }

  return instruction;
}