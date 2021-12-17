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

static const char TEST_PLIC_NAME[8] = "plic";

// Interrupt for the self-test that triggered on write access 
// into read-only register pnp->hwid
static const int PLIC_IRQ_PNP      = 70;

// Context indexes:
static const int CTX_CPU0_M_MODE   = 0;

void isr_plic_empty(void) {
}

void isr_plic(void) {
//    timer_data_type *p = fw_get_ram_data(TEST_TIMER_NAME);
//    gptimers_map *ptmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
//    if (++p->sec > 59) {
//        p->sec = 0;
//        if (++p->min > 59) {
//            p->min = 0;
//            if (++p->hour > 23) {
//                p->hour = 0;
//            }
//        }
//    }
//    ptmr->pending = 0;
//    // 1 Hz output. Interrupts once per sec.
//    printf_uart("Time: %02d:%02d:%02d\r\n", p->hour, p->min, p->sec);
}

void test_plic(void) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;

    int t1 = 0x1 << 11;
    asm("csrc mie, %0" : :"r"(t1));         // MEIE=0: disable external irq from PLIC

    p->src_prioirty[PLIC_IRQ_PNP] = 0x1;    // 1=lowest prioirty; 0 = disabled
    p->ctx_prio[CTX_CPU0_M_MODE].priority = 0;
    p->ctx_ie[CTX_CPU0_M_MODE].irq_enable[PLIC_IRQ_PNP/32] = 1ul << (PLIC_IRQ_PNP & 0x1F);


    // Trigger interrupt 70 (self-test) by writing into RO register HWID:
    pnp->hwid = 0;

    // Check pending bit 70 in PLIC:
    uint32_t p70 = p->pending[PLIC_IRQ_PNP/32];
    p70 >>= (PLIC_IRQ_PNP & 0x1F);
    p70 &= 0x1;

    if (p70 == 0) {
        printf_uart("FAIL: pending[70/32]=%08x not triggered\r\n",
                     p->pending[PLIC_IRQ_PNP/32]);
        return;
    }
    
    // Check claim index
    printf_uart("addr claim_complete = %08x\r\n", &p->ctx_prio[CTX_CPU0_M_MODE].claim_complete);
    p70 = p->ctx_prio[CTX_CPU0_M_MODE].claim_complete;
    if (p70 != PLIC_IRQ_PNP) {
        printf_uart("FAIL: claim_complete=%08x\r\n", p70);
        return;
    }

    // Check that pending bit was cleared
    p70 = p->pending[PLIC_IRQ_PNP/32];
    p70 >>= (PLIC_IRQ_PNP & 0x1F);
    p70 &= 0x1;
    if (p70 != 0) {
        printf_uart("FAIL: pending[70/32]=%08x not zero\r\n",
                     p->pending[PLIC_IRQ_PNP/32]);
        return;
    }

    // write complete
    p->ctx_prio[CTX_CPU0_M_MODE].claim_complete = PLIC_IRQ_PNP;
    printf_uart("PLIC . . . . . .%s", "PASS\r\n");


    // Disable interrupt and timer
//    ptmr->timer[0].init_value = 0;
//    ptmr->timer[0].control = 0;

//    fw_register_isr_handler(CFG_IRQ_GPTIMERS, isr_timer);

//    p = fw_malloc(sizeof(timer_data_type));    
//    fw_register_ram_data(TEST_TIMER_NAME, p);
//    p->sec = 0;
//    p->min = 0;
//    p->hour = 0;
 
//    ptmr->timer[0].init_value = SYS_HZ; // 1 s
//    ptmr->timer[0].control = TIMER_CONTROL_ENA | TIMER_CONTROL_IRQ_ENA;

//    fw_enable_isr(CFG_IRQ_GPTIMERS);
}

