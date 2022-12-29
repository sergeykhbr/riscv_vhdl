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

#include <inttypes.h>
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

const char *const VENDOR_NAME = "Optimizing Technologies.";
const char *const MIKRON_NAME = "Mikron";

const char *const MST_DID_EMPTY_NAME = "Empty master slot";
const char *const SLV_DID_EMPTY_NAME = "Empty slave slot";

const char *const UNKOWN_ID_NAME = "Unknown";

static const char *const XMST_DEVICE_NAMES[] = {
    "Rocket Cached TileLink",           // 0x500
    "Rocket Uncached TileLink",         // 0x501
    "Gaisler Ethernet MAC with DMA ",   // 0x502
    "Gaisler Ethernet EDCL with DMA",   // 0x503
    "Reserved",                         // 0x504
    "RISC-V River CPU",                 // 0x505
    "RISC-V River x4 Workgroup",        // 0x506
    "Reserved",                         // 0x507
    "Reserved",                         // 0x508
    "Reserved",                         // 0x509
    "UART TAP (Debug port)",            // 0x50a
    "JTAG TAP",                         // 0x50b
};

static const char *const XSLV_DEVICE_NAMES[] = {
    "GNSS Sub-System",          // 0x67
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
    "CLINT",                    // 0x83
    "PLIC",                     // 0x84
    "AXI2APB Bridge",           // 0x85
    "AXI Interconnect",         // 0x86
    "PRCI",                     // 0x87
    "DDR Controller",           // 0x88
    "SPI Controller",           // 0x89
    "Workgroup DMI"             // 0x8a
};

static const char *const MIKRON_SLV_DEVICE_NAMES[] = {
    "RF-module",                // 0x1234
};

/**
 * @brief Get device Vendor name by its ID
 */
static const char *get_vendor_name(uint16_t vid) {
    if (vid == VENDOR_GNSSSENSOR || vid == VENDOR_OPTIMITECH) {
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
    if (vid != VENDOR_GNSSSENSOR && vid != VENDOR_OPTIMITECH) {
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
    if (did >= GNSS_SUB_SYSTEM && did <= DID_LAST) {
        return XSLV_DEVICE_NAMES[did - GNSS_SUB_SYSTEM];
    }
    if (did >= RISCV_CACHED_TILELINK && did <= GNSSSENSOR_JTAG_TAP) {
        return XMST_DEVICE_NAMES[did - RISCV_CACHED_TILELINK];
    }
    return UNKOWN_ID_NAME;
}

void print_pnp() {
    dev_cfg_type dcfg;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    int slots_total = (pnp->cfg >> 8) & 0xFF;
    int off = 0;
    uint32_t xsize;

    printf_uart("\r\n# Plug'n'Play info:\r\n");

    for (int i = 0; i < slots_total; i++) {
        dcfg = *(dev_cfg_type *)&pnp->cfg_table[off];
        off += sizeof(dcfg);

        if (dcfg.u.descrtype == PNP_CFG_TYPE_MASTER) {
            printf_uart("# AXI4: mst%d: %s    %s\r\n", i, 
                    get_vendor_name(dcfg.u.vid),
                    get_device_name(dcfg.u.vid, dcfg.u.did));
        } else if (dcfg.u.descrtype == PNP_CFG_TYPE_SLAVE) {
            printf_uart("# AXI4: slv%d: %s    %s\r\n", i, 
                    get_vendor_name(dcfg.u.vid),
                    get_device_name(dcfg.u.vid, dcfg.u.did));

            xsize = dcfg.u.addr_end - dcfg.u.addr_start;

            printf_uart("#    %016llx...%016llx, size = ",
                dcfg.u.addr_start, (dcfg.u.addr_end - 1));
            if (xsize < 1024) {
                printf_uart("%d bytes\r\n", (int)xsize);
            } else if (xsize < 1024*1024) {
                printf_uart("%d KB\r\n", (int)(xsize >> 10));
            } else {
                printf_uart("%d MB\r\n", (int)(xsize >> 20));
            }
        }
    }
}
