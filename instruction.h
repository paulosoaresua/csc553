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
  OP_Assign,
  OP_Call,
  OP_Param,
  OP_Retrieve,
  OP_Enter,
  OP_Return
} OperationType;

typedef struct instr_node {
  OperationType op_type;
  symtabnode *dest;

  union {
    struct op_members {
        symtabnode *src1;
        symtabnode *src2;
    } op_members;
    char* label;
    int const_int;
    char* const_char;
  } val;

  struct instr_node *next;
} inode;

static int label_counter = 0;
static int string_counter = 0;

/**
 * Creates a label instruction for jumping purposes
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
 * Creates an instruction for a constant integer
 *
 * @param constant: constant integer
 * @param dest: destination to store the integer
 *
 * @return new instruction
 */
inode *create_const_int_instruction(int int_val, symtabnode *dest);

/**
 * Creates an instruction for a constant character
 *
 * @param constant: constant character
 * @param dest: destination to store the character
 *
 * @return new instruction
 */
inode *create_const_char_instruction(char* char_val, symtabnode *dest);


#define SRC1(x) (x)->val.op_members.src1
#define SRC2(x) (x)->val.op_members.src2
#define DEST(x) (x)->dest
#define LABEL(x) (x)->val.label
#define CINT(x) (x)->val.const_int
#define CCHAR(x) (x)->val.const_char


#endif // CSC553_INSTRUCTION_H
