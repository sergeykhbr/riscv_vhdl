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
package vip_sdcard_ctrl_pkg;


// 
// SD-card states (see Card Status[12:9] CURRENT_STATE on page 145)
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

typedef struct {
    logic [3:0] sdstate;
    logic [31:0] powerup_cnt;
    logic [7:0] preinit_cnt;
    logic [31:0] delay_cnt;
    logic powerup_done;
    logic cmd_req_ready;
    logic cmd_resp_valid;
    logic cmd_resp_valid_delayed;
    logic [31:0] cmd_resp_data32;
} vip_sdcard_ctrl_registers;

const vip_sdcard_ctrl_registers vip_sdcard_ctrl_r_reset = '{
    SDSTATE_IDLE,                       // sdstate
    '0,                                 // powerup_cnt
    '0,                                 // preinit_cnt
    '0,                                 // delay_cnt
    1'b0,                               // powerup_done
    1'b0,                               // cmd_req_ready
    1'b0,                               // cmd_resp_valid
    1'b0,                               // cmd_resp_valid_delayed
    '0                                  // cmd_resp_data32
};

endpackage: vip_sdcard_ctrl_pkg
