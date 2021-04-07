/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#include "code_optimization.h"
#include "liveness_analysis.h"
#include "min_heap.h"

static bool local_enabled = false;
static bool global_enabled = false;
static bool register_allocation_enabled = false;
FILE *file_3addr;

static int NUM_REGISTERS = 5; // $t2 - $t9 + $s0 - $s7. The first 2 $ts are
                              // reserved for temporary operations and arrays.

static var_list_node *propagated_vars;

// Fast access of a variable via its id.
// It's filled when the interference graph is created.
static symtabnode **local_variables;

static void optimize_locally(inode *instruction_head);
static void run_peephole_optimization(inode *instruction_head);
static void do_copy_propagation();
static void optimize_globally();
static void do_dead_code_elimination();
static bool remove_dead_instructions();
static void print_3addr_instructions(inode *instruction_head);
static void clear_propagated_vars();
static void attach_variable_to_original(symtabnode *var, symtabnode *original);
static void detach_variable_from_original(symtabnode *var);
static void detach_copies_from_original(symtabnode *original);
static void optimize_register_allocation(symtabnode *function_header);
static gnode_list_item *create_interference_graph();
static void create_interference_graph_connections();
static void color_graph(gnode_list_item *graph, symtabnode *function_header);

void enable_local_optimization() { local_enabled = true; }

void enable_global_optimization() { global_enabled = true; }

void enable_register_allocation_optimization() {
  register_allocation_enabled = true;
}

void print_blocks_and_instructions(FILE *file) { file_3addr = file; }

