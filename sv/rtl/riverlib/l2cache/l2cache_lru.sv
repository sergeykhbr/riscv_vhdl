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

module L2CacheLru #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned waybits = 4,                     // Log2 of number of ways. Default 4: 16 ways
    parameter int unsigned ibits = 9                        // Log2 of number of lines per way: 9=64KB (if bytes per line = 32 B)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_req_valid,
    input logic [river_cfg_pkg::L2_REQ_TYPE_BITS-1:0] i_req_type,
    input logic [2:0] i_req_size,
    input logic [2:0] i_req_prot,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_addr,
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_req_wdata,
    input logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] i_req_wstrb,
    output logic o_req_ready,
    output logic o_resp_valid,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_resp_rdata,
    output logic [1:0] o_resp_status,
    // Memory interface:
    input logic i_req_mem_ready,
    output logic o_req_mem_valid,
    output logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] o_req_mem_type,
    output logic [2:0] o_req_mem_size,
    output logic [2:0] o_req_mem_prot,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_req_mem_addr,
    output logic [river_cfg_pkg::L2CACHE_BYTES_PER_LINE-1:0] o_req_mem_strob,
    output logic [river_cfg_pkg::L2CACHE_LINE_BITS-1:0] o_req_mem_data,
    input logic i_mem_data_valid,
    input logic [river_cfg_pkg::L2CACHE_LINE_BITS-1:0] i_mem_data,
    input logic i_mem_data_ack,
    input logic i_mem_load_fault,
    input logic i_mem_store_fault,
    // Flush interface
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_flush_address,
    input logic i_flush_valid,
    output logic o_flush_end
);

import river_cfg_pkg::*;
import l2cache_lru_pkg::*;

localparam int ways = (2**waybits);
localparam bit [31:0] FLUSH_ALL_VALUE = ((2**(ibits + waybits)) - 1);

logic line_direct_access_i;
logic line_invalidate_i;
logic line_re_i;
logic line_we_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_addr_i;
logic [L2CACHE_LINE_BITS-1:0] line_wdata_i;
logic [L2CACHE_BYTES_PER_LINE-1:0] line_wstrb_i;
logic [L2TAG_FL_TOTAL-1:0] line_wflags_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_raddr_o;
logic [L2CACHE_LINE_BITS-1:0] line_rdata_o;
logic [L2TAG_FL_TOTAL-1:0] line_rflags_o;
logic line_hit_o;
// Snoop signals:
logic [CFG_CPU_ADDR_BITS-1:0] line_snoop_addr_i;
logic line_snoop_ready_o;
logic [L2TAG_FL_TOTAL-1:0] line_snoop_flags_o;
L2CacheLru_registers r, rin;

