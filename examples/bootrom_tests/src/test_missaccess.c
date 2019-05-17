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
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

static const uint64_t UNMAPPED_ADDRESS = 0x70000040;

void isr_miss_access(void) {
    uint64_t *ma_reg = (uint64_t *)0x80098010;

    printf_uart("missaccess isr .0x%08x\r\n", *ma_reg);
}

void test_missaccess(void) {
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    uint64_t *ma_reg = (uint64_t *)0x80098010;     // DSU register

    register_isr_handler(CFG_IRQ_MISS_ACCESS, isr_miss_access);
    enable_isr(CFG_IRQ_MISS_ACCESS);

    // Read unmapped address
    pnp->fwdbg1 = *((uint64_t *)UNMAPPED_ADDRESS);

    // Accessed unmapped address should be latched by DSU
    if (*ma_reg != UNMAPPED_ADDRESS) {
        printf_uart("missaccess . . .FAIL: %08x != %08x\r\n",
                    *ma_reg, UNMAPPED_ADDRESS);
    } else {
        printf_uart("%s", "missaccess . . .PASS\r\n");
    }
}
