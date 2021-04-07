#include "min_heap.h"

#include "global.h"

min_heap *create_empty_heap(int max_size) {
  min_heap *heap = zalloc(sizeof(min_heap));
  heap->items = zalloc(max_size * sizeof(gnode_list_item *));
  for(int i = 0; i < max_size; i++) {
    heap->items[i] = NULL;
  }
  heap->size = 0;
  return heap;
}

void add_to_heap(gnode_list_item *elto, min_heap *heap) {
  heap->items[heap->size] = elto;
  heap->size += 1;
}

static void heapify(min_heap *heap, int min_node_idx) {
  int left_child_idx = 2 * min_node_idx + 1;
  int right_child_idx = 2 * min_node_idx + 2;
  int new_max_node_idx = min_node_idx;

  if (left_child_idx < heap->size &&
      heap->items[left_child_idx]->node->cost <
          heap->items[new_max_node_idx]->node->cost) {
    new_max_node_idx = left_child_idx;
  }

  if (right_child_idx < heap->size &&
      heap->items[right_child_idx]->node->cost <
          heap->items[new_max_node_idx]->node->cost) {
    new_max_node_idx = right_child_idx;
  }

  // Swap parent with the child with smallest cost
  if (min_node_idx != new_max_node_idx) {
    gnode_list_item *item = heap->items[min_node_idx];
    heap->items[min_node_idx] = heap->items[new_max_node_idx];
    heap->items[new_max_node_idx] = item;

    heapify(heap, new_max_node_idx);
  }
}

void build_heap(min_heap *heap) {
  for (int i = heap->size - 1; i >= 0; i--) {
    heapify(heap, i);
  }
}

gnode_list_item *extract_heap_root(min_heap *heap) {
  if (heap->size == 0)
    return NULL;

  gnode_list_item *root = heap->items[0];
  heap->items[0] = heap->items[heap->size - 1];
  heap->items[heap->size - 1] = NULL;
  heap->size -= 1;
  heapify(heap, 0);

  return root;
}