/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_CODE_OPTIMIZATION_H
#define CSC553_CODE_OPTIMIZATION_H

#include "control_flow.h"

/**
 * Enables local optimization.
 */
void enable_local_optimization();

/**
 * Enables local optimization.
 */
void enable_global_optimization();

/**
 * Print blocks and instructions.
 *
 * @param file: file where the instructions must be printed
 */
void print_blocks_and_instructions(FILE* file);

/**
 * Optimize code.
 *
 * @param function_body: first syntax-tree node of the current parsed function body.
 */
void optimize_instructions(tnode* function_body);

#endif // CSC553_CODE_OPTIMIZATION_H
