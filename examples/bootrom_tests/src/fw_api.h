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

#include <inttypes.h>
#include <stdarg.h>

#ifndef __TEST_NORF_SRC_GENERAL_H__
#define __TEST_NORF_SRC_GENERAL_H__

#define MAGIC_SWIRQ_TEST_NUMBER 0xaabbccdd11223344ull

typedef struct ram_data_type {
    char name[8];
    void *pattern;
} ram_data_type;

typedef struct malloc_type {
    uint64_t end;
    uint64_t allocated_sz;
    ram_data_type data[128];
    int data_cnt;
} malloc_type;

void fw_malloc_init();
void *fw_malloc(int size);
void fw_register_ram_data(const char *name, void *data);
void *fw_get_ram_data(const char *name);

typedef void (*IRQ_HANDLER)(void);

void flush_tlb();

int fw_get_cpuid();
// mstatus::mie bit controler
void fw_enable_m_interrupts();
void fw_disable_m_interrupts();
void fw_mie_enable(int idx);
void fw_mie_disable(int idx);

// plic isr table entries
void fw_enable_plic_irq(int ctxid, int idx);
void fw_disable_plic_irq(int ctxid, int idx);

void register_ext_interrupt_handler(int idx, IRQ_HANDLER f);

void print_uart(const char *buf, int sz);
void print_uart_hex(uint64_t val);
void printf_uart(const char *fmt, ... );
void uart_isr_init(void);
int uart_tx_nempty();

void led_set(int output);

#endif  // __TEST_NORF_SRC_GENERAL_H__
