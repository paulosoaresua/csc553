/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_HEAP_H
#define CSC553_HEAP_H

#include "graph.h"

typedef struct Heap {
  gnode** items;
  int size;
  bool min;
} heap;

heap * create_empty_heap(int max_size, bool min);

void add_to_heap(gnode* elto, heap * heap);

void build_heap(heap * heap);

gnode* extract_heap_root(heap * heap);

gnode* peek_heap_root(heap * heap);

#endif
