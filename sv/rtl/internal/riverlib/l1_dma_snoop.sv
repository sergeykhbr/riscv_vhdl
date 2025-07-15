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

module l1_dma_snoop #(
    parameter int abits = 48,                               // adress bits used
    parameter logic async_reset = 1'b0,
    parameter int userbits = 1,
    parameter logic [63:0] base_offset = '0,                // Address offset for all DMA transactions
    parameter bit coherence_ena = 0
)
(
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_clk,                                      // CPU clock
    output logic o_req_mem_ready,                           // Ready to accept next data
    input logic i_req_mem_path,                             // instruction/data
    input logic i_req_mem_valid,                            // Request data is ready to accept
    input logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] i_req_mem_type,// read/write/cached
    input logic [2:0] i_req_mem_size,
    input logic [abits-1:0] i_req_mem_addr,                 // Address to read/write
    input logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] i_req_mem_strob,// Byte enabling write strob
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_req_mem_data,// Data to write
    output logic o_resp_mem_path,                           // instruction/data.
    output logic o_resp_mem_valid,                          // Read/Write data is valid. All write transaction with valid response.
    output logic o_resp_mem_load_fault,                     // Error on memory access
    output logic o_resp_mem_store_fault,                    // Error on memory access
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_resp_mem_data,// Read data value
    // D$ Snoop interface
    output logic o_req_snoop_valid,
    output logic [river_cfg_pkg::SNOOP_REQ_TYPE_BITS-1:0] o_req_snoop_type,
    input logic i_req_snoop_ready,
    output logic [abits-1:0] o_req_snoop_addr,
    output logic o_resp_snoop_ready,
    input logic i_resp_snoop_valid,
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_resp_snoop_data,
    input logic [river_cfg_pkg::DTAG_FL_TOTAL-1:0] i_resp_snoop_flags,
    input types_river_pkg::axi4_l1_in_type i_msti,          // L1-cache master input
    output types_river_pkg::axi4_l1_out_type o_msto         // L1-cache master output
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
localparam bit [2:0] state_idle = 3'd0;
localparam bit [2:0] state_ar = 3'd1;
localparam bit [2:0] state_r = 3'd2;
localparam bit [2:0] state_aw = 3'd3;
localparam bit [2:0] state_w = 3'd4;
localparam bit [2:0] state_b = 3'd5;
localparam bit [2:0] snoop_idle = 3'd0;
localparam bit [2:0] snoop_ac_wait_accept = 3'd1;
localparam bit [2:0] snoop_cr = 3'd2;
localparam bit [2:0] snoop_cr_wait_accept = 3'd3;
localparam bit [2:0] snoop_cd = 3'd4;
localparam bit [2:0] snoop_cd_wait_accept = 3'd5;

typedef struct {
    logic [2:0] state;
    logic [abits-1:0] req_addr;
    logic req_path;
    logic [3:0] req_cached;
    logic [L1CACHE_LINE_BITS-1:0] req_wdata;
    logic [L1CACHE_BYTES_PER_LINE-1:0] req_wstrb;
    logic [2:0] req_size;
    logic [2:0] req_prot;
    logic [3:0] req_ar_snoop;
    logic [2:0] req_aw_snoop;
    logic [2:0] snoop_state;
    logic [abits-1:0] ac_addr;
    logic [3:0] ac_snoop;                                   // Table C3-19
    logic [4:0] cr_resp;
    logic [SNOOP_REQ_TYPE_BITS-1:0] req_snoop_type;
    logic [L1CACHE_LINE_BITS-1:0] resp_snoop_data;
    logic cache_access;
    logic [12:0] watchdog;
} l1_dma_snoop_registers;

const l1_dma_snoop_registers l1_dma_snoop_r_reset = '{
    state_idle,                         // state
    '0,                                 // req_addr
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
    4'd0,                               // ac_snoop
    '0,                                 // cr_resp
    '0,                                 // req_snoop_type
    '0,                                 // resp_snoop_data
    1'b0,                               // cache_access
    '0                                  // watchdog
};
l1_dma_snoop_registers r;
l1_dma_snoop_registers rin;

