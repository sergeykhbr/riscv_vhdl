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

/** Function to check Stack Overflow exception */
void recursive_call() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    if (pnp->fwdbg1 < 10 && pnp->fwdbg2 == 0) {
        pnp->fwdbg1++;
        recursive_call();
    }
}

/** Function to check Stack Underflow exception */
void recursive_ret() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uint64_t sp;
    if (pnp->fwdbg1 != 0) {
        pnp->fwdbg1--;
        recursive_ret();
    } else {
        // Read current value of the Stack Ponter
        asm("mv %0, sp" : "=r" (sp));
        // Write CSR_mstackund register as underflow border
        sp += 16*sizeof(uint64_t);         // stack underflow limit
        asm("csrw 0xBC1, %0": : "r"(sp));
    }
}

void test_stackprotect(void) {
    uint64_t sp;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    // clear register. it should be modified from exception handler
    pnp->fwdbg1 = 0;
    pnp->fwdbg2 = 0;

    uint64_t t1 = 0x00000008;

    // Read current value of the Stack Ponter
    asm("mv %0, sp" : "=r" (sp));

    // Write CSR_mstackovr register as overflow border
    sp -= 16*sizeof(uint64_t);         // stack overflow limit
    asm("csrw 0xBC0, %0": : "r"(sp));

    asm("csrc mstatus, %0" : :"r"(t1));  // clear mie
    recursive_call();
    asm("csrs mstatus, %0" : :"r"(t1));  // enable mie

    /** Check result:
          If the StackOverflow exception was called then fwdbg2 must be
        non-zero. This should happen before fwdbg1 counter reaches 10.
    */
    printf_uart("%s", "stack_ovr. . . .");
    if (pnp->fwdbg1 > 1 && pnp->fwdbg1 < 10 && pnp->fwdbg2) {
        printf_uart("%s", "PASS\r\n");
    } else {
        printf_uart("FAIL: %08x, %08x\r\n", pnp->fwdbg1, pnp->fwdbg2);
    }


    // Test Stack Underflow exception
    pnp->fwdbg2 = 0;

    asm("csrc mstatus, %0" : :"r"(t1));  // clear mie
    recursive_ret();
    asm("csrs mstatus, %0" : :"r"(t1));  // enable mie

    printf_uart("%s", "stack_und. . . .");
    if (pnp->fwdbg2) {
        printf_uart("%s", "PASS\r\n");
    } else {
        printf_uart("FAIL: %08x\r\n", pnp->fwdbg2);
    }

}
