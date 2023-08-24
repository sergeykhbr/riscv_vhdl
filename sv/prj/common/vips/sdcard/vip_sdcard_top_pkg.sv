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
package vip_sdcard_top_pkg;


// Generic config parameters
localparam int CFG_SDCARD_POWERUP_DONE_DELAY = 700;         // Delay of busy bits in ACMD41 response
localparam bit [3:0] CFG_SDCARD_VHS = 4'h1;                 // CMD8 Voltage supply mask
localparam bit CFG_SDCARD_PCIE_1_2V = 1'h0;
localparam bit CFG_SDCARD_PCIE_AVAIL = 1'h0;
localparam bit [23:0] CFG_SDCARD_VDD_VOLTAGE_WINDOW = 24'hff8000;
// 
// Receiver CMD state:
localparam bit [3:0] CMDSTATE_IDLE = 4'h0;
localparam bit [3:0] CMDSTATE_REQ_STARTBIT = 4'h1;
localparam bit [3:0] CMDSTATE_REQ_CMD = 4'h2;
localparam bit [3:0] CMDSTATE_REQ_ARG = 4'h3;
localparam bit [3:0] CMDSTATE_REQ_CRC7 = 4'h4;
localparam bit [3:0] CMDSTATE_REQ_STOPBIT = 4'h5;
localparam bit [3:0] CMDSTATE_WAIT_RESP = 4'h6;
localparam bit [3:0] CMDSTATE_RESP = 4'h7;
localparam bit [3:0] CMDSTATE_RESP_CRC7 = 4'h8;

typedef struct {
    logic cmd_dir;
    logic [47:0] cmd_rxshift;
    logic [47:0] cmd_txshift;
    logic [3:0] cmd_state;
    logic [5:0] bitcnt;
    logic [31:0] powerup_cnt;
    logic powerup_done;
} vip_sdcard_top_registers;

const vip_sdcard_top_registers vip_sdcard_top_r_reset = '{
    1'h1,                               // cmd_dir
    '1,                                 // cmd_rxshift
    '1,                                 // cmd_txshift
    CMDSTATE_IDLE,                      // cmd_state
    '0,                                 // bitcnt
    '0,                                 // powerup_cnt
    1'b0                                // powerup_done
};

endpackage: vip_sdcard_top_pkg
