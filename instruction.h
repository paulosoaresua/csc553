/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_INSTRUCTION_H
#define CSC553_INSTRUCTION_H

#include "block.h"
#include "symbol-table.h"

typedef enum OpType {
  OP_Assign_Int,
  OP_Assign_Char,
  OP_Assign,
  OP_Call,
  OP_Param,
  OP_Retrieve,
  OP_Enter,
  OP_Return,
  OP_Global,
  OP_UMinus,
  OP_BinaryArithmetic,
  OP_Label,
  OP_If,
  OP_Goto,
  OP_Index_Array,
  OP_Deref,
} OPType;

typedef enum InstructionType {
  IT_Plus,
  IT_BinaryMinus,
  IT_Mult,
  IT_Div,
  IT_LE,
  IT_EQ,
  IT_LT,
  IT_GT,
  IT_NE,
  IT_GE,
} InstructionType;

typedef struct Instruction {
  enum OpType op_type;
  symtabnode *dest;
  char *label;
  enum InstructionType type;
  struct Instruction *jump_to;

  union {
    struct op_members {
      symtabnode *src1;
      symtabnode *src2;
    } op_members;
    int const_int;
  } val;

  struct Instruction *previous;
  struct Instruction *next;

  // For code optimization
  int order;
  int definition_id; // Unique ID for each instruction that assigns to a
  // variable
  bnode *block;

} inode;

static int label_counter = 0;

/**
 * Creates a pointer to a 3-address instruction.
 *
 * @param i_type: type of the operation
 * @param src1: first operand
 * @param src2: second operand
 * @param dest: destination to store the result of the operation
 *
 * @return new instruction
 */
inode *create_instruction(enum OpType i_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest);

/**
 * Creates a label instruction for jumping purposes.
 *
 * @return new label instruction
 */
inode *create_label_instruction();

/**
 * Creates an instruction for an expression.
 *
 * @param op_type: type of the operation
 * @param src1: first operand
 * @param src2: second operand
 * @param dest: destination to store the result of the operation
 * @param type: expression type
 * @return
 */
inode *create_expr_instruction(enum OpType op_type, symtabnode *src1,
                               symtabnode *src2, symtabnode *dest,
                               enum InstructionType type);

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
 * Creates an instruction for declaration of a global variable.
 *
 * @param var: symbol table entry for the global variable
 *
 * @return new instruction
 */
inode *create_global_decl_instruction(symtabnode *var);

/**
 * Creates a conditional jump instruction to jump to a label.
 *
 * @param src1: first operand
 * @param src2: second operand
 * @param destiny_instruction: instruction to jump to
 * @param type: type of the comparison
 * @return
 */
inode *create_cond_jump_instruction(symtabnode *src1,
                                    symtabnode *src2,
                                    inode *destiny_instruction,
                                    enum InstructionType type);

/**
 * Creates an unconditional jump instruction.
 *
 * @param destiny_instruction: instruction to jump to
 * @return
 */
inode *create_jump_instruction(inode *destiny_instruction);

/**
 * Print 3-addr code instruction
 *
 * @param instruction: instruction
 */
void print_instruction(inode *instruction);

#define SRC1(x) (x)->val.op_members.src1
#define SRC2(x) (x)->val.op_members.src2

/*********************************************************************
 *                                                                   *
 *                           for optimization                        *
 *                                                                   *
 *********************************************************************/

/**
 * Connects a subsequent instruction to its predecessor.
 *
 * @param instructions_head: first of a list of instructions.
 */
void fill_backward_connections(inode *instructions_head);

/**
 * Inverts boolean operator of a boolean expression instruction.
 *
 * @param instruction: instruction
 */
void invert_boolean_operator(inode *instruction);

/**
 * Checks whether the instruction redefines a variable.
 *
 * @param instruction: instruction
 * @return
 */
bool redefines_variable(inode* instruction);

#endif // CSC553_INSTRUCTION_H
