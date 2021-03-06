/*
 * Author: Paulo Soares
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_SET_H
#define CSC553_SET_H

#include <stdbool.h>

// Bit set for numbers from 0 to N. Each location of mask can store up to 32
// elements.
// Maximum number of elements it can have is 32*(2^64)
typedef struct Set {
  int max_size;
  unsigned long long num_partitions;
  unsigned int *mask;
} set;

/**
 * Checks whether the set has not been initialized before.
 *
 * @param set: set
 *
 * @return
 */
bool is_set_undefined(set set);

/**
 * Creates a new set to store an id up to a certain number.
 *
 * @param max_size: maximum number of elements in the set.
 *
 * @return Empty set
 */
set create_empty_set(int max_size);

/**
 * Creates a new set filled with all the elements possible.
 *
 * @param max_size
 *
 * @return Full set
 */
set create_full_set(int size);

// Set operations. They assume the sets have the same maximum number of
// elements.
void add_to_set(int elto, set set);

void remove_from_set(int elto, set set);

bool are_set_equals(set set1, set set2);

set unify_sets(set set1, set set2);

set intersect_sets(set set1, set set2);

set diff_sets(set set1, set set2);

#endif
