#include <stdio.h>
#include <stdint.h>

#include "mssb_16.h"
#include "utils.h"

// Some constants related to the algorithm
static const uint32_t rootW = 4; // sqrt(16)
static const uint16_t F = 0x8888;  
static const uint16_t M = 0x2490; // From perfect_sketch_const.py

// Step 1 in the constant time algorithm
static uint16_t identify_nonempty_clusters(uint16_t x) {
  const uint16_t first_bit_set = x & F;
  const uint16_t remaining_bits = first_bit_set ^ x;
  const uint16_t clusters_nonempty = (((F - remaining_bits) & F) ^ F) | first_bit_set;

  return clusters_nonempty;
}

// Step 2
static uint16_t perfect_sketch(uint16_t x) {
  const uint32_t interm_sketch = (uint32_t)x * M;
  return (interm_sketch >> 16) & 0b1111;
}

// Step 3
static uint32_t parallel_comparison(uint16_t sketch) {
  // For `sketch`s that are equivalent to a value in the `sketch_mode`,
  // this parallel comparison will produce a `which_cluster` which is 
  // is in the next cluster index (because it compares <= rather than <).
  // Adding a 1, as long as it does not overflow will make the comparison
  // a strict < instead
  if (sketch != 0b1111) {
    sketch += 1;
  }

  const uint32_t sketch_spaces = 0b00001000010000100001;
  const uint32_t sketch_mode = 0b10001100101010011000;
  const uint32_t sketch_k = sketch * sketch_spaces;

  const uint32_t difference = sketch_mode - sketch_k;
  const uint32_t monotonic = difference & 0b10000100001000010000;

  const uint32_t count_ones = monotonic * sketch_spaces;
  const uint32_t which_cluster = (count_ones >> 19) & 0b1111;

  return 3 - which_cluster;
}

// Step 4
static uint32_t get_bit_d(uint16_t x, uint32_t which_cluster) {
  // Get the `cluster_of_interest` in `x` based on `which_cluster`
  const uint16_t cluster_shifted = x >> (which_cluster * rootW);
  const uint16_t cluster_of_interest = cluster_shifted & 0b1111;

  // Use the same `parallel_comparison` algorithm on the `cluster_of_interest`
  const uint32_t bit_d = parallel_comparison(cluster_of_interest);
  return bit_d;
}

// Putting everything together
uint32_t const_time_most_significant_set_bit16(uint16_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  const uint16_t clusters_nonempty = identify_nonempty_clusters(value);
  const uint16_t sketch = perfect_sketch(clusters_nonempty);
  const uint32_t which_cluster = parallel_comparison(sketch);

  const uint32_t bit_d = get_bit_d(value, which_cluster);

  const uint32_t answer = which_cluster * rootW + bit_d;
  return answer;
}

uint32_t builtin_most_significant_set_bit16(uint16_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  uint32_t leading_zeros = __builtin_clz(value);
  return 31 - leading_zeros;
}

void scribbles16() {
  const uint32_t which_cluster = parallel_comparison(0b00000000);  
  printf("Which cluster %d\n", which_cluster);
}

bool test16() {
  // Test case for 16 bit
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    const uint64_t const_time_mssb = const_time_most_significant_set_bit16(i);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit16(i);
  
    if (const_time_mssb != builtin_mssb) {
      printf("val: %d FAIL\n", i);
      return false;
    }
  }

  // All of the tests passed
  return true;
}
