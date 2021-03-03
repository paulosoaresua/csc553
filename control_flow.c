///*
// * Author: Paulo Soares.
// * CSC 553 (Spring 2021)
// */
//
//#include "control_flow.h"
//#include "symbol-table.h"
//
//typedef struct block_list {
//  iblock *block;
//  iblock *next;
//} blist;
//
//static blist *find_blocks(tnode *function_body_start_stnode);
//static void append_instruction_to_block(bnode *block, inode *instruction);
//
//void build_control_flow_graph(tnode *function_body_start_stnode) {
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
//static blist *find_blocks(tnode *function_body_start_stnode) {
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
//static void append_instruction_to_block(bnode *block, inode *instruction) {
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