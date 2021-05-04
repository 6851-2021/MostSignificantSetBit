#include <stdio.h>
#include <stdint.h>

#include "mssb_8.h"
#include "utils.h"

uint32_t builtin_most_significant_set_bit8(uint8_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  uint32_t leading_zeros = __builtin_clz(value);
  return 31 - leading_zeros;
}

uint32_t lookup_most_significant_set_bit8(uint8_t value) {
  return lookup_8bit[value];
}

uint32_t iterative_most_significant_set_bit8(uint8_t value) {
  uint32_t mssb = NO_SET_BITS;
  while (value != 0) {
    mssb += 1;
    value >>= 1;
  }

  return mssb;
}

uint32_t comparison_most_significant_set_bit8(uint8_t value) {
  const uint32_t bit0 = value > 0;
  const uint32_t bit1 = value > 0b1;
  const uint32_t bit2 = value > 0b11;
  const uint32_t bit3 = value > 0b111;
  const uint32_t bit4 = value > 0b1111;
  const uint32_t bit5 = value > 0b11111;
  const uint32_t bit6 = value > 0b111111;
  const uint32_t bit7 = value > 0b1111111;

  return (bit0 + bit1 + bit2 + bit3 +
          bit4 + bit5 + bit6 + bit7);
}

void scribbles8() {
  return;
}

bool test8() {
  // Test case for 8 bit

  // Count up all 8 bits
  for (uint32_t i = 0; i <= (uint8_t)-1; i++) {
    const uint32_t builtin_mssb = builtin_most_significant_set_bit8(i);
    const uint32_t lookup_mssb = lookup_most_significant_set_bit8(i);
    const uint32_t iterative_mssb = iterative_most_significant_set_bit8(i);
    const uint32_t comparison_mssb = comparison_most_significant_set_bit8(i);
  
    if (lookup_mssb != builtin_mssb) {
      printf("LOOKUP: %d FAIL\n", i);
      return false;
    }

    if (iterative_mssb != builtin_mssb) {
      printf("ITERATIVE: %d FAIL\n", i);
      return false;
    }

    if (iterative_mssb != builtin_mssb) {
      printf("COMPARISON: %d FAIL\n", i);
      return false;
    }
  }

  // All of the tests passed
  return true;
}

static long double perf8_helper(uint32_t (*mssb_fn)(uint8_t), 
                                const uint32_t n_iters,
                                const uint32_t n_calls) {
  const uint8_t MAX_VALUE = (uint8_t)-1;

  // The location to store all of the run times
  long double run_times[n_iters];

  // Time the builtin function
  for (uint32_t i = 0; i < n_iters; i++) {
    const uint8_t value = i % MAX_VALUE;
    
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

void perf8(uint32_t n_iters, uint32_t n_calls) {
  long double avg_run_time;

  printf("=== 8bit Most Significant Set Bit ===\n");

  avg_run_time = perf8_helper(builtin_most_significant_set_bit8, n_iters, n_calls);
  printf("BUILTIN: %Lg sec\n", avg_run_time);  

  avg_run_time = perf8_helper(lookup_most_significant_set_bit8, n_iters, n_calls);
  printf("LOOKUP: %Lg sec\n", avg_run_time);  
  
  avg_run_time = perf8_helper(iterative_most_significant_set_bit8, n_iters, n_calls);
  printf("ITERATIVE: %Lg sec\n", avg_run_time);  
  
  avg_run_time = perf8_helper(comparison_most_significant_set_bit8, n_iters, n_calls);
  printf("COMPARISON: %Lg sec\n", avg_run_time);
}
