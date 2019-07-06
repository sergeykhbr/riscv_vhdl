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
    Reg64Type a, b, res;
    int64_t ix3;
    uint64_t ux3;
    int err_cnt = 0;
#ifdef FPU_ENABLED
    printf_uart("%s", "HW_FPU . . . . .testing\r\n");
#else
    printf_uart("%s", "SOFT_FPU . . . .testing\r\n");
#endif

#ifdef ENABLE_FADD_TESTS
    printf_uart("Testing %s\r\n", "FADD");
    for (size_t i = 0; i < FADD_LENGTH; i++) {
        a.val = FADD_TESTS[i].a;
        b.val = FADD_TESTS[i].b;
        res.f64 = a.f64 + b.f64;
        if (res.val != FADD_TESTS[i].res) {
            err_cnt++;
            printf_uart("FADD[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FSUB_TESTS
    printf_uart("Testing %s\r\n", "FSUB");
    for (size_t i = 0; i < FSUB_LENGTH; i++) {
        a.val = FSUB_TESTS[i].a;
        b.val = FSUB_TESTS[i].b;
        res.f64 = a.f64 - b.f64;
        if (res.val != FSUB_TESTS[i].res) {
            err_cnt++;
            printf_uart("FSUB[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FDIV_TESTS
    printf_uart("Testing %s\r\n", "FDIV");
    for (size_t i = 0; i < FDIV_LENGTH; i++) {
        a.val = FDIV_TESTS[i].a;
        b.val = FDIV_TESTS[i].b;
        res.f64 = a.f64 / b.f64;
        if (res.val != FDIV_TESTS[i].res) {
            err_cnt++;
            printf_uart("FDIV[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FMUL_TESTS
    printf_uart("Testing %s\r\n", "FMUL");
    for (size_t i = 0; i < FMUL_LENGTH; i++) {
        a.val = FMUL_TESTS[i].a;
        b.val = FMUL_TESTS[i].b;
        res.f64 = a.f64 * b.f64;
        if (res.val != FMUL_TESTS[i].res) {
            err_cnt++;
            printf_uart("FMUL[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FMAX_TESTS
    // Not relevant because compiler uses flt+branch instruction instead
    printf_uart("Testing %s\r\n", "FMAX");
    for (size_t i = 0; i < FMAX_LENGTH; i++) {
        a.val = FMAX_TESTS[i].a;
        b.val = FMAX_TESTS[i].b;
        res.f64 = a.f64 > b.f64 ? a.f64: b.f64;
        if (res.val != FMAX_TESTS[i].res) {
            err_cnt++;
            printf_uart("FMAX[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FCVT_D_W_TESTS
    printf_uart("Testing %s\r\n", "DCVT_D_W");
    for (size_t i = 0; i < FCVT_D_W_LENGTH; i++) {
        a.val = FCVT_D_W_TESTS[i].a;
        b.val = FCVT_D_W_TESTS[i].b;
        res.f64 = (double)a.ibuf32[0];
        if (res.val != FCVT_D_W_TESTS[i].res) {
            err_cnt++;
            printf_uart("DCVT_D_W[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FCVT_D_WU_TESTS
    printf_uart("Testing %s\r\n", "DCVT_D_WU");
    for (size_t i = 0; i < FCVT_D_WU_LENGTH; i++) {
        a.val = FCVT_D_WU_TESTS[i].a;
        b.val = FCVT_D_WU_TESTS[i].b;
        res.f64 = (double)a.buf32[0];
        if (res.val != FCVT_D_WU_TESTS[i].res) {
            err_cnt++;
            printf_uart("DCVT_D_WU[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FCVT_W_D_TESTS
    printf_uart("Testing %s\r\n", "DCVT_W_D");
    for (size_t i = 0; i < FCVT_W_D_LENGTH; i++) {
        a.val = FCVT_W_D_TESTS[i].a;
        b.val = FCVT_W_D_TESTS[i].b;
        res.ival = (int32_t)a.f64;
        if (res.val != FCVT_W_D_TESTS[i].res) {
            err_cnt++;
            printf_uart("DCVT_W_D[%d] fail\r\n", i);
        }
    }
#endif

#ifdef ENABLE_FCVT_WU_D_TESTS
    printf_uart("Testing %s\r\n", "DCVT_WU_D");
    for (size_t i = 0; i < FCVT_WU_D_LENGTH; i++) {
        a.val = FCVT_WU_D_TESTS[i].a;
        b.val = FCVT_WU_D_TESTS[i].b;
        res.val = (uint32_t)a.f64;
        if (res.val != FCVT_WU_D_TESTS[i].res) {
            err_cnt++;
            printf_uart("DCVT_WU_D[%d] fail\r\n", i);
        }
    }
#endif

    a.f64 = 10.323;
    b.f64 = -5.3333;

    ix3 = -55;
    res.f64 = (double)ix3;
    if (res.f64 != -55.0) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DCVT_D_L");
    }

    ux3 = 75;
    res.f64 = (double)ux3;
    if (res.f64 != 75.0) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DCVT_D_LU");
    }

    res.ival = (int64_t)b.f64;
    if (res.ival != (int64_t)(-5.3333)) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DCVT_L_D");
    }

    /** Warning: conversion of negative double to unsigned integer
                 is undefined (C99/C11 6.3.1.4) and result is target dependable.
                 Hardware FPU is oriented on x86 implementation
    */
    res.val = (uint64_t)((int64_t)b.f64);
    if (res.val != (uint64_t)((int64_t)(-5.3333))) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DCVT_LU_D");
    }

    res.f64 = a.f64 * b.f64;
    if (res.f64 != (10.323 * -5.3333)) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DMUL");
    }

    res.f64 = a.f64 / b.f64;
    if (res.f64 != (10.323 / -5.3333)) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DDIV");
    }

    res.f64 = a.f64 + b.f64;
    // It supposed to work only with optimization -O0
    if (res.f64 != (10.323 - 5.3333)) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DADD");
    }

    res.f64 = a.f64 - b.f64;
    if (res.f64 != (10.323 + 5.3333)) {
        err_cnt++;
        printf_uart("%s fail\r\n", "DSUB");
    }

    a.f64 = -17.1;
    b.f64 = -17.05;
    if (a.f64 >= b.f64) {
        err_cnt++;
        printf_uart("FCMP %d fail\r\n", 1);
    }

    a.f64 = 17.1;
    b.f64 = 17.05;
    if (a.f64 < b.f64) {
        err_cnt++;
        printf_uart("FCMP %d fail\r\n", 2);
    }

    a.f64 = -17.1;
    b.f64 = 17.1;
    if (b.f64 <= a.f64) {
        err_cnt++;
        printf_uart("FCMP %d fail\r\n", 3);
    }

    a.f64 = -17.1;
    b.f64 = -17.1;
    if (b.f64 != a.f64) {
        err_cnt++;
        printf_uart("FCMP %d fail\r\n", 4);
    }

    a.f64 = 17.1;
    b.f64 = 17.1;
    if (b.f64 != a.f64) {
        err_cnt++;
        printf_uart("FCMP %d fail\r\n", 5);
    }

    printf_uart("FPU errors . . .%d\r\n", err_cnt);
}
