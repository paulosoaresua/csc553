#include "block.h"

static int block_id = 0;
static blist_node *created_blocks; // List of all blocks created

bnode *create_block() {
  bnode *block = zalloc(sizeof(bnode));
  block->id = block_id++;

  // Add block to the list of created blocks
  blist_node *block_node = zalloc(sizeof(blist_node));
  block_node->block = block;
  block_node->next = created_blocks;
  created_blocks = block_node;

  return block;
}

void connect_blocks(bnode *parent, bnode *child) {
  // Add child to the parent's child list
  if (child) {
    blist_node *child_node = zalloc(sizeof(blist_node));
    child_node->block = child;
    child_node->next = parent->children;
    parent->children = child_node;
  }

  // Add parent to the child's parent list
  if (parent) {
    blist_node *parent_node = zalloc(sizeof(blist_node));
    parent_node->block = parent;
    parent_node->next = child->parents;
    child->parents = parent_node;
  }
}

void clear_created_blocks() {
  block_id = 0;
  created_blocks = NULL;
}

int get_num_created_blocks() { return block_id + 1; }

blist_node *get_all_blocks() { return created_blocks; }