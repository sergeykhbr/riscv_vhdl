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
package sdctrl_regs_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;

typedef struct {
    logic sclk_ena;
    logic clear_cmderr;
    logic [31:0] scaler;
    logic [31:0] scaler_cnt;
    logic [15:0] wdog;
    logic [15:0] wdog_cnt;
    logic level;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
    logic [5:0] last_req_cmd;
    logic [5:0] last_resp_cmd;
    logic [6:0] last_resp_crc7_rx;
    logic [6:0] last_resp_crc7_calc;
    logic [31:0] last_resp_reg;
    logic pcie_12V_support;
    logic pcie_available;
    logic [3:0] voltage_supply;
    logic [7:0] check_pattern;
} sdctrl_regs_registers;

const sdctrl_regs_registers sdctrl_regs_r_reset = '{
    1'b0,                               // sclk_ena
    1'b0,                               // clear_cmderr
    '0,                                 // scaler
    '0,                                 // scaler_cnt
    16'h0fff,                           // wdog
    '0,                                 // wdog_cnt
    1'b0,                               // level
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0,                               // resp_err
    '1,                                 // last_req_cmd
    '0,                                 // last_resp_cmd
    '0,                                 // last_resp_crc7_rx
    '0,                                 // last_resp_crc7_calc
    '0,                                 // last_resp_reg
    1'b0,                               // pcie_12V_support
    1'b0,                               // pcie_available
    4'h1,                               // voltage_supply
    8'h55                               // check_pattern
};

endpackage: sdctrl_regs_pkg
