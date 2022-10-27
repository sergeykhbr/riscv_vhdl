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
package l2_dst_pkg;

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;

localparam bit [2:0] Idle = 3'h0;
localparam bit [2:0] CacheReadReq = 3'h1;
localparam bit [2:0] CacheWriteReq = 3'h2;
localparam bit [2:0] ReadMem = 3'h3;
localparam bit [2:0] WriteMem = 3'h4;
localparam bit [2:0] snoop_ac = 3'h5;
localparam bit [2:0] snoop_cr = 3'h6;
localparam bit [2:0] snoop_cd = 3'h7;

typedef struct {
    logic [2:0] state;
    logic [2:0] srcid;
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr;
    logic [2:0] req_size;
    logic [2:0] req_prot;
    logic [4:0] req_src;
    logic [L2_REQ_TYPE_BITS-1:0] req_type;
    logic [L1CACHE_LINE_BITS-1:0] req_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] req_wstrb;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] ac_valid;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] cr_ready;
    logic [(CFG_SLOT_L1_TOTAL + 1)-1:0] cd_ready;
} L2Destination_registers;

const L2Destination_registers L2Destination_r_reset = '{
    Idle,                               // state
    CFG_SLOT_L1_TOTAL,                  // srcid
    '0,                                 // req_addr
    '0,                                 // req_size
    '0,                                 // req_prot
    '0,                                 // req_src
    '0,                                 // req_type
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // ac_valid
    '0,                                 // cr_ready
    '0                                  // cd_ready
};

endpackage: l2_dst_pkg
