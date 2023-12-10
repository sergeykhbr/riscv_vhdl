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
package sdctrl_sdmode_pkg;

import sdctrl_cfg_pkg::*;

// SD-card states see Card Status[12:9] CURRENT_STATE on page 145:
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
// SD-card 'idle' state substates:
localparam bit [2:0] IDLESTATE_CMD0 = 3'd0;
localparam bit [2:0] IDLESTATE_CMD8 = 3'd1;
localparam bit [2:0] IDLESTATE_CMD55 = 3'd2;
localparam bit [2:0] IDLESTATE_ACMD41 = 3'd3;
localparam bit [2:0] IDLESTATE_CARD_IDENTIFICATION = 3'd5;
// SD-card 'ready' state substates:
localparam bit [1:0] READYSTATE_CMD11 = 2'd0;
localparam bit [1:0] READYSTATE_CMD2 = 2'd1;
localparam bit [1:0] READYSTATE_CHECK_CID = 2'd2;           // State change: ready -> ident
// SD-card 'ident' state substates:
localparam bit IDENTSTATE_CMD3 = 1'b0;
localparam bit IDENTSTATE_CHECK_RCA = 1'b1;                 // State change: ident -> stby

typedef struct {
    logic [6:0] clkcnt;
    logic cmd_req_valid;
    logic [5:0] cmd_req_cmd;
    logic [31:0] cmd_req_arg;
    logic [2:0] cmd_req_rn;
    logic [5:0] cmd_resp_cmd;
    logic [31:0] cmd_resp_arg32;
    logic [31:0] data_addr;
    logic [511:0] data_data;
    logic data_resp_valid;
    logic wdog_ena;
    logic crc16_clear;
    logic [15:0] crc16_calc0;
    logic [15:0] crc16_calc1;
    logic [15:0] crc16_calc2;
    logic [15:0] crc16_calc3;
    logic [15:0] crc16_rx0;
    logic [15:0] crc16_rx1;
    logic [15:0] crc16_rx2;
    logic [15:0] crc16_rx3;
    logic dat_full_ena;
    logic dat_csn;
    logic err_clear;
    logic err_valid;
    logic [3:0] err_code;
    logic sck_400khz_ena;
    logic [3:0] sdstate;
    logic [2:0] initstate;
    logic [1:0] readystate;
    logic identstate;
    logic wait_cmd_resp;
    logic [2:0] sdtype;
    logic HCS;                                              // High Capacity Support
    logic S18;                                              // 1.8V Low voltage
    logic [23:0] OCR_VoltageWindow;                         // all ranges 2.7 to 3.6 V
    logic [11:0] bitcnt;
} sdctrl_sdmode_registers;

const sdctrl_sdmode_registers sdctrl_sdmode_r_reset = '{
    '0,                                 // clkcnt
    1'b0,                               // cmd_req_valid
    '0,                                 // cmd_req_cmd
    '0,                                 // cmd_req_arg
    '0,                                 // cmd_req_rn
    '0,                                 // cmd_resp_cmd
    '0,                                 // cmd_resp_arg32
    '0,                                 // data_addr
    '0,                                 // data_data
    1'b0,                               // data_resp_valid
    1'b0,                               // wdog_ena
    1'b1,                               // crc16_clear
    '0,                                 // crc16_calc0
    '0,                                 // crc16_calc1
    '0,                                 // crc16_calc2
    '0,                                 // crc16_calc3
    '0,                                 // crc16_rx0
    '0,                                 // crc16_rx1
    '0,                                 // crc16_rx2
    '0,                                 // crc16_rx3
    1'b0,                               // dat_full_ena
    1'b1,                               // dat_csn
    1'b0,                               // err_clear
    1'b0,                               // err_valid
    '0,                                 // err_code
    1'b1,                               // sck_400khz_ena
    SDSTATE_IDLE,                       // sdstate
    IDLESTATE_CMD0,                     // initstate
    READYSTATE_CMD11,                   // readystate
    IDENTSTATE_CMD3,                    // identstate
    1'b0,                               // wait_cmd_resp
    SDCARD_UNKNOWN,                     // sdtype
    1'b1,                               // HCS
    1'b0,                               // S18
    24'hFF8000,                         // OCR_VoltageWindow
    '0                                  // bitcnt
};

endpackage: sdctrl_sdmode_pkg
