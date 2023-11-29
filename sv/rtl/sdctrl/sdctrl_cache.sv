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

module sdctrl_cache #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // Data path:
    input logic i_req_valid,
    input logic i_req_write,
    input logic [sdctrl_cfg_pkg::CFG_SDCACHE_ADDR_BITS-1:0] i_req_addr,
    input logic [63:0] i_req_wdata,
    input logic [7:0] i_req_wstrb,
    output logic o_req_ready,
    output logic o_resp_valid,
    output logic [63:0] o_resp_data,
    output logic o_resp_err,
    input logic i_resp_ready,
    // Memory interface:
    input logic i_req_mem_ready,
    output logic o_req_mem_valid,
    output logic o_req_mem_write,
    output logic [sdctrl_cfg_pkg::CFG_SDCACHE_ADDR_BITS-1:0] o_req_mem_addr,
    output logic [sdctrl_cfg_pkg::SDCACHE_LINE_BITS-1:0] o_req_mem_data,
    input logic i_mem_data_valid,
    input logic [sdctrl_cfg_pkg::SDCACHE_LINE_BITS-1:0] i_mem_data,
    input logic i_mem_fault,
    // Debug interface
    input logic i_flush_valid,
    output logic o_flush_end
);

import sdctrl_cfg_pkg::*;
import sdctrl_cache_pkg::*;

localparam bit [31:0] FLUSH_ALL_VALUE = ((2**ibits) - 1);

logic [SDCACHE_LINE_BITS-1:0] line_wdata_i;
logic [SDCACHE_BYTES_PER_LINE-1:0] line_wstrb_i;
logic [SDCACHE_FL_TOTAL-1:0] line_wflags_i;
logic [CFG_SDCACHE_ADDR_BITS-1:0] line_raddr_o;
logic [SDCACHE_LINE_BITS-1:0] line_rdata_o;
logic [SDCACHE_FL_TOTAL-1:0] line_rflags_o;
logic line_hit_o;
// Snoop signals:
logic [CFG_SDCACHE_ADDR_BITS-1:0] line_snoop_addr_i;
logic [SDCACHE_FL_TOTAL-1:0] line_snoop_flags_o;
sdctrl_cache_registers r, rin;

TagMem #(
    .async_reset(async_reset),
    .abus(abus),
    .ibits(ibits),
    .lnbits(lnbits),
    .flbits(flbits),
    .snoop(0)
) mem0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_addr(r.line_addr_i),
    .i_wstrb(line_wstrb_i),
    .i_wdata(line_wdata_i),
    .i_wflags(line_wflags_i),
    .o_raddr(line_raddr_o),
    .o_rdata(line_rdata_o),
    .o_rflags(line_rflags_o),
    .o_hit(line_hit_o),
    .i_snoop_addr(line_snoop_addr_i),
    .o_snoop_flags(line_snoop_flags_o)
);

