/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "reaching_definitions_analysis.h"

static void find_gen_and_kill_sets(blist_node *block_list_head);
static void fill_definitions(blist_node *block_list_head);
//static void clear_definitions_within_block(bnode *block);
static set get_out_set_from_predecessors(bnode *block);
static void clear_static_block_sets(blist_node *block_list_head);

void find_in_and_out_def_sets(blist_node *block_list_head) {
  find_gen_and_kill_sets(block_list_head);

  bool converged = false;
  while (!converged) {
    converged = true;

    blist_node *block_list_node = block_list_head;
    while (block_list_node) {
      set in = get_out_set_from_predecessors(block_list_node->block);
      set diff = diff_sets(in, block_list_node->block->kill);
      set out = unify_sets(block_list_node->block->gen, diff);

      if (!are_set_equals(block_list_node->block->in, in)) {
        block_list_node->block->in = in;
        converged = false;
      }
      if (!are_set_equals(block_list_node->block->out, out)) {
        block_list_node->block->out = out;
        converged = false;
      }
      block_list_node = block_list_node->next;
    }
  }

  // No need to retain gen, kill and definitions after in and out were computed.
  clear_static_block_sets(block_list_head);
}

/**
 * For each block, computes its gen and kill definition sets.
 *
 * @param root_block: first block of a function.
 */
void find_gen_and_kill_sets(blist_node *block_list_head) {
  fill_definitions(block_list_head);
  blist_node *block_list_node = block_list_head;
  int n = get_total_assignment_instructions();

  while (block_list_node) {
    set gen = create_empty_set(n);
    set kill = create_empty_set(n);

    inode *curr_instruction = block_list_node->block->first_instruction;
    while (curr_instruction &&
           curr_instruction->block == block_list_node->block) {
      if (redefines_variable(curr_instruction)) {
        gen = diff_sets(gen, curr_instruction->dest->definitions);
        add_to_set(curr_instruction->definition_id, gen);
        kill = unify_sets(kill, curr_instruction->dest->definitions);
        remove_from_set(curr_instruction->definition_id, kill);
      }
      curr_instruction = curr_instruction->next;
    }

    //    clear_definitions_within_block(block_list_node->block);
    block_list_node->block->gen = gen;
    block_list_node->block->out = gen;
    block_list_node->block->kill = kill;
    block_list_node = block_list_node->next;
  }
}

/**
 * Fills the set of definitions among all instructions for each assigned
 * variable.
 *
 * @param block: block
 */
void fill_definitions(blist_node *block_list_head) {
  blist_node *block_list_node = block_list_head;

  while (block_list_node) {
    inode *curr_instruction = block_list_node->block->first_instruction;
    while (curr_instruction && curr_instruction->block == block_list_node->block) {
      if (redefines_variable(curr_instruction)) {
        if (is_set_undefined(curr_instruction->dest->definitions)) {
          curr_instruction->dest->definitions =
              create_empty_set(get_total_assignment_instructions());
        }
        add_to_set(curr_instruction->definition_id,
                   curr_instruction->dest->definitions);
      }
      curr_instruction = curr_instruction->next;
    }

    block_list_node = block_list_node->next;
  }
}

///**
// * Erases the set of definitions for each assigned variable within a block.
// *
// * @param block: block
// */
//void clear_definitions_within_block(bnode *block) {
//  inode *curr_instruction = block->first_instruction;
//  while (curr_instruction && curr_instruction->block == block) {
//    if (curr_instruction->op_type == OP_Assign_Int ||
//        curr_instruction->op_type == OP_Assign_Char ||
//        curr_instruction->op_type == OP_Assign) {
//
//      set null_set;
//      curr_instruction->dest->definitions = null_set;
//    }
//    curr_instruction = curr_instruction->next;
//  }
//}

/**
 * Gets the union of all reaching variables in the end of the predecessors of
 * a block.
 *
 * @param block: block
 *
 * @return: Union of predecessors' out set.
 */
static set get_out_set_from_predecessors(bnode *block) {
  if (!block->parents) {
    // Empty set if no predecessors
    return create_empty_set(block->gen.max_size);
  }

  blist_node *parent = block->parents;
  set set = parent->block->out;
  parent = parent->next;
  while (parent) {
    set = unify_sets(set, parent->block->out);
    parent = parent->next;
  }

  return set;
}

/**
 * For each block, erases its gen and kill sets.
 *
 * @param root_block: first block of a function.
 */
void clear_static_block_sets(blist_node *block_list_head) {
  blist_node *block_list_node = block_list_head;
  set null_set;

  while (block_list_node) {
    block_list_node->block->gen = null_set;
    block_list_node->block->kill = null_set;
    block_list_node = block_list_node->next;
  }
}