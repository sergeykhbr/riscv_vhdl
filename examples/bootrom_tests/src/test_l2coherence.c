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

// coherence test
void test_l2coherence(void) {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    uint32_t cpu_max = pnp->cfg >> 28;
    uint32_t l2cache_ena = (pnp->cfg >> 24) & 1;
    uint32_t cohtestcnt = 0;
    uint32_t mod40;

    pnp->fwdbg1 = (uint64_t)&cohtestcnt;     // shared variable in local stack
    pnp->fwdbg2 = fw_get_cpuid();  // to debug in RTL and see CPU index

    printf_uart("%s", "L2.Coherence . .");
    if (l2cache_ena == 0) {
        printf_uart("%s\r\n", "Disabled");
        return;
    } else if (cpu_max <= 1) {
        printf_uart("%s\r\n", "SKIPPED");
        return;
    }

    // wake-up hwthreads
    clint->msip[1] = 0x1;
    clint->msip[2] = 0x1;
    clint->msip[3] = 0x1;

    while (cohtestcnt < 75) {
        mod40 = cohtestcnt % 40;
        if (mod40 >= 0 && mod40 < 10) {
            pnp->fwdbg2 = cohtestcnt;//fw_get_cpuid();  // to debug in RTL and see CPU index
            cohtestcnt++;
        }
    }
    printf_uart("%s\r\n", "PASS");
}
