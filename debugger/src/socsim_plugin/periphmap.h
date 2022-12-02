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

#ifndef __DEBUGGER_SRC_COMMON_DEBUG_PERIPHMAP_H__
#define __DEBUGGER_SRC_COMMON_DEBUG_PERIPHMAP_H__

#include <inttypes.h>

namespace debugger {

static const int CFG_NASTI_MASTER_CACHED    = 0;
static const int CFG_NASTI_MASTER_UNCACHED  = 1;
static const int CFG_NASTI_MASTER_ETHMAC    = 2;
static const int CFG_NASTI_MASTER_MSTUART   = 3;
static const int CFG_NASTI_MASTER_TOTAL     = 4;

static const uint16_t MST_DID_EMPTY            = 0x7755;
static const uint16_t SLV_DID_EMPTY            = 0x5577;

static const uint16_t VENDOR_GNSSSENSOR        = 0x00F1;
// Masters IDs
static const uint16_t RISCV_CACHED_TILELINK    = 0x0500;
static const uint16_t RISCV_UNCACHED_TILELINK  = 0x0501;
static const uint16_t GAISLER_ETH_MAC_MASTER   = 0x0502;
static const uint16_t GAISLER_ETH_EDCL_MASTER  = 0x0503;
static const uint16_t RISCV_RIVER_CPU          = 0x0505;
static const uint16_t GNSSSENSOR_UART_TAP      = 0x050A;

// Slaves IDs
static const uint16_t GNSS_SUB_SYSTEM          = 0x0067;
static const uint16_t GNSSSENSOR_ENGINE_STUB   = 0x0068;
static const uint16_t GNSSSENSOR_FSE_V2_GPS    = 0x0069;
static const uint16_t GNSSSENSOR_FSE_V2_GLO    = 0x006a;
static const uint16_t GNSSSENSOR_FSE_V2_GAL    = 0x006c;
static const uint16_t GNSSSENSOR_BOOTROM       = 0x0071;
static const uint16_t GNSSSENSOR_FWIMAGE       = 0x0072;
static const uint16_t GNSSSENSOR_SRAM          = 0x0073;
static const uint16_t GNSSSENSOR_PNP           = 0x0074;
static const uint16_t GNSSSENSOR_QSPI          = 0x0075;
static const uint16_t GNSSSENSOR_GPIO          = 0x0076;
static const uint16_t GNSSSENSOR_RF_CONTROL    = 0x0077;
static const uint16_t GNSSSENSOR_ENGINE        = 0x0078;
static const uint16_t GNSSSENSOR_UART          = 0x007a;
static const uint16_t GNSSSENSOR_ACCELEROMETER = 0x007b;
static const uint16_t GNSSSENSOR_GYROSCOPE     = 0x007c;
static const uint16_t GNSSSENSOR_IRQCTRL       = 0x007d;
static const uint16_t GNSSSENSOR_ETHMAC        = 0x007f;
static const uint16_t GNSSSENSOR_DSU           = 0x0080;
static const uint16_t GNSSSENSOR_GPTIMERS      = 0x0081;
static const uint16_t GNSSSENSOR_OTP_8KB       = 0x0083;

static const uint32_t PNP_CFG_TYPE_INVALID     = 0;
static const uint32_t PNP_CFG_TYPE_MASTER      = 1;
static const uint32_t PNP_CFG_TYPE_SLAVE       = 2;

typedef struct DeviceDescriptorType {
    union DescrType {
        struct bits_type {
            uint32_t descrsize : 8;
            uint32_t descrtype : 2;
            uint32_t rsrv1 : 22;
        } bits;
        uint32_t val;
    } descr;
    uint16_t did;
    uint16_t vid;
    uint64_t reserved;
    uint64_t addr_start;
    uint64_t addr_end;
} DeviceDescriptorType;

typedef struct PnpMapType {
    uint32_t hwid;              /// 0xfffff000: RO: HW ID
    uint32_t fwid;              /// 0xfffff004: RW: FW ID
    union TechType {
        struct bits_type {
            uint64_t plic_irq_total : 8;
            uint64_t cfg_slots : 8;
            uint64_t rsrv1 : 8;
            uint64_t l2cache_ena : 1;
            uint64_t rsrv2 : 3;
            uint64_t cpu_max : 4;
            uint64_t rsrv63_32 : 32;
        } bits;
        uint64_t val;
    } cfg;                     /// 0xfffff008: RO: common SOC defines
    uint64_t idt;               /// 0xfffff010:
    uint64_t malloc_addr;       /// 0xfffff018: RW: memalloc pointer 0x18
    uint64_t malloc_size;       /// 0xfffff020: RW: memalloc size 0x20
    uint64_t fwdbg1;            /// 0xfffff028: RW: FW debug register 1
    uint64_t fwdbg2;            /// 0xfffff030: RW: FW debug register 2
    uint64_t fwdbg3;            /// 0xfffff038
    uint8_t cfg_table[(1 << 12) - 0x40];  /// 0xfffff040: RO: PNP configuration
} PnpMapType;

struct GpioType {
    union {
        struct MapType {
            uint32_t led;
            uint32_t dip;
        } map;
        uint64_t val[1];
        uint8_t buf[8];
    } u;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_COMMON_DEBUG_PERIPHMAP_H__
