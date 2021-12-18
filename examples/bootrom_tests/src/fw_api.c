/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

int fw_get_cpuid() {
    int ret;
    asm("csrr %0, mhartid" : "=r" (ret));
    return ret;
}

// external interrupts
void fw_disable_interrupts() {
    uint64_t t1 = 0x00000008;
    asm("csrc mstatus, %0" : :"r"(t1));  // clear mie
}


void fw_register_isr_handler(int idx, IRQ_HANDLER f) {
    irqctrl_map *p_irqctrl = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    IRQ_HANDLER *tbl;
    if (p_irqctrl->isr_table == 0) {
        p_irqctrl->isr_table = 
            (uint64_t)fw_malloc(CFG_IRQ_TOTAL * sizeof(IRQ_HANDLER));
    }
    tbl = (IRQ_HANDLER *)p_irqctrl->isr_table;
    tbl[idx] = f;
}

void fw_enable_isr(int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    int ctxid = 0;
    p->src_prioirty[irqidx] = 0x1;    // 1=lowest prioirty; 0 = disabled
#ifdef PLIC_MODE_ENABLED
    p->src_mode[irqidx] = PLIC_MODE_RISING_EDGE;
#endif
    p->ctx_prio[ctxid].priority = 0;
    p->ctx_ie[ctxid].irq_enable[irqidx/32] = 1ul << (irqidx & 0x1F);
}

void fw_disable_isr(int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    int ctxid = 0;
    p->src_prioirty[irqidx] = 0;    // 1=lowest prioirty; 0 = disabled
}

void led_set(int output) {
    // [3:0] DIP pins
    ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->ouser = (output << 4);
}

int is_simulation() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    if (pnp->tech & 0xFF) {
        // not inferred
        return 0;
    }
    return 1;
}
