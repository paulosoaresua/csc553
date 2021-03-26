#include "graph.h"

#include "global.h"


gnode_list_item *push_to_graph_node_stack(gnode *node,
                                          gnode_list_item *stack_top) {
  gnode_list_item *new_item = zalloc(sizeof(gnode_list_item *));
  new_item->node = NULL;
  new_item->prev = NULL;
  new_item->next = NULL;

  if (stack_top) {
    // Replaces current top with new item
    new_item->node = node;
    new_item->next = stack_top;
  }

  return new_item;
}

gnode_list_item *pop_from_graph_node_stack(gnode_list_item *stack_top) {
  gnode_list_item* new_top = NULL;
  if(stack_top) {
    new_top = stack_top->next;
    free(stack_top);
  }

  return new_top;
}