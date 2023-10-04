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
package sdctrl_cache_pkg;

import sdctrl_cfg_pkg::*;

localparam int abus = CFG_SDCACHE_ADDR_BITS;
localparam int ibits = CFG_LOG2_SDCACHE_LINEBITS;
localparam int lnbits = CFG_LOG2_SDCACHE_BYTES_PER_LINE;
localparam int flbits = SDCACHE_FL_TOTAL;

// State machine states:
localparam bit [3:0] State_Idle = 4'h0;
localparam bit [3:0] State_CheckHit = 4'h2;
localparam bit [3:0] State_TranslateAddress = 4'h3;
localparam bit [3:0] State_WaitGrant = 4'h4;
localparam bit [3:0] State_WaitResp = 4'h5;
localparam bit [3:0] State_CheckResp = 4'h6;
localparam bit [3:0] State_SetupReadAdr = 4'h1;
localparam bit [3:0] State_WriteBus = 4'h7;
localparam bit [3:0] State_FlushAddr = 4'h8;
localparam bit [3:0] State_FlushCheck = 4'h9;
localparam bit [3:0] State_Reset = 4'ha;
localparam bit [3:0] State_ResetWrite = 4'hb;

localparam bit [CFG_SDCACHE_ADDR_BITS-1:0] LINE_BYTES_MASK = ((2**CFG_LOG2_SDCACHE_BYTES_PER_LINE) - 1);

typedef struct {
    logic req_write;
    logic [CFG_SDCACHE_ADDR_BITS-1:0] req_addr;
    logic [63:0] req_wdata;
    logic [7:0] req_wstrb;
    logic [3:0] state;
    logic req_mem_valid;
    logic req_mem_write;
    logic [CFG_SDCACHE_ADDR_BITS-1:0] mem_addr;
    logic mem_fault;
    logic write_first;
    logic write_flush;
    logic req_flush;
    logic [31:0] flush_cnt;
    logic [CFG_SDCACHE_ADDR_BITS-1:0] line_addr_i;
    logic [SDCACHE_LINE_BITS-1:0] cache_line_i;
    logic [SDCACHE_LINE_BITS-1:0] cache_line_o;
} sdctrl_cache_registers;

const sdctrl_cache_registers sdctrl_cache_r_reset = '{
    1'b0,                               // req_write
    '0,                                 // req_addr
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    State_Reset,                        // state
    1'b0,                               // req_mem_valid
    1'b0,                               // req_mem_write
    '0,                                 // mem_addr
    1'b0,                               // mem_fault
    1'b0,                               // write_first
    1'b0,                               // write_flush
    1'b0,                               // req_flush
    '0,                                 // flush_cnt
    '0,                                 // line_addr_i
    '0,                                 // cache_line_i
    '0                                  // cache_line_o
};

endpackage: sdctrl_cache_pkg
