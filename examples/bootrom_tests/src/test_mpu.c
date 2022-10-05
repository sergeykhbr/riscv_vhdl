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
