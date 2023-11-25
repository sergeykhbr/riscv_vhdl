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
package sdctrl_spimode_pkg;

import sdctrl_cfg_pkg::*;

// Controller state
localparam bit [3:0] STATE_CMD0 = 4'd0;
localparam bit [3:0] STATE_CMD8 = 4'd1;
localparam bit [3:0] STATE_CMD55 = 4'd2;
localparam bit [3:0] STATE_ACMD41 = 4'd3;
localparam bit [3:0] STATE_CMD58 = 4'd4;
localparam bit [3:0] STATE_WAIT_DATA_REQ = 4'd5;
localparam bit [3:0] STATE_CMD17_READ_SINGLE_BLOCK = 4'd6;
localparam bit [3:0] STATE_CMD24_WRITE_SINGLE_BLOCK = 4'd7;
localparam bit [3:0] STATE_WAIT_DATA_START = 4'd8;
localparam bit [3:0] STATE_READING_DATA = 4'd9;
localparam bit [3:0] STATE_READING_CRC15 = 4'd10;
localparam bit [3:0] STATE_READING_END = 4'd11;

typedef struct {
    logic [6:0] clkcnt;
    logic cmd_req_valid;
    logic [5:0] cmd_req_cmd;
    logic [31:0] cmd_req_arg;
    logic [2:0] cmd_req_rn;
    logic [5:0] cmd_resp_cmd;
    logic [31:0] cmd_resp_arg32;
    logic [6:0] cmd_resp_r1;
    logic [7:0] cmd_resp_r2;
    logic [31:0] data_addr;
    logic [511:0] data_data;
    logic data_resp_valid;
    logic wdog_ena;
    logic crc16_clear;
    logic [15:0] crc16_calc0;
    logic [15:0] crc16_rx0;
    logic dat_csn;
    logic dat_reading;
    logic err_clear;
    logic err_valid;
    logic [3:0] err_code;
    logic sck_400khz_ena;
    logic [3:0] state;
    logic wait_cmd_resp;
    logic [2:0] sdtype;
    logic HCS;                                              // High Capacity Support
    logic S18;                                              // 1.8V Low voltage
    logic [23:0] OCR_VoltageWindow;                         // all ranges 2.7 to 3.6 V
    logic [11:0] bitcnt;
} sdctrl_spimode_registers;

const sdctrl_spimode_registers sdctrl_spimode_r_reset = '{
    '0,                                 // clkcnt
    1'b0,                               // cmd_req_valid
    '0,                                 // cmd_req_cmd
    '0,                                 // cmd_req_arg
    '0,                                 // cmd_req_rn
    '0,                                 // cmd_resp_cmd
    '0,                                 // cmd_resp_arg32
    '0,                                 // cmd_resp_r1
    '0,                                 // cmd_resp_r2
    '0,                                 // data_addr
    '0,                                 // data_data
    1'b0,                               // data_resp_valid
    1'b0,                               // wdog_ena
    1'b1,                               // crc16_clear
    '0,                                 // crc16_calc0
    '0,                                 // crc16_rx0
    1'b1,                               // dat_csn
    1'b0,                               // dat_reading
    1'b0,                               // err_clear
    1'b0,                               // err_valid
    '0,                                 // err_code
    1'b1,                               // sck_400khz_ena
    STATE_CMD0,                         // state
    1'b0,                               // wait_cmd_resp
    SDCARD_UNKNOWN,                     // sdtype
    1'b1,                               // HCS
    1'b0,                               // S18
    24'hff8000,                         // OCR_VoltageWindow
    '0                                  // bitcnt
};

endpackage: sdctrl_spimode_pkg
