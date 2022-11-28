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
package types_bus0_pkg;

import types_amba_pkg::*;


// @defgroup master_id_group AXI4 masters generic IDs.
// @details Each master must be assigned to a specific ID that used
//          as an index in the vector array of AXI master bus.
// 
// Total number of CPU limited by config CFG_TOTAL_CPU_MAX
localparam int CFG_BUS0_XMST_GROUP0 = 0;
// DMA master interface generic index.
localparam int CFG_BUS0_XMST_DMA = 1;
// Total Number of master devices on system bus.
localparam int CFG_BUS0_XMST_TOTAL = 2;


// @defgroup slave_id_group AMBA AXI slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief Configuration index of the Boot ROM module visible by the firmware.
localparam int CFG_BUS0_XSLV_BOOTROM = 0;
// Configuration index of the SRAM module visible by the firmware.
localparam int CFG_BUS0_XSLV_SRAM = 1;
// External DDR
localparam int CFG_BUS0_XSLV_DDR = 2;
// Configuration index of the APB Bridge.
localparam int CFG_BUS0_XSLV_PBRIDGE = 3;
// Configuration index of the GPIO (General Purpose In/Out) module.
localparam int CFG_BUS0_XSLV_GPIO = 4;
// Configuration index of the Core Local Interrupt Controller module.
localparam int CFG_BUS0_XSLV_CLINT = 5;
// Configuration index of the External Controller module.
localparam int CFG_BUS0_XSLV_PLIC = 6;
// Configuration index of the Plug-n-Play module.
localparam int CFG_BUS0_XSLV_PNP = 7;
// Total number of the slaves devices.
localparam int CFG_BUS0_XSLV_TOTAL = 8;

typedef axi4_master_in_type bus0_xmst_in_vector[0:CFG_BUS0_XMST_TOTAL - 1];
typedef axi4_master_out_type bus0_xmst_out_vector[0:CFG_BUS0_XMST_TOTAL - 1];
typedef axi4_slave_in_type bus0_xslv_in_vector[0:CFG_BUS0_XSLV_TOTAL - 1];
typedef axi4_slave_out_type bus0_xslv_out_vector[0:CFG_BUS0_XSLV_TOTAL - 1];
typedef mapinfo_type bus0_mapinfo_vector[0:CFG_BUS0_XSLV_TOTAL - 1];

// Bus 0 device tree
const bus0_mapinfo_vector CFG_BUS0_MAP = '{
    '{64'h0000000010000, 64'h0000000020000},                // 0, bootrom
    '{64'h0000008000000, 64'h0000008200000},                // 1, sram, 2MB
    '{64'h0000080000000, 64'h00000C0000000},                // 2, ddr, 512 MB
    '{64'h0000010010000, 64'h0000010011000},                // 3, APB bridge: uart1
    '{64'h0000010060000, 64'h0000010061000},                // 4, gpio
    '{64'h0000002000000, 64'h0000002010000},                // 5, clint
    '{64'h000000C000000, 64'h0000010000000},                // 6, plic
    '{64'h00000100ff000, 64'h0000010100000}                 // 7, pnp
};

endpackage: types_bus0_pkg
