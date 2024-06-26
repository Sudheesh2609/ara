
#include "iconv2d.h"
#include <stdint.h>
#include <stdio.h>

void iconv2d_1x1(int64_t *o, int64_t *i, int64_t *f, int64_t R, int64_t C, int64_t F) {
    int64_t filter_value = f[0]; // The single filter value for 1x1 convolution

    // Iterate over each row of the output matrix
    for (int64_t r = 0; r < R; r++) {
        // Iterate over each column of the output matrix
        for (int64_t c = 0; c < C; c++) {
            int64_t input_value;
            int64_t output_value;

            // Load input value
            asm volatile ("ld %0, 0(%1)" : "=r"(input_value) : "r"(i + r * C + c));

            // Perform the multiplication with the filter value
            asm volatile ("mul %0, %1, %2" : "=r"(output_value) : "r"(input_value), "r"(filter_value));

            // Store the result in the output matrix
            asm volatile ("sd %0, 0(%1)" : : "r"(output_value), "r"(o + r * C + c));
        }
    }
}