TagMemNWay #(
    .async_reset(async_reset),
    .abus(abus),
    .waybits(waybits),
    .ibits(ibits),
    .lnbits(lnbits),
    .flbits(flbits),
    .snoop(0)
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
    L2CacheLru_registers v;
    logic [L2CACHE_LINE_BITS-1:0] vb_cache_line_i_modified;
    logic [L2CACHE_LINE_BITS-1:0] vb_line_rdata_o_modified;
    logic [L2CACHE_BYTES_PER_LINE-1:0] vb_line_rdata_o_wstrb;
    logic v_req_ready;
    logic [L2CACHE_LINE_BITS-1:0] t_cache_line_i;
    logic [L1CACHE_LINE_BITS-1:0] vb_cached_data;
    logic [L1CACHE_LINE_BITS-1:0] vb_uncached_data;
    logic v_resp_valid;
    logic [L1CACHE_LINE_BITS-1:0] vb_resp_rdata;
    logic [L2_REQ_TYPE_BITS-1:0] vb_resp_status;
    logic v_direct_access;
    logic v_invalidate;
    logic v_flush_end;
    logic v_line_cs_read;
    logic v_line_cs_write;                                  // 'cs' should be active when write line and there's no new request
    logic [CFG_CPU_ADDR_BITS-1:0] vb_line_addr;
    logic [L2CACHE_LINE_BITS-1:0] vb_line_wdata;
    logic [L2CACHE_BYTES_PER_LINE-1:0] vb_line_wstrb;
    logic [L1CACHE_LINE_BITS-1:0] vb_req_mask;
    logic [L2TAG_FL_TOTAL-1:0] v_line_wflags;
    int ridx;
    logic v_req_same_line;
    logic v_ready_next;
    logic [L2_REQ_TYPE_BITS-1:0] vb_req_type;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_addr_direct_next;

    vb_cache_line_i_modified = 0;
    vb_line_rdata_o_modified = 0;
    vb_line_rdata_o_wstrb = 0;
    v_req_ready = 0;
    t_cache_line_i = 0;
    vb_cached_data = 0;
    vb_uncached_data = 0;
    v_resp_valid = 0;
    vb_resp_rdata = 0;
    vb_resp_status = 0;
    v_direct_access = 0;
    v_invalidate = 0;
    v_flush_end = 0;
    v_line_cs_read = 0;
    v_line_cs_write = 1'h0;
    vb_line_addr = 0;
    vb_line_wdata = 0;
    vb_line_wstrb = 0;
    vb_req_mask = 0;
    v_line_wflags = 0;
    ridx = 0;
    v_req_same_line = 0;
    v_ready_next = 0;
    vb_req_type = 0;
    vb_addr_direct_next = 0;

    v = r;

    vb_req_type = r.req_type;                               // systemc specific
    if (L2CACHE_LINE_BITS != L1CACHE_LINE_BITS) begin
        ridx = int'(r.req_addr[(CFG_L2_LOG2_BYTES_PER_LINE - 1): (CFG_L2_LOG2_BYTES_PER_LINE - CFG_LOG2_L1CACHE_BYTES_PER_LINE)]);
    end

    vb_cached_data = line_rdata_o[(ridx * L1CACHE_LINE_BITS) +: L1CACHE_LINE_BITS];
    vb_uncached_data = r.cache_line_i[(L1CACHE_LINE_BITS - 1): 0];

    if (r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_L2_LOG2_BYTES_PER_LINE] == i_req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_L2_LOG2_BYTES_PER_LINE]) begin
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

    for (int i = 0; i < L1CACHE_BYTES_PER_LINE; i++) begin
        if (r.req_wstrb[i] == 1'b1) begin
            vb_req_mask[(8 * i) +: 8] = 8'hff;
        end
    end

    vb_line_rdata_o_modified = line_rdata_o;
    vb_cache_line_i_modified = r.cache_line_i;
    for (int i = 0; i < (L2CACHE_BYTES_PER_LINE / L1CACHE_BYTES_PER_LINE); i++) begin
        if (i == ridx) begin
            vb_line_rdata_o_modified[(L1CACHE_LINE_BITS * i) +: L1CACHE_LINE_BITS] = ((vb_line_rdata_o_modified[(L1CACHE_LINE_BITS * i) +: L1CACHE_LINE_BITS]
                            & (~vb_req_mask))
                    | (r.req_wdata & vb_req_mask));
            vb_cache_line_i_modified[(L1CACHE_LINE_BITS * i) +: L1CACHE_LINE_BITS] = ((vb_cache_line_i_modified[(L1CACHE_LINE_BITS * i) +: L1CACHE_LINE_BITS]
                            & (~vb_req_mask))
                    | (r.req_wdata & vb_req_mask));
            vb_line_rdata_o_wstrb[(L1CACHE_BYTES_PER_LINE * i) +: L1CACHE_BYTES_PER_LINE] = r.req_wstrb;
        end
    end

    // Flush counter when direct access
    if (r.req_addr[(waybits - 1): 0] == (ways - 1)) begin
        vb_addr_direct_next = ((r.req_addr + L2CACHE_BYTES_PER_LINE)
                & (~LINE_BYTES_MASK));
    end else begin
        vb_addr_direct_next = (r.req_addr + 1);
    end

    vb_line_addr = r.req_addr;
    vb_line_wdata = r.cache_line_i;

    // System Bus access state machine
    case (r.state)
    State_Idle: begin
        v_ready_next = 1'b1;
    end
    State_CheckHit: begin
        if (line_hit_o == 1'b1) begin
            // Hit
            v_resp_valid = 1'b1;
            vb_resp_rdata = vb_cached_data;
            vb_resp_status = r.rb_resp;
            if (r.req_type[L2_REQ_TYPE_CACHED] == 1'b0) begin
                // Warning: This is a wrong case possible if MPU region changed
                //          without proper cache flushing.
                v_line_cs_write = 1'b0;
                v_invalidate = 1'b1;
                v.state = State_TranslateAddress;
            end else if (r.req_type[L2_REQ_TYPE_UNIQUE] == 1'b1) begin
                v_line_cs_write = 1'b0;
                v_invalidate = 1'b1;
            end else if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                // Modify tagged mem output with request and write back
                v_line_cs_write = 1'b1;
                v_line_wflags[TAG_FL_VALID] = 1'b1;
                v_line_wflags[L2TAG_FL_DIRTY] = 1'b1;
                vb_req_type[L2_REQ_TYPE_WRITE] = 1'b0;
                v.req_type = vb_req_type;
                vb_line_wstrb = vb_line_rdata_o_wstrb;
                vb_line_wdata = vb_line_rdata_o_modified;
                if (v_req_same_line == 1'b1) begin
                    // Write address is the same as the next requested, so use it to write
                    // value and update state machine
                    v_ready_next = 1'b1;
                end
            end else begin
                v_ready_next = 1'b1;
            end
            v.state = State_Idle;
        end else begin
            // Miss
            if ((r.req_type[L2_REQ_TYPE_WRITE] == 1'b1)
                    && (r.req_type[L2_REQ_TYPE_UNIQUE] == 1'b1)) begin
                // This command analog of invalidate line
                // no need to read it form memory
                v.state = State_Idle;
                v_resp_valid = 1'b1;
            end else if ((r.req_type[L2_REQ_TYPE_WRITE] == 1'b0)
                        && (r.req_type[L2_REQ_TYPE_UNIQUE] == 1'b0)
                        && (r.req_type[L2_REQ_TYPE_SNOOP] == 1'b1)) begin
                v.state = State_Idle;
                v_resp_valid = 1'b1;

                // Save into cache read via Snoop channel data
                v_line_cs_write = 1'b1;
                v_line_wflags[TAG_FL_VALID] = 1'b1;
                v_line_wflags[L2TAG_FL_DIRTY] = 1'b1;
                vb_line_wstrb = vb_line_rdata_o_wstrb;
                vb_line_wdata = vb_line_rdata_o_modified;
            end else begin
                v.state = State_TranslateAddress;
            end
        end
    end
    State_TranslateAddress: begin
        v.req_mem_valid = 1'b1;
        v.state = State_WaitGrant;

        if (r.req_type[L2_REQ_TYPE_CACHED] == 1'b1) begin
            if ((line_rflags_o[TAG_FL_VALID] == 1'b1)
                    && (line_rflags_o[L2TAG_FL_DIRTY] == 1'b1)) begin
                v.write_first = 1'b1;
                v.req_mem_type = WriteBack();
                v.mem_addr = {line_raddr_o[(CFG_CPU_ADDR_BITS - 1): CFG_L2_LOG2_BYTES_PER_LINE], {CFG_L2_LOG2_BYTES_PER_LINE{1'b0}}};
            end else begin
                v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_L2_LOG2_BYTES_PER_LINE], {CFG_L2_LOG2_BYTES_PER_LINE{1'b0}}};
                if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                    v.req_mem_type = ReadMakeUnique();
                end else begin
                    v.req_mem_type = ReadShared();
                end
            end
            v.mem_wstrb = '1;
            v.cache_line_o = line_rdata_o;
        end else begin
            v.mem_addr = r.req_addr;
            v.mem_wstrb = {'0, r.req_wstrb};
            if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                v.req_mem_type = WriteNoSnoop();
            end else begin
                v.req_mem_type = ReadNoSnoop();
            end
            t_cache_line_i[63: 0] = r.req_wdata;
            v.cache_line_o = t_cache_line_i;
        end

        v.cache_line_i = '0;
        v.rb_resp = '0;
    end
    State_WaitGrant: begin
        if (i_req_mem_ready == 1'b1) begin
            if ((r.write_flush == 1'b1)
                    || (r.write_first == 1'b1)
                    || ((r.req_type[L2_REQ_TYPE_WRITE] == 1'b1)
                            && (r.req_type[L2_REQ_TYPE_CACHED] == 1'b0))) begin
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
                v.rb_resp = 2'h2;                           // SLVERR
            end
        end
    end
    State_CheckResp: begin
        if ((r.req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b0)
                || (r.rb_resp[1] == 1'b1)) begin
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1'b1;
            vb_resp_rdata = vb_uncached_data;
            vb_resp_status = r.rb_resp;
            v.state = State_Idle;
        end else begin
            v.state = State_SetupReadAdr;
            v_line_cs_write = 1'b1;
            v_line_wflags[TAG_FL_VALID] = 1'b1;
            vb_line_wstrb = '1;                             // write full line
            if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
                // Modify tagged mem output with request before write
                vb_req_type[L2_REQ_TYPE_WRITE] = 1'b0;
                v.req_type = vb_req_type;
                v_line_wflags[L2TAG_FL_DIRTY] = 1'b1;
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
            if (r.write_flush == 1'b1) begin
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            end else if (r.write_first == 1'b1) begin
                v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_L2_LOG2_BYTES_PER_LINE], {CFG_L2_LOG2_BYTES_PER_LINE{1'b0}}};
                v.req_mem_valid = 1'b1;
                v.write_first = 1'b0;
                if (r.req_type[L2_REQ_TYPE_WRITE] == 1'b1) begin
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
                vb_resp_status[1] = i_mem_store_fault;      // rb_resp
            end
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
                && (line_rflags_o[L2TAG_FL_DIRTY] == 1'b1)) begin
            // Off-load valid line
            v.write_flush = 1'b1;
            v.mem_addr = line_raddr_o;
            v.req_mem_valid = 1'b1;
            v.req_mem_type = WriteBack();
            v.req_size = CFG_L2_LOG2_BYTES_PER_LINE;
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
                v.req_addr = (r.req_addr + L2CACHE_BYTES_PER_LINE);
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
    default: begin
    end
    endcase

    if (v_ready_next == 1'b1) begin
        if (r.req_flush == 1'b1) begin
            v.state = State_FlushAddr;
            v.req_flush = 1'b0;
            v.cache_line_i = '0;
            v.req_addr = (r.req_flush_addr & (~LINE_BYTES_MASK));
            v.req_size = CFG_L2_LOG2_BYTES_PER_LINE;
            v.flush_cnt = r.req_flush_cnt;
        end else begin
            v_line_cs_read = i_req_valid;
            v_req_ready = 1'b1;
            vb_line_addr = i_req_addr;
            if (i_req_valid == 1'b1) begin
                v.req_addr = i_req_addr;
                v.req_wstrb = i_req_wstrb;
                v.req_wdata = i_req_wdata;
                v.req_type = i_req_type;
                v.req_size = i_req_size;
                v.req_prot = i_req_prot;
                v.rb_resp = '0;                             // RESP OK
                v.state = State_CheckHit;
            end
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = L2CacheLru_r_reset;
    end

    line_direct_access_i = v_direct_access;
    line_invalidate_i = v_invalidate;
    line_re_i = v_line_cs_read;
    line_we_i = v_line_cs_write;
    line_addr_i = vb_line_addr;
    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = v_line_wflags;

    o_req_ready = v_req_ready;
    o_req_mem_valid = r.req_mem_valid;
    o_req_mem_type = r.req_mem_type;
    o_req_mem_size = r.req_size;
    o_req_mem_prot = r.req_prot;
    o_req_mem_addr = r.mem_addr;
    o_req_mem_strob = r.mem_wstrb;
    o_req_mem_data = r.cache_line_o;

    // always 1 clock messages
    o_resp_valid = v_resp_valid;
    o_resp_rdata = vb_resp_rdata;
    o_resp_status = vb_resp_status;
    o_flush_end = v_flush_end;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= L2CacheLru_r_reset;
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

endmodule: L2CacheLru
