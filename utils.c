#include <stdint.h>
#include <stdio.h>

const uint32_t NO_SET_BITS = (uint32_t)-1;

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
