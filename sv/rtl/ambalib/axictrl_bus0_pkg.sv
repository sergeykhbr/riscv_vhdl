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
package axictrl_bus0_pkg;

import types_pnp_pkg::*;
import types_amba_pkg::*;
import types_bus0_pkg::*;

typedef struct {
    logic [CFG_BUS0_XMST_LOG2_TOTAL-1:0] r_midx;
    logic [CFG_BUS0_XSLV_LOG2_TOTAL-1:0] r_sidx;
    logic [CFG_BUS0_XMST_LOG2_TOTAL-1:0] w_midx;
    logic [CFG_BUS0_XSLV_LOG2_TOTAL-1:0] w_sidx;
    logic [CFG_BUS0_XMST_LOG2_TOTAL-1:0] b_midx;
    logic [CFG_BUS0_XSLV_LOG2_TOTAL-1:0] b_sidx;
} axictrl_bus0_registers;

const axictrl_bus0_registers axictrl_bus0_r_reset = '{
    CFG_BUS0_XMST_TOTAL,                // r_midx
    CFG_BUS0_XSLV_TOTAL,                // r_sidx
    CFG_BUS0_XMST_TOTAL,                // w_midx
    CFG_BUS0_XSLV_TOTAL,                // w_sidx
    CFG_BUS0_XMST_TOTAL,                // b_midx
    CFG_BUS0_XSLV_TOTAL                 // b_sidx
};

endpackage: axictrl_bus0_pkg
