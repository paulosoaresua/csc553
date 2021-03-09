/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_CONTROL_FLOW_H
#define CSC553_CONTROL_FLOW_H

#include "instruction.h"
#include "protos.h"
#include "syntax-tree.h"

/**
 * Builds a control flow graph for the set of instructions within a function
 * body.
 *
 * @param instruction_head: first instruction of a function
 */
void build_control_flow_graph(inode* instruction_head);

/**
 * Print blocks (and their leaders' ids) and their connections
 *
 * @param file: file to print the graph
 */
void print_control_flow_graph(FILE* file);

/**
 * Gets the total number of instructions created within a function.
 *
 * @return
 */
int get_total_instructions();

/**
 * Gets the total number of assignment instructions created within a function.
 *
 * @return
 */
int get_total_assignment_instructions();

#endif

