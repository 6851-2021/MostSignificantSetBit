#include <stdio.h>

#include "utils.h"

void print_binary(uint64_t n, uint32_t nbits) {
  int32_t i = nbits - 1;
  for (; i >= 0; i--) {
    const uint64_t mask = 0b1LL << i;
    if (n & mask) {
      printf("1");
    } else {
      printf("0");
    }

    // Print a divider every 8 bits, except after last byte
    if (!(i % 8) && i) {
      printf("|");
    }
  }
  printf("\n");
}

// Adds up `array` by halves to mitigate floating point loss
static long double recursive_addition(long double array[], uint32_t n_items) {
  // Sanity check
  assert(n_items >= 1);

  // Base case
  if (n_items == 1) {
    return array[0];
  }

  const uint32_t left_items = n_items / 2;
  const uint32_t right_items = n_items - left_items;

  return (recursive_addition(array, left_items) +
          recursive_addition(array + left_items, right_items));  
}

long double average(long double array[], uint32_t n_items) {
  const long double sum = recursive_addition(array, n_items);
  return sum / (long double)n_items;
}
