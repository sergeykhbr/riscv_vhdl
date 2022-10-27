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

module ic_csr_m2_s1 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // master[0]:
    input logic i_m0_req_valid,
    output logic o_m0_req_ready,
    input logic [river_cfg_pkg::CsrReq_TotalBits-1:0] i_m0_req_type,
    input logic [11:0] i_m0_req_addr,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_m0_req_data,
    output logic o_m0_resp_valid,
    input logic i_m0_resp_ready,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_m0_resp_data,
    output logic o_m0_resp_exception,
    // master[1]
    input logic i_m1_req_valid,
    output logic o_m1_req_ready,
    input logic [river_cfg_pkg::CsrReq_TotalBits-1:0] i_m1_req_type,
    input logic [11:0] i_m1_req_addr,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_m1_req_data,
    output logic o_m1_resp_valid,
    input logic i_m1_resp_ready,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_m1_resp_data,
    output logic o_m1_resp_exception,
    // slave[0]
    output logic o_s0_req_valid,
    input logic i_s0_req_ready,
    output logic [river_cfg_pkg::CsrReq_TotalBits-1:0] o_s0_req_type,
    output logic [11:0] o_s0_req_addr,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s0_req_data,
    input logic i_s0_resp_valid,
    output logic o_s0_resp_ready,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_s0_resp_data,
    input logic i_s0_resp_exception
);

import river_cfg_pkg::*;
import ic_csr_m2_s1_pkg::*;

ic_csr_m2_s1_registers r, rin;

always_comb
begin: comb_proc
    ic_csr_m2_s1_registers v;
    v = r;

    if ((r.acquired == 1'b0) && ((i_m0_req_valid || i_m1_req_valid) == 1'b1)) begin
        v.acquired = 1'b1;
        if (i_m0_req_valid) begin
            v.midx = 1'b0;
        end else begin
            v.midx = 1'b1;
        end
    end
    if (((r.midx == 1'b0) && ((i_s0_resp_valid && i_m0_resp_ready) == 1'b1))
            || ((r.midx == 1'b1) && ((i_s0_resp_valid && i_m1_resp_ready) == 1'b1))) begin
        v.acquired = 1'b0;
    end

    if ((r.midx == 1'b0) || (((~r.acquired) && i_m0_req_valid) == 1'b1)) begin
        o_s0_req_valid = i_m0_req_valid;
        o_m0_req_ready = i_s0_req_ready;
        o_s0_req_type = i_m0_req_type;
        o_s0_req_addr = i_m0_req_addr;
        o_s0_req_data = i_m0_req_data;
        o_m0_resp_valid = i_s0_resp_valid;
        o_s0_resp_ready = i_m0_resp_ready;
        o_m0_resp_data = i_s0_resp_data;
        o_m0_resp_exception = i_s0_resp_exception;
        o_m1_req_ready = 1'b0;
        o_m1_resp_valid = 1'b0;
        o_m1_resp_data = '0;
        o_m1_resp_exception = 1'b0;
    end else begin
        o_s0_req_valid = i_m1_req_valid;
        o_m1_req_ready = i_s0_req_ready;
        o_s0_req_type = i_m1_req_type;
        o_s0_req_addr = i_m1_req_addr;
        o_s0_req_data = i_m1_req_data;
        o_m1_resp_valid = i_s0_resp_valid;
        o_s0_resp_ready = i_m1_resp_ready;
        o_m1_resp_data = i_s0_resp_data;
        o_m1_resp_exception = i_s0_resp_exception;
        o_m0_req_ready = 1'b0;
        o_m0_resp_valid = 1'b0;
        o_m0_resp_data = '0;
        o_m0_resp_exception = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = ic_csr_m2_s1_r_reset;
    end

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= ic_csr_m2_s1_r_reset;
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

endmodule: ic_csr_m2_s1
