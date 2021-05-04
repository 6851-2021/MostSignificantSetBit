
#ifndef MSSB_64_H
#define MSSB_64_H

#include "utils.h"

uint32_t const_time_most_significant_set_bit64(uint64_t value);
uint32_t builtin_most_significant_set_bit64(uint64_t value);
uint32_t binary_search_most_significant_set_bit64(uint64_t value);
uint32_t iterative_most_significant_set_bit64(uint64_t value);

void scribbles64();
bool test64();
void perf64(uint32_t n_iters, uint32_t n_calls);

#endif // MSSB_64_H
