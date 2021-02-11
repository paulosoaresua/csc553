/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#include "instruction.h"

// Global variable that stores all the string instructions created
static inode *string_instructions_head = NULL;

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

inode *create_const_string_instruction(char *str_label) {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_Assign_Str;
  instruction->label = str_label;

  return instruction;
}

inode *create_string_instruction(char *str) {
  inode *instruction = zalloc(sizeof(*instruction));
  instruction->op_type = OP_String;
  instruction->val.const_char = malloc(strlen(str) * sizeof(char));
  instruction->val.const_char = strcpy(instruction->val.const_char, str);

  // Unique identifier for the string label
  instruction->label = malloc(16 * sizeof(char));
  sprintf(instruction->label, "__Str%d", string_counter++);

  save_string_instruction(instruction);

  return instruction;
}

void save_string_instruction(inode *instruction) {
  if (string_instructions_head) {
    string_instructions_head->next = instruction;
  } else {
    string_instructions_head = instruction;
  }
}

inode *get_string_instruction_head() { return string_instructions_head; }

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