function logic [3:0] reqtype2arsnoop(input logic [REQ_MEM_TYPE_BITS-1:0] reqtype);
logic [3:0] ret;
begin
    ret = 4'd0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 1'b0) begin
        ret = ARSNOOP_READ_NO_SNOOP;
    end else begin
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 1'b0) begin
            ret = ARSNOOP_READ_SHARED;
        end else begin
            ret = ARSNOOP_READ_MAKE_UNIQUE;
        end
    end
    return ret;
end
endfunction: reqtype2arsnoop

function logic [3:0] reqtype2awsnoop(input logic [REQ_MEM_TYPE_BITS-1:0] reqtype);
logic [3:0] ret;
begin
    ret = 4'd0;
    if (reqtype[REQ_MEM_TYPE_CACHED] == 1'b0) begin
        ret = AWSNOOP_WRITE_NO_SNOOP;
    end else begin
        if (reqtype[REQ_MEM_TYPE_UNIQUE] == 1'b0) begin
            ret = AWSNOOP_WRITE_BACK;
        end else begin
            ret = AWSNOOP_WRITE_LINE_UNIQUE;
        end
    end
    return ret;
end
endfunction: reqtype2awsnoop


always_comb
begin: comb_proc
    l1_dma_snoop_registers v;
    logic v_resp_mem_valid;
    logic v_mem_er_load_fault;
    logic v_mem_er_store_fault;
    logic v_next_ready;
    axi4_l1_out_type vmsto;
    logic v_snoop_next_ready;
    logic req_snoop_valid;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_req_snoop_addr;
    logic [SNOOP_REQ_TYPE_BITS-1:0] vb_req_snoop_type;
    logic v_cr_valid;
    logic [4:0] vb_cr_resp;
    logic v_cd_valid;
    logic [L1CACHE_LINE_BITS-1:0] vb_cd_data;

    v = r;
    v_resp_mem_valid = 1'b0;
    v_mem_er_load_fault = 1'b0;
    v_mem_er_store_fault = 1'b0;
    v_next_ready = 1'b0;
    vmsto = axi4_l1_out_none;
    v_snoop_next_ready = 1'b0;
    req_snoop_valid = 1'b0;
    vb_req_snoop_addr = '0;
    vb_req_snoop_type = '0;
    v_cr_valid = 1'b0;
    vb_cr_resp = '0;
    v_cd_valid = 1'b0;
    vb_cd_data = '0;

    vmsto.ar_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    vmsto.aw_bits.burst = AXI_BURST_INCR;                   // INCR (possible any value actually)
    case (r.state)
    state_idle: begin
        v_next_ready = 1'b1;
        if (i_req_mem_valid == 1'b1) begin
            v.req_path = i_req_mem_path;
            v.req_addr = i_req_mem_addr;
            v.req_size = i_req_mem_size;
            // [0] 0=Unpriv/1=Priv;
            // [1] 0=Secure/1=Non-secure;
            // [2] 0=Data/1=Instruction
            v.req_prot = {i_req_mem_path, {2{1'b0}}};
            if (i_req_mem_type[REQ_MEM_TYPE_WRITE] == 1'b0) begin
                v.state = state_ar;
                v.req_wdata = '0;
                v.req_wstrb = 32'd0;
                if (i_req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b1) begin
                    v.req_cached = ARCACHE_WRBACK_READ_ALLOCATE;
                end else begin
                    v.req_cached = ARCACHE_DEVICE_NON_BUFFERABLE;
                end
                if (coherence_ena == 1'b1) begin
                    v.req_ar_snoop = reqtype2arsnoop(i_req_mem_type);
                end
            end else begin
                v.state = state_aw;
                v.req_wdata = i_req_mem_data;
                v.req_wstrb = i_req_mem_strob;
                if (i_req_mem_type[REQ_MEM_TYPE_CACHED] == 1'b1) begin
                    v.req_cached = AWCACHE_WRBACK_WRITE_ALLOCATE;
                end else begin
                    v.req_cached = AWCACHE_DEVICE_NON_BUFFERABLE;
                end
                if (coherence_ena == 1'b1) begin
                    v.req_aw_snoop = reqtype2awsnoop(i_req_mem_type);
                end
            end
        end
    end
    state_ar: begin
        vmsto.ar_valid = 1'b1;
        vmsto.ar_bits.addr = r.req_addr;
        vmsto.ar_bits.cache = r.req_cached;
        vmsto.ar_bits.size = r.req_size;
        vmsto.ar_bits.prot = r.req_prot;
        vmsto.ar_snoop = r.req_ar_snoop;
        if ((i_msti.ar_ready == 1'b1) || r.watchdog[12]) begin
            v.state = state_r;
        end
    end
    state_r: begin
        vmsto.r_ready = 1'b1;
        v_mem_er_load_fault = (i_msti.r_resp[1] | r.watchdog[12]);
        v_resp_mem_valid = i_msti.r_valid;
        // r_valid and r_last always should be in the same time
        if (((i_msti.r_valid == 1'b1) && (i_msti.r_last == 1'b1)) || r.watchdog[12]) begin
            v.state = state_idle;
        end
    end
    state_aw: begin
        vmsto.aw_valid = 1'b1;
        vmsto.aw_bits.addr = r.req_addr;
        vmsto.aw_bits.cache = r.req_cached;
        vmsto.aw_bits.size = r.req_size;
        vmsto.aw_bits.prot = r.req_prot;
        vmsto.aw_snoop = r.req_aw_snoop;
        // axi lite to simplify L2-cache
        vmsto.w_valid = 1'b1;
        vmsto.w_last = 1'b1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if ((i_msti.aw_ready == 1'b1) || r.watchdog[12]) begin
            if (i_msti.w_ready == 1'b1) begin
                v.state = state_b;
            end else begin
                v.state = state_w;
            end
        end
    end
    state_w: begin
        // Shoudln't get here because of Lite interface:
        vmsto.w_valid = 1'b1;
        vmsto.w_last = 1'b1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if ((i_msti.w_ready == 1'b1) || (r.watchdog[12] == 1'b1)) begin
            v.state = state_b;
        end
    end
    state_b: begin
        vmsto.b_ready = 1'b1;
        v_resp_mem_valid = i_msti.b_valid;
        v_mem_er_store_fault = (i_msti.b_resp[1] | r.watchdog[12]);
        if ((i_msti.b_valid == 1'b1) || r.watchdog[12]) begin
            v.state = state_idle;
        end
    end
    default: begin
    end
    endcase

    if (r.state == state_idle) begin
        v.watchdog = 13'd0;
    end else begin
        v.watchdog = (r.watchdog + 1);
    end

    // Snoop processing:
    case (r.snoop_state)
    snoop_idle: begin
        v_snoop_next_ready = 1'b1;
    end
    snoop_ac_wait_accept: begin
        req_snoop_valid = 1'b1;
        vb_req_snoop_addr = r.ac_addr;
        vb_req_snoop_type = r.req_snoop_type;
        if (i_req_snoop_ready == 1'b1) begin
            if (r.cache_access == 1'b0) begin
                v.snoop_state = snoop_cr;
            end else begin
                v.snoop_state = snoop_cd;
            end
        end
    end
    snoop_cr: begin
        if (i_resp_snoop_valid == 1'b1) begin
            v_cr_valid = 1'b1;
            if ((i_resp_snoop_flags[TAG_FL_VALID] == 1'b1)
                    && ((i_resp_snoop_flags[DTAG_FL_SHARED] == 1'b0)
                            || (r.ac_snoop == AC_SNOOP_READ_UNIQUE))) begin
                // Need second request with cache access
                v.cache_access = 1'b1;
                // see table C3-21 "Snoop response bit allocation"
                vb_cr_resp[0] = 1'b1;                       // will be Data transfer
                vb_cr_resp[4] = 1'b1;                       // WasUnique
                if (r.ac_snoop == AC_SNOOP_READ_UNIQUE) begin
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READCLEAN] = 1'b1;
                end else begin
                    vb_req_snoop_type[SNOOP_REQ_TYPE_READDATA] = 1'b1;
                end
                v.req_snoop_type = vb_req_snoop_type;
                v.snoop_state = snoop_ac_wait_accept;
                if (i_msti.cr_ready == 1'b1) begin
                    v.snoop_state = snoop_ac_wait_accept;
                end else begin
                    v.snoop_state = snoop_cr_wait_accept;
                end
            end else begin
                vb_cr_resp = 5'd0;
                if (i_msti.cr_ready == 1'b1) begin
                    v.snoop_state = snoop_idle;
                end else begin
                    v.snoop_state = snoop_cr_wait_accept;
                end
            end
            v.cr_resp = vb_cr_resp;
        end
    end
    snoop_cr_wait_accept: begin
        v_cr_valid = 1'b1;
        vb_cr_resp = r.cr_resp;
        if (i_msti.cr_ready == 1'b1) begin
            if (r.cache_access == 1'b1) begin
                v.snoop_state = snoop_ac_wait_accept;
            end else begin
                v.snoop_state = snoop_idle;
            end
        end
    end
    snoop_cd: begin
        if (i_resp_snoop_valid == 1'b1) begin
            v_cd_valid = 1'b1;
            vb_cd_data = i_resp_snoop_data;
            v.resp_snoop_data = i_resp_snoop_data;
            if (i_msti.cd_ready == 1'b1) begin
                v.snoop_state = snoop_idle;
            end else begin
                v.snoop_state = snoop_cd_wait_accept;
            end
        end
    end
    snoop_cd_wait_accept: begin
        v_cd_valid = 1'b1;
        vb_cd_data = r.resp_snoop_data;
        if (i_msti.cd_ready == 1'b1) begin
            v.snoop_state = snoop_idle;
        end
    end
    default: begin
    end
    endcase

    if ((coherence_ena == 1'b1)
            && (v_snoop_next_ready == 1'b1)
            && (i_msti.ac_valid == 1'b1)) begin
        req_snoop_valid = 1'b1;
        v.cache_access = 1'b0;
        vb_req_snoop_type = 2'd0;                           // First snoop operation always just to read flags
        v.req_snoop_type = 2'd0;
        vb_req_snoop_addr = i_msti.ac_addr;
        v.ac_addr = i_msti.ac_addr;
        v.ac_snoop = i_msti.ac_snoop;
        if (i_req_snoop_ready == 1'b1) begin
            v.snoop_state = snoop_cr;
        end else begin
            v.snoop_state = snoop_ac_wait_accept;
        end
    end else begin
        v_snoop_next_ready = 1'b1;
        v_cr_valid = 1'b1;
    end

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = l1_dma_snoop_r_reset;
    end

    vmsto.ac_ready = v_snoop_next_ready;
    vmsto.cr_valid = v_cr_valid;
    vmsto.cr_resp = vb_cr_resp;
    vmsto.cd_valid = v_cd_valid;
    vmsto.cd_data = vb_cd_data;
    vmsto.cd_last = v_cd_valid;
    vmsto.rack = 1'b0;
    vmsto.wack = 1'b0;

    o_req_mem_ready = v_next_ready;
    o_resp_mem_path = r.req_path;
    o_resp_mem_valid = v_resp_mem_valid;
    o_resp_mem_data = i_msti.r_data;
    o_resp_mem_load_fault = v_mem_er_load_fault;
    o_resp_mem_store_fault = v_mem_er_store_fault;
    // AXI Snoop IOs:
    o_req_snoop_valid = req_snoop_valid;
    o_req_snoop_type = vb_req_snoop_type;
    o_req_snoop_addr = vb_req_snoop_addr;
    o_resp_snoop_ready = 1'b1;

    o_msto = vmsto;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= l1_dma_snoop_r_reset;
            end else begin
                r <= rin;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r <= rin;
        end

    end: async_r_dis
endgenerate

endmodule: l1_dma_snoop
