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
package icache_lru_pkg;

import river_cfg_pkg::*;

localparam int abus = CFG_CPU_ADDR_BITS;
localparam int lnbits = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
localparam int flbits = ITAG_FL_TOTAL;
// State machine states:
localparam bit [3:0] State_Idle = 4'h0;
localparam bit [3:0] State_CheckHit = 4'h1;
localparam bit [3:0] State_TranslateAddress = 4'h2;
localparam bit [3:0] State_WaitGrant = 4'h3;
localparam bit [3:0] State_WaitResp = 4'h4;
localparam bit [3:0] State_CheckResp = 4'h5;
localparam bit [3:0] State_SetupReadAdr = 4'h6;
localparam bit [3:0] State_FlushAddr = 4'h8;
localparam bit [3:0] State_FlushCheck = 4'h9;
localparam bit [3:0] State_Reset = 4'ha;
localparam bit [3:0] State_ResetWrite = 4'hb;

localparam bit [CFG_CPU_ADDR_BITS-1:0] LINE_BYTES_MASK = ((2**CFG_LOG2_L1CACHE_BYTES_PER_LINE) - 1);

typedef struct {
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr;
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr_next;
    logic [CFG_CPU_ADDR_BITS-1:0] write_addr;
    logic [3:0] state;
    logic req_mem_valid;
    logic [CFG_CPU_ADDR_BITS-1:0] mem_addr;
    logic [REQ_MEM_TYPE_BITS-1:0] req_mem_type;
    logic [2:0] req_mem_size;
    logic load_fault;
    logic req_flush;                                        // init flush request
    logic req_flush_all;
    logic [CFG_CPU_ADDR_BITS-1:0] req_flush_addr;           // [0]=1 flush all
    logic [31:0] req_flush_cnt;
    logic [31:0] flush_cnt;
    logic [L1CACHE_LINE_BITS-1:0] cache_line_i;
} ICacheLru_registers;

const ICacheLru_registers ICacheLru_r_reset = '{
    '0,                                 // req_addr
    '0,                                 // req_addr_next
    '0,                                 // write_addr
    State_Reset,                        // state
    1'b0,                               // req_mem_valid
    '0,                                 // mem_addr
    '0,                                 // req_mem_type
    '0,                                 // req_mem_size
    1'b0,                               // load_fault
    1'h1,                               // req_flush
    1'b0,                               // req_flush_all
    '0,                                 // req_flush_addr
    '0,                                 // req_flush_cnt
    '0,                                 // flush_cnt
    '0                                  // cache_line_i
};

endpackage: icache_lru_pkg
