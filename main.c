// Followed notes: 
// https://courses.csail.mit.edu/6.851/spring21/lectures/L12.html?notes=8
// https://courses.csail.mit.edu/6.851/spring21/scribe/lec12.pdf

#include <stdint.h>
#include <stdio.h>

const uint32_t NO_SET_BITS = (uint32_t)-1;

// Some constants related to the algorithm
const uint32_t rootW = 4; // sqrt(16)
const uint16_t F = 0x8888;  
const uint16_t M = 0x2490; // From perfect_sketch_const.py

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

// Step 1 in the constant time algorithm
uint16_t identify_nonempty_clusters(uint16_t x) {
  const uint16_t first_bit_set = x & F;
  const uint16_t remaining_bits = first_bit_set ^ x;
  const uint16_t clusters_nonempty = (((F - remaining_bits) & F) ^ F) | first_bit_set;

  return clusters_nonempty;
}

// Step 2
uint16_t perfect_sketch(uint16_t x) {
  const uint32_t interm_sketch = (uint32_t)x * M;
  return (interm_sketch >> 16) & 0b1111;
}

// Step 3
uint32_t parallel_comparison(uint16_t sketch) {
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
uint32_t get_bit_d(uint16_t x, uint32_t which_cluster) {
  // Get the `cluster_of_interest` in `x` based on `which_cluster`
  const uint16_t cluster_shifted = x >> (which_cluster * rootW);
  const uint16_t cluster_of_interest = cluster_shifted & 0b1111;

  // Use the same `parallel_comparison` algorithm on the `cluster_of_interest`
  const uint32_t bit_d = parallel_comparison(cluster_of_interest);
  return bit_d;
}

// Putting everything together
uint32_t const_time_most_significant_set_bit(uint16_t value) {
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

uint32_t builtin_most_significant_set_bit(uint16_t value) {
  if (!value) {
    return NO_SET_BITS;
  }

  uint32_t leading_zeros = __builtin_clz(value);
  return 31 - leading_zeros;
}

uint32_t main() {
  const uint16_t value = 1;
  const uint64_t const_time_mssb = const_time_most_significant_set_bit(value);
  const uint32_t builtin_mssb = builtin_most_significant_set_bit(value);

  /* printf("Value: ");
   * print_binary(value, 16);
   * printf("CONST TIME mssb: %ld\n", const_time_mssb);
   * printf("BUILTIN mssb: %d\n", builtin_mssb); */

  // Test case
  for (uint32_t i = 0; i <= (uint16_t)-1; i++) {
    const uint64_t const_time_mssb = const_time_most_significant_set_bit(i);
    const uint32_t builtin_mssb = builtin_most_significant_set_bit(i);

    if (const_time_mssb != builtin_mssb) {
      printf("i: %d FAIL\n", i);
      return 1;
    }
  }

  return 0;
}
