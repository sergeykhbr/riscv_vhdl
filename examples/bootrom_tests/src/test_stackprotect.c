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

void recursive_call() {
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    if (pnp->fwdbg1 < 10) {
        pnp->fwdbg1++;
        recursive_call();
    }
}

void test_stackprotect(void) {
    uint64_t sp;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    // clear register. it should be modified from exception handler
    pnp->fwdbg1 = 0;

    asm("mv %0, sp" : "=r" (sp));

    sp -= 4;
    asm("csrw 0x350, %0": : "r"(sp));

    recursive_call();

    printf_uart("%s %08x\r\n", "stack ovr. . . .", pnp->fwdbg1);
}
