#include "set.h"

#include "global.h"
#include <math.h>

#define BYTES_PER_PARTITION 4
#define BITS_PER_PARTITION 32

bool is_set_undefined(set set) {
  return set.mask == NULL;
}

set create_empty_set(int max_size) {
  set set;

  set.max_size = max_size;
  set.num_partitions = ceil((double) max_size / BITS_PER_PARTITION);
  set.mask = zalloc(fmax(1, set.num_partitions * BYTES_PER_PARTITION));
  for (int i = 0; i < set.num_partitions; i++) {
    set.mask[i] = 0;
  }

  return set;
}

set create_full_set(int max_size) {
  set set = create_empty_set(max_size);
  for (int i = 0; i < set.num_partitions; i++) {
    set.mask[i] = ~0;
  }

  return set;
}

void add_to_set(int elto, set set) {
  int partition = elto / BITS_PER_PARTITION;
  int pos_in_partition = elto % BITS_PER_PARTITION;
  set.mask[partition] |= 1 << pos_in_partition;
}

void remove_from_set(int elto, set set) {
  int partition = elto / BITS_PER_PARTITION;
  int pos_in_partition = elto % BITS_PER_PARTITION;
  set.mask[partition] &= ~(1 << pos_in_partition);
}

bool are_set_equals(set set1, set set2) {
  if(is_set_undefined(set1) || is_set_undefined(set2)) {
    return false;
  }

  for (int i = 0; i < set1.num_partitions; i++) {
    if (set1.mask[i] != set2.mask[i]) {
      return false;
    }
  }

  return true;
}

set unify_sets(set set1, set set2) {
  set new_set = create_empty_set(set1.max_size);

  for (int i = 0; i < set1.num_partitions; i++) {
    new_set.mask[i] = set1.mask[i] | set2.mask[i];
  }

  return new_set;
}

set intersect_sets(set set1, set set2) {
  set new_set = create_empty_set(set1.max_size);

  for (int i = 0; i < set1.num_partitions; i++) {
    new_set.mask[i] = set1.mask[i] & set2.mask[i];
  }

  return new_set;
}

set diff_sets(set set1, set set2) {
  set new_set = create_empty_set(set1.max_size);

  for (int i = 0; i < set1.num_partitions; i++) {
    new_set.mask[i] = set1.mask[i] & ~set2.mask[i];
  }

  return new_set;
}

set clone_set(set original_set) {
  set cloned_set = create_empty_set(original_set.max_size);
  cloned_set = unify_sets(cloned_set, original_set);

  return cloned_set;
}

bool is_set_empty(set set) {
  for(int i = 0; i < set.num_partitions; i++) {
    if(set.mask[i] != 0) {
      return false;
    }
  }

  return true;
}

bool does_elto_belong_to_set(int elto, set set) {
  if (is_set_undefined(set)) return false;

  int partition = elto / BITS_PER_PARTITION;
  int pos_in_partition = elto % BITS_PER_PARTITION;
  int elto_mask = (1 << pos_in_partition);
  return (set.mask[partition] & elto_mask) == elto_mask;
}