always_comb
begin: comb_proc
    sdctrl_cache_registers v;
    logic [SDCACHE_LINE_BITS-1:0] vb_cache_line_i_modified;
    logic [SDCACHE_LINE_BITS-1:0] vb_line_rdata_o_modified;
    logic [SDCACHE_BYTES_PER_LINE-1:0] vb_line_rdata_o_wstrb;
    logic v_req_ready;
    logic [SDCACHE_LINE_BITS-1:0] t_cache_line_i;
    logic [63:0] vb_cached_data;
    logic [63:0] vb_uncached_data;
    logic v_resp_valid;
    logic [63:0] vb_resp_data;
    logic v_flush_end;
    logic [SDCACHE_LINE_BITS-1:0] vb_line_wdata;
    logic [SDCACHE_BYTES_PER_LINE-1:0] vb_line_wstrb;
    logic [63:0] vb_req_mask;
    logic [SDCACHE_FL_TOTAL-1:0] vb_line_wflags;
    logic [(CFG_LOG2_SDCACHE_BYTES_PER_LINE - 3)-1:0] ridx;
    logic v_req_same_line;
    logic v_mem_addr_last;
    logic [CFG_SDCACHE_ADDR_BITS-1:0] vb_addr_direct_next;

    vb_cache_line_i_modified = '0;
    vb_line_rdata_o_modified = '0;
    vb_line_rdata_o_wstrb = '0;
    v_req_ready = 1'b0;
    t_cache_line_i = '0;
    vb_cached_data = '0;
    vb_uncached_data = '0;
    v_resp_valid = 1'b0;
    vb_resp_data = '0;
    v_flush_end = 1'b0;
    vb_line_wdata = '0;
    vb_line_wstrb = '0;
    vb_req_mask = '0;
    vb_line_wflags = '0;
    ridx = '0;
    v_req_same_line = 1'b0;
    v_mem_addr_last = 1'b0;
    vb_addr_direct_next = '0;

    v = r;

    ridx = r.req_addr[(CFG_LOG2_SDCACHE_BYTES_PER_LINE - 1): 3];
    v_mem_addr_last = (&r.mem_addr[9: CFG_LOG2_SDCACHE_BYTES_PER_LINE]);

    vb_cached_data = line_rdata_o[(64 * int'(ridx)) +: 64];
    vb_uncached_data = r.cache_line_i[63: 0];

    if (r.req_addr[(CFG_SDCACHE_ADDR_BITS - 1): CFG_LOG2_SDCACHE_BYTES_PER_LINE] == i_req_addr[(CFG_SDCACHE_ADDR_BITS - 1): CFG_LOG2_SDCACHE_BYTES_PER_LINE]) begin
        v_req_same_line = 1'b1;
    end

    if (i_flush_valid == 1'b1) begin
        v.req_flush = 1'b1;
    end

    for (int i = 0; i < 8; i++) begin
        if (r.req_wstrb[i] == 1'b1) begin
            vb_req_mask[(8 * i) +: 8] = 8'hff;
        end
    end

    vb_line_rdata_o_modified = line_rdata_o;
    vb_cache_line_i_modified = r.cache_line_i;
    for (int i = 0; i < (SDCACHE_BYTES_PER_LINE / 8); i++) begin
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
    vb_addr_direct_next = (r.req_addr + 1);

    vb_line_wdata = r.cache_line_i;

    // System Bus access state machine
    case (r.state)
    State_Idle: begin
        v.mem_fault = 1'b0;
        if (r.req_flush == 1'b1) begin
            v.state = State_FlushAddr;
            v.cache_line_i = 512'd0;
            v.flush_cnt = FLUSH_ALL_VALUE;
        end else begin
            v_req_ready = 1'b1;
            if (i_req_valid == 1'b1) begin
                v.line_addr_i = i_req_addr;
                v.req_addr = i_req_addr;
                v.req_wstrb = i_req_wstrb;
                v.req_wdata = i_req_wdata;
                v.req_write = i_req_write;
                if (v_req_same_line == 1'b1) begin
                    // Write address is the same as the next requested, so use it to write
                    // value and update state machine
                    v.state = State_CheckHit;
                end else begin
                    v.state = State_SetupReadAdr;
                end
            end
        end
    end
    State_SetupReadAdr: begin
        v.state = State_CheckHit;
    end
    State_CheckHit: begin
        vb_resp_data = vb_cached_data;
        if (line_hit_o == 1'b1) begin
            // Hit
            v_resp_valid = 1'b1;
            if (i_resp_ready == 1'b1) begin
                v.state = State_Idle;
                if (r.req_write == 1'b1) begin
                    // Modify tagged mem output with request and write back
                    vb_line_wflags[SDCACHE_FL_VALID] = 1'b1;
                    vb_line_wflags[SDCACHE_FL_DIRTY] = 1'b1;
                    v.req_write = 1'b0;
                    vb_line_wstrb = vb_line_rdata_o_wstrb;
                    vb_line_wdata = vb_line_rdata_o_modified;
                    v.state = State_Idle;
                end
            end
        end else begin
            // Miss
            v.state = State_TranslateAddress;
        end
    end
    State_TranslateAddress: begin
        v.req_mem_valid = 1'b1;
        v.mem_fault = 1'b0;
        v.state = State_WaitGrant;
        if ((line_rflags_o[SDCACHE_FL_VALID] == 1'b1)
                && (line_rflags_o[SDCACHE_FL_DIRTY] == 1'b1)) begin
            v.write_first = 1'b1;
            v.req_mem_write = 1'b1;
            v.mem_addr = {line_raddr_o[(CFG_SDCACHE_ADDR_BITS - 1): 9], {9{1'b0}}};
        end else begin
            // 1. Read -> Save cache
            // 2. Read -> Modify -> Save cache
            v.mem_addr = {r.req_addr[(CFG_SDCACHE_ADDR_BITS - 1): 9], {9{1'b0}}};
            v.req_mem_write = r.req_write;
        end
        v.cache_line_o = line_rdata_o;
        v.cache_line_i = 512'd0;
    end
    State_WaitGrant: begin
        if (i_req_mem_ready == 1'b1) begin
            if ((r.write_flush == 1'b1)
                    || (r.write_first == 1'b1)
                    || (r.req_write == 1'b1)) begin
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
            v.mem_fault = i_mem_fault;
        end
    end
    State_CheckResp: begin
        if (r.mem_fault == 1'b1) begin
            // uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid = 1'b1;
            vb_resp_data = vb_uncached_data;
            if (i_resp_ready == 1'b1) begin
                v.state = State_Idle;
            end
        end else begin
            vb_line_wflags[SDCACHE_FL_VALID] = 1'b1;
            vb_line_wstrb = '1;                             // write full line
            v.mem_addr = (r.mem_addr + SDCACHE_BYTES_PER_LINE);
            if (v_mem_addr_last == 1'b1) begin
                v.state = State_SetupReadAdr;
                v.line_addr_i = r.req_addr;
            end else begin
                v.state = State_WaitResp;
                v.line_addr_i = (r.line_addr_i + SDCACHE_BYTES_PER_LINE);
            end
        end
    end
    State_WriteBus: begin
        if (i_mem_data_valid == 1'b1) begin
            if (r.write_flush == 1'b1) begin
                // Offloading Cache line on flush request
                v.state = State_FlushAddr;
            end else if (r.write_first == 1'b1) begin
                // Obsolete line was offloaded, now read new line
                v.mem_addr = {r.req_addr[(CFG_SDCACHE_ADDR_BITS - 1): CFG_LOG2_SDCACHE_BYTES_PER_LINE], {CFG_LOG2_SDCACHE_BYTES_PER_LINE{1'b0}}};
                v.req_mem_valid = 1'b1;
                v.write_first = 1'b0;
                v.req_mem_write = r.req_write;
                v.state = State_WaitGrant;
            end else begin
                // Non-cached write
                v.state = State_Idle;
                v_resp_valid = 1'b1;
                v.mem_fault = i_mem_fault;
            end
        end
    end
    State_FlushAddr: begin
        v.state = State_FlushCheck;
        vb_line_wstrb = '1;
        vb_line_wflags = 2'd0;
        v.write_flush = 1'b0;
        v.cache_line_i = 512'd0;
    end
    State_FlushCheck: begin
        v.cache_line_o = line_rdata_o;
        if ((line_rflags_o[SDCACHE_FL_VALID] == 1'b1)
                && (line_rflags_o[SDCACHE_FL_DIRTY] == 1'b1)) begin
            // Off-load valid line
            v.write_flush = 1'b1;
            v.mem_addr = line_raddr_o;
            v.req_mem_valid = 1'b1;
            v.req_mem_write = 1'b1;
            v.state = State_WaitGrant;
        end else begin
            // Write clean line
            v.state = State_FlushAddr;
            if ((|r.flush_cnt) == 1'b0) begin
                v.state = State_Idle;
                v_flush_end = 1'b1;
            end
        end
        v.line_addr_i = (r.line_addr_i + SDCACHE_BYTES_PER_LINE);
        if ((|r.flush_cnt) == 1'b1) begin
            v.flush_cnt = (r.flush_cnt - 1);
        end
    end
    State_Reset: begin
        // Write clean line
        v.line_addr_i = 48'd0;
        v.flush_cnt = FLUSH_ALL_VALUE;                      // Init after power-on-reset
        v.state = State_ResetWrite;
    end
    State_ResetWrite: begin
        vb_line_wstrb = '1;
        vb_line_wflags = 2'd0;
        v.line_addr_i = (r.line_addr_i + SDCACHE_BYTES_PER_LINE);
        if ((|r.flush_cnt) == 1'b1) begin
            v.flush_cnt = (r.flush_cnt - 1);
        end else begin
            v.state = State_Idle;
        end
    end
    default: begin
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_cache_r_reset;
    end

    line_wdata_i = vb_line_wdata;
    line_wstrb_i = vb_line_wstrb;
    line_wflags_i = vb_line_wflags;
    line_snoop_addr_i = 48'd0;

    o_req_ready = v_req_ready;
    o_req_mem_valid = r.req_mem_valid;
    o_req_mem_addr = r.mem_addr;
    o_req_mem_write = r.req_mem_write;
    o_req_mem_data = r.cache_line_o;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_err = r.mem_fault;

    o_flush_end = v_flush_end;

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_cache_r_reset;
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

endmodule: sdctrl_cache
