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
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

typedef struct master_cfg_bits_type {
    uint32_t descrsize : 8;
    uint32_t descrtype : 2;
    uint32_t rsrv : 22;
    uint32_t did : 16;
    uint32_t vid : 16;
} master_cfg_bits_type;

typedef union master_cfg_type {
    master_cfg_bits_type u;
    uint64_t v[1];
} master_cfg_type;


typedef struct slave_cfg_bits_type {
    uint32_t descrsize : 8;
    uint32_t descrtype : 2;
    uint32_t rsrv : 22;
    uint32_t did : 16;
    uint32_t vid : 16;
    uint32_t xmask;
    uint32_t xaddr;
} slave_cfg_bits_type;

typedef union slave_cfg_type {
    slave_cfg_bits_type u;
    uint64_t v[2];
} slave_cfg_type;

const char *const VENDOR_NAME = "Optimizing Technologies.";
const char *const MIKRON_NAME = "Mikron";

const char *const MST_DID_EMPTY_NAME = "Empty master slot";
const char *const SLV_DID_EMPTY_NAME = "Empty slave slot";

const char *const UNKOWN_ID_NAME = "Unknown";

static const char *const GNSS_SENSOR_MST_DEVICE_NAMES[] = {
    "Rocket Cached TileLink",           // 0x500
    "Rocket Uncached TileLink",         // 0x501
    "Gaisler Ethernet MAC with DMA ",   // 0x502
    "Gaisler Ethernet EDCL with DMA",   // 0x503
    "Reserved",                         // 0x504
    "RISC-V River CPU",                 // 0x505
    "Reserved",                         // 0x506
    "Reserved",                         // 0x507
    "Reserved",                         // 0x508
    "Reserved",                         // 0x509
    "UART TAP (Debug port)",            // 0x50a
    "JTAG TAP",                         // 0x50b
};

static const char *const GNSS_SENSOR_SLV_DEVICE_NAMES[] = {
    "GNSS Engine stub",         // 0x68
    "Reserved",                 // 0x69
    "Reserved",                 // 0x6a
    "Reserved",                 // 0x6b
    "Reserved",                 // 0x6c
    "Reserved",                 // 0x6d
    "Reserved",                 // 0x6e
    "Reserved",                 // 0x6f
    "Reserved",                 // 0x70
    "Boot ROM",                 // 0x71
    "FW Image ROM",             // 0x72
    "Internal SRAM",            // 0x73
    "Plug'n'Play support",      // 0x74
    "SD Controller",            // 0x75
    "Generic GPIO",             // 0x76
    "RF front-end controller",  // 0x77
    "GNSS Engine",              // 0x78
    "GPS FSE",                  // 0x79
    "Generic UART",             // 0x7a
    "Accelerometer",            // 0x7b
    "Gyroscope",                // 0x7c
    "Interrupt Controller",     // 0x7d
    "Reserved",                 // 0x7e
    "Ethernet MAC",             // 0x7f
    "Debug Support Unit (DSU)", // 0x80
    "GP Timers",                // 0x81
    "ADC Recorder",             // 0x82
    "OTP Bank, 8KB",            // 0x83
};

static const char *const MIKRON_SLV_DEVICE_NAMES[] = {
    "RF-module",                // 0x1234
};

/**
 * @brief Get device Vendor name by its ID
 */
static const char *get_vendor_name(uint16_t vid) {
    if (vid == VENDOR_GNSSSENSOR) {
        return VENDOR_NAME;
    }
    if (vid == 0xABCD) {
        return MIKRON_NAME;
    }
    return UNKOWN_ID_NAME;
}

/**
 * @brief Get device Name by Vendor ID and Device ID
 */
static const char *get_device_name(uint16_t vid, uint16_t did)
{
    if (vid != VENDOR_GNSSSENSOR) {
        if (did == 0x1234) {
            return MIKRON_SLV_DEVICE_NAMES[0];
        }
        return UNKOWN_ID_NAME;
    }
    if (did == MST_DID_EMPTY) {
        return MST_DID_EMPTY_NAME;
    }
    if (did == SLV_DID_EMPTY) {
        return SLV_DID_EMPTY_NAME;
    }
    if (did >= GNSSSENSOR_ENGINE_STUB && did <= GNSSSENSOR_OTP_8KB) {
        return GNSS_SENSOR_SLV_DEVICE_NAMES[did - GNSSSENSOR_ENGINE_STUB];
    }
    if (did >= RISCV_CACHED_TILELINK && did <= GNSSSENSOR_JTAG_TAP) {
        return GNSS_SENSOR_MST_DEVICE_NAMES[did - RISCV_CACHED_TILELINK];
    }
    return UNKOWN_ID_NAME;
}


void print_pnp() {
    master_cfg_type mcfg;
    slave_cfg_type scfg;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    int slv_total = (pnp->tech >> 8) & 0xFF;
    int mst_total = (pnp->tech >> 16) & 0xFF;
    int off = 0;
    uint32_t xsize;

    printf_uart("\n# Plug'n'Play info:\r\n");

    for (int i = 0; i < mst_total; i++) {
        mcfg.v[0] = *(uint64_t *)&pnp->cfg_table[off];

        printf_uart("# AXI4: mst%d: %s    %s\r\n", i, 
                get_vendor_name(mcfg.u.vid),
                get_device_name(mcfg.u.vid, mcfg.u.did));
        off += pnp->cfg_table[off];
    }

    for (int i = 0; i < slv_total; i++) {
        scfg.v[0] = *(uint64_t *)&pnp->cfg_table[off];
        scfg.v[1] = *(uint64_t *)&pnp->cfg_table[off + 8];

        printf_uart("# AXI4: slv%d: %s    %s\n", i, 
                get_vendor_name(scfg.u.vid),
                get_device_name(scfg.u.vid, scfg.u.did));

        scfg.u.xmask ^= 0xFFFFFFFFul;
        xsize = scfg.u.xmask + 1;

        printf_uart("#    %08x...%08x, size = ",
            scfg.u.xaddr, (scfg.u.xaddr + scfg.u.xmask));
        if (xsize < 1024) {
            printf_uart("%d bytes\n", (int)xsize);
        } else if (xsize < 1024*1024) {
            printf_uart("%d KB\n", (int)(xsize >> 10));
        } else {
            printf_uart("%d MB\n", (int)(xsize >> 20));
        }
        off += pnp->cfg_table[off];
    }
}
