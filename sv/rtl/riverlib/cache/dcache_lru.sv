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

`timescale 1ns/10ps

module DCacheLru #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned waybits = 2,                     // Log2 of number of ways. Default 2: 4 ways
    parameter int unsigned ibits = 7,                       // Log2 of number of lines per way: 7=16KB; 8=32KB; .. (if bytes per line = 32 B)
    parameter bit coherence_ena = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // Data path:
    input logic i_req_valid,
    input logic [river_cfg_pkg::MemopType_Total-1:0] i_req_type,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_addr,
    input logic [63:0] i_req_wdata,
    input logic [7:0] i_req_wstrb,
    input logic [1:0] i_req_size,
    output logic o_req_ready,
    output logic o_resp_valid,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_resp_addr,
    output logic [63:0] o_resp_data,
    output logic o_resp_er_load_fault,
    output logic o_resp_er_store_fault,
    input logic i_resp_ready,
    // Memory interface:
    input logic i_req_mem_ready,
    output logic o_req_mem_valid,
    output logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] o_req_mem_type,
    output logic [2:0] o_req_mem_size,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_req_mem_addr,
    output logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] o_req_mem_strob,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_req_mem_data,
    input logic i_mem_data_valid,
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_mem_data,
    input logic i_mem_load_fault,
    input logic i_mem_store_fault,
    // Mpu interface
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_mpu_addr,
    input logic i_pma_cached,
    input logic i_pmp_r,                                    // PMP Read access
    input logic i_pmp_w,                                    // PMP Write access
    // D$ Snoop interface
    input logic i_req_snoop_valid,
    input logic [river_cfg_pkg::SNOOP_REQ_TYPE_BITS-1:0] i_req_snoop_type,
    output logic o_req_snoop_ready,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_snoop_addr,
    input logic i_resp_snoop_ready,
    output logic o_resp_snoop_valid,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_resp_snoop_data,
    output logic [river_cfg_pkg::DTAG_FL_TOTAL-1:0] o_resp_snoop_flags,
    // Debug interface
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_flush_address,
    input logic i_flush_valid,
    output logic o_flush_end
);

import river_cfg_pkg::*;
import dcache_lru_pkg::*;

localparam int ways = (2**waybits);
localparam bit [31:0] FLUSH_ALL_VALUE = ((2**(ibits + waybits)) - 1);

logic line_direct_access_i;
logic line_invalidate_i;
logic line_re_i;
logic line_we_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_addr_i;
logic [L1CACHE_LINE_BITS-1:0] line_wdata_i;
logic [L1CACHE_BYTES_PER_LINE-1:0] line_wstrb_i;
logic [DTAG_FL_TOTAL-1:0] line_wflags_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_raddr_o;
logic [L1CACHE_LINE_BITS-1:0] line_rdata_o;
logic [DTAG_FL_TOTAL-1:0] line_rflags_o;
logic line_hit_o;
// Snoop signals:
logic [CFG_CPU_ADDR_BITS-1:0] line_snoop_addr_i;
logic line_snoop_ready_o;
logic [DTAG_FL_TOTAL-1:0] line_snoop_flags_o;
DCacheLru_registers r, rin;

TagMemNWay #(
    .async_reset(async_reset),
    .abus(abus),
    .waybits(waybits),
    .ibits(ibits),
    .lnbits(lnbits),
    .flbits(flbits),
    .snoop(1)
) mem0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_direct_access(line_direct_access_i),
    .i_invalidate(line_invalidate_i),
    .i_re(line_re_i),
    .i_we(line_we_i),
    .i_addr(line_addr_i),
    .i_wdata(line_wdata_i),
    .i_wstrb(line_wstrb_i),
    .i_wflags(line_wflags_i),
    .o_raddr(line_raddr_o),
    .o_rdata(line_rdata_o),
    .o_rflags(line_rflags_o),
    .o_hit(line_hit_o),
    .i_snoop_addr(line_snoop_addr_i),
    .o_snoop_ready(line_snoop_ready_o),
    .o_snoop_flags(line_snoop_flags_o)
);


always_comb
begin: comb_proc
    DCacheLru_registers v;
    logic [L1CACHE_LINE_BITS-1:0] vb_cache_line_i_modified;
    logic [L1CACHE_LINE_BITS-1:0] vb_line_rdata_o_modified;
    logic [L1CACHE_BYTES_PER_LINE-1:0] vb_line_rdata_o_wstrb;
    logic v_req_ready;
    logic [L1CACHE_LINE_BITS-1:0] t_cache_line_i;
    logic [63:0] vb_cached_data;
    logic [63:0] vb_uncached_data;
    logic v_resp_valid;
    logic [63:0] vb_resp_data;
    logic v_resp_er_load_fault;
    logic v_resp_er_store_fault;
    logic v_direct_access;
    logic v_invalidate;
    logic v_flush_end;
    logic v_line_cs_read;
    logic v_line_cs_write;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_line_addr;
    logic [L1CACHE_LINE_BITS-1:0] vb_line_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] vb_line_wstrb;
    logic [63:0] vb_req_mask;
    logic [DTAG_FL_TOTAL-1:0] v_line_wflags;
    logic [(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 3)-1:0] ridx;
    logic v_req_same_line;
    logic v_ready_next;
    logic v_req_snoop_ready;
    logic v_req_snoop_ready_on_wait;
    logic v_resp_snoop_valid;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_addr_direct_next;
    logic [MemopType_Total-1:0] t_req_type;

    vb_cache_line_i_modified = 0;
    vb_line_rdata_o_modified = 0;
    vb_line_rdata_o_wstrb = 0;
    v_req_ready = 0;
    t_cache_line_i = 0;
    vb_cached_data = 0;
    vb_uncached_data = 0;
    v_resp_valid = 0;
    vb_resp_data = 0;
    v_resp_er_load_fault = 0;
    v_resp_er_store_fault = 0;
    v_direct_access = 0;
    v_invalidate = 0;
    v_flush_end = 0;
    v_line_cs_read = 0;
    v_line_cs_write = 0;
    vb_line_addr = 0;
    vb_line_wdata = 0;
    vb_line_wstrb = 0;
    vb_req_mask = 0;
    v_line_wflags = 0;
    ridx = 0;
    v_req_same_line = 0;
    v_ready_next = 0;
    v_req_snoop_ready = 0;
    v_req_snoop_ready_on_wait = 0;
    v_resp_snoop_valid = 0;
    vb_addr_direct_next = 0;
    t_req_type = 0;

    v = r;

    t_req_type = r.req_type;
    v_resp_snoop_valid = r.snoop_flags_valid;
    ridx = r.req_addr[(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 1): 3];

    vb_cached_data = line_rdata_o[(64 * int'(ridx)) +: 64];
    vb_uncached_data = r.cache_line_i[63: 0];

    if (r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE] == i_req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE]) begin
        v_req_same_line = 1'b1;
    end

    if (i_flush_valid == 1'b1) begin
        v.req_flush = 1'b1;
        v.req_flush_all = i_flush_address[0];
        if (i_flush_address[0] == 1'b1) begin
            v.req_flush_cnt = FLUSH_ALL_VALUE;
            v.req_flush_addr = '0;
        end else begin
            v.req_flush_cnt = '0;
            v.req_flush_addr = i_flush_address;
        end
    end

    for (int i = 0; i < 8; i++) begin
        if (r.req_wstrb[i] == 1'b1) begin
            vb_req_mask[(8 * i) +: 8] = 8'hff;
        end
    end

    vb_line_rdata_o_modified = line_rdata_o;
    vb_cache_line_i_modified = r.cache_line_i;
    for (int i = 0; i < (L1CACHE_BYTES_PER_LINE / 8); i++) begin
        if (i == int'(ridx)) begin
            vb_line_rdata_o_modified[(64 * i) +: 64] = ((vb_line_rdata_o_modified[(64 * i) +: 64]
                            & (~vb_req_mask))
                    | (r.req_wdata & vb_req_mask));
            vb_cache_line_i_modified[(64 * i) +: 64] = ((vb_cache_line_i_modified[(64 * i) +: 64]
                            & (~vb_req_mask))
                    | (r.req_wdata & vb_req_mask));
            vb_line_rdata_o_wstrb[(8 * i) +: 8] = r.req_wstrb;
        end
    end

    // Flush counter when direct access
    if (r.req_addr[(waybits - 1): 0] == (ways - 1)) begin
        vb_addr_direct_next = ((r.req_addr + L1CACHE_BYTES_PER_LINE)
                & (~LINE_BYTES_MASK));
    end else begin
        vb_addr_direct_next = (r.req_addr + 1);
    end

    vb_line_addr = r.req_addr;
    vb_line_wdata = r.cache_line_i;

    // System Bus access state machine
    case (r.state)
    State_Idle: begin
        v.load_fault = 1'b0;
        v_ready_next = 1'b1;
    end
    State_CheckHit: begin
        vb_resp_data = vb_cached_data;
        if (line_hit_o == 1'b1) begin
            // Hit
            v_resp_valid = 1'b1;
            if (i_resp_ready == 1'b1) begin
                if (r.req_type[MemopType_Store] == 1'b1) begin
                    // Modify tagged mem output with request and write back
                    v_line_cs_write = 1'b1;
                    v_line_wflags[TAG_FL_VALID] = 1'b1;
                    v_line_wflags[DTAG_FL_DIRTY] = 1'b1;
                    v_line_wflags[DTAG_FL_RESERVED] = 1'b0;
                    t_req_type[MemopType_Store] = 1'b0;
                    v.req_type = t_req_type;                // clear MemopType_Store bit
                    vb_line_wstrb = vb_line_rdata_o_wstrb;
                    vb_line_wdata = vb_line_rdata_o_modified;
                    if (r.req_type[MemopType_Release] == 1'b1) begin
                        vb_resp_data = '0;
                    end
                    if ((r.req_type[MemopType_Release] == 1'b1)
                            && (line_rflags_o[DTAG_FL_RESERVED] == 1'b0)) begin
                        // ignore writing if cacheline wasn't reserved before:
                        v_line_cs_write = 1'b0;
                        vb_line_wstrb = '0;
                        vb_resp_data = 64'd1;               // return error
                        v.state = State_Idle;
                    end else begin
                        if (coherence_ena && (line_rflags_o[DTAG_FL_SHARED] == 1'b1)) begin
                            // Make line: 'shared' -> 'unique' using write request
                            v.write_share = 1'b1;
                            v.state = State_TranslateAddress;
                        end else begin
                            if (v_req_same_line == 1'b1) begin
                                // Write address is the same as the next requested, so use it to write
                                // value and update state machine
                                v_ready_next = 1'b1;
                            end
                            v.state = State_Idle;
                        end
                    end
                end else if ((r.req_type[MemopType_Store] == 1'b0)
                            && (r.req_type[MemopType_Reserve] == 1'b1)) begin
                    // Load with reserve
                    v_line_cs_write = 1'b1;
                    v_line_wflags = line_rflags_o;
                    v_line_wflags[DTAG_FL_RESERVED] = 1'b1;
                    vb_line_wdata = vb_cached_data;
                    vb_line_wstrb = '1;                     // flags will be modified only if wstrb != 0
                    v.state = State_Idle;
                end else begin
                    v_ready_next = 1'b1;
                    v.state = State_Idle;
                end
            end
        end else begin
            // Miss
            if ((r.req_type[MemopType_Store] == 1'b1)
                    && (r.req_type[MemopType_Release] == 1'b1)) begin
                vb_resp_data = 64'd1;                       // return error. Cannot store into unreserved cache line
                v.state = State_Idle;
            end else begin
                v.state = State_TranslateAddress;
            end
        end
    end
    State_TranslateAddress: begin
        if ((r.req_type[MemopType_Store] == 1'b1) && (i_pmp_w == 1'b0)) begin
            v.load_fault = 1'b1;
            t_cache_line_i = '0;
            v.cache_line_i = (~t_cache_line_i);
            v.state = State_CheckResp;
        end else if ((r.req_type[MemopType_Store] == 1'b0) && (i_pmp_r == 1'b0)) begin
            v.load_fault = 1'b1;
            t_cache_line_i = '0;
            v.cache_line_i = (~t_cache_line_i);
            v.state = State_CheckResp;
        end else begin
            v.req_mem_valid = 1'b1;
            v.load_fault = 1'b0;
            v.state = State_WaitGrant;
            if (i_pma_cached == 1'b1) begin
                // Cached:
                if (r.write_share == 1'b1) begin
                    v.req_mem_type = WriteLineUnique();
                    v.mem_addr = {line_raddr_o[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                end else if ((line_rflags_o[TAG_FL_VALID] == 1'b1)
                            && (line_rflags_o[DTAG_FL_DIRTY] == 1'b1)) begin
                    v.write_first = 1'b1;
                    v.req_mem_type = WriteBack();
                    v.mem_addr = {line_raddr_o[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                end else begin
                    // 1. Read -> Save cache
                    // 2. Read -> Modify -> Save cache
                    v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                    if (r.req_type[MemopType_Store] == 1'b1) begin
                        v.req_mem_type = ReadMakeUnique();
                    end else begin
                        v.req_mem_type = ReadShared();
                    end
                end
                v.req_mem_size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
                v.mem_wstrb = '1;
                v.cache_line_o = line_rdata_o;
            end else begin
                // Uncached read/write
                v.mem_addr = r.req_addr;
                v.mem_wstrb = {'0, r.req_wstrb};
                v.req_mem_size = r.req_size;
                if (r.req_type[MemopType_Store] == 1'b1) begin
                    v.req_mem_type = WriteNoSnoop();
                end else begin
                    v.req_mem_type = ReadNoSnoop();
                end
                t_cache_line_i = '0;
                t_cache_line_i[63: 0] = r.req_wdata;
                v.cache_line_o = t_cache_line_i;
            end
        end
        v.cache_line_i = '0;
    end
    State_WaitGrant: begin
        if (i_req_mem_ready == 1'b1) begin
            if ((r.write_flush == 1'b1)
                    || (r.write_first == 1'b1)
                    || (r.write_share == 1'b1)
                    || ((r.req_type[MemopType_Store] == 1'b1)
                            && (r.req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b0))) begin
                v.state = State_WriteBus;
            end else begin
                // 1. uncached read
                // 2. cached read or write
                v.state = State_WaitResp;
            end
            v.req_mem_valid = 1'b0;
        end
    end
    State_WaitResp: begin
        if (i_mem_data_valid == 1'b1) begin
            v.cache_line_i = i_mem_data;
            v.state = State_CheckResp;
            if (i_mem_load_fault == 1'b1) begin
                v.load_fault = 1'b1;
            end
        end else if (coherence_ena
                    && ((i_req_snoop_valid == 1'b1) && ((|i_req_snoop_type) == 1'b1))) begin
            // Access cache data
            v_req_snoop_ready_on_wait = 1'b1;
            v.snoop_restore_wait_resp = 1'b1;
            v.req_addr_restore = r.req_addr;
            v.req_addr = i_req_snoop_addr;
            v.req_snoop_type = i_req_snoop_type;
            v.state = State_SnoopSetupAddr;
        end
    end
    State_CheckResp: begin
        if ((r.req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b0)
                || (r.load_fault == 1'b1)) begin
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1'b1;
            vb_resp_data = vb_uncached_data;
            v_resp_er_store_fault = (r.load_fault && r.req_type[MemopType_Store]);
            v_resp_er_load_fault = (r.load_fault && (~r.req_type[MemopType_Store]));
            if (i_resp_ready == 1'b1) begin
                v.state = State_Idle;
            end
        end else begin
            v.state = State_SetupReadAdr;
            v_line_cs_write = 1'b1;
            v_line_wflags[TAG_FL_VALID] = 1'b1;
            v_line_wflags[DTAG_FL_SHARED] = 1'b1;
            v_line_wflags[DTAG_FL_RESERVED] = r.req_type[MemopType_Reserve];
            vb_line_wstrb = '1;                             // write full line
            if (r.req_type[MemopType_Store] == 1'b1) begin
                // Modify tagged mem output with request before write
                t_req_type[MemopType_Store] = 1'b0;
                v.req_type = t_req_type;                    // clear MemopType_Store bit
                v_line_wflags[DTAG_FL_DIRTY] = 1'b1;
                v_line_wflags[DTAG_FL_SHARED] = 1'b0;
                v_line_wflags[DTAG_FL_RESERVED] = 1'b0;
                vb_line_wdata = vb_cache_line_i_modified;
                v_resp_valid = 1'b1;
                v.state = State_Idle;
            end
        end
    end
    State_SetupReadAdr: begin
        v.state = State_CheckHit;
    end
    State_WriteBus: begin
        if (i_mem_data_valid == 1'b1) begin
            if (r.write_share == 1'b1) begin
                v.write_share = 1'b0;
                v.state = State_Idle;
            end else if (r.write_flush == 1'b1) begin
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            end else if (r.write_first == 1'b1) begin
                // Obsolete line was offloaded, now read new line
                v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                v.req_mem_valid = 1'b1;
                v.write_first = 1'b0;
                if (r.req_type[MemopType_Store] == 1'b1) begin
                    // read request: read-modify-save cache line
                    v.req_mem_type = ReadMakeUnique();
                end else begin
                    v.req_mem_type = ReadShared();
                end
                v.state = State_WaitGrant;
            end else begin
                // Non-cached write
                v.state = State_Idle;
                v_resp_valid = 1'b1;
                v_resp_er_store_fault = i_mem_store_fault;
            end
        end else if (coherence_ena
                    && ((i_req_snoop_valid == 1'b1) && ((|i_req_snoop_type) == 1'b1))) begin
            // Access cache data cannot be in the same clock as i_mem_data_valid
            v_req_snoop_ready_on_wait = 1'b1;
            v.snoop_restore_write_bus = 1'b1;
            v.req_addr_restore = r.req_addr;
            v.req_addr = i_req_snoop_addr;
            v.req_snoop_type = i_req_snoop_type;
            v.state = State_SnoopSetupAddr;
        end
    end
    State_FlushAddr: begin
        v.state = State_FlushCheck;
        v_direct_access = r.req_flush_all;                  // 0=only if hit; 1=will be applied ignoring hit
        v_invalidate = 1'b1;                                // generate: wstrb='1; wflags='0
        v.write_flush = 1'b0;
        v.cache_line_i = '0;
    end
    State_FlushCheck: begin
        v.cache_line_o = line_rdata_o;
        v_direct_access = r.req_flush_all;
        v_line_cs_write = r.req_flush_all;
        if ((line_rflags_o[TAG_FL_VALID] == 1'b1)
                && (line_rflags_o[DTAG_FL_DIRTY] == 1'b1)) begin
            // Off-load valid line
            v.write_flush = 1'b1;
            v.mem_addr = line_raddr_o;
            v.req_mem_valid = 1'b1;
            v.req_mem_type = WriteBack();
            v.mem_wstrb = '1;
            v.state = State_WaitGrant;
        end else begin
            // Write clean line
            v.state = State_FlushAddr;
            if ((|r.flush_cnt) == 1'b0) begin
                v.state = State_Idle;
                v_flush_end = 1'b1;
            end
        end
        if ((|r.flush_cnt) == 1'b1) begin
            v.flush_cnt = (r.flush_cnt - 1);
            if (r.req_flush_all == 1'b1) begin
                v.req_addr = vb_addr_direct_next;
            end else begin
                v.req_addr = (r.req_addr + L1CACHE_BYTES_PER_LINE);
            end
        end
    end
    State_Reset: begin
        // Write clean line
        if (r.req_flush == 1'b1) begin
            v.req_flush = 1'b0;
            v.flush_cnt = FLUSH_ALL_VALUE;                  // Init after power-on-reset
        end
        v_direct_access = 1'b1;
        v_invalidate = 1'b1;                                // generate: wstrb='1; wflags='0
        v.state = State_ResetWrite;
    end
    State_ResetWrite: begin
        v_direct_access = 1'b1;
        v_line_cs_write = 1'b1;
        v.state = State_Reset;
        if ((|r.flush_cnt) == 1'b1) begin
            v.flush_cnt = (r.flush_cnt - 1);
            v.req_addr = vb_addr_direct_next;
        end else begin
            v.state = State_Idle;
        end
    end
    State_SnoopSetupAddr: begin
        v.state = State_SnoopReadData;
        v_invalidate = r.req_snoop_type[SNOOP_REQ_TYPE_READCLEAN];
    end
    State_SnoopReadData: begin
        v_resp_snoop_valid = 1'b1;
        if (r.req_snoop_type[SNOOP_REQ_TYPE_READCLEAN] == 1'b0) begin
            v_line_cs_write = 1'b1;
            vb_line_wdata = line_rdata_o;
            vb_line_wstrb = '1;
            v_line_wflags = line_rflags_o;
            v_line_wflags[DTAG_FL_DIRTY] = 1'b0;
            v_line_wflags[DTAG_FL_SHARED] = 1'b1;
            v_line_wflags[DTAG_FL_RESERVED] = 1'b0;
        end
        // restore state
        v.snoop_restore_wait_resp = 1'b0;
        v.snoop_restore_write_bus = 1'b0;
        if (r.snoop_restore_wait_resp == 1'b1) begin
            v.req_addr = r.req_addr_restore;
            v.state = State_WaitResp;
        end else if (r.snoop_restore_write_bus == 1'b1) begin
            v.req_addr = r.req_addr_restore;
            v.state = State_WriteBus;
        end else begin
            v.state = State_Idle;
        end
    end
    default: begin
    end
    endcase

    v_req_snoop_ready = ((line_snoop_ready_o && (~(|i_req_snoop_type)))
            || (coherence_ena && v_ready_next && (|i_req_snoop_type))
            || v_req_snoop_ready_on_wait);

    v.snoop_flags_valid = (i_req_snoop_valid
            && line_snoop_ready_o
            && (~(|i_req_snoop_type)));

    if (v_ready_next == 1'b1) begin
        if (coherence_ena
                && ((i_req_snoop_valid == 1'b1) && ((|i_req_snoop_type) == 1'b1))) begin
            // Access cache data
            v.req_addr = i_req_snoop_addr;
            v.req_snoop_type = i_req_snoop_type;
            v.state = State_SnoopSetupAddr;
        end else if (r.req_flush == 1'b1) begin
            v.state = State_FlushAddr;
            v.req_flush = 1'b0;
            v.cache_line_i = '0;
            v.req_addr = (r.req_flush_addr & (~LINE_BYTES_MASK));
            v.req_mem_size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
            v.flush_cnt = r.req_flush_cnt;
        end else begin
            v_req_ready = 1'b1;
            v_line_cs_read = i_req_valid;
            vb_line_addr = i_req_addr;
            if (i_req_valid == 1'b1) begin
                v.req_addr = i_req_addr;
                v.req_wstrb = i_req_wstrb;
                v.req_size = {1'h0, i_req_size};
                v.req_wdata = i_req_wdata;
                v.req_type = i_req_type;
                v.state = State_CheckHit;
            end
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = DCacheLru_r_reset;
    end

    line_direct_access_i = v_direct_access;
    line_invalidate_i = v_invalidate;
    line_re_i = v_line_cs_read;
    line_we_i = v_line_cs_write;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;
    line_snoop_addr_i = i_req_snoop_addr;

    o_req_ready = v_req_ready;
    o_req_mem_valid = r.req_mem_valid;
    o_req_mem_addr = r.mem_addr;
    o_req_mem_type = r.req_mem_type;
    o_req_mem_size = r.req_mem_size;
    o_req_mem_strob = r.mem_wstrb;
    o_req_mem_data = r.cache_line_o;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_addr = r.req_addr;
    o_resp_er_load_fault = v_resp_er_load_fault;
    o_resp_er_store_fault = v_resp_er_store_fault;
    o_mpu_addr = r.req_addr;

    o_req_snoop_ready = v_req_snoop_ready;
    o_resp_snoop_valid = v_resp_snoop_valid;
    o_resp_snoop_data = line_rdata_o;
    o_resp_snoop_flags = line_snoop_flags_o;

    o_flush_end = v_flush_end;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DCacheLru_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: DCacheLru
