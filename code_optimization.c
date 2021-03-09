/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_optimization.h"
#include "liveness_analysis.h"

bool local_enabled = false;
bool global_enabled = false;
FILE *file_3addr;

static var_list_node *propagated_vars;

static void optimize_locally(inode *instruction_head);
static void run_peephole_optimization(inode *instruction_head);
static void do_copy_propagation();
static void optimize_globally(inode *instruction_head);
static void do_dead_code_elimination();
static bool remove_dead_instructions();
static void print_3addr_instructions(inode *instruction_head);
static void clear_propagated_vars();
static void attach_variable_to_original(symtabnode *var, symtabnode *original);
static void detach_variable_from_original(symtabnode *var);
static void detach_copies_from_original(symtabnode *original);

void enable_local_optimization() { local_enabled = true; }

void enable_global_optimization() { global_enabled = true; }

void print_blocks_and_instructions(FILE *file) { file_3addr = file; }

void optimize_instructions(tnode *function_body) {
  if (local_enabled || global_enabled) {
    fill_backward_connections(function_body->code_head);
    build_control_flow_graph(function_body->code_head);
    if (file_3addr) {
      fprintf(file_3addr, "\nBefore Optimization\n");
      print_3addr_instructions(function_body->code_head);
    }
    optimize_locally(function_body->code_head);
    optimize_globally(function_body->code_head);
    if (file_3addr) {
      fprintf(file_3addr, "\nAfter Optimization\n");
      print_3addr_instructions(function_body->code_head);
    }
  }
}

void optimize_locally(inode *instruction_head) {
  if (local_enabled) {
    run_peephole_optimization(instruction_head);
    do_copy_propagation();
  }
}

void run_peephole_optimization(inode *instruction_head) {
  inode *curr_instruction = instruction_head;

  while (curr_instruction) {
    switch (curr_instruction->op_type) {
    case OP_Goto:
      if (curr_instruction->jump_to == curr_instruction->next) {
        curr_instruction->dead = true;
      }
      break;
    case OP_If:
      if (curr_instruction->next &&
          curr_instruction->next->op_type == OP_Goto &&
          curr_instruction->next->next &&
          curr_instruction->jump_to == curr_instruction->next->next) {

        invert_boolean_operator(curr_instruction);
        curr_instruction->jump_to = curr_instruction->next->jump_to;
        curr_instruction->next->dead = true;
      }
      break;
    case OP_Assign_Int:
    case OP_Assign_Char:
      if (curr_instruction->next) {
        if (curr_instruction->next->op_type == OP_Assign &&
            curr_instruction->dest == SRC1(curr_instruction->next) &&
            curr_instruction->dest->is_temporary) {

          curr_instruction->next->op_type = OP_Assign_Int;
          curr_instruction->next->val.const_int =
              curr_instruction->val.const_int;
          curr_instruction->dead = true;
        }
      }
      break;
    case OP_Assign:
      if (curr_instruction->dest == SRC1(curr_instruction)) {
        curr_instruction->dead = true;
      }

      if (curr_instruction->next) {
        // Duplicate assignment instructions
        if (curr_instruction->dest == curr_instruction->next->dest &&
            SRC1(curr_instruction) == SRC1(curr_instruction->next)) {
          curr_instruction->dead = true;
        }
      }
      break;
    default:
      break;
    }

    curr_instruction = curr_instruction->next;
  }
}

void do_copy_propagation() {
  blist_node *block_list_node = get_all_blocks();
  while (block_list_node) {
    inode *curr_instruction = block_list_node->block->first_instruction;

    while (curr_instruction) {
      if (curr_instruction->dead) {
        curr_instruction = curr_instruction->next;
        continue;
      }

      if (curr_instruction->dest) {
        if (curr_instruction->dest->copied_from) {
          // The LHS variable was copied from another one. Remove the
          // dependency with the original variable so that copy is no longer
          // propagated since the variable is being redefined here.
          detach_variable_from_original(curr_instruction->dest);
        } else if (curr_instruction->dest->copied_to) {
          // This variable was copied to other variables. Remove all the
          // dependencies to this variable so it can no longer be
          // propagated by the variables that point to it.
          detach_copies_from_original(curr_instruction->dest);
        }
      }

      if (is_rhs_variable(curr_instruction)) {
        if (curr_instruction->op_type == OP_Assign) {
          // We make the LHS variable point to the RHS variable.
          if (SRC1(curr_instruction)->copied_from) {
            // LHS points to another variable (the original variable
            // propagated). We make LHS point to the original as well.
            attach_variable_to_original(curr_instruction->dest,
                                        SRC1(curr_instruction)->copied_from);
          } else {
            attach_variable_to_original(curr_instruction->dest,
                                        SRC1(curr_instruction));
          }
        }

        // After the copy pointers have been defined. We can replace the RHS
        // variables with the original propagated variables if they are
        // available.
        if (SRC1(curr_instruction)->copied_from) {
          SRC1(curr_instruction) = SRC1(curr_instruction)->copied_from;
        }

        if (SRC2(curr_instruction) && SRC1(curr_instruction)->copied_from) {
          SRC2(curr_instruction) = SRC2(curr_instruction)->copied_from;
        }
      }

      curr_instruction = curr_instruction->next;
    }

    clear_propagated_vars();
    block_list_node = block_list_node->next;
  }
}

