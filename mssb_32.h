
#ifndef MSSB_32_H
#define MSSB_32_H

#include "utils.h"

uint32_t const_time_most_significant_set_bit32(uint32_t value);
uint32_t builtin_most_significant_set_bit32(uint32_t value);
uint32_t binary_search_most_significant_set_bit32(uint32_t value);
uint32_t iterative_most_significant_set_bit32(uint32_t value);

void scribbles32();
bool test32();
void perf32(uint32_t n_iters, uint32_t n_calls);

#endif // MSSB_32_H
