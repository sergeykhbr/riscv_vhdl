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
#include <encoding.h>
#include <fw_api.h>
#include <axi_maps.h>

void setup_pmp(void)
{
  // Set up a PMP to permit access to all of memory.
  // Ignore the illegal-instruction trap if PMPs aren't supported.
  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  asm volatile ("la t0, 1f\n\t"
                "csrrw t0, mtvec, t0\n\t"
                "csrw pmpaddr0, %1\n\t"
                "csrw pmpcfg0, %0\n\t"
                ".align 2\n\t"
                "1: csrw mtvec, t0"
                : : "r" (pmpc), "r" (-1UL) : "t0");
}

void protect_memory(void)
{
  // Check to see if up to four PMP registers are implemented.
  // Ignore the illegal-instruction trap if PMPs aren't supported.
  uintptr_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, tmp, cfg;
  asm volatile ("la %[tmp], 1f\n\t"
                "csrrw %[tmp], mtvec, %[tmp]\n\t"
                "csrw pmpaddr0, %[m1]\n\t"
                "csrr %[a0], pmpaddr0\n\t"
                "csrw pmpaddr1, %[m1]\n\t"
                "csrr %[a1], pmpaddr1\n\t"
                "csrw pmpaddr2, %[m1]\n\t"
                "csrr %[a2], pmpaddr2\n\t"
                "csrw pmpaddr3, %[m1]\n\t"
                "csrr %[a3], pmpaddr3\n\t"
                ".align 2\n\t"
                "1: csrw mtvec, %[tmp]"
                : [tmp] "=&r" (tmp),
                  [a0] "+r" (a0), [a1] "+r" (a1), [a2] "+r" (a2), [a3] "+r" (a3)
                : [m1] "r" (-1UL));

  // We need at least four PMP registers to protect M-mode from S-mode.
  if (!(a0 & a1 & a2 & a3))
    return setup_pmp();

  // Prevent S-mode access to our part of memory.
  extern char _ftext, _end;
  a0 = (uintptr_t)&_ftext >> PMP_SHIFT;
  a1 = (uintptr_t)&_end >> PMP_SHIFT;
  cfg = PMP_TOR << 8;
  // Give S-mode free rein of everything else.
  a2 = -1;
  cfg |= (PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 16;
  // No use for PMP 3 just yet.
  a3 = 0;

  // Plug it all in.
  asm volatile ("csrw pmpaddr0, %[a0]\n\t"
                "csrw pmpaddr1, %[a1]\n\t"
                "csrw pmpaddr2, %[a2]\n\t"
                "csrw pmpaddr3, %[a3]\n\t"
                "csrw pmpcfg0, %[cfg]"
                :: [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3),
                   [cfg] "r" (cfg));
}

#define W32_NO_INTERRUPT 0x33445566

// Enable U,S,MPRV for all regions
void init_pmp() {
    uint64_t a0 = -1;  // Give S-mode free rein of everything else.
    uint64_t cfg = (PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 0; // [] channel: whole memory range

    // Plug it all in.
    asm volatile ("csrw pmpaddr0, %[a0]\n\t"
                  "csrw pmpcfg0, %[cfg]"
                  :: [a0] "r" (a0),
                     [cfg] "r" (cfg));
}

// Restore state before the test:
void restore_state(uint64_t vec, uint64_t status) {
    asm volatile ("csrw mtvec, %[vec]\n\t"
                  "csrw mstatus, %[status]\n\t"
                  "csrw pmpcfg0, %[cfg]"
                  :: [vec] "r" (vec),
                     [status] "r" (status),
                     [cfg] "r" (0));
    init_pmp();

}

typedef struct pmp_type {
    uint64_t mtvec;
    uint64_t mstatus;
    volatile uint32_t r_only;
    volatile uint32_t *p_r_only;
    volatile uint64_t w_only;
} pmp_type;

void test_pmp() {
    uintptr_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, tlabel;
    uint64_t cfg;
    uint32_t wValue;

    pmp_type *pmp = (pmp_type *)fw_malloc(sizeof(pmp_type));
    fw_register_ram_data("pmp", pmp);

    printf_uart("%s", "PMP. . . . . . .");

    pmp->p_r_only = &pmp->r_only;
    asm volatile ("mv t0, %[p_r_only]\n\t" : [p_r_only] "+r" (pmp->p_r_only));
    pmp->mstatus = read_csr(mstatus);

    asm volatile ("csrr %[mtvec_z], mtvec\n\t" :
                  [mtvec_z] "=r" (pmp->mtvec));

    a0 = (uintptr_t)&pmp->r_only >> PMP_SHIFT;
    a1 = (uintptr_t)&pmp->w_only >> PMP_SHIFT;
    a2 = -1;  // Give S-mode free rein of everything else.

    cfg = (PMP_NA4 | PMP_R) << 0;     // [0] channel 0: 4 bytes region
    cfg |= (PMP_NAPOT | PMP_W) << 8;  // [1] channel 1: 8 bytes region
    cfg |= (PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 16; // [2] channel: whole memory range

    // Plug it all in.
    asm volatile ("csrw pmpaddr0, %[a0]\n\t"
                  "csrw pmpaddr1, %[a1]\n\t"
                  "csrw pmpaddr2, %[a2]\n\t"
                  "csrw pmpcfg0, %[cfg]"
                  :: [a0] "r" (a0), [a1] "r" (a1), [a2] "r" (a2),
                     [cfg] "r" (cfg));

    // Check read32 write without interrupt
    wValue = W32_NO_INTERRUPT;
    *pmp->p_r_only = 0;
    asm volatile ("la %[tlabel], 1f\n\t"
                  "csrrw %[tlabel], mtvec, %[tlabel]\n\t"
                  "sw %[wValue],0(t0)\n\t"
                  "fence.i\n\t"
                  "j 2f\n\t"
                  ".align 2\n\t"
                  "1: mv %[wValue], %[m1]\r\n"
                  "2: nop\r\n"
                : [tlabel] "=&r" (tlabel),
                  [wValue] "+r" (wValue)
                : [m1] "r" (-1U));

    if (wValue != W32_NO_INTERRUPT || *pmp->p_r_only != W32_NO_INTERRUPT) {
        printf_uart("FAIL(1), wValue=%08x r_only=%08x\r\b", wValue, *pmp->p_r_only);
        restore_state(pmp->mtvec, pmp->mstatus);
        return;
    }

    uint64_t mstatus_s = pmp->mstatus;
    mstatus_s &= ~(MSTATUS_MPP_M);
    mstatus_s |= MSTATUS_MPP_S;  // set MPP to S-mode
    mstatus_s |= MSTATUS_MPRV;
    write_csr(mstatus, mstatus_s);

    // Check write32 and read32 access to RO with interrupt
    wValue = W32_NO_INTERRUPT;
    asm volatile ("la %[tlabel], 1f\n\t"
                  "csrrw %[tlabel], mtvec, %[tlabel]\n\t"
                  "fence.i\n\t"
                  "sw %[wValue],0(t0)\n\t"
                  "fence.i\n\t"
                  "j 2f\n\t"
                  ".align 2\n\t"
                  "1: mv %[wValue], %[m1]\r\n"
                  "2: nop\r\n"
                : [tlabel] "=&r" (tlabel),
                  [wValue] "+r" (wValue)
                : [m1] "r" (-1U));

    if (wValue != ~0 || *pmp->p_r_only != W32_NO_INTERRUPT) {
        printf_uart("FAIL(2), wValue=%08x, r_only=%08x\r\n", wValue, *pmp->p_r_only);
        restore_state(pmp->mtvec, pmp->mstatus);
        return;
    }

    restore_state(pmp->mtvec, pmp->mstatus);
    printf_uart("%s", "PASS\r\n");
}
