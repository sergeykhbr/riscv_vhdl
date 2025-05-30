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
package dcache_lru_pkg;

import river_cfg_pkg::*;
import target_cfg_pkg::*;

localparam int abus = CFG_CPU_ADDR_BITS;
localparam int lnbits = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
localparam int flbits = DTAG_FL_TOTAL;
localparam int ways = (2**CFG_DLOG2_NWAYS);

// State machine states:
localparam bit [3:0] State_Idle = 4'd0;
localparam bit [3:0] State_CheckHit = 4'd1;
localparam bit [3:0] State_TranslateAddress = 4'd2;
localparam bit [3:0] State_WaitGrant = 4'd3;
localparam bit [3:0] State_WaitResp = 4'd4;
localparam bit [3:0] State_CheckResp = 4'd5;
localparam bit [3:0] State_SetupReadAdr = 4'd6;
localparam bit [3:0] State_WriteBus = 4'd7;
localparam bit [3:0] State_FlushAddr = 4'd8;
localparam bit [3:0] State_FlushCheck = 4'd9;
localparam bit [3:0] State_Reset = 4'd10;
localparam bit [3:0] State_ResetWrite = 4'd11;
localparam bit [3:0] State_SnoopSetupAddr = 4'd12;
localparam bit [3:0] State_SnoopReadData = 4'd13;

localparam bit [CFG_CPU_ADDR_BITS-1:0] LINE_BYTES_MASK = ((2**CFG_LOG2_L1CACHE_BYTES_PER_LINE) - 1);
localparam bit [31:0] FLUSH_ALL_VALUE = ((2**(CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS)) - 1);// Actual bitwidth is (ibits + waybits) but to avoid sc template generation use 32-bits

typedef struct {
    logic [MemopType_Total-1:0] req_type;
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr;
    logic [63:0] req_wdata;
    logic [7:0] req_wstrb;
    logic [2:0] req_size;
    logic [3:0] state;
    logic req_mem_valid;
    logic [REQ_MEM_TYPE_BITS-1:0] req_mem_type;
    logic [2:0] req_mem_size;
    logic [CFG_CPU_ADDR_BITS-1:0] mem_addr;
    logic load_fault;
    logic write_first;
    logic write_flush;
    logic write_share;
    logic [L1CACHE_BYTES_PER_LINE-1:0] mem_wstrb;
    logic req_flush;                                        // init flush request
    logic req_flush_all;
    logic [CFG_CPU_ADDR_BITS-1:0] req_flush_addr;           // [0]=1 flush all
    logic [31:0] req_flush_cnt;
    logic [31:0] flush_cnt;
    logic [L1CACHE_LINE_BITS-1:0] cache_line_i;
    logic [L1CACHE_LINE_BITS-1:0] cache_line_o;
    logic [SNOOP_REQ_TYPE_BITS-1:0] req_snoop_type;
    logic snoop_flags_valid;
    logic snoop_restore_wait_resp;
    logic snoop_restore_write_bus;
    logic [CFG_CPU_ADDR_BITS-1:0] req_addr_restore;
} DCacheLru_registers;

const DCacheLru_registers DCacheLru_r_reset = '{
    '0,                                 // req_type
    '0,                                 // req_addr
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_size
    State_Reset,                        // state
    1'b0,                               // req_mem_valid
    '0,                                 // req_mem_type
    '0,                                 // req_mem_size
    '0,                                 // mem_addr
    1'b0,                               // load_fault
    1'b0,                               // write_first
    1'b0,                               // write_flush
    1'b0,                               // write_share
    '0,                                 // mem_wstrb
    1'b1,                               // req_flush
    1'b0,                               // req_flush_all
    '0,                                 // req_flush_addr
    '0,                                 // req_flush_cnt
    '0,                                 // flush_cnt
    '0,                                 // cache_line_i
    '0,                                 // cache_line_o
    '0,                                 // req_snoop_type
    1'b0,                               // snoop_flags_valid
    1'b0,                               // snoop_restore_wait_resp
    1'b0,                               // snoop_restore_write_bus
    '0                                  // req_addr_restore
};

endpackage: dcache_lru_pkg
