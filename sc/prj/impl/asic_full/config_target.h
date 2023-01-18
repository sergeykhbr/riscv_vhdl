// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>

namespace debugger {

static const bool CFG_ASYNC_RESET = 0;

// @brief   Number of processors in a system
// @details This value may be in a range 1 to CFG_TOTAL_CPU_MAX-1
static const int CFG_CPU_NUM = 1;

// @brief Caches size parameters.
// @note Caches line size configured in river_cfg file and affects L1 memory bus width.
static const int CFG_ILOG2_LINES_PER_WAY = 7;               // I$ length: 7=16KB; 8=32KB; ..
static const int CFG_ILOG2_NWAYS = 2;                       // I$ associativity. Default bits width = 2, means 4 ways

static const int CFG_DLOG2_LINES_PER_WAY = 7;               // D$ length: 7=16KB; 8=32KB; ..
static const int CFG_DLOG2_NWAYS = 2;                       // D$ associativity. Default bits width = 2, means 4 ways

// @brief Enable/disable L2 caching. L2 can be enabled even in 1 CPU config
static const int CFG_L2CACHE_ENA = 1;
static const int CFG_L2_LOG2_NWAYS = 4;
static const int CFG_L2_LOG2_LINES_PER_WAY = 9;             // 7=16KB; 8=32KB; 9=64KB, ..

// Internal Boot ROM size:
static const int CFG_BOOTROM_LOG2_SIZE = 16;                // 16=64 KB (default); 17=128KB; ..

// Internal SRAM block:
//     - Increase memory map if need > 2MB FU740
//     - Change bootloader stack pointer if need less than 512 KB
static const int CFG_SRAM_LOG2_SIZE = 21;                   // 19=512 KB (KC705); 21=2 MB (ASIC); ..

// UART simulation speed-up rate. Directly use as a divider for the 'scaler' register
// 0=no speed-up, 1=2x speed, 2=4x speed, 3=8x speed, 4=16x speed, .. etc
static const int CFG_UART_SPEED_UP_RATE = 3;

}  // namespace debugger

