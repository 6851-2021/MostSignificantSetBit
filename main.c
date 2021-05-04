// Followed notes: 
// https://courses.csail.mit.edu/6.851/spring21/lectures/L12.html?notes=8
// https://courses.csail.mit.edu/6.851/spring21/scribe/lec12.pdf

#include <stdint.h>
#include <stdio.h>

#include "mssb_8.h"
#include "mssb_16.h"
#include "mssb_32.h"
#include "mssb_64.h"
#include "utils.h"

uint32_t main() {
  //
  // Test the different bit functions
  //

  test8();
  test16();
  test32();
  test64();

  const uint32_t n_iters = 100000;
  const uint32_t n_calls = 1000;

  perf8(n_iters, n_calls);
  perf16(n_iters, n_calls);
  perf32(n_iters, n_calls);
  perf64(n_iters, n_calls);

  return 0;
}
