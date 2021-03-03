///*
// * Author: Paulo Soares.
// * CSC 553 (Spring 2021)
// */
//
//#ifndef CSC553_BLOCK_H
//#define CSC553_BLOCK_H
//
//#include "instruction.h"
//#include "protos.h"
//#include "syntax-tree.h"
//
//// Represents a node in a block of instructions
//typedef struct InstructionBlockNodeType {
//  inode* instruction;
//  struct InstructionBlockNodeType* next;
//} iblock_node;
//
//// Represents a list of instructions in a block
//typedef struct InstructionBlockType {
//  iblock_node* head;
//  iblock_node* tail;
//} iblock;
//
//// Represents a node in a list of blocks
//typedef struct BlockListNodeType {
//  iblock* block;
//  struct BlockListNodeType* next;
//
//  struct BlockListNodeType* children;
//  struct BlockListNodeType* parents;
//} blist_node;
//
//// Represents a list of blocks
//typedef struct BlockListType {
//  blist_node* head;
//  blist_node* tail;
//} blist;
//
//blist_node* create_new_block();
//void append_block_to_list(blist_node* block, blist* list);
//void append_instruction_to_block(inode* instruction, blist_node* block);
//void add_child_to_block(blist_node* block, blist_node* child);
//void add_parent_to_block(blist_node* block, blist_node* parent);
//iblock_node * create_new_block_instruction(inode* instruction);
//
///**
// * Builds a control flow graph for a given function.
// *
// * @param function_body_start_stnode: first syntax-tree node of a parsed
// * function body.
// */
////void build_control_flow_graph(tnode *function_body_start_stnode);
//
//#endif // CSC553_CODE_TRANSLATION_H