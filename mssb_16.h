
#ifndef MSSB_16_H
#define MSSB_16_H

#include "utils.h"

extern const uint32_t lookup_16bit[];

uint32_t const_time_most_significant_set_bit16(uint16_t value);
uint32_t builtin_most_significant_set_bit16(uint16_t value);
uint32_t binary_search_most_significant_set_bit16(uint16_t value);
uint32_t lookup_most_significant_set_bit16(uint16_t value);
uint32_t iterative_most_significant_set_bit16(uint16_t value);

void scribbles16();
bool test16();
void perf16(uint32_t n_iters, uint32_t n_calls);

#endif // MSSB_16_H
