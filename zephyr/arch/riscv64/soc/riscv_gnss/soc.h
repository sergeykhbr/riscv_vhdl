/*
 * Copyright (c) 2016, GNSS Sensor Ltd.
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
#include <memaccess.h>

#include "soc_registers.h"

#define VENDOR_GNSSSENSOR        0x00F1

#define GNSSSENSOR_EMPTY         0x5577     /// Dummy device
#define GNSSSENSOR_BOOTROM       0x0071     /// Boot ROM Device ID
#define GNSSSENSOR_FWIMAGE       0x0072     /// FW ROM image Device ID
#define GNSSSENSOR_SRAM          0x0073     /// Internal SRAM block Device ID
#define GNSSSENSOR_PNP           0x0074     /// Configuration Registers Module Device ID provided by gnsslib
#define GNSSSENSOR_SPI_FLASH     0x0075     /// SD-card controller Device ID provided by gnsslib
#define GNSSSENSOR_GPIO          0x0076     /// General purpose IOs Device ID provided by gnsslib
#define GNSSSENSOR_RF_CONTROL    0x0077     /// RF front-end controller Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE        0x0078     /// GNSS Engine Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE_STUB   0x0068     /// GNSS Engine stub
#define GNSSSENSOR_FSE_V2        0x0079     /// Fast Search Engines Device ID provided by gnsslib
#define GNSSSENSOR_UART          0x007a     /// rs-232 UART Device ID
#define GNSSSENSOR_ACCELEROMETER 0x007b     /// Accelerometer Device ID provided by gnsslib
#define GNSSSENSOR_GYROSCOPE     0x007c     /// Gyroscope Device ID provided by gnsslib
#define GNSSSENSOR_IRQCTRL       0x007d     /// Interrupt controller
#define GNSSSENSOR_ETHMAC        0x007f
#define GNSSSENSOR_DSU           0x0080
#define GNSSSENSOR_GPTIMERS      0x0081


#define TECH_INFERRED       0
#define TECH_VIRTEX6        36
#define TECH_KINTEX7        49



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

typedef uint64_t adr_type;

typedef struct IsrEntryType {
    uint64_t arg;
    uint64_t handler;
} IsrEntryType;

typedef void (*IRQ_HANDLER)(void *arg);


#define __PNP	((volatile struct pnp_map *)ADDR_NASTI_SLAVE_PNP)
/** @todo remove hardcoded addresses except __PNP. */
#define __UART1	((volatile struct uart_map *)ADDR_NASTI_SLAVE_UART1)
#define __IRQCTRL	((volatile struct irqctrl_map *)ADDR_NASTI_SLAVE_IRQCTRL)
#define __TIMERS ((volatile struct gptimers_map *)ADDR_NASTI_SLAVE_GPTIMERS)

/** 
 * @brief Print Plug'n'Play information
 *
 * Each devices in a SOC implements sideband signals that are connected to
 * PNP module. These signals provide such information as Vendor/Device IDs,
 * memory address and allocated memory range.
 */
extern void soc_print_pnp();

/**
 * @brief Check hardware target configuration is it inferred or not.
 *
 * inferred hardware target is used for RTL simulation of the whole SOC design.
 */
extern uint32_t soc_is_rtl_simulation();

#endif /* !_ASMLANGUAGE */

#endif /* _RISCV_SOC_GNSS_H_ */
