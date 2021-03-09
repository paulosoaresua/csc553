/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "control_flow.h"

static int total_instructions = 0;
static int total_assignment_instructions = 0;

static void find_block_leaders(inode *instruction_head);
static void update_blocks(inode *instruction_head);
static void find_dominators();
static set get_dominators_from_predecessors(bnode *block);

void build_control_flow_graph(inode *instruction_head) {
  clear_created_blocks();
  find_block_leaders(instruction_head);
  update_blocks(instruction_head);
  find_dominators();
}

/**
 * Find block leaders and associate new blocks to them.
 *
 * @param instruction_head: first instruction of a function
 */
void find_block_leaders(inode *instruction_head) {
  inode *curr_instruction = instruction_head;

  while (curr_instruction) {
    if(curr_instruction->op_type == OP_Global) {
      // Ignore Global variables declaration
      curr_instruction = curr_instruction->next;
      continue;
    }

    curr_instruction->order =
        total_instructions++; // Unique id for the instruction (it
    // represents the order of the instruction in the list of instructions)

    if (redefines_variable(curr_instruction)) {
      curr_instruction->definition_id = total_assignment_instructions++;
    }

    switch (curr_instruction->op_type) {
    case OP_Enter:
    case OP_Call:
      if (!curr_instruction->block) {
        // It could have been set as leader before by another instruction
        curr_instruction->block = create_block(curr_instruction, true);
        curr_instruction->block->first_instruction = curr_instruction;
      }
      break;

    case OP_If:
    case OP_Goto:
      // Destiny of the jump starts a new block
      if (!curr_instruction->jump_to->block) {
        curr_instruction->jump_to->block =
            create_block(curr_instruction->jump_to, true);
        curr_instruction->jump_to->block->first_instruction =
            curr_instruction->jump_to;
      }

      if (curr_instruction->next) {
        // Next node starts a new block
        if (!curr_instruction->next->block) {
          curr_instruction->next->block =
              create_block(curr_instruction->next, true);
          curr_instruction->next->block->first_instruction =
              curr_instruction->next;
        }
      }
      break;
    default:
      break;
    }

    curr_instruction = curr_instruction->next;
  }
}

/**
 * Update each instruction with the blocks they belong to, connections
 * between subsequent blocks, and blocks' last instructions.
 *
 * @param instruction_head: first instruction of a function
 */
void update_blocks(inode *instruction_head) {
  inode *curr_instruction = instruction_head;

  if (curr_instruction) {
    bnode *curr_block = curr_instruction->block;
    curr_instruction = curr_instruction->next;

    while (curr_instruction) {
      if(curr_instruction->op_type == OP_Global) {
        // Ignore Global variables declaration
        curr_instruction = curr_instruction->next;
        continue;
      }

      if (!curr_instruction->block) {
        curr_instruction->block = curr_block;
      } else {
        // This instruction is the leader of another block. Save this block
        // to propagate further.
        if (curr_instruction->previous &&
            curr_instruction->previous->jump_to != curr_instruction) {
          // We don'' connect if the previous instruction was a jump to the
          // current one as this was already handled by the previous
          // instruction.
          if(curr_instruction->previous->block) {
            connect_blocks(curr_instruction->previous->block,
                           curr_instruction->block);
          }
        }
        curr_block = curr_instruction->block;
      }

      // When we switch to a new block, this will have the last instruction
      // of the previous block.
      curr_block->last_instruction = curr_instruction;

      // Add the instruction this one jumps to as a child of the current
      // instruction's block
      if (curr_instruction->op_type == OP_If ||
          curr_instruction->op_type == OP_Goto) {
        connect_blocks(curr_block, curr_instruction->jump_to->block);
      }

      curr_instruction = curr_instruction->next;
    }
  }
}

void find_dominators() {
  int n = get_num_created_blocks();
  set universe_set = create_full_set(n);

  // Initialization
  blist_node *block_list_head = get_all_blocks();
  blist_node *block_list_node = block_list_head;
  while (block_list_node) {
    if (!block_list_node->block->parents) {
      set root_set = create_empty_set(n);
      add_to_set(block_list_node->block->id, root_set);
      block_list_node->block->dominators = root_set;
    } else {
      block_list_node->block->dominators = universe_set;
    }

    block_list_node = block_list_node->next;
  }

  // Update dominators until convergence
  bool converged = false;
  while (!converged) {
    block_list_node = block_list_head;
    converged = true;
    while (block_list_node && block_list_node->block->parents) {
      set block_set = create_empty_set(n);
      add_to_set(block_list_node->block->id, block_set);
      set new_set = unify_sets(
          block_set, get_dominators_from_predecessors(block_list_node->block));
      if (!are_set_equals(block_list_node->block->dominators, new_set)) {
        block_list_node->block->dominators = new_set;
        converged = false;
      }

      block_list_node = block_list_node->next;
    }
  }
}

/**
 * Finds the set comprised by the intersection of dominators of the
 * predecessors of a block.
 *
 * @param block: block
 *
 * @return Set formed by the intersection of the dominators from all the
 * predecessors of a block.
 */
set get_dominators_from_predecessors(bnode *block) {
  if (!block->parents) {
    // Empty set if no predecessors
    return create_empty_set(block->dominators.max_size);
  }

  blist_node *parent = block->parents;
  set set = parent->block->dominators;
  parent = parent->next;
  while (parent) {
    set = intersect_sets(set, parent->block->dominators);
    parent = parent->next;
  }

  return set;
}

void print_control_flow_graph(FILE* file) {
  blist_node *block_list_head = get_all_blocks();
  blist_node *block_list_node = block_list_head;

  while (block_list_node) {
    printf("Block %d [Leader: ", block_list_node->block->id);
    print_instruction(block_list_node->block->first_instruction, file);
    printf("] -> ");
    blist_node *block_list_child = block_list_node->block->children;
    while (block_list_child) {
      if (block_list_child->next) {
        printf("%d, ", block_list_child->block->id);
      } else {
        printf("%d\n", block_list_child->block->id);
      }
      block_list_child = block_list_child->next;
    }

    block_list_node = block_list_node->next;
  }
}

int get_total_instructions() { return total_instructions; }

int get_total_assignment_instructions() {
  return total_assignment_instructions;
}