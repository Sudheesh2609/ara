// Copyright 2020 ETH Zurich and University of Bologna.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Matteo Perotti

/*
  Optimized convolution for Ara
  The code is long only because of:
  1) Special cases related to the first/last 7 rows
  2) Unrolling of the loops to hide the latency of the moves, slides, mem ops

  At the end of the file, you can find the not-unrolled main loop in a comment,
  without the edge-code.

  Algorithm:
  a) Load the next input row
  b) Calculate its contributions to the F = 7 output rows using one column of
  the filter c) Slide down the input row by 1, injecting the next input scalar
  element in the tail d) Repeat from b), taking the next colum of the filter,
  until the last column is fetched e) Store the first output row, the one that
  is complete f) Move all the output rows up by one register, to restore the
  initial condition g) Repeat from a)

  Every time a new input row is loaded, a new output row is created.

  The first 6 rows and the last 6 rows do not follow this pattern, thus we wrote
  dedicated code. Because of the unrolling, we counted for this the first and
  last 7 rows, instead of 6

  This algorithm helps in minimizing the data dependencies, as every input rows
  is used To calculate 7 different output rows.
*/

#include "iconv2d.h"
#include <stdint.h>

void iconv2d_7x7(int64_t *o, int64_t *i, int64_t *f, int64_t M, int64_t N, int64_t F) {
    int64_t *input_ptr = i;
    int64_t *output_ptr = o;
    int64_t *filter_ptr = f;

    // Iterate over the output rows
    for (int64_t r = 0; r < M; r++) {
        // Iterate over the output columns
        for (int64_t c = 0; c < N; c++) {
            int64_t sum = 0;

            // Iterate over the filter rows
            for (int64_t fr = 0; fr < F; fr++) {
                // Iterate over the filter columns
                for (int64_t fc = 0; fc < F; fc++) {
                    int64_t input_val, filter_val;

                    // Load input value
                    asm volatile ("ld %0, 0(%1)" : "=r"(input_val) : "r"(input_ptr + (r + fr) * (N + F - 1) + (c + fc)));

                    // Load filter value
                    asm volatile ("ld %0, 0(%1)" : "=r"(filter_val) : "r"(filter_ptr + fr * F + fc));

                    // Multiply and accumulate
                    asm volatile ("mul %0, %1, %2" : "=r"(input_val) : "r"(input_val), "r"(filter_val));
                    asm volatile ("add %0, %0, %1" : "+r"(sum) : "r"(input_val));
                }
            }

            // Store the result in the output matrix
            asm volatile ("sd %0, 0(%1)" : : "r"(sum), "r"(output_ptr + r * N + c));
        }
    }
}
