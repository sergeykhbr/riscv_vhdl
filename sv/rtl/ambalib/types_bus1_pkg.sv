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

// @brief Worjgroup DMI interface.
localparam int CFG_BUS1_PSLV_DMI = 0;
// @brief UART0 APB device.
localparam int CFG_BUS1_PSLV_UART1 = 1;
// Total number of the APB slaves devices on Bus[1].
localparam int CFG_BUS1_PSLV_TOTAL = 2;

typedef apb_in_type bus1_apb_in_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef apb_out_type bus1_apb_out_vector[0:CFG_BUS1_PSLV_TOTAL - 1];
typedef mapinfo_type bus1_mapinfo_vector[0:CFG_BUS1_PSLV_TOTAL - 1];

// Bus 1 device tree
const bus1_mapinfo_vector CFG_BUS1_MAP = '{
    '{64'h000001001E000, 64'h000001001F000},                // 0, dmi 4KB. TODO: change base address
    '{64'h0000010010000, 64'h0000010011000}                 // 1, uart1 4KB
};

endpackage: types_bus1_pkg
