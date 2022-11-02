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

module L2Amba #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    output logic o_req_ready,
    input logic i_req_valid,
    input logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] i_req_type,
    input logic [2:0] i_req_size,
    input logic [2:0] i_req_prot,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_addr,
    input logic [river_cfg_pkg::L2CACHE_BYTES_PER_LINE-1:0] i_req_strob,
    input logic [river_cfg_pkg::L2CACHE_LINE_BITS-1:0] i_req_data,
    output logic [river_cfg_pkg::L2CACHE_LINE_BITS-1:0] o_resp_data,
    output logic o_resp_valid,
    output logic o_resp_ack,
    output logic o_resp_load_fault,
    output logic o_resp_store_fault,
    input types_river_pkg::axi4_l2_in_type i_msti,
    output types_river_pkg::axi4_l2_out_type o_msto
);

import river_cfg_pkg::*;
import types_amba_pkg::*;
import types_river_pkg::*;
import l2_amba_pkg::*;

L2Amba_registers r, rin;

always_comb
begin: comb_proc
    L2Amba_registers v;
    logic v_req_mem_ready;
    logic v_resp_mem_valid;
    logic v_resp_mem_ack;
    logic v_mem_er_load_fault;
    logic v_mem_er_store_fault;
    logic v_next_ready;
    axi4_l2_out_type vmsto;

    v_req_mem_ready = 0;
    v_resp_mem_valid = 0;
    v_resp_mem_ack = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;
    vmsto = axi4_l2_out_none;

    v = r;

    vmsto.r_ready = 1'b1;
    vmsto.w_valid = 1'b0;
    vmsto.w_last = 1'b0;
    vmsto.ar_valid = 1'b0;
    vmsto.aw_valid = 1'b0;

    case (r.state)
    idle: begin
        v_next_ready = 1'b1;
    end
    reading: begin
        vmsto.r_ready = 1'b1;
        v_resp_mem_valid = i_msti.r_valid;
        // r_valid and r_last always should be in the same time
        if ((i_msti.r_valid == 1'b1) && (i_msti.r_last == 1'b1)) begin
            v_mem_er_load_fault = i_msti.r_resp[1];
            v_next_ready = 1'b1;
            v_resp_mem_ack = 1'b1;
            v.state = idle;
        end
    end
    writing: begin
        vmsto.w_valid = 1'b1;
        vmsto.w_last = 1'b1;
        // Write full line without burst transactions:
        if (i_msti.w_ready == 1'b1) begin
            v.state = wack;
        end
    end
    wack: begin
        vmsto.b_ready = 1'b1;
        if (i_msti.b_valid == 1'b1) begin
            v_resp_mem_valid = 1'b1;
            v_mem_er_store_fault = i_msti.b_resp[1];
            v_next_ready = 1'b1;
            v_resp_mem_ack = 1'b1;
            v.state = idle;
        end
    end
    default: begin
    end
    endcase

    if ((v_next_ready == 1'b1) && (i_req_valid == 1'b1)) begin
        if (i_req_type[REQ_MEM_TYPE_WRITE] == 1'b0) begin
            vmsto.ar_valid = 1'b1;
            if (i_msti.ar_ready == 1'b1) begin
                v_req_mem_ready = 1'b1;
                v.state = reading;
            end
        end else begin
            vmsto.aw_valid = 1'b1;
            if (i_msti.aw_ready == 1'b1) begin
                v_req_mem_ready = 1'b1;
                v.state = writing;
            end
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = L2Amba_r_reset;
    end

    o_resp_data = i_msti.r_data;                            // can't directly pass to lower level

    // o_msto_aw_valid = vmsto_aw_valid;
    vmsto.aw_bits.addr = i_req_addr;
    vmsto.aw_bits.len = '0;
    vmsto.aw_bits.size = i_req_size;                        // 0=1B; 1=2B; 2=4B; 3=8B; 4=16B; 5=32B; 6=64B; 7=128B
    vmsto.aw_bits.burst = 2'h1;                             // 00=FIX; 01=INCR; 10=WRAP
    vmsto.aw_bits.lock = 1'b0;
    vmsto.aw_bits.cache = i_req_type[REQ_MEM_TYPE_CACHED];
    vmsto.aw_bits.prot = i_req_prot;
    vmsto.aw_bits.qos = '0;
    vmsto.aw_bits.region = '0;
    vmsto.aw_id = 1'b0;
    vmsto.aw_user = 1'b0;
    // vmsto.w_valid = vmsto_w_valid;
    vmsto.w_data = i_req_data;
    // vmsto.w_last = vmsto_w_last;
    vmsto.w_strb = i_req_strob;
    vmsto.w_user = 1'b0;
    vmsto.b_ready = 1'b1;

    // vmsto.ar_valid = vmsto_ar_valid;
    vmsto.ar_bits.addr = i_req_addr;
    vmsto.ar_bits.len = '0;
    vmsto.ar_bits.size = i_req_size;                        // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.ar_bits.burst = 2'h1;                             // INCR
    vmsto.ar_bits.lock = 1'b0;
    vmsto.ar_bits.cache = i_req_type[REQ_MEM_TYPE_CACHED];
    vmsto.ar_bits.prot = i_req_prot;
    vmsto.ar_bits.qos = '0;
    vmsto.ar_bits.region = '0;
    vmsto.ar_id = 1'b0;
    vmsto.ar_user = 1'b0;
    // vmsto.r_ready = vmsto_r_ready;

    o_msto = vmsto;

    o_req_ready = v_req_mem_ready;
    o_resp_valid = v_resp_mem_valid;
    o_resp_ack = v_resp_mem_ack;
    o_resp_load_fault = v_mem_er_load_fault;
    o_resp_store_fault = v_mem_er_store_fault;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= L2Amba_r_reset;
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

endmodule: L2Amba
