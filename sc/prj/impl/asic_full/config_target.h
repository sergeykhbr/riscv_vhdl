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
// @brief Enable/disable L2 caching. L2 can be enabled even in 1 CPU config
static const int CFG_L2CACHE_ENA = 1;

}  // namespace debugger

