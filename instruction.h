/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_INSTRUCTION_H
#define CSC553_INSTRUCTION_H

#include "global.h"
#include "symbol-table.h"

typedef enum InstructionType {
  I_Assign_Int,
  I_Assign_Char,
  I_Assign_Str,
  I_Assign,
  I_Call,
  I_Param,
  I_Retrieve,
  I_Enter,
  I_Return,
  I_String,
  I_Global,
  I_UMinus,
  I_BinaryArithmetic
} InstructionType;

typedef enum ExprType {
  IT_Int,
  IT_Char,
  IT_Plus,
  IT_BinaryMinus,
  IT_Mult,
  IT_Div
} ExprType;

typedef struct instr_node {
  InstructionType i_type;
  symtabnode *dest;
  char *label;
  ExprType
      type; // type of the data char, int or type of an operation +, -. *, /...

  union {
    struct op_members {
      symtabnode *src1;
      symtabnode *src2;
    } op_members;
    int const_int;
    char *const_char;
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
 * @param i_type: type of the operation
 * @param src1: first operand
 * @param src2: second operand
 * @param dest: destination to store the result of the operation
 *
 * @return new instruction
 */
inode *create_instruction(InstructionType i_type, symtabnode *src1,
                          symtabnode *src2, symtabnode *dest);

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
inode *create_expr_instruction(InstructionType op_type, symtabnode *src1,
                               symtabnode *src2, symtabnode *dest,
                               ExprType type);

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
inode *create_const_string_instruction(char *str_label);

/**
 * Creates an instruction for a global string and adds it to the list of
 * strings.
 *
 * @param str: string
 *
 * @return new instruction
 */
inode *create_string_instruction(char *str);

/**
 * Append a string instruction to the list of all string instructions already
 * created.
 */
void save_string_instruction(inode *instruction);

/**
 * Return head of the list of string instructions.
 *
 * @return pointer to string instruction head
 */
inode *get_string_instruction_head();

/**
 * Creates an instruction for declaration of a global variable.
 *
 * @param id_name: name of the variable
 * @param type: type of the variable
 *
 * @return new instruction
 */
inode *create_global_decl_instruction(char *id_name, int data_type);

#define SRC1(x) (x)->val.op_members.src1
#define SRC2(x) (x)->val.op_members.src2
#define DEST(x) (x)->dest
#define LABEL(x) (x)->val.label
#define CINT(x) (x)->val.const_int
#define CCHAR(x) (x)->val.const_char

#endif // CSC553_INSTRUCTION_H
