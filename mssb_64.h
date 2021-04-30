
#ifndef MSSB_64_H
#define MSSB_64_H

#include "utils.h"

uint32_t const_time_most_significant_set_bit64(uint64_t value);
uint32_t builtin_most_significant_set_bit64(uint64_t value);

void scribbles64();
bool test64();

#endif // MSSB_64_H
