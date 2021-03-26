/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_BLOCK_H
#define CSC553_BLOCK_H

#include "global.h"
#include "set.h"

typedef struct BlockListNode {
  struct Block* block;
  struct BlockListNode* next;
} blist_node;

typedef struct Block {
  int id;
  struct Instruction* first_instruction;
  struct Instruction* last_instruction;

  blist_node* children;
  blist_node* parents;

  set dominators;

  union{
    set gen;
    set def;
  };

  union {
    set kill;
    set use;
  };

  set in;
  set out;
} bnode;

/**
 * Creates a new block.
 *
 * @return block
 */
bnode *create_block();

/**
 * Creates a parent to child and a child to parent connections.
 *
 * @param child: child block
 * @param parent: parent block
 *
 */
void connect_blocks(bnode *parent, bnode *child);

/**
 * Clear the list of created blocks and resets the global block ID.
 *
 */
void clear_created_blocks();

/**
 * Gets the total number of blocks created.
 *
 * @return
 */
int get_num_created_blocks();

/**
 * Gets the list of all blocks created.
 *
 * @return Block list head
 */
blist_node *get_all_blocks();

#endif
