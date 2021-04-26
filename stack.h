/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_STACK_H
#define CSC553_STACK_H

#import "graph.h"

/**
 * Adds a node to the stack
 *
 * @param node: graph node
 * @param stack_top: top of stack
 *
 * @return New top of the stack after the addition of the new item
 */
gnode_list_item *push_to_graph_node_stack(gnode *node,
                                          gnode_list_item *stack_top);

/**
 * Removes the top of the stack and returns the new top.
 *
 * @param stack_top: top of the stack
 *
 * @return Top of the stack after removal of the previous top
 */
gnode_list_item *pop_from_graph_node_stack(gnode_list_item *stack_top);

#endif
