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

// Number of context in PLIC controller.
// Example FU740: S7 Core0 (M) + 4xU74 Cores (M+S).
static const int CFG_PLIC_CONTEXT_TOTAL = 9;
// Any number up to 1024. Zero interrupt must be 0.
static const int CFG_PLIC_IRQ_TOTAL = 73;

}  // namespace debugger

