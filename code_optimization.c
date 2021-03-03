/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_optimization.h"
#include "instruction.h"

bool local_enabled = false;
bool global_enabled = false;

static void optimize_locally(tnode *node);
static void optimize_globally(tnode *node);
static void run_peephole_optimization(tnode *node);
static void remove_instruction(tnode *node, inode *instruction);

void enable_local_optimization() { local_enabled = true; }

void enable_global_optimization() { global_enabled = true; }

void optimize_instructions(tnode *node) {
  if (local_enabled || global_enabled) {
    fill_backward_connections(node->code_head);
    optimize_locally(node);
    optimize_globally(node);
  }
}

static void optimize_locally(tnode *node) {
  if (local_enabled) {
    run_peephole_optimization(node);
    run_peephole_optimization(node);
  }
}

static void optimize_globally(tnode *node) {
  if (global_enabled) {
  }
}

static void run_peephole_optimization(tnode *node) {
  inode *curr_instruction = node->code_head;

  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Goto:
      if (curr_instruction->jump_to == curr_instruction->next) {
        remove_instruction(node, curr_instruction);
      }
      break;
    case OP_If:
      if (curr_instruction->next &&
          curr_instruction->next->op_type == OP_Goto &&
          curr_instruction->next->next &&
          curr_instruction->jump_to == curr_instruction->next->next) {

        invert_boolean_operator(curr_instruction);
        curr_instruction->jump_to = curr_instruction->next->jump_to;
        remove_instruction(node, curr_instruction->next);
      }
      break;
    case OP_Assign_Int:
    case OP_Assign_Char:
      if (curr_instruction->next) {
        if (curr_instruction->next->op_type == OP_Assign &&
            curr_instruction->dest == SRC1(curr_instruction->next)) {

          curr_instruction->next->op_type = OP_Assign_Int;
          curr_instruction->next->val.const_int =
              curr_instruction->val.const_int;
          remove_instruction(node, curr_instruction);
        }
      }
      break;
    default:
      break;
    }

    curr_instruction = curr_instruction->next;
  }
}

static void remove_instruction(tnode *node, inode *instruction) {
  if (instruction) {
    if (instruction == node->code_head) {
      node->code_head = instruction->next;
    } else {
      instruction->previous->next = instruction->next;
      if (instruction->next) {
        instruction->next->previous = instruction->previous;
      }
    }

    instruction->previous = NULL;
    instruction->next = NULL;
    free(instruction);
  }
}

//
//#include "control_flow.h"
//#include "symbol-table.h"
//
// typedef struct block_list {
//  iblock *block;
//  iblock *next;
//} blist;
//
// static blist *find_blocks(tnode *function_body_start_stnode);
// static void append_instruction_to_block(bnode *block, inode *instruction);
//
// void build_control_flow_graph(tnode *function_body_start_stnode) {
//  blist *blist_node = find_blocks(function_body_start_stnode);
//
//  while (blist_node) {
//    print_instruction(blist_node->block->instruction);
//    blist_node = blist_node->next;
//  }
//
//  //  inode *curr_instruction = function_body_start_stnode->code_head;
//  //
//  //  while (curr_instruction) {
//  //
//  //    curr_instruction = curr_instruction->next;
//  //  }
//}
//
// static blist *find_blocks(tnode *function_body_start_stnode) {
//  blist *blist_header;
//
//  inode *curr_instruction = function_body_start_stnode->code_head;
//  if (curr_instruction) {
//    // The first instruction of a function is always a leader
//    blist_header = zalloc(sizeof(blist));
//    blist *curr_blist_node = blist_header;
//
//    blist *curr_block = zalloc(sizeof(bnode));
//    while (curr_instruction) {
//      if (curr_instruction->op_type == OP_Label) {
//        // Instruction starts a new block
//        curr_blist_node =
//            append_block_to_block_list(curr_block, curr_blist_node);
//        curr_block->next = zalloc(sizeof(bnode));
//        curr_block = curr_block->next;
//      } else if (curr_instruction->op_type == OP_If ||
//                 curr_instruction->op_type == OP_Goto) {
//        // Append current instruction to the current block and move to the
//        // next instruction that must be part of a new block
//        append_instruction_to_block(curr_instruction, curr_block);
//
//        curr_instruction = curr_instruction->next;
//        if (curr_instruction) {
//          // Next instruction starts a new block
//          curr_block->next = zalloc(sizeof(bnode));
//          curr_block = curr_block->next;
//        } else {
//          break;
//        }
//      }
//
//      append_instruction_to_block(curr_instruction, curr_block);
//
//      curr_instruction = curr_instruction->next;
//    }
//  }
//
//  return header;
//}
//
// static void append_instruction_to_block(bnode *block, inode *instruction) {
//  if (!block->instructions_head) {
//    block->instructions_head = zalloc(sizeof(iblock));
//    block->instructions_tail = block->instructions_head;
//  }
//
//  if (block->instructions_tail->instruction) {
//    block->instructions_tail->next = zalloc(sizeof(iblock));
//    block->instructions_tail->next->instruction = instruction;
//    block->instructions_tail = block->instructions_tail->next;
//  } else {
//    block->instructions_tail->instruction = instruction;
//  }
//}