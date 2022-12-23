// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "types_amba.h"

namespace debugger {


// @defgroup master_id_group AXI4 masters generic IDs.
// @details Each master must be assigned to a specific ID that used
//          as an index in the vector array of AXI master bus.
// 
// Total number of CPU limited by config CFG_TOTAL_CPU_MAX
static const int CFG_BUS0_XMST_GROUP0 = 0;
// DMA master interface generic index.
static const int CFG_BUS0_XMST_DMA = 1;
// Total Number of master devices on system bus.
static const int CFG_BUS0_XMST_TOTAL = 2;


// @defgroup slave_id_group AMBA AXI slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief Configuration index of the Boot ROM module visible by the firmware.
static const int CFG_BUS0_XSLV_BOOTROM = 0;
// Configuration index of the Core Local Interrupt Controller module.
static const int CFG_BUS0_XSLV_CLINT = 1;
// Configuration index of the SRAM module visible by the firmware.
static const int CFG_BUS0_XSLV_SRAM = 2;
// Configuration index of the External Controller module.
static const int CFG_BUS0_XSLV_PLIC = 3;
// Configuration index of the APB Bridge.
static const int CFG_BUS0_XSLV_PBRIDGE = 4;
// External DDR
static const int CFG_BUS0_XSLV_DDR = 5;
// Total number of the slaves devices.
static const int CFG_BUS0_XSLV_TOTAL = 6;

typedef sc_vector<sc_signal<axi4_master_in_type>> bus0_xmst_in_vector;
typedef sc_vector<sc_signal<axi4_master_out_type>> bus0_xmst_out_vector;
typedef sc_vector<sc_signal<axi4_slave_in_type>> bus0_xslv_in_vector;
typedef sc_vector<sc_signal<axi4_slave_out_type>> bus0_xslv_out_vector;
typedef sc_vector<sc_signal<mapinfo_type>> bus0_mapinfo_vector;

// Bus 0 device tree
static const mapinfo_type CFG_BUS0_MAP[CFG_BUS0_XSLV_TOTAL] = {
    {0x0000000010000, 0x0000000020000},                     // 0, bootrom
    {0x0000002000000, 0x0000002010000},                     // 1, clint
    {0x0000008000000, 0x0000008200000},                     // 2, sram, 2MB
    {0x000000C000000, 0x0000010000000},                     // 3, plic
    {0x0000010000000, 0x0000010100000},                     // 4, APB bridge: uart1
    {0x0000080000000, 0x00000C0000000}                      // 5, ddr, 512 MB
};

}  // namespace debugger

