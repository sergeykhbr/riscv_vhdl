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
localparam bit [3:0] SDSTATE_IDLE = 4'd0;
localparam bit [3:0] SDSTATE_READY = 4'd1;
localparam bit [3:0] SDSTATE_IDENT = 4'd2;
localparam bit [3:0] SDSTATE_STBY = 4'd3;
localparam bit [3:0] SDSTATE_TRAN = 4'd4;
localparam bit [3:0] SDSTATE_DATA = 4'd5;
localparam bit [3:0] SDSTATE_RCV = 4'd6;
localparam bit [3:0] SDSTATE_PRG = 4'd7;
localparam bit [3:0] SDSTATE_DIS = 4'd8;
localparam bit [3:0] SDSTATE_INA = 4'd9;

// Data block access state machine:
localparam bit [2:0] DATASTATE_IDLE = 3'd0;
localparam bit [2:0] DATASTATE_START = 3'd1;
localparam bit [2:0] DATASTATE_CRC15 = 3'd2;
localparam bit [2:0] DATASTATE_STOP = 3'd3;

typedef struct {
    logic [3:0] sdstate;
    logic [2:0] datastate;
    logic [31:0] powerup_cnt;
    logic [7:0] preinit_cnt;
    logic [31:0] delay_cnt;
    logic powerup_done;
    logic cmd_req_ready;
    logic cmd_resp_valid;
    logic cmd_resp_valid_delayed;
    logic [31:0] cmd_resp_data32;
    logic cmd_resp_r1b;
    logic cmd_resp_r2;
    logic cmd_resp_r3;
    logic cmd_resp_r7;
    logic illegal_cmd;
    logic ocr_hcs;
    logic [23:0] ocr_vdd_window;
    logic req_mem_valid;
    logic [40:0] req_mem_addr;
    logic [15:0] shiftdat;
    logic [12:0] bitcnt;
    logic crc16_clear;
    logic crc16_next;
    logic dat_trans;
} vip_sdcard_ctrl_registers;

const vip_sdcard_ctrl_registers vip_sdcard_ctrl_r_reset = '{
    SDSTATE_IDLE,                       // sdstate
    DATASTATE_IDLE,                     // datastate
    '0,                                 // powerup_cnt
    '0,                                 // preinit_cnt
    '0,                                 // delay_cnt
    1'b0,                               // powerup_done
    1'b0,                               // cmd_req_ready
    1'b0,                               // cmd_resp_valid
    1'b0,                               // cmd_resp_valid_delayed
    '0,                                 // cmd_resp_data32
    1'b0,                               // cmd_resp_r1b
    1'b0,                               // cmd_resp_r2
    1'b0,                               // cmd_resp_r3
    1'b0,                               // cmd_resp_r7
    1'b0,                               // illegal_cmd
    1'b0,                               // ocr_hcs
    '0,                                 // ocr_vdd_window
    1'b0,                               // req_mem_valid
    '0,                                 // req_mem_addr
    '1,                                 // shiftdat
    '0,                                 // bitcnt
    1'b0,                               // crc16_clear
    1'b0,                               // crc16_next
    1'b0                                // dat_trans
};

endpackage: vip_sdcard_ctrl_pkg
