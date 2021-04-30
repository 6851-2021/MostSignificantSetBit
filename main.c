// Followed notes: 
// https://courses.csail.mit.edu/6.851/spring21/lectures/L12.html?notes=8
// https://courses.csail.mit.edu/6.851/spring21/scribe/lec12.pdf

#include <stdint.h>
#include <stdio.h>

#include "mssb_16.h"
#include "mssb_64.h"
#include "utils.h"

uint32_t main() {
  //
  // Test the different bit functions
  //

  test16();
  test64();

  return 0;
}
