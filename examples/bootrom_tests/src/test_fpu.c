/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string.h>
#include "fw_api.h"
#include "test_fpu.h"

void test_fpu(void) { 
#ifdef FPU_ENABLED
    printf_uart("%s", "HW_FPU . . . . .");
#else
    printf_uart("%s", "SOFT_FPU . . . .");
#endif

    double x1 = 10.323;
    double x2 = -5.3333;
    double x3;
    int64_t ix3;
    uint64_t ux3;

    ix3 = -55;
    x3 = (double)ix3;
    if (x3 != -55.0) {
        print_uart("FAIL (DCVT_D_L)\r\n", 16);
        return;
    }

    ux3 = 75;
    x3 = (double)ux3;
    if (x3 != 75.0) {
        print_uart("FAIL (DCVT_D_LU)\r\n", 16);
        return;
    }

    ix3 = (int64_t)x2;
    if (ix3 != (int64_t)(-5.3333)) {
        print_uart("FAIL (DCVT_L_D)\r\n", 16);
        return;
    }

    /** Warning: conversion of negative double to unsigned integer
                 is undefined (C99/C11 6.3.1.4) and result is target dependable.
                 Hardware FPU is oriented on x86 implementation
    */
    ux3 = (uint64_t)((int64_t)x2);
    if (ux3 != (uint64_t)((int64_t)(-5.3333))) {
        print_uart("FAIL (DCVT_LU_D)\r\n", 17);
        return;
    }

    x3 = x1 * x2;
    if (x3 != (10.323 * -5.3333)) {
        print_uart("FAIL (DMUL)\r\n", 12);
        return;
    }

    x3 = x1 / x2;
    if (x3 != (10.323 / -5.3333)) {
        print_uart("FAIL (DDIV)\r\n", 12);
        return;
    }

    x3 = x1 + x2;
    // It supposed to work only with optimization -O0
    if (x3 != (10.323 - 5.3333)) {
        print_uart("FAIL (DADD)\r\n", 12);
        return;
    }

    x3 = x1 - x2;
    if (x3 != (10.323 + 5.3333)) {
        print_uart("FAIL (DSUB)\r\n", 12);
        return;
    }

    x1 = -17.1;
    x2 = -17.05;
    if (x1 >= x2) {
        printf_uart("FAIL (FCMP %d)\r\n", 1);
        return;
    }

    x1 = 17.1;
    x2 = 17.05;
    if (x1 < x2) {
        printf_uart("FAIL (FCMP %d)\r\n", 2);
        return;
    }

    x1 = -17.1;
    x2 = 17.1;
    if (x2 <= x1) {
        printf_uart("FAIL (FCMP %d)\r\n", 3);
        return;
    }

    x1 = -17.1;
    x2 = -17.1;
    if (x2 != x1) {
        printf_uart("FAIL (FCMP %d)\r\n", 4);
        return;
    }

    x1 = 17.1;
    x2 = 17.1;
    if (x2 != x1) {
        printf_uart("FAIL (FCMP %d)\r\n", 5);
        return;
    }

    print_uart("PASS\r\n", 6);

#ifdef ENABLE_FADD_TESTS
    Reg64Type a, b, res;
    for (size_t i = 0; i < FADD_LENGTH; i++) {
        a.val = FADD_TESTS[i].a;
        b.val = FADD_TESTS[i].b;
        res.f64 = a.f64 + b.f64;
        if (res.val != FADD_TESTS[i].res) {
            printf_uart("FADD[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FSUB_TESTS
    Reg64Type a, b, res;
    for (size_t i = 0; i < FSUB_LENGTH; i++) {
        a.val = FSUB_TESTS[i].a;
        b.val = FSUB_TESTS[i].b;
        res.f64 = a.f64 - b.f64;
        if (res.val != FSUB_TESTS[i].res) {
            printf_uart("FSUB[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FDIV_TESTS
    Reg64Type a, b, res;
    for (size_t i = 0; i < FDIV_LENGTH; i++) {
        a.val = FDIV_TESTS[i].a;
        b.val = FDIV_TESTS[i].b;
        res.f64 = a.f64 / b.f64;
        if (res.val != FDIV_TESTS[i].res) {
            printf_uart("FDIV[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FMUL_TESTS
    Reg64Type a, b, res;
    for (size_t i = 0; i < FMUL_LENGTH; i++) {
        a.val = FMUL_TESTS[i].a;
        b.val = FMUL_TESTS[i].b;
        res.f64 = a.f64 * b.f64;
        if (res.val != FMUL_TESTS[i].res) {
            printf_uart("FMUL[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FMAX_TESTS
    // Not relevant because compiler uses flt+branch instruction instead
    Reg64Type a, b, res;
    for (size_t i = 0; i < FMAX_LENGTH; i++) {
        a.val = FMAX_TESTS[i].a;
        b.val = FMAX_TESTS[i].b;
        res.f64 = a.f64 > b.f64 ? a.f64: b.f64;
        if (res.val != FMAX_TESTS[i].res) {
            printf_uart("FMAX[%d] fail\r\n", i);
        }
    }
#endif
}
