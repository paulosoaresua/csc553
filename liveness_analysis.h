/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_REACHING_DEFINITIONS_ANALYSIS_H
#define CSC553_REACHING_DEFINITIONS_ANALYSIS_H

#include "control_flow.h"

/**
 * Iteratively computes def and use sets for each block of a control flow
 * graph.
 *
 * @param block_list_head: first block in a list of blocks
 */
bool find_in_and_out_liveness_sets(blist_node* block_list_head);

#endif // CSC553_REACHING_DEFINITIONS_ANALYSIS_H
