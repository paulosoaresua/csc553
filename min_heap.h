/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_MIN_HEAP_H
#define CSC553_MIN_HEAP_H

#import "graph.h"

typedef struct MinHeap {
  gnode_list_item** items;
  int size;
} min_heap;

min_heap * create_empty_heap(int max_size);

void add_to_heap(gnode_list_item* elto, min_heap * heap);

void build_heap(min_heap * heap);

gnode_list_item* extract_heap_root(min_heap * heap);

#endif
