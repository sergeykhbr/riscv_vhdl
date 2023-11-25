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
package vip_sdcard_cmdio_pkg;


// 
// Receiver CMD state:
localparam bit [3:0] CMDSTATE_REQ_STARTBIT = 4'd0;
localparam bit [3:0] CMDSTATE_REQ_TXBIT = 4'd1;
localparam bit [3:0] CMDSTATE_REQ_CMD = 4'd2;
localparam bit [3:0] CMDSTATE_REQ_ARG = 4'd3;
localparam bit [3:0] CMDSTATE_REQ_CRC7 = 4'd4;
localparam bit [3:0] CMDSTATE_REQ_STOPBIT = 4'd5;
localparam bit [3:0] CMDSTATE_REQ_VALID = 4'd6;
localparam bit [3:0] CMDSTATE_WAIT_RESP = 4'd7;
localparam bit [3:0] CMDSTATE_RESP = 4'd8;
localparam bit [3:0] CMDSTATE_RESP_CRC7 = 4'd9;
localparam bit [3:0] CMDSTATE_RESP_STOPBIT = 4'd10;
localparam bit [3:0] CMDSTATE_INIT = 4'd15;

typedef struct {
    logic [7:0] clkcnt;
    logic cs;
    logic spi_mode;
    logic cmdz;
    logic cmd_dir;
    logic [47:0] cmd_rxshift;
    logic [47:0] cmd_txshift;
    logic [3:0] cmd_state;
    logic cmd_req_crc_err;
    logic [5:0] bitcnt;
    logic txbit;
    logic [6:0] crc_calc;
    logic [6:0] crc_rx;
    logic cmd_req_valid;
    logic [5:0] cmd_req_cmd;
    logic [31:0] cmd_req_data;
    logic cmd_resp_ready;
} vip_sdcard_cmdio_registers;

const vip_sdcard_cmdio_registers vip_sdcard_cmdio_r_reset = '{
    '0,                                 // clkcnt
    1'b0,                               // cs
    1'b0,                               // spi_mode
    1'b1,                               // cmdz
    1'b1,                               // cmd_dir
    '1,                                 // cmd_rxshift
    '1,                                 // cmd_txshift
    CMDSTATE_INIT,                      // cmd_state
    1'b0,                               // cmd_req_crc_err
    '0,                                 // bitcnt
    1'b0,                               // txbit
    '0,                                 // crc_calc
    '0,                                 // crc_rx
    1'b0,                               // cmd_req_valid
    '0,                                 // cmd_req_cmd
    '0,                                 // cmd_req_data
    1'b0                                // cmd_resp_ready
};

endpackage: vip_sdcard_cmdio_pkg
