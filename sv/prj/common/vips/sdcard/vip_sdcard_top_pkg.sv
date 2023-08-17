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


// Receiver CMD state:
localparam bit [2:0] CMDSTATE_IDLE = 3'h0;
localparam bit [2:0] CMDSTATE_REQ_ARG = 3'h1;
localparam bit [2:0] CMDSTATE_REQ_CRC7 = 3'h2;
localparam bit [2:0] CMDSTATE_REQ_STOPBIT = 3'h3;
localparam bit [2:0] CMDSTATE_WAIT_RESP = 3'h4;
localparam bit [2:0] CMDSTATE_RESP = 3'h5;

typedef struct {
    logic cmd_dir;
    logic [47:0] cmd_rxshift;
    logic [47:0] cmd_txshift;
    logic [2:0] cmd_state;
    logic [5:0] bitcnt;
} vip_sdcard_top_registers;

const vip_sdcard_top_registers vip_sdcard_top_r_reset = '{
    1'h1,                               // cmd_dir
    '1,                                 // cmd_rxshift
    '1,                                 // cmd_txshift
    CMDSTATE_IDLE,                      // cmd_state
    '0                                  // bitcnt
};

endpackage: vip_sdcard_top_pkg
