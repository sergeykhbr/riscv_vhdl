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
package axi_slv_pkg;

import types_amba_pkg::*;
import types_pnp_pkg::*;

localparam bit [2:0] State_Idle = 3'd0;
localparam bit [2:0] State_w = 3'd1;
localparam bit [2:0] State_burst_w = 3'd2;
localparam bit [2:0] State_addr_r = 3'd3;
localparam bit [2:0] State_data_r = 3'd4;
localparam bit [2:0] State_out_r = 3'd5;
localparam bit [2:0] State_b = 3'd6;

typedef struct {
    logic [2:0] state;
    logic req_valid;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr;
    logic req_write;
    logic [CFG_SYSBUS_DATA_BITS-1:0] req_wdata;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] req_wstrb;
    logic [7:0] req_xsize;
    logic [7:0] req_len;
    logic [CFG_SYSBUS_USER_BITS-1:0] req_user;
    logic [CFG_SYSBUS_ID_BITS-1:0] req_id;
    logic [1:0] req_burst;
    logic req_last_a;
    logic req_last_r;
    logic req_done;
    logic resp_valid;
    logic resp_last;
    logic [CFG_SYSBUS_DATA_BITS-1:0] resp_rdata;
    logic resp_err;
} axi_slv_registers;

const axi_slv_registers axi_slv_r_reset = '{
    State_Idle,                         // state
    1'b0,                               // req_valid
    '0,                                 // req_addr
    1'b0,                               // req_write
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_xsize
    '0,                                 // req_len
    '0,                                 // req_user
    '0,                                 // req_id
    '0,                                 // req_burst
    1'b0,                               // req_last_a
    1'b0,                               // req_last_r
    1'b0,                               // req_done
    1'b0,                               // resp_valid
    1'b0,                               // resp_last
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

endpackage: axi_slv_pkg
