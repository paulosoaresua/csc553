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
 * Prints code for pre-defined functions.
 */
void print_pre_defined_instructions();

/**
 * Converts and prints on the terminal a series of 3-address instructions to
 * MIPS assembly code
 */
void print_instructions(tnode* node);

/**
 * Prints string declarations.
 */
void print_strings();

#endif // CSC553_CODE_TRANSLATION_H