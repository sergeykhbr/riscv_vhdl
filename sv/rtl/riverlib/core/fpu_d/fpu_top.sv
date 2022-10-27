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

module FpuTop #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,
    input logic [river_cfg_pkg::Instr_FPU_Total-1:0] i_ivec,
    input logic [63:0] i_a,                                 // Operand 1
    input logic [63:0] i_b,                                 // Operand 2
    output logic [63:0] o_res,                              // Result
    output logic o_ex_invalidop,                            // Exception: invalid operation
    output logic o_ex_divbyzero,                            // Exception: divide by zero
    output logic o_ex_overflow,                             // Exception: overflow
    output logic o_ex_underflow,                            // Exception: underflow
    output logic o_ex_inexact,                              // Exception: inexact
    output logic o_valid                                    // Result is valid
);

import river_cfg_pkg::*;
import fpu_top_pkg::*;

logic w_fadd_d;
logic w_fsub_d;
logic w_feq_d;
logic w_flt_d;
logic w_fle_d;
logic w_fmax_d;
logic w_fmin_d;
logic w_fcvt_signed;
logic [63:0] wb_res_fadd;
logic w_valid_fadd;
logic w_illegalop_fadd;
logic w_overflow_fadd;
logic w_busy_fadd;
logic [63:0] wb_res_fdiv;
logic w_valid_fdiv;
logic w_illegalop_fdiv;
logic w_divbyzero_fdiv;
logic w_overflow_fdiv;
logic w_underflow_fdiv;
logic w_busy_fdiv;
logic [63:0] wb_res_fmul;
logic w_valid_fmul;
logic w_illegalop_fmul;
logic w_overflow_fmul;
logic w_busy_fmul;
logic [63:0] wb_res_d2l;
logic w_valid_d2l;
logic w_overflow_d2l;
logic w_underflow_d2l;
logic w_busy_d2l;
logic [63:0] wb_res_l2d;
logic w_valid_l2d;
logic w_busy_l2d;
FpuTop_registers r, rin;

DoubleAdd #(
    .async_reset(async_reset)
) fadd_d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(r.ena_fadd),
    .i_add(w_fadd_d),
    .i_sub(w_fsub_d),
    .i_eq(w_feq_d),
    .i_lt(w_flt_d),
    .i_le(w_fle_d),
    .i_max(w_fmax_d),
    .i_min(w_fmin_d),
    .i_a(r.a),
    .i_b(r.b),
    .o_res(wb_res_fadd),
    .o_illegal_op(w_illegalop_fadd),
    .o_overflow(w_overflow_fadd),
    .o_valid(w_valid_fadd),
    .o_busy(w_busy_fadd)
);


DoubleDiv #(
    .async_reset(async_reset)
) fdiv_d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(r.ena_fdiv),
    .i_a(r.a),
    .i_b(r.b),
    .o_res(wb_res_fdiv),
    .o_illegal_op(w_illegalop_fdiv),
    .o_divbyzero(w_divbyzero_fdiv),
    .o_overflow(w_overflow_fdiv),
    .o_underflow(w_underflow_fdiv),
    .o_valid(w_valid_fdiv),
    .o_busy(w_busy_fdiv)
);


DoubleMul #(
    .async_reset(async_reset)
) fmul_d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(r.ena_fmul),
    .i_a(r.a),
    .i_b(r.b),
    .o_res(wb_res_fmul),
    .o_illegal_op(w_illegalop_fmul),
    .o_overflow(w_overflow_fmul),
    .o_valid(w_valid_fmul),
    .o_busy(w_busy_fmul)
);


Double2Long #(
    .async_reset(async_reset)
) d2l_d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(r.ena_d2l),
    .i_signed(w_fcvt_signed),
    .i_w32(r.ena_w32),
    .i_a(r.a),
    .o_res(wb_res_d2l),
    .o_overflow(w_overflow_d2l),
    .o_underflow(w_underflow_d2l),
    .o_valid(w_valid_d2l),
    .o_busy(w_busy_d2l)
);


Long2Double #(
    .async_reset(async_reset)
) l2d_d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(r.ena_l2d),
    .i_signed(w_fcvt_signed),
    .i_w32(r.ena_w32),
    .i_a(r.a),
    .o_res(wb_res_l2d),
    .o_valid(w_valid_l2d),
    .o_busy(w_busy_l2d)
);


