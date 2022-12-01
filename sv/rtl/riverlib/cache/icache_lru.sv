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

module ICacheLru #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned waybits = 2,                     // Log2 of number of ways. Default 2: 4 ways
    parameter int unsigned ibits = 7                        // Log2 of number of lines per way: 7=16KB; 8=32KB; .. (if bytes per line = 32 B)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // Control path:
    input logic i_req_valid,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_addr,
    output logic o_req_ready,
    output logic o_resp_valid,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_resp_addr,
    output logic [63:0] o_resp_data,
    output logic o_resp_load_fault,
    input logic i_resp_ready,
    // Memory interface:
    input logic i_req_mem_ready,
    output logic o_req_mem_valid,
    output logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] o_req_mem_type,
    output logic [2:0] o_req_mem_size,
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_req_mem_addr,
    output logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] o_req_mem_strob,// unused
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_req_mem_data,
    input logic i_mem_data_valid,
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_mem_data,
    input logic i_mem_load_fault,
    // Mpu interface
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_mpu_addr,
    input logic i_pma_cached,
    input logic i_pmp_x,                                    // PMP eXecute access
    // Flush interface
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_flush_address,
    input logic i_flush_valid
);

import river_cfg_pkg::*;
import icache_lru_pkg::*;

localparam int ways = (2**waybits);
localparam bit [31:0] FLUSH_ALL_VALUE = ((2**(ibits + waybits)) - 1);

logic line_direct_access_i;
logic line_invalidate_i;
logic line_re_i;
logic line_we_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_addr_i;
logic [L1CACHE_LINE_BITS-1:0] line_wdata_i;
logic [(2**CFG_LOG2_L1CACHE_BYTES_PER_LINE)-1:0] line_wstrb_i;
logic [ITAG_FL_TOTAL-1:0] line_wflags_i;
logic [CFG_CPU_ADDR_BITS-1:0] line_raddr_o;
logic [(L1CACHE_LINE_BITS + 32)-1:0] line_rdata_o;
logic [ITAG_FL_TOTAL-1:0] line_rflags_o;
logic line_hit_o;
logic line_hit_next_o;
ICacheLru_registers r, rin;

TagMemCoupled #(
    .async_reset(async_reset),
    .abus(abus),
    .waybits(waybits),
    .ibits(ibits),
    .lnbits(lnbits),
    .flbits(flbits)
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
    .o_hit_next(line_hit_next_o)
);


