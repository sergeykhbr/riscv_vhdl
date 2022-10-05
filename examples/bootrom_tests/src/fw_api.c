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

void flush_tlb()
{
    asm volatile ("sfence.vma");
}

// external interrupts
void fw_enable_m_interrupts() {
    uint64_t t1 = 0x00000008;
    asm("csrs mstatus, %0" : :"r"(t1));  // set mie
}

void fw_disable_m_interrupts() {
    uint64_t t1 = 0x00000008;
    asm("csrc mstatus, %0" : :"r"(t1));  // clear mie
}

void fw_mie_enable(int idx) {
    int xie = 1ull << idx;
    asm("csrs mie, %0" : :"r"(xie));
}

void fw_mie_disable(int idx) {
    int xie = 1ull << idx;
    asm("csrc mie, %0" : :"r"(xie));
}

void fw_enable_plic_irq(int ctxid, int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    p->src_prioirty[irqidx] = 0x1;    // 1=lowest prioirty; 0 = disabled
#ifdef PLIC_MODE_ENABLED
    p->src_mode[irqidx] = PLIC_MODE_RISING_EDGE;
#endif
    p->ctx_prio[ctxid].priority = 0;
    p->ctx_ie[ctxid].irq_enable[irqidx/32] = 1ul << (irqidx & 0x1F);
}

void fw_disable_plic_irq(int ctxid, int irqidx) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    p->src_prioirty[irqidx] = 0;    // 1=lowest prioirty; 0 = disabled
}

void led_set(int output) {
    // [3:0] DIP pins
    ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->output_val = (output << 4);
}

