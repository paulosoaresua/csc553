/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_CODE_TRANSLATION_H
#define CSC553_CODE_TRANSLATION_H

#include "instruction.h"
#include "protos.h"
#include "syntax-tree.h"

/**
 * Prints code for global variables and pre-defined functions.
 */
void print_globals();

/**
 * Converts and prints on the terminal a series of 3-address instructions to
 * MIPS assembly code
 */
void print_function(tnode* node);

/**
 * Prints MIPS assembly code for the predefined function println.
 */
void print_println();

/**
 * Prints a comment with the function name before the function definition.
 *
 * @param function_name: name of the function
 */
void print_function_header(char* function_name);

void print_strings();

#endif // CSC553_CODE_TRANSLATION_H