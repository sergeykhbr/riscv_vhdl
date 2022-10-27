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
package l2dummy_pkg;

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;

localparam bit [2:0] Idle = 3'h0;
localparam bit [2:0] state_ar = 3'h1;
localparam bit [2:0] state_r = 3'h2;
localparam bit [2:0] l1_r_resp = 3'h3;
localparam bit [2:0] state_aw = 3'h4;
localparam bit [2:0] state_w = 3'h5;
localparam bit [2:0] state_b = 3'h6;
localparam bit [2:0] l1_w_resp = 3'h7;

typedef struct {
    logic [2:0] state;
    logic [2:0] srcid;
    logic [CFG_SYSBUS_ADDR_BITS-1:0] req_addr;
    logic [2:0] req_size;
    logic [2:0] req_prot;
    logic req_lock;
    logic [CFG_CPU_ID_BITS-1:0] req_id;
    logic [CFG_CPU_USER_BITS-1:0] req_user;
    logic [L1CACHE_LINE_BITS-1:0] req_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] req_wstrb;
    logic [L1CACHE_LINE_BITS-1:0] rdata;
    logic [1:0] resp;
} L2Dummy_registers;

const L2Dummy_registers L2Dummy_r_reset = '{
    Idle,                               // state
    CFG_SLOT_L1_TOTAL,                  // srcid
    '0,                                 // req_addr
    '0,                                 // req_size
    '0,                                 // req_prot
    1'b0,                               // req_lock
    1'b0,                               // req_id
    1'b0,                               // req_user
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // rdata
    '0                                  // resp
};

endpackage: l2dummy_pkg
