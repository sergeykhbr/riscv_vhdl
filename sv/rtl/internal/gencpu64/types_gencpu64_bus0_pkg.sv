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
package types_gencpu64_bus0_pkg;

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
// Necessary bus width to store index + 1.
localparam int CFG_BUS0_XMST_LOG2_TOTAL = 2;                // $clog2(CFG_BUS0_XMST_TOTAL + 1)


// @defgroup slave_id_group AMBA AXI slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief Configuration index of the Boot ROM module visible by the firmware.
localparam int CFG_BUS0_XSLV_BOOTROM = 0;
// Configuration index of the Core Local Interrupt Controller module.
localparam int CFG_BUS0_XSLV_CLINT = 1;
// Configuration index of the SRAM module visible by the firmware.
localparam int CFG_BUS0_XSLV_SRAM = 2;
// Configuration index of the External Controller module.
localparam int CFG_BUS0_XSLV_PLIC = 3;
// Configuration index of the APB Bridge.
localparam int CFG_BUS0_XSLV_PBRIDGE = 4;
// External DDR
localparam int CFG_BUS0_XSLV_DDR = 5;
// SD-card direct memory access.
localparam int CFG_BUS0_XSLV_SDCTRL_MEM = 6;
// Unmapped error access
localparam int CFG_BUS0_XSLV_UNMAP = 7;
// Total number of the slaves devices.
localparam int CFG_BUS0_XSLV_TOTAL = 8;
// Necessary bus width to store index.
localparam int CFG_BUS0_XSLV_LOG2_TOTAL = 3;                // $clog2(CFG_BUS0_XSLV_TOTAL)

typedef axi4_master_in_type bus0_xmst_in_vector[0:CFG_BUS0_XMST_TOTAL - 1];
typedef axi4_master_out_type bus0_xmst_out_vector[0:CFG_BUS0_XMST_TOTAL - 1];
typedef axi4_slave_in_type bus0_xslv_in_vector[0:CFG_BUS0_XSLV_TOTAL - 1];
typedef axi4_slave_out_type bus0_xslv_out_vector[0:CFG_BUS0_XSLV_TOTAL - 1];
typedef mapinfo_type bus0_mapinfo_vector[0:CFG_BUS0_XSLV_TOTAL - 1];

// Bus 0 device tree
const bus0_mapinfo_vector CFG_BUS0_MAP = '{
        '{
            64'h0000000000010000,       // addr_start
            64'h0000000000050000        // addr_end
        },// 0, bootrom, 256 KB
        '{
            64'h0000000002000000,       // addr_start
            64'h0000000002010000        // addr_end
        },// 1, clint
        '{
            64'h0000000008000000,       // addr_start
            64'h0000000008200000        // addr_end
        },// 2, sram, 2MB
        '{
            64'h000000000C000000,       // addr_start
            64'h0000000010000000        // addr_end
        },// 3, plic
        '{
            64'h0000000010000000,       // addr_start
            64'h0000000010100000        // addr_end
        },// 4, APB bridge: uart1
        '{
            64'h0000000080000000,       // addr_start
            64'h00000000C0000000        // addr_end
        },// 5, ddr, 1 GB
        '{
            64'h0000000800000000,       // addr_start
            64'h0000001000000000        // addr_end
        },// 6, sdctrl, 32 GB
        '{
            64'h0000000000000000,       // addr_start
            64'h0000000000000000        // addr_end
        }// Unampped access, lowest priority
};

endpackage: types_gencpu64_bus0_pkg
