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
#include "axi_maps.h"
#include "encoding.h"
#include "fw_api.h"


int hwthread2() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    uint32_t *tdata;
    uint32_t mod40;
    uint64_t msie = 0x1ull << 3;

    clint->msip[fw_get_cpuid()] = 0;          // clear SW pending bit in CLINT
    fw_mie_enable(HART_IRQ_MSIP);
    fw_enable_m_interrupts();

    // Without Coherent-L2 CPUs can't see updated value (without flush) 
    // because it is always stored in L1-cache in each processor.
    //
    // Print 'passed' only on second cycle. First cycle is the test itself.
    while (1) {
        asm("wfi");

        tdata = (uint32_t *)pnp->fwdbg1;    // thread0 should initialize SRAM location of the variable

        while (*tdata < 75) {
            mod40 = (*tdata) % 40;
            if (mod40 >= 20 && mod40 < 30) {
                pnp->fwdbg2 = fw_get_cpuid();  // to debug in RTL and see CPU index
                (*tdata)++;
            }
        }
    }

    return 0;
}
