/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_GRAPH_H
#define CSC553_GRAPH_H

#import "global.h"
#import "set.h"

typedef struct GraphNode gnode;

// Double linked list of graph nodes
typedef struct NodeListItem {
  gnode *node;

  struct NodeListItem *next;
  struct NodeListItem *prev;
} gnode_list_item;

typedef struct GraphNode {
  int id;
  int reg; // register where the live range is allocated (-1 if in memory)
  set regs_to_avoid;
  int num_neighbors;
  gnode_list_item *neighbors;
  set neighbor_set;
} gnode;

/**
 * Creates a graph node
 *
 * @param var: live range variable
 * @param live_range: set of instructions part of the live range of the
 * variable
 * @param max_neighbors: maximum number of neighbors the node can have
 *
 * @return New graph node.
 */
gnode *create_graph_node(int id, int max_neighbors);

/**
 * Adds a node to a graph
 *
 * @param node: node
 * @param graph_head: head node in the list of nodes from a graph
 *
 * @return new head of the list of nodes from a graph
 */
gnode_list_item *add_node_to_graph(gnode *node, gnode_list_item *graph_head);

/**
 * Adds an edge between two nodes
 *
 * @param node1: node1
 * @param node2: node2
 */
void add_edge(gnode *node1, gnode *node2);

/**
 * Remove a node from the list of nodes in a graph and from the list of
 * neighbors of its neighbors list
 *
 * @param graph_item: node item from the list of nodes in a graph
 * @param graph_head: head node in the list of nodes from a graph
 *
 * @return new head of the list of nodes from a graph
 */
gnode_list_item *remove_node_from_graph(gnode_list_item *graph_item,
                                        gnode_list_item *graph_head);

/**
 * Print the the graph's adjacency list
 *
 * @param graph_head: head in the list of nodes from a graph
 */
void print_graph(gnode_list_item *graph_head, FILE *file);

#endif
