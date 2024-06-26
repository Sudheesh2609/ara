#include "iconv2d.h"
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void iconv2d_1x1(int64_t *o, int64_t *i, int64_t *f, int64_t R, int64_t C, int64_t F) {
    int64_t *i_ = i;
    int64_t *o_ = o;

    // Load the single filter value (1x1 filter)
    int64_t filter_value = f[0];

    // Set vector length to C (number of columns)
    asm volatile("vsetvli zero, %0, e64, m2, ta, ma" ::"r"(C));

    // Iterate over the rows
    for (int64_t r = 0; r < R; r++) {
        // Load input row
        asm volatile("vle64.v v8, (%0)" : "+&r"(i_));

        // Multiply input row with filter value
        asm volatile("vmul.vx v8, v8, %0" ::"r"(filter_value));

        // Store the result to output row
        asm volatile("vse64.v v8, (%0)" : "+&r"(o_));

        // Move to the next row in input and output matrices
        i_ += C;
        o_ += C;
    }
}
