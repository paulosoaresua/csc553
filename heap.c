#include "heap.h"

#include "global.h"

heap *create_empty_heap(int max_size, bool min) {
  heap *heap = zalloc(sizeof(heap));
  heap->items = zalloc(max_size * sizeof(gnode *));
  heap->min = min;
  for(int i = 0; i < max_size; i++) {
    heap->items[i] = NULL;
  }
  heap->size = 0;
  return heap;
}

void add_to_heap(gnode *elto, heap *heap) {
  heap->items[heap->size] = elto;
  heap->size += 1;
}

static void heapify(heap *heap, int min_node_idx) {
  int left_child_idx = 2 * min_node_idx + 1;
  int right_child_idx = 2 * min_node_idx + 2;
  int new_max_node_idx = min_node_idx;

  if (heap->min) {
    // Min Heap
    if (left_child_idx < heap->size &&
        heap->items[left_child_idx]->cost <
            heap->items[new_max_node_idx]->cost) {
      new_max_node_idx = left_child_idx;
    }

    if (right_child_idx < heap->size &&
        heap->items[right_child_idx]->cost <
            heap->items[new_max_node_idx]->cost) {
      new_max_node_idx = right_child_idx;
    }
  } else {
    // Max Heap
    if (left_child_idx < heap->size &&
        heap->items[left_child_idx]->cost >
        heap->items[new_max_node_idx]->cost) {
      new_max_node_idx = left_child_idx;
    }

    if (right_child_idx < heap->size &&
        heap->items[right_child_idx]->cost >
        heap->items[new_max_node_idx]->cost) {
      new_max_node_idx = right_child_idx;
    }
  }

  // Swap parent with the child with smallest/largest cost
  if (min_node_idx != new_max_node_idx) {
    gnode *item = heap->items[min_node_idx];
    heap->items[min_node_idx] = heap->items[new_max_node_idx];
    heap->items[new_max_node_idx] = item;

    heapify(heap, new_max_node_idx);
  }
}

void build_heap(heap *heap) {
  for (int i = heap->size - 1; i >= 0; i--) {
    heapify(heap, i);
  }
}

gnode *extract_heap_root(heap *heap) {
  if (heap->size == 0)
    return NULL;

  gnode *root = heap->items[0];
  heap->items[0] = heap->items[heap->size - 1];
  heap->items[heap->size - 1] = NULL;
  heap->size -= 1;
  heapify(heap, 0);

  return root;
}

gnode *peek_heap_root(heap *heap) {
  if (heap->size == 0)
    return NULL;

  return heap->items[0];
}