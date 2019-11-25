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
#include "maps/map_irqctrl.h"
#include "maps/map_rfctrl.h"
#include "maps/map_gnssengine.h"
#include "maps/map_ethmac.h"
#include "maps/map_spiflash.h"
#include "maps/map_fsev2.h"

#define ADDR_BUS0_XSLV_OTP          0x00010000
#define ADDR_BUS0_XSLV_FWIMAGE      0x00100000
#define ADDR_BUS0_XSLV_EXTFLASH     0x00200000
#define ADDR_BUS0_XSLV_SRAM         0x10000000
#define ADDR_BUS0_XSLV_GPIO         0x80000000
#define ADDR_BUS0_XSLV_UART1        0x80001000
#define ADDR_BUS0_XSLV_IRQCTRL      0x80002000
#define ADDR_BUS0_XSLV_GPTIMERS     0x80005000
#define ADDR_BUS0_XSLV_GNSS_SS      0x80008000
#define ADDR_BUS0_XSLV_ETHMAC       0x80040000
#define ADDR_BUS0_XSLV_PNP          0xfffff000
// GNSS Sub System
#define ADDR_GNSS_SS_RFCTRL         0x80008000
#define ADDR_GNSS_SS_ENGINE         0x80009000
#define ADDR_GNSS_SS_FSEGPS         0x8000a000

// Interrupt pins assignemts:
#define CFG_IRQ_UNUSED      0
#define CFG_IRQ_UART1       1
#define CFG_IRQ_ETHMAC      2
#define CFG_IRQ_GPTIMERS    3
#define CFG_IRQ_GNSS_SS     4
#define CFG_IRQ_TOTAL       5


#endif  // __AXI_MAPS_H__
