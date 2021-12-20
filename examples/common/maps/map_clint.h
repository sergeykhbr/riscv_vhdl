/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#define CLINT_HART_MAX   4096

typedef struct clint_map {
    volatile uint32_t msip[CLINT_HART_MAX];         // [0x0000] MSIP for Hart n
    volatile uint64_t mtimecmp[CLINT_HART_MAX-1];   // [0x0400] mtimecmp for hart n
    volatile uint64_t mtime;                        // [0xbff8] timer register
} clint_map;

