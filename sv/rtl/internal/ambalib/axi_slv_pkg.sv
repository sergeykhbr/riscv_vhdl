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

localparam bit [4:0] State_r_idle = 5'd0;
localparam bit [4:0] State_r_addr = 5'h01;
localparam bit [4:0] State_r_data = 5'h02;
localparam bit [4:0] State_r_last = 5'h04;
localparam bit [4:0] State_r_buf = 5'h08;
localparam bit [4:0] State_r_wait_writing = 5'h10;
localparam bit [6:0] State_w_idle = 7'd0;
localparam bit [6:0] State_w_wait_reading = 7'h01;
localparam bit [6:0] State_w_wait_reading_light = 7'h02;
localparam bit [6:0] State_w_req = 7'h04;
localparam bit [6:0] State_w_pipe = 7'h08;
localparam bit [6:0] State_w_buf = 7'h10;
localparam bit [6:0] State_w_resp = 7'h20;
localparam bit [6:0] State_b = 7'h40;

typedef struct {
    logic [4:0] rstate;
    logic [6:0] wstate;
    logic ar_ready;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] ar_addr;
    logic [8:0] ar_len;
    logic [XSIZE_TOTAL-1:0] ar_bytes;
    logic [1:0] ar_burst;
    logic [CFG_SYSBUS_ID_BITS-1:0] ar_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] ar_user;
    logic ar_last;
    logic aw_ready;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] aw_addr;
    logic [XSIZE_TOTAL-1:0] aw_bytes;
    logic [1:0] aw_burst;
    logic [CFG_SYSBUS_ID_BITS-1:0] aw_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] aw_user;
    logic w_last;
    logic w_ready;
    logic r_valid;
    logic r_last;
    logic [CFG_SYSBUS_DATA_BITS-1:0] r_data;
    logic r_err;
    logic [CFG_SYSBUS_DATA_BITS-1:0] r_data_buf;
    logic r_err_buf;
    logic r_last_buf;
    logic b_err;
    logic b_valid;
    logic req_valid;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr;
    logic req_last;
    logic req_write;
    logic [CFG_SYSBUS_DATA_BITS-1:0] req_wdata;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] req_wstrb;
    logic [7:0] req_bytes;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr_buf;
    logic req_last_buf;
    logic [CFG_SYSBUS_DATA_BITS-1:0] req_wdata_buf;
    logic [CFG_SYSBUS_DATA_BYTES-1:0] req_wstrb_buf;
} axi_slv_registers;

const axi_slv_registers axi_slv_r_reset = '{
    State_r_idle,                       // rstate
    State_w_idle,                       // wstate
    1'b0,                               // ar_ready
    '0,                                 // ar_addr
    '0,                                 // ar_len
    '0,                                 // ar_bytes
    '0,                                 // ar_burst
    '0,                                 // ar_id
    '0,                                 // ar_user
    1'b0,                               // ar_last
    1'b0,                               // aw_ready
    '0,                                 // aw_addr
    '0,                                 // aw_bytes
    '0,                                 // aw_burst
    '0,                                 // aw_id
    '0,                                 // aw_user
    1'b0,                               // w_last
    1'b0,                               // w_ready
    1'b0,                               // r_valid
    1'b0,                               // r_last
    '0,                                 // r_data
    1'b0,                               // r_err
    '0,                                 // r_data_buf
    1'b0,                               // r_err_buf
    1'b0,                               // r_last_buf
    1'b0,                               // b_err
    1'b0,                               // b_valid
    1'b0,                               // req_valid
    '0,                                 // req_addr
    1'b0,                               // req_last
    1'b0,                               // req_write
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_bytes
    '0,                                 // req_addr_buf
    1'b0,                               // req_last_buf
    '0,                                 // req_wdata_buf
    '0                                  // req_wstrb_buf
};
endpackage: axi_slv_pkg
