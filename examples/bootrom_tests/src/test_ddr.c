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

int test_ddr(void) {
    prci_map *prci = (prci_map *)ADDR_BUS1_APB_PRCI;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    uint64_t *ddr = (uint64_t *)ADDR_BUS0_XSLV_DDR;

    printf_uart("%s", "DDR Init . .");

    uint64_t t_start = clint->mtime;

    while ((prci->ddr_status & PRCI_DDR_STATUS_CALIB_DONE) == 0) {
        // 3 seconds timeout:
        if ((clint->mtime - t_start) > 3 * SYS_HZ) {
            printf_uart("%s", "NO_CALIB\r\n");
            return -1;
        }
    }
    printf_uart("%s", "DONE\r\n");

    // Cache L1 associativity is 4, write more than 4 lines to trigger cache offloading.    
    ddr[0] = 0x1122334455667788ull;
    ddr[1] = 0xffeeddccbbaa9988ull;
    for (int i = 1; i < 18; i++) {
        ddr[i*1024*1024] = 0xcafef00ddead0001ull + 0x100*i;
    }
    printf_uart("DDR[0] . . . . .0x%016llx expected 0x1122334455667788ull\r\n", ddr[0]);
    printf_uart("DDR[1] . . . . .0x%016llx expected 0xffeeddccbbaa9988ull\r\n", ddr[1]);
    printf_uart("DDR[9MB] . . . .0x%016llx expected 0xcafef00ddead0901ull\r\n", ddr[9*1024*1024]);

    int err = 0;

    if (ddr[0] != 0x1122334455667788ull) {
        err = 1;
    }
    if (err == 0 && ddr[1] != 0xffeeddccbbaa9988ull) {
        err = 2;
    }
    if (err == 0) {
        for (int i = 1; i < 18; i++) {
            if (ddr[i*1024*1024] != 0xcafef00ddead0001ull + 0x100*i) {
                err = 2 + i;
                break;
            }
        }
    }

    if (err == 0) {
        printf_uart("DDR. . . . . . .%s\r\n", "PASS");
        return 0;
    }
    printf_uart("DDR. . . . . . .FAILED,err=%d\r\n", err);
    return -2;
}

