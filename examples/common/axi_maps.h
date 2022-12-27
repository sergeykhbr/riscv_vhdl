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

#ifndef __AXI_MAPS_H__
#define __AXI_MAPS_H__

#include <inttypes.h>
#include "axi_const.h"
#include "maps/map_pnp.h"
#include "maps/map_gpio.h"
#include "maps/map_gptimers.h"
#include "maps/map_uart.h"
#include "maps/map_rfctrl.h"
#include "maps/map_gnssengine.h"
#include "maps/map_ethmac.h"
#include "maps/map_qspi.h"
#include "maps/map_fsev2.h"
#include "maps/map_plic.h"
#include "maps/map_clint.h"
#include "maps/map_mpu.h"
#include "maps/map_prci.h"

#define ADDR_BUS0_XSLV_CLINT        0x02000000 // Core-local interruptor (CLINT)
#define ADDR_BUS0_XSLV_SRAM         0x08000000 // 0x0800_0000..0x081F_FFFF = L2 Cache Controller
#define ADDR_BUS0_XSLV_FWIMAGE      0x09000000 // ROM FU740 compatible
#define ADDR_BUS0_XSLV_PLIC         0x0C000000 // FU740 compatible
#define ADDR_BUS0_XSLV_UART0        0x10010000 // 0x10000000: qemu, 0x10010000: FU740 compatible
#define ADDR_BUS0_XSLV_UART1        0x10011000 // FU740 compatible
#define ADDR_BUS1_APB_PRCI          0x10012000 // not compatible with FU740
#define ADDR_BUS1_APB_QSPI2         0x10050000 // FU740 compatible
#define ADDR_BUS0_XSLV_GPIO         0x10060000 // FU740 compatible
#define ADDR_BUS0_XSLV_OTP          0x10070000
#define ADDR_BUS0_XSLV_ETHMAC       0x10090000 // 0x1009_0000 .. 0x1009_1FFF Ethernet on FU740
// GNSS Sub System
#define ADDR_BUS0_XSLV_RF_CTRL      0x100f0000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_GNSS_SS      0x100f1000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_FSE          0x100f2000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_PNP          0x100ff000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_DDR          0x0000000080000000ull // DDR memory banks

// Hart irq bus
#define HART_IRQ_SSIP 1
#define HART_IRQ_MSIP 3
#define HART_IRQ_STIP 5
#define HART_IRQ_MTIP 7
#define HART_IRQ_SEIP 9
#define HART_IRQ_MEIP 11
#define HART_IRQ_TOTAL 16

// Interrupt pins assignemts:
#define CFG_IRQ_UNUSED      0
#define CFG_IRQ_GPIO_0      23   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_1      24   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_2      25   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_3      26   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_4      27   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_5      28   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_6      29   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_7      30   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_8      31   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_9      32   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_10     33   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_11     34   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_12     35   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_13     36   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_14     37   // The same in 740 (unmatched)
#define CFG_IRQ_GPIO_15     38   // The same in 740 (unmatched)
#define CFG_IRQ_UART0       39   // The same in 740 (unmatched)
#define CFG_IRQ_UART1       40   // The same in 740 (unmatched)
// Interrupt for the self-test that triggered on write access 
// into read-only register pnp->hwid
#define PLIC_IRQ_PNP        70
#define CFG_IRQ_GNSS_SS     71
#define PLIC_ISR_MAX        73 // Any number up to 1024

#ifndef __ASSEMBLER__
#ifdef __GNUC__

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
  asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "rK"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define rdtime() read_csr(time)
#define rdcycle() read_csr(cycle)
#define rdinstret() read_csr(instret)

#endif // __GNUC__
#endif // __ASSEMBLER__

#endif  // __AXI_MAPS_H__
