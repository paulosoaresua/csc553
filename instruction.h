/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_INSTRUCTION_H
#define CSC553_INSTRUCTION_H

#include "global.h"
#include "symbol-table.h"

typedef enum OperationType {
  OP_Assign_Int,
  OP_Assign_Char,
  OP_Assign_Str,
  OP_Assign,
  OP_Call,
  OP_Param,
  OP_Retrieve,
  OP_Enter,
  OP_Return,
  OP_String
} OperationType;

typedef struct instr_node {
  OperationType op_type;
  symtabnode *dest;
  char* label;

  union {
    struct op_members {
        symtabnode *src1;
        symtabnode *src2;
    } op_members;
    int const_int;
    char* const_char;
  } val;

  struct instr_node *next;
} inode;

static int label_counter = 0;
static int string_counter = 0;

/**
 * Creates a label instruction for jumping purposes.
 *
 * @return new label instruction
 */
inode *create_label_instruction();

/**
 * Creates a pointer to a 3-address instruction.
 *
 * @param op_type: type of the operation
 * @param src1: first operand
 * @param src2: second operand
 * @param dest: destination to store the result of the operation
 *
 * @return new instruction
 */
inode *create_instruction(OperationType op_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest);

/**
 * Creates an instruction for a constant integer.
 *
 * @param constant: constant integer
 * @param dest: destination to store the integer
 *
 * @return new instruction
 */
inode *create_const_int_instruction(int int_val, symtabnode *dest);

/**
 * Creates an instruction for a constant character.
 *
 * @param char_val: constant char as an integer
 * @param dest: destination to store the char
 *
 * @return new instruction
 */
inode *create_const_char_instruction(int char_val, symtabnode *dest);

/**
 * Creates an instruction for a constant string.
 *
 * @param str_label: label created for the string globally
 *
 * @return new instruction
 */
inode *create_const_string_instruction(char* str_label);

/**
 * Creates an instruction for a global string and adds it to the list of
 * strings.
 *
 * @param str: string
 *
 * @return new instruction
 */
inode* create_string_instruction(char* str);

/**
 * Append a string instruction to the list of all string instructions already
 * created.
 */
void save_string_instruction(inode* instruction);

/**
 * Return head of the list of string instructions.
 *
 * @return pointer to string instruction head
 */
inode* get_string_instruction_head();


#define SRC1(x) (x)->val.op_members.src1
#define SRC2(x) (x)->val.op_members.src2
#define DEST(x) (x)->dest
#define LABEL(x) (x)->val.label
#define CINT(x) (x)->val.const_int
#define CCHAR(x) (x)->val.const_char


#endif // CSC553_INSTRUCTION_H
