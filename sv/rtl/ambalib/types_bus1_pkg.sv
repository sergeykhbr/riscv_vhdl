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
package types_bus1_pkg;

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
// Configuration index of the SPI SD-card.
localparam int CFG_BUS1_PSLV_SPI = 3;
// Configuration index of the GPIO (General Purpose In/Out) module.
localparam int CFG_BUS1_PSLV_GPIO = 4;
// @brief DDR control register.
localparam int CFG_BUS1_PSLV_DDR = 5;
// Configuration index of the Plug-n-Play module.
localparam int CFG_BUS1_PSLV_PNP = 6;
// Total number of the APB slaves devices on Bus[1].
localparam int CFG_BUS1_PSLV_TOTAL = 7;

typedef apb_in_type bus1_apb_in_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef apb_out_type bus1_apb_out_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef mapinfo_type bus1_mapinfo_vector[0:CFG_BUS1_PSLV_TOTAL - 1];

// Bus 1 device tree
const bus1_mapinfo_vector CFG_BUS1_MAP = '{
    '{64'h0000010010000, 64'h0000010011000},                // 0, uart1 4KB
    '{64'h0000010012000, 64'h0000010013000},                // 1, PRCI 4KB
    '{64'h000001001E000, 64'h000001001F000},                // 2, dmi 4KB. TODO: change base address
    '{64'h0000010050000, 64'h0000010051000},                // 4, SPI SD-card 4KB
    '{64'h0000010060000, 64'h0000010061000},                // 3, GPIO 4KB
    '{64'h00000100C0000, 64'h00000100C1000},                // 5, DDR MGMT 4KB
    '{64'h00000100ff000, 64'h0000010100000}                 // 6, Plug'n'Play 4KB
};

endpackage: types_bus1_pkg
