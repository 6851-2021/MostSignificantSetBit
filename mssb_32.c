#include <stdio.h>
#include <stdint.h>

#include "mssb_32.h"

#include "mssb_16.h"
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

uint32_t binary_search_most_significant_set_bit32(uint32_t value) {
  uint32_t hi = value >> 16;
  uint32_t lo = value & 0x0000ffff;

  if (hi != 0) {
    return binary_search_most_significant_set_bit16(hi) + 16;
  } else {
    return binary_search_most_significant_set_bit16(lo);
  }
}

uint32_t iterative_most_significant_set_bit32(uint32_t value) {
  uint32_t mssb = NO_SET_BITS;
  while (value != 0) {
    mssb += 1;
    value >>= 1;
  }

  return mssb;
}

void scribbles32() {
  const uint32_t which_cluster = parallel_comparison(0b00000000);  
  printf("Which cluster %d\n", which_cluster);
}

bool test32() {
  // Test case for 32 bit

  // Count up all 16 bits, populating form the right
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    const uint32_t builtin_mssb = builtin_most_significant_set_bit32(i);
    const uint32_t const_time_mssb = const_time_most_significant_set_bit32(i);
    const uint32_t binary_search_mssb = binary_search_most_significant_set_bit32(i);
    const uint32_t iterative_mssb = iterative_most_significant_set_bit32(i);
  
    if (const_time_mssb != builtin_mssb) {
      printf("CONST TIME: %d FAIL\n", i);
      return false;
    }

    if (binary_search_mssb != builtin_mssb) {
      printf("BINARY SEARCH: %d FAIL\n", i);
      return false;      
    }

    if (iterative_mssb != builtin_mssb) {
      printf("ITERATIVE: %d FAIL\n", i);
      return false;      
    }
  }

  // Count down all 16 bits, depopulating from the left
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    // Move the bottom 16 bits of `i` to the top bits to get `value`
    uint32_t bottom_bits = i & 0xFFFF;
    uint32_t value = bottom_bits << 16;

    const uint32_t builtin_mssb = builtin_most_significant_set_bit32(value);
    const uint32_t const_time_mssb = const_time_most_significant_set_bit32(value);
    const uint32_t binary_search_mssb = binary_search_most_significant_set_bit32(value);
    const uint32_t iterative_mssb = iterative_most_significant_set_bit32(value);
  
    if (const_time_mssb != builtin_mssb) {
      printf("CONST TIME: %d FAIL\n", value);
      return false;
    }

    if (binary_search_mssb != builtin_mssb) {
      printf("BINARY SEARCH: %d FAIL\n", i);
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

// TODO: Come up with a smarter iteration to test all representative cases
static long double perf32_helper(uint32_t (*mssb_fn)(uint32_t), 
                                 const uint32_t n_iters,
                                 const uint32_t n_calls) {
  const uint32_t MAX_VALUE = (uint32_t)-1;

  // The location to store all of the run times
  long double run_times[n_iters];

  // Time the builtin function
  for (uint32_t i = 0; i < n_iters; i++) {
    const uint32_t value = i % MAX_VALUE;
    
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

void perf32(uint32_t n_iters, uint32_t n_calls) {
  long double avg_run_time;

  printf("=== 32bit Most Significant Set Bit ===\n");

  avg_run_time = perf32_helper(builtin_most_significant_set_bit32, n_iters, n_calls);
  printf("BUILTIN: %Lg sec\n", avg_run_time);  

  avg_run_time = perf32_helper(const_time_most_significant_set_bit32, n_iters, n_calls);
  printf("CONST TIME: %Lg sec\n", avg_run_time);  

  avg_run_time = perf32_helper(binary_search_most_significant_set_bit32, n_iters, n_calls);
  printf("BINARY SEARCH: %Lg sec\n", avg_run_time);  

  avg_run_time = perf32_helper(iterative_most_significant_set_bit32, n_iters, n_calls);
  printf("ITERATIVE: %Lg sec\n", avg_run_time);  
  
  //avg_run_time = perf16_helper(comparison_most_significant_set_bit16, n_iters, n_calls);
  //printf("COMPARISON: %Lg sec\n", avg_run_time);
}