void optimize_instructions(symtabnode *function_header, tnode *function_body) {
  if (local_enabled || global_enabled || register_allocation_enabled) {
    fill_backward_connections(function_body->code_head);
    build_control_flow_graph(function_body->code_head);
    if (file_3addr) {
      print_control_flow_graph(file_3addr);
      fprintf(file_3addr, "\n\nBefore Optimization\n");
      print_3addr_instructions(function_body->code_head);
    }
    optimize_locally(function_body->code_head);
    optimize_globally();
    optimize_register_allocation(function_header);
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

      if (curr_instruction->previous &&
          curr_instruction->dest->type != t_Addr) {
        // Bypass temporary assignment
        if (SRC1(curr_instruction) == curr_instruction->previous->dest &&
            SRC1(curr_instruction)->is_temporary) {

          curr_instruction->previous->dest = curr_instruction->dest;
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

    while (curr_instruction &&
           curr_instruction->block == block_list_node->block) {
      if (curr_instruction->dead) {
        curr_instruction = curr_instruction->next;
        continue;
      }

      if (curr_instruction->op_type == OP_Assign) {
        symtabnode *root_dest = curr_instruction->dest->copied_from
                                    ? curr_instruction->dest->copied_from
                                    : curr_instruction->dest;
        symtabnode *root_src = SRC1(curr_instruction)->copied_from
                                   ? SRC1(curr_instruction)->copied_from
                                   : SRC1(curr_instruction);
        if (root_dest == root_src) {
          curr_instruction->dead = true;
          curr_instruction = curr_instruction->next;
          continue;
        }
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
        if (curr_instruction->op_type == OP_Assign &&
            curr_instruction->dest->type == SRC1(curr_instruction)->type) {
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

        if (SRC2(curr_instruction) && SRC2(curr_instruction)->copied_from) {
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
  bool any_change = true;
  while (any_change) {
    find_in_and_out_liveness_sets(get_all_blocks());
    any_change = remove_dead_instructions();
  }
}

bool remove_dead_instructions() {
  blist_node *block_list_node = get_all_blocks();
  int n = get_total_local_variables();
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

      if (curr_instruction->op_type == OP_Assign &&
          curr_instruction->dest == SRC1(curr_instruction)) {
        // Ignore null assignments
        curr_instruction->dead = true;
        curr_instruction = curr_instruction->previous;
        continue;
      }

      set lhs_set = create_empty_set(n);
      set rhs_set = create_empty_set(n);

      if (curr_instruction->dest && curr_instruction->dest->scope == Local) {
        if (curr_instruction->dest->type == t_Addr) {
          // In this scenario, we consider that the LHS variable is being used
          // as it contains the address of the variable that it's effectively
          // going to change.
          add_to_set(curr_instruction->dest->id, rhs_set);
        } else {
          add_to_set(curr_instruction->dest->id, lhs_set);
        }
      }

      if (is_rhs_variable(curr_instruction)) {
        if (SRC1(curr_instruction) && SRC1(curr_instruction)->scope == Local &&
            !SRC1(curr_instruction)->is_constant) {
          add_to_set(SRC1(curr_instruction)->id, rhs_set);
        }

        if (SRC2(curr_instruction) && SRC2(curr_instruction)->scope == Local &&
            !SRC2(curr_instruction)->is_constant) {
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

void print_3addr_instructions(inode *instruction_head) {
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

void optimize_register_allocation(symtabnode *function_header) {
  if (register_allocation_enabled) {
    gnode_list_item *graph = create_interference_graph();
    find_in_and_out_liveness_sets(get_all_blocks());
    create_interference_graph_connections();
    if (file_3addr) {
      fprintf(file_3addr, "\nInterference Graph:\n\n");
      fprintf(file_3addr, "\nVariable IDs:\n");
      for (int i = 0; i < get_total_local_variables(); i++) {
        fprintf(file_3addr, "%s: %d\n", local_variables[i]->name,
                (int)local_variables[i]->id);
      }
      fprintf(file_3addr, "\nAdjacency List:\n");
      print_graph(graph, file_3addr);
    }
    color_graph(graph, function_header);
  }
}

gnode_list_item *create_interference_graph() {
  gnode_list_item *graph = NULL;
  symtabnode **entries = get_symbol_table_entries(Local);
  int table_size = get_symbol_table_size();
  int n = get_total_local_variables();

  // Println is hardcoded, therefore we know that it does not use any os the
  // reserved registers we use here.
  symtabnode* println_function = SymTabLookup("println", Global);
  println_function->entered = true;
  println_function->registers_used = create_empty_set(NUM_REGISTERS);

  local_variables = zalloc(n * sizeof(symtabnode *));

  for (int i = 0; i < table_size; i++) {
    symtabnode *var = entries[i];
    while (var) {
      if (var->type != t_Array && var->type != t_Addr) {
        // This optimization is not carried out for arrays
        var->live_range_node = create_graph_node(var->id, n - 1);
        var->live_range_node->cost = var->cost;
        var->live_range_node->regs_to_avoid = create_empty_set(NUM_REGISTERS);
        var->live_range_node->preferential_regs =
            create_full_set(NUM_REGISTERS);
        graph = add_node_to_graph(var->live_range_node, graph);
      }
      local_variables[var->id] = var;
      var = var->next;
    }
  }

  return graph;
}

void create_interference_graph_connections(symtabnode *function_header) {
  blist_node *block_list_node = get_all_blocks();
  int n = get_total_local_variables();

  while (block_list_node) {
    set live_now = clone_set(block_list_node->block->out);

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
        if (curr_instruction->dest->type != t_Addr) {
          // This optimization is not carried out for arrays
          add_to_set(curr_instruction->dest->id, lhs_set);
        }
      }

      if (is_rhs_variable(curr_instruction)) {
        if (SRC1(curr_instruction) && SRC1(curr_instruction)->scope == Local &&
            !SRC1(curr_instruction)->is_constant &&
            SRC1(curr_instruction)->type != t_Addr) {
          add_to_set(SRC1(curr_instruction)->id, rhs_set);
        }

        if (SRC2(curr_instruction) && SRC2(curr_instruction)->scope == Local &&
            !SRC2(curr_instruction)->is_constant &&
            SRC2(curr_instruction)->type != t_Addr) {
          add_to_set(SRC2(curr_instruction)->id, rhs_set);
        }
      }

      bool is_call_to_pre_parsed_function =
          curr_instruction->op_type == OP_Call &&
          SRC1(curr_instruction)->entered;
      if ((!is_set_empty(lhs_set) || is_call_to_pre_parsed_function) &&
          !is_set_empty(live_now)) {

        if (is_call_to_pre_parsed_function) {
          // The registers used in the current function must also account for
          // the registers used in the functions called by it
          function_header->registers_used =
              unify_sets(function_header->registers_used,
                         SRC1(curr_instruction)->registers_used);
        }

        // Link the live_range node of the variable being assigned to to
        // all the variables in the current live set.
        set tmp_set = clone_set(live_now);
        for (int i = 0; i < n; i++) {
          if (does_elto_belong_to_set(i, tmp_set)) {
            symtabnode *var = get_variable_by_id(i);

            if (is_call_to_pre_parsed_function) {
              // Remove from the preferential registers set of a variable,
              // the registers used inside the function being called.
              var->live_range_node->preferential_regs =
                  diff_sets(var->live_range_node->preferential_regs,
                            SRC1(curr_instruction)->registers_used);
            } else {
              if (curr_instruction->dest != var && var->live_range_node) {
                // No self-loops or multiple edges between the same nodes
                add_edge(curr_instruction->dest->live_range_node,
                         var->live_range_node);
              }
            }
            remove_from_set(i, tmp_set);
          }

          if (is_set_empty(tmp_set))
            break;
        }
      }

      if (curr_instruction->op_type == OP_Call) {
        // Here we store in the call instruction the set of variables live after
        // the function call. We will use this information to know which
        // registers need to be saved and loaded back by the caller before and
        // after this function call.
        curr_instruction->live_at_call = clone_set(live_now);
      }

      live_now = diff_sets(live_now, lhs_set);
      live_now = unify_sets(live_now, rhs_set);

      curr_instruction = curr_instruction->previous;
    }

    block_list_node = block_list_node->next;
  }
}

symtabnode *get_variable_by_id(int id) { return local_variables[id]; }

void color_graph(gnode_list_item *graph, symtabnode *function_header) {
  if (NUM_REGISTERS <= 0) {
    return;
  }

  function_header->registers_used = create_empty_set(NUM_REGISTERS);

  // Construct max heap to find variable to spill more efficiently;
  min_heap *heap = create_empty_heap(get_total_local_variables());
  gnode_list_item *list_item = graph;
  while (list_item) {
    add_to_heap(list_item, heap);
    list_item = list_item->next;
  }
  build_heap(heap);

  while (graph) {
    bool any_colored = true;
    while (any_colored) {
      any_colored = false;
      gnode_list_item *list_item = graph;
      while (list_item) {
        gnode_list_item *next = list_item->next;

        if (list_item->node->num_neighbors < NUM_REGISTERS) {
          set available_regs = create_full_set(NUM_REGISTERS);
          available_regs =
              diff_sets(available_regs, list_item->node->regs_to_avoid);
          if (!is_set_empty(list_item->node->preferential_regs)) {
            // Choose among a preferential set
            set tmp = intersect_sets(available_regs,
                                     list_item->node->preferential_regs);
            if (!is_set_empty(tmp)) {
              // Only if the intersection is not empty, we can to choose a
              // register in the preferential list
              available_regs = tmp;
            }
          }
          for (int reg = 0; reg < NUM_REGISTERS; reg++) {
            if (does_elto_belong_to_set(reg, available_regs)) {
              list_item->node->reg = reg;
              break;
            }
          }

          // Add used register to the function being processed
          add_to_set(list_item->node->reg, function_header->registers_used);

          // Avoid using the same register in the neighbors of the current node.
          gnode_list_item *neighbor = list_item->node->neighbors;
          while (neighbor) {
            add_to_set(list_item->node->reg, neighbor->node->regs_to_avoid);
            neighbor = neighbor->next;
          }

          graph = remove_node_from_graph(list_item, graph);
          any_colored = true;
        }

        list_item = next;
      }
    }

    if (graph) {
      // Spill the node with highest cost
      gnode_list_item *node_to_spill = extract_heap_root(heap);
      graph = remove_node_from_graph(node_to_spill, graph);
    }
  }
}