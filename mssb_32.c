#include <stdio.h>
#include <stdint.h>

#include "mssb_32.h"
#include "utils.h"

// Some constants related to the algorithm
static const uint32_t rootW = 6; // sqrt(36)
static const uint64_t F = 0x820820820LL;  
static const uint64_t M = 0x84210840; // From perfect_sketch_const.py

// Step 1 in the constant time algorithm
static uint64_t identify_nonempty_clusters(uint64_t x) {
  const uint64_t first_bit_set = x & F;
  const uint64_t remaining_bits = first_bit_set ^ x;
  const uint64_t clusters_nonempty = (((F - remaining_bits) & F) ^ F) | first_bit_set;

  return clusters_nonempty;
}

// Step 2
static uint64_t perfect_sketch(uint64_t x) {
  const uint64_t interm_sketch = x * M;
  return (interm_sketch >> 36) & 0b111111;
}

// Step 3
static uint32_t parallel_comparison(uint64_t sketch) {
  // For `sketch`s that are equivalent to a value in the `sketch_mode`,
  // this parallel comparison will produce a `which_cluster` which is 
  // is in the next cluster index (because it compares <= rather than <).
  // Adding a 1, as long as it does not overflow will make the comparison
  // a strict < instead
  if (sketch != 0b111111) {
    sketch += 1;
  }

  const uint64_t sketch_spaces = 0b000000100000010000001000000100000010000001LL;
  const uint64_t sketch_mode = 0b100000110000101000100100100010100001100000LL;
  const uint64_t sketch_k = sketch * sketch_spaces;

  const uint64_t difference = sketch_mode - sketch_k;
  const uint64_t monotonic = difference & 0b100000010000001000000100000010000001000000LL;

  const uint64_t count_ones = monotonic * sketch_spaces;
  const uint32_t which_cluster = (count_ones >> 41) & 0b111111;

  return 5 - which_cluster;
}

// Step 4
static uint32_t get_bit_d(uint64_t x, uint32_t which_cluster) {
  // Get the `cluster_of_interest` in `x` based on `which_cluster`
  const uint64_t cluster_shifted = x >> (which_cluster * rootW);
  const uint64_t cluster_of_interest = cluster_shifted & 0b111111;

  // Use the same `parallel_comparison` algorithm on the `cluster_of_interest`
  const uint32_t bit_d = parallel_comparison(cluster_of_interest);
  return bit_d;
}

// Putting everything together
uint32_t const_time_most_significant_set_bit32(uint32_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  const uint64_t clusters_nonempty = identify_nonempty_clusters(value);
  const uint64_t sketch = perfect_sketch(clusters_nonempty);
  const uint32_t which_cluster = parallel_comparison(sketch);

  const uint32_t bit_d = get_bit_d(value, which_cluster);

  const uint32_t answer = which_cluster * rootW + bit_d;
  return answer;
}

uint32_t builtin_most_significant_set_bit32(uint32_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  uint32_t leading_zeros = __builtin_clz(value);
  return 31 - leading_zeros;
}

void scribbles32() {
  const uint32_t which_cluster = parallel_comparison(0b00000000);  
  printf("Which cluster %d\n", which_cluster);
}

bool test32() {
  // Test case for 32 bit

  // Count up all 16 bits, populating form the right
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    const uint64_t const_time_mssb = const_time_most_significant_set_bit32(i);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit32(i);
  
    if (const_time_mssb != builtin_mssb) {
      printf("val: %d FAIL\n", i);
      return false;
    }
  }

  // Count down all 16 bits, depopulating from the left
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    // Move the bottom 16 bits of `i` to the top bits to get `value`
    uint32_t bottom_bits = i & 0xFFFF;
    uint32_t value = bottom_bits << 16;

    const uint64_t const_time_mssb = const_time_most_significant_set_bit32(value);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit32(value);
  
    if (const_time_mssb != builtin_mssb) {
      printf("val: %d FAIL\n", value);
      return false;
    }
  }

  // All of the tests passed
  return true;
}
