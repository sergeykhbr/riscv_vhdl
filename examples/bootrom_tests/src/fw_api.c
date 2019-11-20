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

void fw_enable_isr(int idx) {
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    uint32_t msk = p_irq->irq_mask;
    msk &= ~(1ul << idx);
    p_irq->irq_mask = msk;
}

void fw_disable_isr(int idx) {
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    p_irq->irq_mask |= (1ul << idx);
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
