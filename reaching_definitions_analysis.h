/*
 * Author: Paulo Soares.
 * CSC 553 (Spring 2021)
 */

#ifndef CSC553_LIVENESS_ANALYSIS_ANALYSIS_H
#define CSC553_LIVENESS_ANALYSIS_ANALYSIS_H

#include "control_flow.h"

/**
 * Iteratively computes in and out sets for each block of a control flow
 * graph.
 *
 * @param block_list_head: first block in a list of blocks
 */
void find_in_and_out_def_sets(blist_node* block_list_head);

#endif // CSC553_LIVENESS_ANALYSIS_ANALYSIS_H
