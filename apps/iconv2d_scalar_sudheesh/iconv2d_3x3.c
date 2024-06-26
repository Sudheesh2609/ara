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

#include "iconv2d.h"
#include <stdio.h>
#include <stdint.h>

void iconv2d_3x3(int64_t *o, int64_t *i, int64_t *f, int64_t R, int64_t C, int64_t F) {
    int64_t *input = i;
    int64_t *output = o;
    int64_t *filter = f;

    // Iterate over each row of the output matrix
    for (int64_t r = 0; r < R; r++) {
        // Iterate over each column of the output matrix
        for (int64_t c = 0; c < C; c++) {
            int64_t sum = 0;

            // Iterate over each row of the filter
            for (int64_t fr = 0; fr < F; fr++) {
                // Iterate over each column of the filter
                for (int64_t fc = 0; fc < F; fc++) {
                    int64_t input_val, filter_val;
                    
                    // Load input value
                    asm volatile ("ld %0, 0(%1)" : "=r"(input_val) : "r"(input + (r + fr) * (C + F - 1) + (c + fc)));
                    
                    // Load filter value
                    asm volatile ("ld %0, 0(%1)" : "=r"(filter_val) : "r"(filter + fr * F + fc));
                    
                    // Multiply and accumulate
                    asm volatile ("mul %0, %1, %2" : "=r"(input_val) : "r"(input_val), "r"(filter_val));
                    asm volatile ("add %0, %0, %1" : "+r"(sum) : "r"(input_val));
                }
            }
            
            // Store the result in the output matrix
            asm volatile ("sd %0, 0(%1)" : : "r"(sum), "r"(output + r * C + c));
        }
    }
}

