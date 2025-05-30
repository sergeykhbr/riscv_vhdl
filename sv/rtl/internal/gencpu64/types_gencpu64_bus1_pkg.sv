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
package types_gencpu64_bus1_pkg;

import types_amba_pkg::*;

// @defgroup slave_id_group AMBA APB slaves generic IDs.
// @details Each module in a SoC has to be indexed by unique identificator.
//          In current implementation it is used sequential indexing for it.
//          Indexes are used to specify a device bus item in a vectors.

// @brief UART0 APB device.
localparam int CFG_BUS1_PSLV_UART1 = 0;
// @brief System status and control registers device.
localparam int CFG_BUS1_PSLV_PRCI = 1;
// @brief Worjgroup DMI interface.
localparam int CFG_BUS1_PSLV_DMI = 2;
// Configuration index of the SD-card control registers.
localparam int CFG_BUS1_PSLV_SDCTRL_REG = 3;
// Configuration index of the GPIO (General Purpose In/Out) module.
localparam int CFG_BUS1_PSLV_GPIO = 4;
// @brief DDR control register.
localparam int CFG_BUS1_PSLV_DDR = 5;
// Configuration index of the Plug-n-Play module.
localparam int CFG_BUS1_PSLV_PNP = 6;
// Total number of the APB slaves devices on Bus[1].
localparam int CFG_BUS1_PSLV_TOTAL = 7;
// Necessary bus width to store index + 1.
localparam int CFG_BUS1_PSLV_LOG2_TOTAL = 3;                // $clog2(CFG_BUS1_PSLV_TOTAL + 1)

typedef apb_in_type bus1_apb_in_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef apb_out_type bus1_apb_out_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef mapinfo_type bus1_mapinfo_vector[0:CFG_BUS1_PSLV_TOTAL - 1];

// Bus 1 device tree
const bus1_mapinfo_vector CFG_BUS1_MAP = '{
        '{
            64'h0000000000010000,       // addr_start
            64'h0000000000011000        // addr_end
        },// uart1 4KB
        '{
            64'h0000000000012000,       // addr_start
            64'h0000000000013000        // addr_end
        },// PRCI 4KB
        '{
            64'h000000000001E000,       // addr_start
            64'h000000000001F000        // addr_end
        },// dmi 4KB. TODO: change base address
        '{
            64'h0000000000050000,       // addr_start
            64'h0000000000051000        // addr_end
        },// SPI SD-card 4KB
        '{
            64'h0000000000060000,       // addr_start
            64'h0000000000061000        // addr_end
        },// GPIO 4KB
        '{
            64'h00000000000C0000,       // addr_start
            64'h00000000000C1000        // addr_end
        },// DDR MGMT 4KB
        '{
            64'h00000000000FF000,       // addr_start
            64'h0000000000100000        // addr_end
        }// Plug'n'Play 4KB
};

endpackage: types_gencpu64_bus1_pkg
