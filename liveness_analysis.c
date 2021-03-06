/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "liveness_analysis.h"

static void find_def_and_use_sets(blist_node *block_list_head);
static set get_in_set_from_sucessors(bnode *block);
static void clear_def_and_use_sets(blist_node *block_list_head);

void find_in_and_out_liveness_sets(blist_node *block_list_head) {
  find_def_and_use_sets(block_list_head);

  bool converged = false;
  while (!converged) {
    converged = true;

    blist_node *block_list_node = block_list_head;
    while (block_list_node) {
      set out = get_in_set_from_sucessors(block_list_node->block);
      set diff = diff_sets(out, block_list_node->block->def);
      set in = unify_sets(block_list_node->block->use, diff);

      if (!are_set_equals(block_list_node->block->out, out)) {
        block_list_node->block->out = out;
        converged = false;
      }
      if (!are_set_equals(block_list_node->block->in, in)) {
        block_list_node->block->in = in;
        converged = false;
      }
      block_list_node = block_list_node->next;
    }
  }

  // No need to retain gen and kill after in and out were computed.
  //  clear_gen_and_kill_sets(block_list_head);
}

/**
 * For each block, computes its gen and kill definition sets.
 *
 * @param root_block: first block of a function.
 */
void find_def_and_use_sets(blist_node *block_list_head) {
  blist_node *block_list_node = block_list_head;
  // Global variables are always live
  int n = get_total_local_variables();

  while (block_list_node) {
    set def = create_empty_set(n);
    set use = create_empty_set(n);

    inode *curr_instruction = block_list_node->block->last_instruction;
    while (curr_instruction &&
           curr_instruction->block == block_list_node->block) {

      set lhs_set = create_empty_set(n);
      set rhs_set = create_empty_set(n);

      if (curr_instruction->dest) {
        add_to_set(curr_instruction->dest->id, lhs_set);
      }

      if (curr_instruction->op_type != OP_Assign_Int &&
          curr_instruction->op_type != OP_Assign_Char &&
          curr_instruction->op_type != OP_Call &&
          curr_instruction->op_type != OP_Enter) {
        if (SRC1(curr_instruction)) {
          add_to_set(SRC1(curr_instruction)->id, rhs_set);
        }

        if (SRC2(curr_instruction)) {
          add_to_set(SRC2(curr_instruction)->id, rhs_set);
        }
      }

      def = unify_sets(lhs_set, def);
      def = diff_sets(def, rhs_set);
      use = diff_sets(use, lhs_set);
      use = unify_sets(use, rhs_set);

      curr_instruction = curr_instruction->previous;
    }

    block_list_node->block->def = def;
    block_list_node->block->in = use;
    block_list_node->block->use = use;
    block_list_node = block_list_node->next;
  }
}

/**
 * Gets the union of all live variables in the beginning of the successors of
 * a block.
 *
 * @param block: block
 *
 * @return: Union of successors' in set.
 */
static set get_in_set_from_sucessors(bnode *block) {
  if (!block->children) {
    // Empty set if no predecessors
    return create_empty_set(block->def.max_size);
  }

  blist_node *child = block->children;
  set set = child->block->in;
  child = child->next;
  while (child) {
    set = unify_sets(set, child->block->in);
    child = child->next;
  }

  return set;
}

/**
 * For each block, erases its def and use sets.
 *
 * @param root_block: first block of a function.
 */
void clear_def_and_use_sets(blist_node *block_list_head) {
  blist_node *block_list_node = block_list_head;
  set null_set;

  while (block_list_node) {
    block_list_node->block->def = null_set;
    block_list_node->block->use = null_set;
    block_list_node = block_list_node->next;
  }
}