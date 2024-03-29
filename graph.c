#include "graph.h"

gnode_list_item *create_graph() {
  gnode_list_item *graph_head = zalloc(sizeof(gnode_list_item *));
  graph_head->node = NULL;
  graph_head->prev = NULL;
  graph_head->next = NULL;

  return graph_head;
}

gnode *create_graph_node(int id, int max_neighbors) {
  gnode *node = zalloc(sizeof(gnode));
  node->id = id;
  node->reg = -1;
  node->neighbors = NULL;
  node->num_neighbors = 0;
  node->neighbor_set = create_empty_set(max_neighbors);

  return node;
}

gnode_list_item *add_node_to_graph(gnode *node, gnode_list_item *graph_head) {
  gnode_list_item *new_item = zalloc(sizeof(gnode_list_item));
  new_item->node = node;
  new_item->prev = NULL;
  new_item->next = NULL;

  if (graph_head) {
    // Add a new item in the beginning of the list of nodes from a graph
    new_item->next = graph_head;
    graph_head->prev = new_item;
  }

  return new_item;
}

void add_edge(gnode *node1, gnode *node2) {
  if (does_elto_belong_to_set(node1->id, node2->neighbor_set)) {
    // No multiple edges between the same nodes.
    return;
  }

  // Add node2 to the list of neighbors of node1
  gnode_list_item *new_item = zalloc(sizeof(gnode_list_item));
  new_item->node = node2;
  if (node1->neighbors) {
    new_item->next = node1->neighbors;
    node1->neighbors->prev = new_item;
  }
  node1->neighbors = new_item;
  node1->num_neighbors++;
  add_to_set(node2->id, node1->neighbor_set);

  // Add node1 to the list of neighbors of node2
  new_item = zalloc(sizeof(gnode_list_item));
  new_item->node = node1;
  if (node2->neighbors) {
    new_item->next = node2->neighbors;
    node2->neighbors->prev = new_item;
  }
  node2->neighbors = new_item;
  node2->num_neighbors++;
  add_to_set(node1->id, node2->neighbor_set);
}

gnode_list_item *remove_node_from_graph(gnode_list_item *graph_item,
                                        gnode_list_item *graph_head) {

  if (!graph_head || !graph_item) {
    return graph_head;
  }

  // Remove edges
  gnode_list_item *neighbor = graph_item->node->neighbors;
  while (neighbor) {
    neighbor->node->num_neighbors--;

    if (neighbor->node->neighbors->node == graph_item->node) {
      // Node is the first in the list
      gnode_list_item *tmp = neighbor->node->neighbors;
      neighbor->node->neighbors = neighbor->node->neighbors->next;
      tmp->next = NULL;
      free(tmp);
    } else {
      // Find the node in the list of neighbors of its neighbor
      gnode_list_item *tmp = neighbor->node->neighbors;
      while (tmp->node != graph_item->node) {
        // TODO - O(n) search. Change strategy if this becomes a bottleneck.
        tmp = tmp->next;
      }

      tmp->prev->next = tmp->next;
      if (tmp->next) {
        tmp->next->prev = tmp->prev;
      }

      // Free memory
      tmp->node = NULL;
      tmp->prev = NULL;
      tmp->next = NULL;
      // Do not free the node itself because it carry instructions about
      // location where the variable must be allocated (register or memory).
      // Just remove it from the graph.
      free(tmp);
    }

    neighbor = neighbor->next;
  }

  // Remove node from the list of nodes in the graph
  graph_item->node->neighbors = NULL;
  graph_item->node = NULL;
  gnode_list_item *new_head = graph_head;
  if (graph_item == graph_head) {
    new_head = graph_item->next;
    graph_item->next = NULL;
  } else {
    graph_item->prev->next = graph_item->next;
    if (graph_item->next) {
      graph_item->next->prev = graph_item->prev;
    }
  }
  free(graph_item);

  if (new_head) {
    new_head->prev = NULL;
  }

  return new_head;
}

void print_graph(gnode_list_item *graph_head, FILE *file) {
  fprintf(file, "GRAPH \n");
  fprintf(file, "----- \n");
  gnode_list_item *item = graph_head;
  while (item) {
    fprintf(file, "%d: ", item->node->id);
    gnode_list_item *neighbor = item->node->neighbors;
    while (neighbor) {
      if (neighbor->next) {
        fprintf(file, "%d, ", neighbor->node->id);
      } else {
        fprintf(file, "%d", neighbor->node->id);
      }
      neighbor = neighbor->next;
    }
    fprintf(file, "\n");
    item = item->next;
  }
}
