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
package sdctrl_cmd_transmitter_pkg;

import sdctrl_cfg_pkg::*;

// Command request states:
localparam bit [3:0] CMDSTATE_IDLE = 4'h0;
localparam bit [3:0] CMDSTATE_REQ_CONTENT = 4'h1;
localparam bit [3:0] CMDSTATE_REQ_CRC7 = 4'h2;
localparam bit [3:0] CMDSTATE_REQ_STOPBIT = 4'h3;
localparam bit [3:0] CMDSTATE_RESP_WAIT = 4'h4;
localparam bit [3:0] CMDSTATE_RESP_TRANSBIT = 4'h5;
localparam bit [3:0] CMDSTATE_RESP_CMD_MIRROR = 4'h6;
localparam bit [3:0] CMDSTATE_RESP_R1 = 4'h7;
localparam bit [3:0] CMDSTATE_RESP_REG = 4'h8;
localparam bit [3:0] CMDSTATE_RESP_CID_CSD = 4'h9;
localparam bit [3:0] CMDSTATE_RESP_CRC7 = 4'ha;
localparam bit [3:0] CMDSTATE_RESP_STOPBIT = 4'hb;

typedef struct {
    logic [5:0] req_cmd;
    logic [2:0] req_rn;
    logic resp_valid;
    logic [5:0] resp_cmd;
    logic [31:0] resp_arg;
    logic [39:0] cmdshift;
    logic [5:0] cmdmirror;
    logic [31:0] regshift;
    logic [119:0] cidshift;
    logic [6:0] crc_calc;
    logic [6:0] crc_rx;
    logic [6:0] cmdbitcnt;
    logic crc7_clear;
    logic [3:0] cmdstate;
    logic [3:0] cmderr;
    logic [15:0] watchdog;
} sdctrl_cmd_transmitter_registers;

const sdctrl_cmd_transmitter_registers sdctrl_cmd_transmitter_r_reset = '{
    '0,                                 // req_cmd
    '0,                                 // req_rn
    1'b0,                               // resp_valid
    '0,                                 // resp_cmd
    '0,                                 // resp_arg
    '1,                                 // cmdshift
    '0,                                 // cmdmirror
    '0,                                 // regshift
    '0,                                 // cidshift
    '0,                                 // crc_calc
    '0,                                 // crc_rx
    '0,                                 // cmdbitcnt
    1'h1,                               // crc7_clear
    CMDSTATE_IDLE,                      // cmdstate
    CMDERR_NONE,                        // cmderr
    '0                                  // watchdog
};

endpackage: sdctrl_cmd_transmitter_pkg
