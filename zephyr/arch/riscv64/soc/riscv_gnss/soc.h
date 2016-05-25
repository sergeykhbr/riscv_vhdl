/*
 * Copyright (c) 2016 Intel Corporation.
 * Copyright (c) 2013-2015 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file SoC configuration macros for the GNSS Sensor Risc-V family processors.
 */

#ifndef _RISCV_SOC_GNSS_H_
#define _RISCV_SOC_GNSS_H_

#include <device.h>
#include <misc/util.h>
#include <drivers/rand32.h>

#include "soc_registers.h"


#define ADDR_NASTI_SLAVE_FWIMAGE    0x00100000
#define ADDR_NASTI_SLAVE_SRAM       0x10000000
#define ADDR_NASTI_SLAVE_GPIO       0x80000000
#define ADDR_NASTI_SLAVE_UART1      0x80001000
#define ADDR_NASTI_SLAVE_IRQCTRL    0x80002000
#define ADDR_NASTI_SLAVE_GNSSENGINE 0x80003000
#define ADDR_NASTI_SLAVE_RFCTRL     0x80004000
#define ADDR_NASTI_SLAVE_GPTIMERS   0x80005000
#define ADDR_NASTI_SLAVE_FSEGPS     0x8000a000
#define ADDR_NASTI_SLAVE_ETHMAC     0x80040000
#define ADDR_NASTI_SLAVE_PNP        0xfffff000

/* Interrupts pins assignments */
#define CFG_IRQ_GNSS_ENGINE 0
#define CFG_IRQ_UART1       1
#define CFG_IRQ_ETH         2
#define CFG_IRQ_SYS_TIMER   3

/* Use this general purpose timer as a system timer */
#define CFG_SYS_TIMER_IDX 0

/* PNP */
#define PNP_CONFIG_DEFAULT_BYTES 16

/* uart configuration settings */
#define UART_IRQ_FLAGS 0    // used in console driver


#ifndef _ASMLANGUAGE


typedef struct IsrEntryType {
    uint64_t arg;
    uint64_t handler;
} IsrEntryType;

typedef void (*IRQ_HANDLER)(void *arg);


#define __PNP	((volatile struct pnp_map *)ADDR_NASTI_SLAVE_PNP)
#define __UART1	((volatile struct uart_map *)ADDR_NASTI_SLAVE_UART1)
#define __IRQCTRL	((volatile struct irqctrl_map *)ADDR_NASTI_SLAVE_IRQCTRL)
#define __TIMERS ((volatile struct gptimers_map *)ADDR_NASTI_SLAVE_GPTIMERS)

extern uint32_t READ32(volatile uint32_t *addr);
extern uint64_t READ64(volatile uint64_t *addr);
extern void WRITE32(volatile uint32_t *addr, uint32_t val);
extern void WRITE64(volatile uint64_t *addr, uint64_t val);

#ifdef _WIN32
extern void LIBH_write(uint64_t addr, uint8_t *buf, int size);
extern void LIBH_read(uint64_t addr, uint8_t *buf, int size);
extern void LIBH_swap(uint64_t tc_addr);
extern void LIBH_swap_preemptive(uint64_t tc_addr);
#endif

#endif /* !_ASMLANGUAGE */

#endif /* _RISCV_SOC_GNSS_H_ */
