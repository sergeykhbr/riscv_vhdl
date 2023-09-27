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
localparam bit [3:0] SDSTATE_PRE_INIT = 4'hf;
localparam bit [3:0] SDSTATE_IDLE = 4'h0;
localparam bit [3:0] SDSTATE_READY = 4'h1;
localparam bit [3:0] SDSTATE_IDENT = 4'h2;
localparam bit [3:0] SDSTATE_STBY = 4'h3;
localparam bit [3:0] SDSTATE_TRAN = 4'h4;
localparam bit [3:0] SDSTATE_DATA = 4'h5;
localparam bit [3:0] SDSTATE_RCV = 4'h6;
localparam bit [3:0] SDSTATE_PRG = 4'h7;
localparam bit [3:0] SDSTATE_DIS = 4'h8;
localparam bit [3:0] SDSTATE_INA = 4'h9;
// SD-card 'idle' state substates:
localparam bit [2:0] IDLESTATE_CMD0 = 3'h0;
localparam bit [2:0] IDLESTATE_CMD8 = 3'h1;
localparam bit [2:0] IDLESTATE_CMD55 = 3'h2;
localparam bit [2:0] IDLESTATE_ACMD41 = 3'h3;
localparam bit [2:0] IDLESTATE_CMD58 = 3'h4;
localparam bit [2:0] IDLESTATE_CARD_IDENTIFICATION = 3'h5;
// SD-card 'ready' state substates:
localparam bit [1:0] READYSTATE_CMD11 = 2'h0;
localparam bit [1:0] READYSTATE_CMD2 = 2'h1;
localparam bit [1:0] READYSTATE_CHECK_CID = 2'h2;           // State change: ready -> ident
// SD-card 'ident' state substates:
localparam bit IDENTSTATE_CMD3 = 1'h0;
localparam bit IDENTSTATE_CHECK_RCA = 1'h1;                 // State change: ident -> stby

typedef struct {
    logic [6:0] clkcnt;
    logic cmd_set_low;
    logic cmd_req_valid;
    logic [5:0] cmd_req_cmd;
    logic [31:0] cmd_req_arg;
    logic [2:0] cmd_req_rn;
    logic [5:0] cmd_resp_r1;
    logic [31:0] cmd_resp_reg;
    logic [14:0] cmd_resp_spistatus;
    logic crc15_clear;
    logic [3:0] dat;
    logic dat_dir;
    logic dat3_dir;
    logic [3:0] sdstate;
    logic [2:0] initstate;
    logic [1:0] readystate;
    logic identstate;
    logic wait_cmd_resp;
    logic [2:0] sdtype;
    logic HCS;                                              // High Capacity Support
    logic S18;                                              // 1.8V Low voltage
    logic [31:0] RCA;                                       // Relative Address
    logic [23:0] OCR_VoltageWindow;                         // all ranges 2.7 to 3.6 V
} sdctrl_registers;

const sdctrl_registers sdctrl_r_reset = '{
    '0,                                 // clkcnt
    1'b0,                               // cmd_set_low
    1'b0,                               // cmd_req_valid
    '0,                                 // cmd_req_cmd
    '0,                                 // cmd_req_arg
    '0,                                 // cmd_req_rn
    '0,                                 // cmd_resp_r1
    '0,                                 // cmd_resp_reg
    '0,                                 // cmd_resp_spistatus
    1'h1,                               // crc15_clear
    '1,                                 // dat
    DIR_OUTPUT,                         // dat_dir
    DIR_INPUT,                          // dat3_dir
    SDSTATE_PRE_INIT,                   // sdstate
    IDLESTATE_CMD0,                     // initstate
    READYSTATE_CMD11,                   // readystate
    IDENTSTATE_CMD3,                    // identstate
    1'b0,                               // wait_cmd_resp
    SDCARD_UNKNOWN,                     // sdtype
    1'h1,                               // HCS
    1'b0,                               // S18
    '0,                                 // RCA
    24'hff8000                          // OCR_VoltageWindow
};

endpackage: sdctrl_pkg