always_comb
begin: comb_proc
    ICacheLru_registers v;
    logic [L1CACHE_LINE_BITS-1:0] t_cache_line_i;
    logic v_req_ready;
    logic v_resp_valid;
    logic [63:0] vb_cached_data;
    logic [63:0] vb_uncached_data;
    logic [63:0] vb_resp_data;
    logic v_resp_er_load_fault;
    logic v_direct_access;
    logic v_invalidate;
    logic v_line_cs_read;
    logic v_line_cs_write;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_line_addr;
    logic [L1CACHE_LINE_BITS-1:0] vb_line_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] vb_line_wstrb;
    logic [ITAG_FL_TOTAL-1:0] v_line_wflags;
    int sel_cached;
    int sel_uncached;
    logic v_ready_next;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_addr_direct_next;

    t_cache_line_i = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vb_cached_data = 0;
    vb_uncached_data = 0;
    vb_resp_data = 0;
    v_resp_er_load_fault = 0;
    v_direct_access = 0;
    v_invalidate = 0;
    v_line_cs_read = 0;
    v_line_cs_write = 0;
    vb_line_addr = 0;
    vb_line_wdata = 0;
    vb_line_wstrb = 0;
    v_line_wflags = 0;
    sel_cached = 0;
    sel_uncached = 0;
    v_ready_next = 0;
    vb_addr_direct_next = 0;

    v = r;

    sel_cached = int'(r.req_addr[(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 1): 2]);
    sel_uncached = int'(r.req_addr[2: 2]);
    vb_cached_data = line_rdata_o[(32 * sel_cached) +: 64];
    vb_uncached_data = r.cache_line_i[(32 * sel_uncached) +: 64];

    // flush request via debug interface
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

    // Flush counter when direct access
    if (r.req_addr[(waybits - 1): 0] == (ways - 1)) begin
        vb_addr_direct_next = ((r.req_addr + L1CACHE_BYTES_PER_LINE) & (~LINE_BYTES_MASK));
    end else begin
        vb_addr_direct_next = (r.req_addr + 1);
    end

    vb_line_addr = r.req_addr;
    vb_line_wdata = r.cache_line_i;

    case (r.state)
    State_Idle: begin
        v_ready_next = 1'b1;
    end
    State_CheckHit: begin
        vb_resp_data = vb_cached_data;
        if ((line_hit_o == 1'b1) && (line_hit_next_o == 1'b1)) begin
            // Hit
            v_resp_valid = 1'b1;
            if (i_resp_ready == 1'b1) begin
                v_ready_next = 1'b1;
                v.state = State_Idle;
            end
        end else begin
            // Miss
            v.state = State_TranslateAddress;
        end
    end
    State_TranslateAddress: begin
        if (i_pmp_x == 1'b0) begin
            t_cache_line_i = '0;
            v.cache_line_i = (~t_cache_line_i);
            v.state = State_CheckResp;
            v.load_fault = 1'b1;
        end else begin
            v.req_mem_valid = 1'b1;
            v.state = State_WaitGrant;
            v.write_addr = r.req_addr;
            v.load_fault = 1'b0;

            if (i_pma_cached == 1'b1) begin
                if (line_hit_o == 1'b0) begin
                    v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                end else begin
                    v.write_addr = r.req_addr_next;
                    v.mem_addr = {r.req_addr_next[(CFG_CPU_ADDR_BITS - 1): CFG_LOG2_L1CACHE_BYTES_PER_LINE], {CFG_LOG2_L1CACHE_BYTES_PER_LINE{1'b0}}};
                end
                v.req_mem_type = ReadShared();
                v.req_mem_size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
            end else begin
                v.mem_addr = {r.req_addr[(CFG_CPU_ADDR_BITS - 1): 3], 3'h0};
                v.req_mem_type = ReadNoSnoop();
                v.req_mem_size = 3'h4;                      // uncached, 16 B
            end
        end
    end
    State_WaitGrant: begin
        if (i_req_mem_ready == 1'b1) begin
            v.state = State_WaitResp;
            v.req_mem_valid = 1'b0;
        end
    end
    State_WaitResp: begin
        if (i_mem_data_valid == 1'b1) begin
            v.cache_line_i = i_mem_data;
            v.state = State_CheckResp;
            v.write_addr = r.req_addr;                      // Swap addres for 1 clock to write line
            v.req_addr = r.write_addr;
            if (i_mem_load_fault == 1'b1) begin
                v.load_fault = 1'b1;
            end
        end
    end
    State_CheckResp: begin
        v.req_addr = r.write_addr;                          // Restore req_addr after line write
        if ((r.req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b0)
                || (r.load_fault == 1'b1)) begin
            v_resp_valid = 1'b1;
            vb_resp_data = vb_uncached_data;
            v_resp_er_load_fault = r.load_fault;
            if (i_resp_ready == 1'b1) begin
                v.state = State_Idle;
            end
        end else begin
            v.state = State_SetupReadAdr;
            v_line_cs_write = 1'b1;
            v_line_wflags[TAG_FL_VALID] = 1'b1;
            vb_line_wstrb = '1;                             // write full line
        end
    end
    State_SetupReadAdr: begin
        v.state = State_CheckHit;
    end
    State_FlushAddr: begin
        v.state = State_FlushCheck;
        v_direct_access = r.req_flush_all;                  // 0=only if hit; 1=will be applied ignoring hit
        v_invalidate = 1'b1;                                // generate: wstrb='1; wflags='0
        v.cache_line_i = '0;
    end
    State_FlushCheck: begin
        v.state = State_FlushAddr;
        v_direct_access = r.req_flush_all;
        v_line_cs_write = r.req_flush_all;
        if ((|r.flush_cnt) == 1'b1) begin
            v.flush_cnt = (r.flush_cnt - 1);
            if (r.req_flush_all == 1'b1) begin
                v.req_addr = vb_addr_direct_next;
            end else begin
                v.req_addr = (r.req_addr + L1CACHE_BYTES_PER_LINE);
            end
        end else begin
            v.state = State_Idle;
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
            v.flush_cnt = r.req_flush_cnt;
        end else begin
            v_req_ready = 1'b1;
            v_line_cs_read = i_req_valid;
            vb_line_addr = i_req_addr;
            if (i_req_valid == 1'b1) begin
                v.req_addr = i_req_addr;
                v.req_addr_next = (i_req_addr + L1CACHE_BYTES_PER_LINE);
                v.state = State_CheckHit;
            end
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = ICacheLru_r_reset;
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
    o_req_mem_addr = r.mem_addr;
    o_req_mem_type = r.req_mem_type;
    o_req_mem_size = r.req_mem_size;
    o_req_mem_strob = '0;
    o_req_mem_data = '0;

    o_resp_valid = v_resp_valid;
    o_resp_data = vb_resp_data;
    o_resp_addr = r.req_addr;
    o_resp_load_fault = v_resp_er_load_fault;
    o_mpu_addr = r.req_addr;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= ICacheLru_r_reset;
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

endmodule: ICacheLru
