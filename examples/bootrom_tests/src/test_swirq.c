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

#include <inttypes.h>
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

static const char TEST_SWIRQ_NAME[8] = "swirq";

void test_swirq(void) {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    uint64_t msie = 0x1ull << 3;
    uint64_t mip;

    fw_disable_m_interrupts();
 
    fw_mie_enable(HART_IRQ_MSIP);

    clint->msip[fw_get_cpuid()] = 0x1;

    // It is necessary to implement memory barrier because
    // memory access could be postponed relative next CSR read:
    pnp->fwdbg1 = 0;
    while (pnp->fwdbg1 != 0) {
        // Instead of membarier use this uncached read-write
        pnp->fwdbg1 = 0;
    }

    // Check mip[3] = msip
    asm("csrr %0, mip" : "=r" (mip));
    if (((mip >> 3) & 0x1) == 0) {
        printf_uart("FAIL: %s\r\n", "mip[3] = 0");
    }

    fw_enable_m_interrupts();

    while (pnp->fwdbg1 == 0) {}               // should be update int irq handler

    fw_mie_disable(HART_IRQ_MSIP);

    if (pnp->fwdbg1 == MAGIC_SWIRQ_TEST_NUMBER) {
        printf_uart("SWIRQ. . . . . .%s", "PASS\r\n");
    } else {
        printf_uart("SWIRQ. . . . . .%s", "FAIL\r\n");
    }

}

