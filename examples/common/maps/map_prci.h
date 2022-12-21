/*
 *  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once
#include <inttypes.h>

#define PRCI_DDR_STATUS_CALIB_DONE (0x1 << 0)

typedef struct prci_map {
    uint32_t pll_status;              /// 0x000: RO: PLL Status
    uint32_t rst_status;              /// 0x004: RO: Reset Statuses
    uint32_t ddr_status;              /// 0x008: RO: DDR controller status
} prci_map;


