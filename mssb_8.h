
#ifndef MSSB_8_H
#define MSSB_8_H

#include "utils.h"

extern const uint32_t lookup_8bit[];

uint32_t builtin_most_significant_set_bit8(uint8_t value);
uint32_t lookup_most_significant_set_bit8(uint8_t value);
uint32_t iterative_most_significant_set_bit8(uint8_t value);

void scribbles8();
bool test8();
void perf8(uint32_t n_iters, uint32_t n_calls);

#endif // MSSB_8_H
