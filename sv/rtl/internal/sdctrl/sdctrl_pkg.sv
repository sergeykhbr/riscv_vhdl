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
package sdctrl_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;
import sdctrl_cfg_pkg::*;

// SD controller modes:
localparam bit [1:0] MODE_PRE_INIT = 2'd0;
localparam bit [1:0] MODE_SPI = 2'd1;
localparam bit [1:0] MODE_SD = 2'd2;

typedef struct {
    logic nrst_spimode;
    logic nrst_sdmode;
    logic [6:0] clkcnt;
    logic cmd_set_low;
    logic [1:0] mode;
} sdctrl_registers;

const sdctrl_registers sdctrl_r_reset = '{
    1'b0,                               // nrst_spimode
    1'b0,                               // nrst_sdmode
    '0,                                 // clkcnt
    1'b0,                               // cmd_set_low
    MODE_PRE_INIT                       // mode
};

endpackage: sdctrl_pkg