void clear_propagated_vars() {
  while (propagated_vars) {
    detach_copies_from_original(propagated_vars->var);
  }
  clear_list_of_variables(propagated_vars);
  propagated_vars = NULL;
}

void attach_variable_to_original(symtabnode *var, symtabnode *original) {
  propagated_vars = add_to_list_of_variables(original, propagated_vars);
  original->copied_to = add_to_list_of_variables(var, original->copied_to);
  var->copied_from = original;
}

void detach_variable_from_original(symtabnode *var) {
  propagated_vars =
      remove_from_list_of_variables(var->copied_from, propagated_vars);
  var->copied_from->copied_to =
      remove_from_list_of_variables(var, var->copied_from->copied_to);
  var->copied_from = NULL;
}

void detach_copies_from_original(symtabnode *original) {
  propagated_vars = remove_from_list_of_variables(original, propagated_vars);
  var_list_node *list_head = original->copied_to;
  var_list_node *list_node = list_head;
  while (list_node) {
    list_node->var->copied_from = NULL;
    var_list_node *next = list_node->next;
    free(list_node);
    list_node = next;
  }
  original->copied_to = NULL;
}

void optimize_globally() {
  if (global_enabled) {
    do_dead_code_elimination();
  }
}

void do_dead_code_elimination() {
  bool dead_instructions_found = true;
  while (dead_instructions_found) {
    find_in_and_out_liveness_sets(get_all_blocks());
    dead_instructions_found = remove_dead_instructions();
  }
}

bool remove_dead_instructions() {
  blist_node *block_list_node = get_all_blocks();
  int n = get_total_assignment_instructions();
  bool dead_instructions_found = false;

  while (block_list_node) {
    set live_instructions = clone_set(block_list_node->block->out);

    inode *curr_instruction = block_list_node->block->last_instruction;
    while (curr_instruction &&
           curr_instruction->block == block_list_node->block) {
      if (curr_instruction->dead) {
        curr_instruction = curr_instruction->previous;
        continue;
      }

      set lhs_set = create_empty_set(n);
      set rhs_set = create_empty_set(n);

      if (curr_instruction->dest && curr_instruction->dest->scope == Local) {
        add_to_set(curr_instruction->dest->id, lhs_set);
      }

      if (is_rhs_variable(curr_instruction)) {
        if (SRC1(curr_instruction) && SRC1(curr_instruction)->scope == Local) {
          add_to_set(SRC1(curr_instruction)->id, rhs_set);
        }

        if (SRC2(curr_instruction) && SRC2(curr_instruction)->scope == Local) {
          add_to_set(SRC2(curr_instruction)->id, rhs_set);
        }
      }

      if (redefines_variable(curr_instruction) &&
          curr_instruction->dest->scope == Local) {
        if (!does_elto_belong_to_set(curr_instruction->dest->id,
                                     live_instructions)) {
          curr_instruction->dead = true;
          dead_instructions_found = true;
        }
      }

      live_instructions = diff_sets(live_instructions, lhs_set);
      live_instructions = unify_sets(live_instructions, rhs_set);

      curr_instruction = curr_instruction->previous;
    }

    block_list_node = block_list_node->next;
  }

  return dead_instructions_found;
}

static void print_3addr_instructions(inode *instruction_head) {
  inode *curr_instruction = instruction_head;
  bnode *curr_block;
  while (curr_instruction) {
    if (curr_instruction->dead || curr_instruction->op_type == OP_Global) {
      curr_instruction = curr_instruction->next;
      continue;
    }

    if (curr_instruction->block != curr_block) {
      curr_block = curr_instruction->block;
      fprintf(file_3addr, "\n-----------\n");
      fprintf(file_3addr, " Block %d    \n", curr_block->id);
      fprintf(file_3addr, "-----------  \n");
    }
    fprintf(file_3addr, "%d: ", curr_instruction->order);
    print_instruction(curr_instruction, file_3addr);
    if (redefines_variable(curr_instruction)) {
      fprintf(file_3addr, " [%d]", curr_instruction->definition_id);
    }
    fprintf(file_3addr, "\n");
    curr_instruction = curr_instruction->next;
  }
}
