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
 * @param function_body: first node of a function body
 */
void build_control_flow_graph(tnode* function_body);

#endif





//
//#ifndef CSC553_CODE_TRANSLATION_H
//#define CSC553_CODE_TRANSLATION_H
//
//#include "instruction.h"
//#include "protos.h"
//#include "syntax-tree.h"
//
//typedef struct instruction_block {
//  inode* instruction;
//  struct instruction_block* next;
//} iblock;
//
//typedef struct block_node {
//  iblock* instructions_head;
//  iblock* instructions_tail;
//  struct block_node *parents;
//  struct block_node *children;
//  struct block_node *next;
//} bnode;
//
///**
// * Builds a control flow graph for a given function.
// *
// * @param function_body_start_stnode: first syntax-tree node of a parsed
// * function body.
// */
//void build_control_flow_graph(tnode *function_body_start_stnode);
//
//#endif // CSC553_CODE_TRANSLATION_H