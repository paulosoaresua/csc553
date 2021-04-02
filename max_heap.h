/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_MAX_HEAP_H
#define CSC553_MAX_HEAP_H

#import "graph.h"

typedef struct HeapNode {
  gnode_list_item** graph_item;
  int size;
} hnode;

hnode create_empty_heap(int max_size);

void add_to_heap(gnode_list_item* elto, hnode heap);

void heapify(hnode heap);

gnode_list_item* get_heap_root(hnode heap);

#endif