always_comb
begin: comb_proc
    FpuTop_registers v;
    logic [Instr_FPU_Total-1:0] iv;

    iv = 0;

    v = r;

    iv = i_ivec;
    v.ena_fadd = 1'b0;
    v.ena_fdiv = 1'b0;
    v.ena_fmul = 1'b0;
    v.ena_d2l = 1'b0;
    v.ena_l2d = 1'b0;
    v.ready = 1'b0;
    if ((i_ena == 1'b1) && (r.busy == 1'b0)) begin
        v.busy = 1'b1;
        v.a = i_a;
        v.b = i_b;
        v.ivec = i_ivec;
        v.ex_invalidop = 1'b0;
        v.ex_divbyzero = 1'b0;
        v.ex_overflow = 1'b0;
        v.ex_underflow = 1'b0;
        v.ex_inexact = 1'b0;

        v.ena_fadd = (iv[(Instr_FADD_D - Instr_FADD_D)]
                || iv[(Instr_FSUB_D - Instr_FADD_D)]
                || iv[(Instr_FLE_D - Instr_FADD_D)]
                || iv[(Instr_FLT_D - Instr_FADD_D)]
                || iv[(Instr_FEQ_D - Instr_FADD_D)]
                || iv[(Instr_FMAX_D - Instr_FADD_D)]
                || iv[(Instr_FMIN_D - Instr_FADD_D)]);
        v.ena_fdiv = iv[(Instr_FDIV_D - Instr_FADD_D)];
        v.ena_fmul = iv[(Instr_FMUL_D - Instr_FADD_D)];
        v.ena_d2l = (iv[(Instr_FCVT_LU_D - Instr_FADD_D)]
                || iv[(Instr_FCVT_L_D - Instr_FADD_D)]
                || iv[(Instr_FCVT_WU_D - Instr_FADD_D)]
                || iv[(Instr_FCVT_W_D - Instr_FADD_D)]);
        v.ena_l2d = (iv[(Instr_FCVT_D_LU - Instr_FADD_D)]
                || iv[(Instr_FCVT_D_L - Instr_FADD_D)]
                || iv[(Instr_FCVT_D_WU - Instr_FADD_D)]
                || iv[(Instr_FCVT_D_W - Instr_FADD_D)]);
        v.ena_w32 = (iv[(Instr_FCVT_WU_D - Instr_FADD_D)]
                || iv[(Instr_FCVT_W_D - Instr_FADD_D)]
                || iv[(Instr_FCVT_D_WU - Instr_FADD_D)]
                || iv[(Instr_FCVT_D_W - Instr_FADD_D)]);
    end
    if ((r.busy == 1'b1)
            && ((r.ivec[(Instr_FMOV_X_D - Instr_FADD_D)]
                    || r.ivec[(Instr_FMOV_D_X - Instr_FADD_D)]) == 1'b1)) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = r.a;
    end else if (w_valid_fadd == 1'b1) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = wb_res_fadd;
        v.ex_invalidop = w_illegalop_fadd;
        v.ex_overflow = w_overflow_fadd;
    end else if (w_valid_fdiv == 1'b1) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = wb_res_fdiv;
        v.ex_invalidop = w_illegalop_fdiv;
        v.ex_divbyzero = w_divbyzero_fdiv;
        v.ex_overflow = w_overflow_fdiv;
        v.ex_underflow = w_underflow_fdiv;
    end else if (w_valid_fmul == 1'b1) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = wb_res_fmul;
        v.ex_invalidop = w_illegalop_fmul;
        v.ex_overflow = w_overflow_fmul;
    end else if (w_valid_d2l == 1'b1) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = wb_res_d2l;
        v.ex_overflow = w_overflow_d2l;
        v.ex_underflow = w_underflow_d2l;
    end else if (w_valid_l2d == 1'b1) begin
        v.busy = 1'b0;
        v.ready = 1'b1;
        v.result = wb_res_l2d;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = FpuTop_r_reset;
    end

    w_fadd_d = r.ivec[(Instr_FADD_D - Instr_FADD_D)];
    w_fsub_d = r.ivec[(Instr_FSUB_D - Instr_FADD_D)];
    w_feq_d = r.ivec[(Instr_FEQ_D - Instr_FADD_D)];
    w_flt_d = r.ivec[(Instr_FLT_D - Instr_FADD_D)];
    w_fle_d = r.ivec[(Instr_FLE_D - Instr_FADD_D)];
    w_fmax_d = r.ivec[(Instr_FMAX_D - Instr_FADD_D)];
    w_fmin_d = r.ivec[(Instr_FMIN_D - Instr_FADD_D)];
    w_fcvt_signed = (r.ivec[(Instr_FCVT_L_D - Instr_FADD_D)]
            || r.ivec[(Instr_FCVT_D_L - Instr_FADD_D)]
            || r.ivec[(Instr_FCVT_W_D - Instr_FADD_D)]
            || r.ivec[(Instr_FCVT_D_W - Instr_FADD_D)]);

    o_res = r.result;
    o_ex_invalidop = r.ex_invalidop;
    o_ex_divbyzero = r.ex_divbyzero;
    o_ex_overflow = r.ex_overflow;
    o_ex_underflow = r.ex_underflow;
    o_ex_inexact = r.ex_inexact;
    o_valid = r.ready;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= FpuTop_r_reset;
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

endmodule: FpuTop
