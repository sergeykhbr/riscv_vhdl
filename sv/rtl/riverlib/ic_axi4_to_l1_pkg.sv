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
package ic_axi4_to_l1_pkg;

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;

localparam bit [3:0] Idle = 4'h0;                           // axi ar_ready=1,aw_ready=1
localparam bit [3:0] ReadLineRequest = 4'h1;                // l1 ar_valid=1
localparam bit [3:0] WaitReadLineResponse = 4'h2;           // l1 r_ready=1
localparam bit [3:0] WriteDataAccept = 4'h3;                // axi w_ready=1
localparam bit [3:0] WriteLineRequest = 4'h4;               // l1 w_valid=1
localparam bit [3:0] WaitWriteConfirmResponse = 4'h5;       // l1 b_ready
localparam bit [3:0] WaitWriteAccept = 4'h6;                // axi b_valid
localparam bit [3:0] WaitReadAccept = 4'h7;                 // axi r_valid
localparam bit [3:0] CheckBurst = 4'h8;

typedef struct {
    logic [3:0] state;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr;
    logic [CFG_SYSBUS_ID_BITS-1:0] req_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] req_user;
    logic [7:0] req_wstrb;
    logic [63:0] req_wdata;
    logic [7:0] req_len;
    logic [2:0] req_size;
    logic [2:0] req_prot;
    logic writing;
    logic read_modify_write;
    logic [L1CACHE_LINE_BITS-1:0] line_data;
    logic [L1CACHE_BYTES_PER_LINE-1:0] line_wstrb;
    logic [63:0] resp_data;
} ic_axi4_to_l1_registers;

const ic_axi4_to_l1_registers ic_axi4_to_l1_r_reset = '{
    Idle,                               // state
    '0,                                 // req_addr
    '0,                                 // req_id
    1'b0,                               // req_user
    '0,                                 // req_wstrb
    '0,                                 // req_wdata
    '0,                                 // req_len
    '0,                                 // req_size
    '0,                                 // req_prot
    1'b0,                               // writing
    1'b0,                               // read_modify_write
    '0,                                 // line_data
    '0,                                 // line_wstrb
    '0                                  // resp_data
};

endpackage: ic_axi4_to_l1_pkg
