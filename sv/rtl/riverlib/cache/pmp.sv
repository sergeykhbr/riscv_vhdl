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

module PMP #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,                                      // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_iaddr,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_daddr,
    input logic i_we,                                       // write enable into PMP
    input logic [river_cfg_pkg::CFG_PMP_TBL_WIDTH-1:0] i_region,// selected PMP region
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_start_addr,// PMP region start address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_end_addr, // PMP region end address (inclusive)
    input logic [river_cfg_pkg::CFG_PMP_FL_TOTAL-1:0] i_flags,// {ena, lock, r, w, x}
    output logic o_r,
    output logic o_w,
    output logic o_x
);

import river_cfg_pkg::*;
import pmp_pkg::*;

PMP_registers r, rin;

always_comb
begin: comb_proc
    PMP_registers v;
    logic v_r;
    logic v_w;
    logic v_x;
    logic [RISCV_ARCH-1:0] vb_start_addr;
    logic [RISCV_ARCH-1:0] vb_end_addr;
    logic [CFG_PMP_FL_TOTAL-1:0] vb_flags;

    v_r = 0;
    v_w = 0;
    v_x = 0;
    vb_start_addr = 0;
    vb_end_addr = 0;
    vb_flags = 0;

    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
        v.tbl[i].start_addr = r.tbl[i].start_addr;
        v.tbl[i].end_addr = r.tbl[i].end_addr;
        v.tbl[i].flags = r.tbl[i].flags;
    end

    // PMP is active for S,U modes or in M-mode when L-bit is set:
    v_r = (~i_ena);
    v_w = (~i_ena);
    v_x = (~i_ena);

    vb_flags = i_flags;
    if (i_flags[CFG_PMP_FL_V] == 1'b1) begin
        vb_start_addr = i_start_addr;
        vb_end_addr = i_end_addr;
    end else begin
        vb_start_addr = '0;
        vb_end_addr = '0;
    end

    for (int i = (CFG_PMP_TBL_SIZE - 1); i >= 0; i--) begin
        if ((i_iaddr >= r.tbl[i].start_addr[(CFG_CPU_ADDR_BITS - 1): 0])
                && (i_iaddr <= r.tbl[i].end_addr[(CFG_CPU_ADDR_BITS - 1): 0])) begin
            if ((r.tbl[i].flags[CFG_PMP_FL_V] == 1'b1)
                    && (i_ena || r.tbl[i].flags[CFG_PMP_FL_L])) begin
                v_x = r.tbl[i].flags[CFG_PMP_FL_X];
            end
        end

        if ((i_daddr >= r.tbl[i].start_addr[(CFG_CPU_ADDR_BITS - 1): 0])
                && (i_daddr <= r.tbl[i].end_addr[(CFG_CPU_ADDR_BITS - 1): 0])) begin
            if ((r.tbl[i].flags[CFG_PMP_FL_V] == 1'b1)
                    && (i_ena || r.tbl[i].flags[CFG_PMP_FL_L])) begin
                v_r = r.tbl[i].flags[CFG_PMP_FL_R];
                v_w = r.tbl[i].flags[CFG_PMP_FL_W];
            end
        end
    end

    if (i_we == 1'b1) begin
        v.tbl[int'(i_region)].start_addr = vb_start_addr;
        v.tbl[int'(i_region)].end_addr = vb_end_addr;
        v.tbl[int'(i_region)].flags = vb_flags;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
            v.tbl[i].start_addr = 64'h0000000000000000;
            v.tbl[i].end_addr = 64'h0000000000000000;
            v.tbl[i].flags = 5'h00;
        end
    end

    o_r = v_r;
    o_w = v_w;
    o_x = v_x;

    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
        rin.tbl[i].start_addr = v.tbl[i].start_addr;
        rin.tbl[i].end_addr = v.tbl[i].end_addr;
        rin.tbl[i].flags = v.tbl[i].flags;
    end
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                    r.tbl[i].start_addr <= 64'h0000000000000000;
                    r.tbl[i].end_addr <= 64'h0000000000000000;
                    r.tbl[i].flags <= 5'h00;
                end
            end else begin
                for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                    r.tbl[i].start_addr <= rin.tbl[i].start_addr;
                    r.tbl[i].end_addr <= rin.tbl[i].end_addr;
                    r.tbl[i].flags <= rin.tbl[i].flags;
                end
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                r.tbl[i].start_addr <= rin.tbl[i].start_addr;
                r.tbl[i].end_addr <= rin.tbl[i].end_addr;
                r.tbl[i].flags <= rin.tbl[i].flags;
            end
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: PMP
