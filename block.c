///*
// * Author: Paulo Soares.
// * CSC 553 (Spring 2021)
// */
//
//#include "block.h"
//#include "symbol-table.h"
//
//blist_node* create_new_block() {
//  blist_node* block = zalloc(sizeof(blist_node));
//
//  return block;
//}
//
//void append_block_to_list(blist_node* block, blist* list) {
//  if(list->head) {
//    list->tail->next = block;
//    list->tail = list->tail->next;
//  } else {
//    list->head = zalloc(sizeof(blist_node));
//    list->tail = list->head;
//  }
//}
//
//void append_instruction_to_block(inode* instruction, blist_node* block) {
//  if(!block->block) {
//    block->block = zalloc(sizeof(iblock));
//  }
//
//  if(block->block->head) {
//    block->block->tail->next = zalloc(sizeof(iblock_node));
//    block->block->tail = block->block->tail->next;
//  } else {
//    block->block->head = zalloc(sizeof(blist_node));
//    block->block->tail = block->block->head;
//  }
//
//  block->block->tail->instruction = instruction;
//}
//
//void add_child_to_block(blist_node* block, blist_node* child) {
//
//}
//
//void add_parent_to_block(blist_node* block, blist_node* parent) {
//
//}
//
//iblock_node * create_new_block_instruction(inode* instruction) {
//
//}