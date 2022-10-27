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
package river_amba_pkg;

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;

localparam bit [2:0] state_idle = 3'h0;
localparam bit [2:0] state_ar = 3'h1;
localparam bit [2:0] state_r = 3'h2;
localparam bit [2:0] state_aw = 3'h3;
localparam bit [2:0] state_w = 3'h4;
localparam bit [2:0] state_b = 3'h5;
localparam bit [2:0] snoop_idle = 3'h0;
localparam bit [2:0] snoop_ac_wait_accept = 3'h1;
localparam bit [2:0] snoop_cr = 3'h2;
localparam bit [2:0] snoop_cr_wait_accept = 3'h3;
localparam bit [2:0] snoop_cd = 3'h4;
localparam bit [2:0] snoop_cd_wait_accept = 3'h5;

typedef struct {
    logic [2:0] state;
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr;
    logic req_path;
    logic [3:0] req_cached;
    logic [L1CACHE_LINE_BITS-1:0] req_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] req_wstrb;
    logic [2:0] req_size;
    logic [2:0] req_prot;
    logic [3:0] req_ar_snoop;
    logic [2:0] req_aw_snoop;
    logic [2:0] snoop_state;
    logic [CFG_CPU_ADDR_BITS-1:0] ac_addr;
    logic [3:0] ac_snoop;                                   // Table C3-19
    logic [4:0] cr_resp;
    logic [SNOOP_REQ_TYPE_BITS-1:0] req_snoop_type;
    logic [L1CACHE_LINE_BITS-1:0] resp_snoop_data;
    logic cache_access;
} RiverAmba_registers;

const RiverAmba_registers RiverAmba_r_reset = '{
    '0,                                 // state
    state_idle,                         // req_addr
    1'b0,                               // req_path
    '0,                                 // req_cached
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_size
    '0,                                 // req_prot
    '0,                                 // req_ar_snoop
    '0,                                 // req_aw_snoop
    snoop_idle,                         // snoop_state
    '0,                                 // ac_addr
    '0,                                 // ac_snoop
    '0,                                 // cr_resp
    '0,                                 // req_snoop_type
    '0,                                 // resp_snoop_data
    1'b0                                // cache_access
};

endpackage: river_amba_pkg
