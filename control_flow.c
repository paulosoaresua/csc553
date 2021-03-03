/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "control_flow.h"

static void find_block_leaders(tnode *function_body);
static void update_blocks(tnode *function_body);

void build_control_flow_graph(tnode *function_body) {
  find_block_leaders(function_body);
  update_blocks(function_body);
}

/**
 * Find block leaders and associate new blocks to them.
 *
 * @param function_body: first node of a function body
 */
static void find_block_leaders(tnode *function_body) {
  inode *curr_instruction = function_body->code_head;
  bnode *curr_block;

  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Enter:
      curr_block = create_block(curr_instruction, false);
      curr_instruction->block = curr_block;

    case OP_Call:
      curr_instruction->block = create_block(curr_instruction, true);
      add_child_block(curr_instruction->block, curr_block);
      curr_block = curr_instruction->block;
      break;
    case OP_If:
    case OP_Goto:
      // Destiny of the jump starts a new block
      curr_instruction->jump_to->block =
          create_block(curr_instruction->jump_to, true);
      add_child_block(curr_instruction->jump_to->block, curr_block);

      if (curr_instruction->next) {
        // Next node starts a new block
        curr_instruction->next->block =
            create_block(curr_instruction->next, true);
        add_child_block(curr_instruction->next->block, curr_block);
        curr_block = curr_instruction->next->block;
        curr_instruction = curr_instruction->next;
      }
      break;
    default:
      break;
    }

    curr_instruction = curr_instruction->next;
  }
}

/**
 * Update each instruction with the blocks they belong to, and for each block
 * update its last instruction.
 *
 * @param function_body: first node of a function body
 */
void update_blocks(tnode *function_body) {
  inode *curr_instruction = function_body->code_head;

  if (curr_instruction) {
    bnode *curr_block = curr_instruction->block;
    curr_instruction = curr_instruction->next;

    while (curr_instruction) {
      if (!curr_instruction->block) {
        curr_instruction->block = curr_block;
        // When we switch to a new block, this will have the last instruction
        // of this block.
        curr_block->last_instruction = curr_instruction;
      } else {
        // This instruction is the leader of another block. Save this block
        // to propagate further.
        curr_block = curr_instruction->block;
      }

      curr_instruction = curr_instruction->next;
    }
  }
}

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