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

uint32_t plic_claim(int ctxid);
void plic_complete(int ctxid, int irqid);


void plic_enable_irq(int ctxid, int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    p->src_prioirty[irqidx] = 0x1;    // 1=lowest prioirty; 0 = disabled
#ifdef PLIC_MODE_ENABLED
    p->src_mode[irqidx] = PLIC_MODE_RISING_EDGE;
#endif
    p->ctx_prio[ctxid].priority = 0;
    p->ctx_ie[ctxid].irq_enable[irqidx/32] = 1ul << (irqidx & 0x1F);
}

uint32_t plic_is_pending(int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    uint32_t ret = p->pending[irqidx/32];
    ret >>= (irqidx & 0x1F);
    ret &= 0x1;
    return ret;
}


void test_generate_interrupt() {
    // Trigger interrupt 70 (self-test) by writing into RO register HWID:
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    pnp->hwid = 0;
}


void test_plic(void) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;

    fw_disable_m_interrupts();
    fw_mie_enable(HART_IRQ_MEIP);

    plic_enable_irq(CTX_CPU0_M_MODE, PLIC_IRQ_PNP);

    test_generate_interrupt();

    if (plic_is_pending(PLIC_IRQ_PNP) == 0) {
        printf_uart("FAIL: pending[70/32]=%08x not triggered\r\n",
                     p->pending[PLIC_IRQ_PNP/32]);
        return;
    }

    uint64_t mip;
    asm("csrr %0, mip" : "=r" (mip));

    // Check mip[11] = meip
    if (((mip >> 11) & 0x1) == 0) {
        printf_uart("FAIL: %s\r\n", "mip[11] = 0");
    }
    
    // Check claim index
    uint32_t irqid = plic_claim(CTX_CPU0_M_MODE);
    if (irqid != PLIC_IRQ_PNP) {
        printf_uart("FAIL: claim_complete=%08x\r\n", irqid);
        return;
    }

    // Check that pending bit was cleared
    if (plic_is_pending(PLIC_IRQ_PNP) != 0) {
        printf_uart("FAIL: pending[70/32]=%08x not zero\r\n",
                     p->pending[PLIC_IRQ_PNP/32]);
        return;
    }

    // write complete
    plic_complete(CTX_CPU0_M_MODE, irqid);
    printf_uart("PLIC . . . . . .%s", "PASS\r\n");

    fw_mie_disable(HART_IRQ_MEIP);
    fw_enable_m_interrupts();
}

