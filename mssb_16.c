#include <stdio.h>
#include <stdint.h>

#include "mssb_16.h"

#include "mssb_8.h"
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

uint32_t binary_search_most_significant_set_bit16(uint16_t value) {
  uint16_t hi = value >> 8;
  uint16_t lo = value & 0x00ff;

  if (hi != 0) {
    return lookup_most_significant_set_bit8(hi) + 8;
  } else {
    return lookup_most_significant_set_bit8(lo);
  }
}

uint32_t lookup_most_significant_set_bit16(uint16_t value) {
  return lookup_16bit[value];
}

uint32_t iterative_most_significant_set_bit16(uint16_t value) {
  uint32_t mssb = NO_SET_BITS;
  while (value != 0) {
    mssb += 1;
    value >>= 1;
  }

  return mssb;
}

uint32_t comparison_most_significant_set_bit16(uint16_t value) {
  const uint32_t bit0 = value > 0;
  const uint32_t bit1 = value > 0b1;
  const uint32_t bit2 = value > 0b11;
  const uint32_t bit3 = value > 0b111;
  const uint32_t bit4 = value > 0b1111;
  const uint32_t bit5 = value > 0b11111;
  const uint32_t bit6 = value > 0b111111;
  const uint32_t bit7 = value > 0b1111111;

  const uint32_t bit8 = value > 0b11111111;
  const uint32_t bit9 = value > 0b111111111;
  const uint32_t bit10 = value > 0b111111111;
  const uint32_t bit11 = value > 0b1111111111;
  const uint32_t bit12 = value > 0b11111111111;
  const uint32_t bit13 = value > 0b111111111111;
  const uint32_t bit14 = value > 0b1111111111111;
  const uint32_t bit15 = value > 0b11111111111111;

  return (bit0 + bit1 + bit2 + bit3 +
          bit4 + bit5 + bit6 + bit7 +
          bit8 + bit9 + bit10 + bit11 +
          bit12 + bit13 + bit14 + bit15);
}

void scribbles16() {
  const uint32_t which_cluster = parallel_comparison(0b00000000);  
  printf("Which cluster %d\n", which_cluster);
}

bool test16() {
  // Test case for 16 bit
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    const uint32_t builtin_mssb = builtin_most_significant_set_bit16(i);
    const uint32_t const_time_mssb = const_time_most_significant_set_bit16(i);
    const uint32_t binary_search_mssb = binary_search_most_significant_set_bit16(i);
    const uint32_t lookup_mssb = lookup_most_significant_set_bit16(i);
    const uint32_t iterative_mssb = iterative_most_significant_set_bit16(i);

    if (const_time_mssb != builtin_mssb) {
      printf("CONST TIME: %d FAIL\n", i);
      return false;
    }

    if (binary_search_mssb != builtin_mssb) {
      printf("BINARY SEARCH: %d FAIL\n", i);
      return false;
    }

    if (lookup_mssb != builtin_mssb) {
      printf("LOOKUP: %d FAIL\n", i);
      return false;
    }

    if (iterative_mssb != builtin_mssb) {
      printf("ITERATIVE: %d FAIL\n", i);
      return false;
    }
  }

  // All of the tests passed
  return true;
}

static long double perf16_helper(uint32_t (*mssb_fn)(uint16_t), 
                                 const uint32_t n_iters,
                                 const uint32_t n_calls) {
  const uint16_t MAX_VALUE = (uint16_t)-1;

  // The location to store all of the run times
  long double run_times[n_iters];

  // Time the builtin function
  for (uint32_t i = 0; i < n_iters; i++) {
    const uint16_t value = i % MAX_VALUE;
    
    clock_t start_clk = clock();

    // Measure the time to perform `n_calls` function calls
    for (uint32_t j = 0; j < n_calls; j++) {
      // Do not optimize out the function call
      volatile uint32_t ret = mssb_fn(value);
    }

    clock_t end_clk = clock();

    run_times[i] = (end_clk - start_clk) / (long double)CLOCKS_PER_SEC;
  }

  /* // Debugging, print the run times
   * for (uint32_t i = 0; i < n_iters; i++) {
   *   printf("%Lg ", run_times[i]);
   * }
   * printf("\n"); */

  // Return the average run time
  return average(run_times, n_iters);
}

void perf16(uint32_t n_iters, uint32_t n_calls) {
  long double avg_run_time;

  printf("=== 16bit Most Significant Set Bit ===\n");

  avg_run_time = perf16_helper(builtin_most_significant_set_bit16, n_iters, n_calls);
  printf("BUILTIN: %Lg sec\n", avg_run_time);  

  avg_run_time = perf16_helper(const_time_most_significant_set_bit16, n_iters, n_calls);
  printf("CONST TIME: %Lg sec\n", avg_run_time);  

  avg_run_time = perf16_helper(binary_search_most_significant_set_bit16, n_iters, n_calls);
  printf("BINARY SEARCH: %Lg sec\n", avg_run_time);  

  avg_run_time = perf16_helper(lookup_most_significant_set_bit16, n_iters, n_calls);
  printf("LOOKUP: %Lg sec\n", avg_run_time);  
  
  avg_run_time = perf16_helper(iterative_most_significant_set_bit16, n_iters, n_calls);
  printf("ITERATIVE: %Lg sec\n", avg_run_time);  
  
  avg_run_time = perf16_helper(comparison_most_significant_set_bit16, n_iters, n_calls);
  printf("COMPARISON: %Lg sec\n", avg_run_time);
}
