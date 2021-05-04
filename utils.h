#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#define NO_SET_BITS (uint32_t)-1

void print_binary(uint64_t n, uint32_t nbits);
long double average(long double array[], uint32_t n_items);

#endif // UTILS_H
