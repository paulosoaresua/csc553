/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_INSTRUCTION_H
#define CSC553_INSTRUCTION_H

#include "global.h"
#include "symbol-table.h"

typedef enum OperationType {
  OP_Call,
  OP_Const_Int
} OperationType;

typedef struct instr_node {
  OperationType op_type;

  union {
    struct op_members {
        symtabnode *src1;
        symtabnode *src2;
        symtabnode *dest;
    } op_members;
    char* label;
    int const_int;
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
 *
 * @return new instruction
 */
inode *create_const_int_instruction(int constant);

#endif // CSC553_INSTRUCTION_H
