#pragma once

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

#include <inttypes.h>

typedef struct mpu_ctrl_wrbits_type {
    uint64_t WR    : 1;     // [0]
    uint64_t RD    : 1;     // [1]
    uint64_t EXEC  : 1;     // [2]
    uint64_t CACHABLE : 1;  // [3]
    uint64_t ENA    : 1;    // [4]
    uint64_t rsrv1  : 2;    // [6:5]
    uint64_t WE     : 1;    // [7] write entry into MPU
    uint64_t IDX    : 8;    // [15:8]
    uint64_t rsrv2  : 48;   // 
} mpu_ctrl_wrbits_type;

typedef union mpu_ctrl_type {
    mpu_ctrl_wrbits_type bits;
    uint64_t value;
} mpu_ctrl_type;

// Read CSR_MPU_ctrl vendor specific register
static int mpu_region_total() {
    uint64_t val = 0;
    asm("csrr %0, 0xBC4" : "=r" (val));
    return (int)(val >> 8);
}

static void mpu_disable_region(int idx) {
    mpu_ctrl_type ctrl;
    ctrl.value = 0;
    ctrl.bits.IDX = idx;
    ctrl.bits.WE = 1;
    asm("csrw 0xBC4, %0" : :"r"(ctrl.value));
}

static void mpu_enable_region(int idx,
                       uint64_t bar,
                       uint64_t KB,
                       int cached,
                       const char *rwx) {
    uint64_t mask = (~0ull) << 10;
    const char *p = rwx;
    mpu_ctrl_type ctrl;

    asm("csrw 0xBC2, %0" : :"r"(bar));

    KB >>= 1;
    while (KB) {
        mask <<= 1;
        KB >>= 1;
    }
    asm("csrw 0xBC3, %0" : :"r"(mask));

    ctrl.value = 0;
    ctrl.bits.IDX = idx;
    ctrl.bits.ENA = 1;
    ctrl.bits.CACHABLE = cached;
    ctrl.bits.WE = 1; // write into MPU
    while (*p) {
        if (*p == 'r') {
            ctrl.bits.RD = 1;
        }
        if (*p == 'w') {
            ctrl.bits.WR = 1;
        }
        if (*p == 'x') {
            ctrl.bits.EXEC = 1;
        }
        p++;
    }
    asm("csrw 0xBC4, %0" : :"r"(ctrl.value));
}
