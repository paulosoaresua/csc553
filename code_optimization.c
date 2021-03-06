/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_optimization.h"
#include "reaching_definitions_analysis.h"
#include "liveness_analysis.h"

bool local_enabled = false;
bool global_enabled = false;

static void optimize_locally(tnode *function_body);
static void optimize_globally(tnode *function_body);
static void run_peephole_optimization(tnode *function_body);
static void remove_instruction(tnode *node, inode *instruction);

void enable_local_optimization() { local_enabled = true; }

void enable_global_optimization() { global_enabled = true; }

void optimize_instructions(tnode *function_body) {
  if (local_enabled || global_enabled) {
    fill_backward_connections(function_body->code_head);
    build_control_flow_graph(function_body->code_head);
    //    print_control_flow_graph();
    inode *curr_instruction = function_body->code_head;
    bnode *curr_block;
    while (curr_instruction) {
      if (curr_instruction->block != curr_block) {
        curr_block = curr_instruction->block;
        printf("\n-----------\n");
        printf(" Block %d    \n", curr_block->id);
        printf("-----------  \n");
      }
      printf("%d: ", curr_instruction->order);
      print_instruction(curr_instruction);
      if (redefines_variable(curr_instruction)) {
        printf(" [%d]", curr_instruction->definition_id);
      }
      printf("\n");
      curr_instruction = curr_instruction->next;
    }

//    find_in_and_out_def_sets(get_all_blocks());
    find_in_and_out_liveness_sets(get_all_blocks());
    optimize_locally(function_body);
    optimize_globally(function_body);
  }
}

static void optimize_locally(tnode *function_body) {
  if (local_enabled) {
    //    run_peephole_optimization(node);
  }
}

static void optimize_globally(tnode *function_body) {
  if (global_enabled) {
  }
}

static void run_peephole_optimization(tnode *function_body) {
  inode *curr_instruction = function_body->code_head;

  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Goto:
      if (curr_instruction->jump_to == curr_instruction->next) {
        remove_instruction(function_body, curr_instruction);
      }
      break;
    case OP_If:
      if (curr_instruction->next &&
          curr_instruction->next->op_type == OP_Goto &&
          curr_instruction->next->next &&
          curr_instruction->jump_to == curr_instruction->next->next) {

        invert_boolean_operator(curr_instruction);
        curr_instruction->jump_to = curr_instruction->next->jump_to;
        remove_instruction(function_body, curr_instruction->next);
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
          remove_instruction(function_body, curr_instruction);
        }
      }
      break;
    default:
      break;
    }

    curr_instruction = curr_instruction->next;
  }
}

static void remove_instruction(tnode *function_body, inode *instruction) {
  if (instruction) {
    if (instruction == function_body->code_head) {
      function_body->code_head = instruction->next;
    } else {
      instruction->previous->next = instruction->next;
      if (instruction->next) {
        instruction->next->previous = instruction->previous;
      }
    }

    // Adjust block boundaries
    if (instruction->block &&
        instruction->block->first_instruction == instruction) {
      instruction->block->first_instruction = instruction->next;
    }

    if (instruction->block &&
        instruction->block->last_instruction == instruction) {
      instruction->block->last_instruction = instruction->previous;
    }

    instruction->previous = NULL;
    instruction->next = NULL;
    free(instruction);
  }
}
