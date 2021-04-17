#include <stdint.h>
#include <stdio.h>

void print_binary64(uint64_t n) {
  int32_t i = 63;
  for (; i >= 0; i--) {
    const uint64_t mask = 0b1LL << i;
    if (n & mask) {
      printf("1");
    } else {
      printf("0");
    }

    // Print a divider every 8 bits
    if (i % 8 == 0) {
      printf("|");
    }
  }
  printf("\n");
}

// Set 1 in the constant time algorithm
uint64_t identify_nonempty_clusters(uint64_t x) {
  const uint32_t rootW = 8; // sqrt(64) = 8
  const uint64_t F = 0x8080808080808080LL;
  
  const uint64_t first_bit_set = x & F;
  const uint64_t remaining_bits = first_bit_set ^ x;
  const uint64_t clusters_nonempty = (((F - remaining_bits) & F) ^ F) | first_bit_set;

  print_binary64(clusters_nonempty);
}

uint32_t main() {
  identify_nonempty_clusters(0x0045627364125602LL);

  return 0;
}
