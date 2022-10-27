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

module RegIntBank #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [5:0] i_radr1,                              // Port 1 read address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_rdata1,  // Port 1 read value
    output logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] o_rtag1,// Port 1 read tag value
    input logic [5:0] i_radr2,                              // Port 2 read address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_rdata2,  // Port 2 read value
    output logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] o_rtag2,// Port 2 read tag value
    input logic [5:0] i_waddr,                              // Writing value
    input logic i_wena,                                     // Writing is enabled
    input logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] i_wtag,// Writing register tag
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_wdata,    // Writing value
    input logic i_inorder,                                  // Writing only if tag sequenced
    output logic o_ignored,                                 // Sequenced writing is ignored because it was overwritten by executor (need for tracer)
    input logic [5:0] i_dport_addr,                         // Debug port address
    input logic i_dport_ena,                                // Debug port is enabled
    input logic i_dport_write,                              // Debug port write is enabled
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_wdata,// Debug port write value
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_rdata,// Debug port read value
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_ra,      // Return address for branch predictor
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_sp,      // Stack Pointer for border control
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_gp,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_tp,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t0,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t1,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t2,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_fp,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s1,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a0,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a1,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a2,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a3,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a4,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a5,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a6,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_a7,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s2,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s3,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s4,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s5,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s6,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s7,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s8,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s9,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s10,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_s11,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t3,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t4,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t5,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_t6
);

import river_cfg_pkg::*;
import regibank_pkg::*;

RegIntBank_registers r, rin;

always_comb
begin: comb_proc
    RegIntBank_registers v;
    int int_daddr;
    int int_waddr;
    int int_radr1;
    int int_radr2;
    logic v_inordered;
    logic [CFG_REG_TAG_WIDTH-1:0] next_tag;

    int_daddr = 0;
    int_waddr = 0;
    int_radr1 = 0;
    int_radr2 = 0;
    v_inordered = 0;
    next_tag = 0;

    for (int i = 0; i < REGS_TOTAL; i++) begin
        v.arr[i].val = r.arr[i].val;
        v.arr[i].tag = r.arr[i].tag;
    end

    int_daddr = int'(i_dport_addr);
    int_waddr = int'(i_waddr);
    int_radr1 = int'(i_radr1);
    int_radr2 = int'(i_radr2);

    next_tag = (r.arr[int_waddr].tag + 1);
    if (next_tag == i_wtag) begin
        v_inordered = 1'b1;
    end

    // Debug port has lower priority to avoid system hangup due the tags error
    if ((i_wena == 1'b1) && ((|i_waddr) == 1'b1) && (((~i_inorder) || v_inordered) == 1'b1)) begin
        v.arr[int_waddr].val = i_wdata;
        v.arr[int_waddr].tag = i_wtag;
    end else if ((i_dport_ena && i_dport_write) == 1'b1) begin
        if ((|i_dport_addr) == 1'b1) begin
            v.arr[int_daddr].val = i_dport_wdata;
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        for (int i = 0; i < REGS_TOTAL; i++) begin
            v.arr[i].val = 64'h0000000000000000;
            v.arr[i].tag = 3'h0;
        end
    end

    o_ignored = (i_wena && (|i_waddr) && i_inorder && (~v_inordered));
    o_rdata1 = r.arr[int_radr1].val;
    o_rtag1 = r.arr[int_radr1].tag;
    o_rdata2 = r.arr[int_radr2].val;
    o_rtag2 = r.arr[int_radr2].tag;
    o_dport_rdata = r.arr[int_daddr].val;
    o_ra = r.arr[REG_RA].val;
    o_sp = r.arr[REG_SP].val;
    o_gp = r.arr[REG_GP].val;
    o_tp = r.arr[REG_TP].val;
    o_t0 = r.arr[REG_T0].val;
    o_t1 = r.arr[REG_T1].val;
    o_t2 = r.arr[REG_T2].val;
    o_fp = r.arr[REG_S0].val;
    o_s1 = r.arr[REG_S1].val;
    o_a0 = r.arr[REG_A0].val;
    o_a1 = r.arr[REG_A1].val;
    o_a2 = r.arr[REG_A2].val;
    o_a3 = r.arr[REG_A3].val;
    o_a4 = r.arr[REG_A4].val;
    o_a5 = r.arr[REG_A5].val;
    o_a6 = r.arr[REG_A6].val;
    o_a7 = r.arr[REG_A7].val;
    o_s2 = r.arr[REG_S2].val;
    o_s3 = r.arr[REG_S3].val;
    o_s4 = r.arr[REG_S4].val;
    o_s5 = r.arr[REG_S5].val;
    o_s6 = r.arr[REG_S6].val;
    o_s7 = r.arr[REG_S7].val;
    o_s8 = r.arr[REG_S8].val;
    o_s9 = r.arr[REG_S9].val;
    o_s10 = r.arr[REG_S10].val;
    o_s11 = r.arr[REG_S11].val;
    o_t3 = r.arr[REG_T3].val;
    o_t4 = r.arr[REG_T4].val;
    o_t5 = r.arr[REG_T5].val;
    o_t6 = r.arr[REG_T6].val;

    for (int i = 0; i < REGS_TOTAL; i++) begin
        rin.arr[i].val = v.arr[i].val;
        rin.arr[i].tag = v.arr[i].tag;
    end
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < REGS_TOTAL; i++) begin
                    r.arr[i].val <= 64'h0000000000000000;
                    r.arr[i].tag <= 3'h0;
                end
            end else begin
                for (int i = 0; i < REGS_TOTAL; i++) begin
                    r.arr[i].val <= rin.arr[i].val;
                    r.arr[i].tag <= rin.arr[i].tag;
                end
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < REGS_TOTAL; i++) begin
                r.arr[i].val <= rin.arr[i].val;
                r.arr[i].tag <= rin.arr[i].tag;
            end
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: RegIntBank
