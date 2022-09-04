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

static const char INTERRUPT_TABLE_NAME[8] = "irqtbl";

typedef union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 63;   // 11 - Machine external interrupt
        uint64_t irq    : 1;
    } bits;
    uint64_t value;
} csr_mcause_type;

void allocate_interrupt_table() {
    IRQ_HANDLER *tbl = (IRQ_HANDLER *)
        fw_malloc(PLIC_ISR_MAX * sizeof(IRQ_HANDLER));    
    fw_register_ram_data(INTERRUPT_TABLE_NAME, tbl);
    for (int i = 0; i < PLIC_ISR_MAX; i++) {
        tbl[i] = 0;
    }
}

void register_ext_interrupt_handler(int idx, IRQ_HANDLER f) {
    IRQ_HANDLER *tbl = 
       (IRQ_HANDLER *)fw_get_ram_data(INTERRUPT_TABLE_NAME);
    tbl[idx] = f;
}


/** Fatal Error Handler can be used to transmit dump image
 *  or trigger watchdog
 */
void fatal_error() {
    led_set(0xF0);
    printf_uart("fatal_error()\r\n");
    while (1) {}
}

uint32_t plic_claim(int ctxid) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    return p->ctx_prio[ctxid].claim_complete;
}

void plic_complete(int ctxid, int irqid) {
    plic_map *p = (plic_map *)ADDR_BUS0_XSLV_PLIC;
    p->ctx_prio[ctxid].claim_complete = irqid;
}

void interrupt_s_software_c() {
}

void interrupt_m_software_c() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;

    clint->msip[fw_get_cpuid()] = 0x0;     // clear pending bit
    if (fw_get_cpuid() == 0) {
        pnp->fwdbg1 = MAGIC_SWIRQ_TEST_NUMBER; // to pass test write this value
    }
}

void interrupt_s_timer_c() {
}

void interrupt_m_timer_c() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    pnp->fwdbg1 = clint->mtime;

    // just to give time before next interrupt
    clint->mtimecmp[fw_get_cpuid()] = clint->mtime + 10000;
}

void interrupt_s_external_c() {
    IRQ_HANDLER *tbl = (IRQ_HANDLER *)fw_get_ram_data(INTERRUPT_TABLE_NAME);
    uint32_t irqid = plic_claim(CTX_CPU0_S_MODE);
    if (tbl[irqid]) {
        tbl[irqid]();
    } else {
        fatal_error();
    }
    plic_complete(CTX_CPU0_S_MODE, irqid);
}

void interrupt_m_external_c() {
    IRQ_HANDLER *tbl = (IRQ_HANDLER *)fw_get_ram_data(INTERRUPT_TABLE_NAME);
    uint32_t irqid = plic_claim(CTX_CPU0_M_MODE);
    if (tbl[irqid]) {
        tbl[irqid]();
    } else {
        fatal_error();
    }
    plic_complete(CTX_CPU0_M_MODE, irqid);
}


/** Interrupt handler in non-vector mode (MODE=0) */
long interrupt_handler_c(long cause, long epc, long long regs[32]) {
    csr_mcause_type mcause;
    mcause.value = cause;

    if (mcause.bits.irq == 0x1 && mcause.bits.code == 11) {
         IRQ_HANDLER *tbl = (IRQ_HANDLER *)fw_get_ram_data(INTERRUPT_TABLE_NAME);
         uint32_t irqid = plic_claim(CTX_CPU0_M_MODE);
         if (tbl[irqid]) {
             tbl[irqid]();
         } else {
             fatal_error();
         }
         plic_complete(CTX_CPU0_M_MODE, irqid);
    } else {
       print_uart("mcause:", 7);
       print_uart_hex(cause);
       print_uart(",mepc:", 6);
       print_uart_hex(epc);
       print_uart("\r\n", 2);
       /// Exception trap
       led_set(0xF0);
       while (1) {}
    }

    return epc;
}
