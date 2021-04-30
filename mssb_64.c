#include <stdint.h>
#include <stdio.h>

#include "mssb_64.h"
#include "utils.h"

static const uint32_t rootW = 8; // sqrt(64)
static const uint64_t F = 0x8080808080808080LL;  
static const uint64_t M = 0x0204081020408100LL; // From perfect_sketch_const.py

// Adapted from https://stackoverflow.com/questions/31652875/fastest-way-to-multiply-two-64-bit-ints-to-128-bit-then-to-64-bit
/* Compute (a * b) >> 64 */
uint64_t mulshift64(uint64_t a, uint64_t b, uint64_t *_regular_bits) {
  uint64_t overflow_bits, regular_bits;

    __asm__ (
        "movq  %2, %%rax;\n\t"          // rax = a
        "imulq %3;\n\t"                 // rdx:rax = a * b
        "movq  %%rdx, %0;\n\t"          // `overflow_bits` = rdx
        "movq  %%rax, %1;\n\t"          // `regular_bits` = rax
        : "=rm" (overflow_bits), "=rm" (regular_bits)
        : "rm"(a), "rm"(b)
        : "%rax", "%rdx");

    // If `_regular_bits` is non-NULL, set it to `regular_bits`
    if (_regular_bits) {
      *_regular_bits = regular_bits;
    }

    return overflow_bits;
}

// Step 1 in the constant time algorithm
static uint64_t identify_nonempty_clusters(uint64_t x) {
  const uint64_t first_bit_set = x & F;
  const uint64_t remaining_bits = first_bit_set ^ x;
  const uint64_t clusters_nonempty = (((F - remaining_bits) & F) ^ F) | first_bit_set;

  return clusters_nonempty;
}

// Step 2
static uint64_t perfect_sketch(uint64_t x) {
  const uint64_t interm_sketch = mulshift64(x, M, NULL);
  return interm_sketch & 0b11111111;
}

// Step 3
static uint32_t parallel_comparison(uint64_t sketch) {
  // Because of overflow issue, the leftmost sketch (0b00000001)
  // could not be included. Luckily, this is a singular case for
  // when `sketch == 0`, the `which_cluster` is -1
  if (!sketch) {
    return NO_SET_BITS;
  }

  // For `sketch`s that are equivalent to a value in the `sketch_mode`,
  // this parallel comparison will produce a `which_cluster` which is 
  // is in the next cluster index (because it compares <= rather than <).
  // Adding a 1, as long as it does not overflow will make the comparison
  // a strict < instead
  if (sketch != 0b11111111) {
    sketch += 1;
  }

  const uint64_t sketch_spaces = 0b000000001000000001000000001000000001000000001000000001000000001LL;
  const uint64_t sketch_mode = 0b100000010100000100100001000100010000100100000101000000110000000LL;
  const uint64_t sketch_k = sketch * sketch_spaces;

  const uint64_t difference = sketch_mode - sketch_k;
  const uint64_t monotonic = difference & 0b100000000100000000100000000100000000100000000100000000100000000LL;

  uint64_t regular_bits;
  const uint64_t overflow_bits = mulshift64(monotonic, sketch_spaces, &regular_bits);
  const uint64_t which_cluster = ((overflow_bits & 0b111111) << 2) | (regular_bits >> 62) & 0b11;

  return 7 - which_cluster;
}

// Step 4
static uint32_t get_bit_d(uint64_t x, uint32_t which_cluster) {
  // Get the `cluster_of_interest` in `x` based on `which_cluster`
  const uint64_t cluster_shifted = x >> (which_cluster * rootW);
  const uint64_t cluster_of_interest = cluster_shifted & 0b11111111;

  // Use the same `parallel_comparison` algorithm on the `cluster_of_interest`
  const uint32_t bit_d = parallel_comparison(cluster_of_interest);
  return bit_d;
}

// Putting everything together
uint32_t const_time_most_significant_set_bit64(uint64_t value) {
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

uint32_t builtin_most_significant_set_bit64(uint64_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  uint32_t leading_zeros = __builtin_clzll(value);
  return 63 - leading_zeros;
}

void scribbles64() {
  const uint64_t value = 0xaf0000045dLL;
  /* const uint64_t clusters_nonempty = identify_nonempty_clusters(value);
   * const uint64_t sketch = perfect_sketch(clusters_nonempty);
   * const uint32_t which_cluster = parallel_comparison(0b00000000);
   * 
   * printf("Value:\n");
   * print_binary(value, 64);
   * print_binary(clusters_nonempty, 64);
   * print_binary(sketch, 64);
   * printf("Which cluster %d\n", which_cluster);
   * // printf("Clusters nonempty: %ld\n", clusters_nonempty); */

  const uint32_t mssb = const_time_most_significant_set_bit64(value);

  print_binary(value, 64);
  printf("MSSB %d\n", mssb);  
}

bool test64() {
  // Test case for 64 bit

  // Count up all 16 bits, populating from the right
  for (uint64_t i = 0; i <= (uint16_t)-1; i++) {
    const uint64_t const_time_mssb = const_time_most_significant_set_bit64(i);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit64(i);
  
    if (const_time_mssb != builtin_mssb) {
      printf("val: %ld FAIL\n", i);
      return false;
    }
  }

  // Count down all 16 bits, depopulating from the left
  for (uint64_t i = 0; i <= (uint16_t)-1; i++) {
    // Move the bottom 16 bits of `i` to the top bits to get `value`
    uint64_t bottom_bits = i & 0xFFFF;
    uint64_t value = bottom_bits << 48;

    const uint64_t const_time_mssb = const_time_most_significant_set_bit64(value);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit64(value);
  
    if (const_time_mssb != builtin_mssb) {
      printf("val: %ld FAIL\n", value);
      return false;
    }
  }

  // All of the tests passed
  return true;
}
