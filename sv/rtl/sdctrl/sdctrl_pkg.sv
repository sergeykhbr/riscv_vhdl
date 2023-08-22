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

localparam int log2_fifosz = 9;
localparam int fifo_dbits = 8;
// SD-card states see Card Status[12:9] CURRENT_STATE on page 145:
localparam bit [3:0] SDSTATE_PRE_INIT = 4'h0;
localparam bit [3:0] SDSTATE_IDLE = 4'h1;
localparam bit [3:0] SDSTATE_READY = 4'h2;
localparam bit [3:0] SDSTATE_IDENT = 4'h3;
localparam bit [3:0] SDSTATE_STBY = 4'h4;
localparam bit [3:0] SDSTATE_TRAN = 4'h5;
localparam bit [3:0] SDSTATE_DATA = 4'h6;
localparam bit [3:0] SDSTATE_RCV = 4'h7;
localparam bit [3:0] SDSTATE_PRG = 4'h8;
localparam bit [3:0] SDSTATE_DIS = 4'h9;
// SD-card initalization state:
localparam bit [3:0] INITSTATE_CMD0 = 4'h0;
localparam bit [3:0] INITSTATE_CMD8 = 4'h1;
localparam bit [3:0] INITSTATE_ACMD41 = 4'h2;
localparam bit [3:0] INITSTATE_CMD11 = 4'h3;
localparam bit [3:0] INITSTATE_CMD2 = 4'h4;
localparam bit [3:0] INITSTATE_CMD3 = 4'h5;
localparam bit [3:0] INITSTATE_WAIT_RESP = 4'h6;
localparam bit [3:0] INITSTATE_ERROR = 4'h7;
localparam bit [3:0] INITSTATE_DONE = 4'h8;

typedef struct {
    logic [6:0] clkcnt;
    logic cmd_req_ena;
    logic [5:0] cmd_req_cmd;
    logic [31:0] cmd_req_arg;
    logic [2:0] cmd_req_rn;
    logic [5:0] cmd_resp_r1;
    logic [31:0] cmd_resp_reg;
    logic crc16_clear;
    logic [3:0] dat;
    logic dat_dir;
    logic [3:0] sdstate;
    logic [3:0] initstate;
    logic [3:0] initstate_next;
} sdctrl_registers;

const sdctrl_registers sdctrl_r_reset = '{
    '0,                                 // clkcnt
    1'b0,                               // cmd_req_ena
    '0,                                 // cmd_req_cmd
    '0,                                 // cmd_req_arg
    '0,                                 // cmd_req_rn
    '0,                                 // cmd_resp_r1
    '0,                                 // cmd_resp_reg
    1'h1,                               // crc16_clear
    '1,                                 // dat
    DIR_INPUT,                          // dat_dir
    SDSTATE_PRE_INIT,                   // sdstate
    INITSTATE_CMD0,                     // initstate
    INITSTATE_CMD0                      // initstate_next
};

endpackage: sdctrl_pkg
