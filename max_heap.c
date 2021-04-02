#include "max_heap.h"

#include "global.h"

hnode create_empty_heap(int max_size) {
  hnode heap;
  heap.graph_item = zalloc(max_size * sizeof(gnode_list_item*));
  heap.size = 0;
  return heap;
}

void add_to_heap(gnode_list_item* elto, hnode heap) {

  heap.size += 1;
}

void heapify(hnode heap) {

}

gnode_list_item* get_heap_root(hnode heap) {

}