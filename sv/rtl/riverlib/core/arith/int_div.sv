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

module IntDiv #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,                                      // Enable bit
    input logic i_unsigned,                                 // Unsigned operands
    input logic i_rv32,                                     // 32-bits operands enabled
    input logic i_residual,                                 // Compute: 0 =division; 1=residual
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a1,       // Operand 1
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a2,       // Operand 2
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_res,     // Result
    output logic o_valid                                    // Result is valid
);

import river_cfg_pkg::*;
import int_div_pkg::*;

logic [123:0] wb_divisor0_i;
logic [123:0] wb_divisor1_i;
logic [63:0] wb_resid0_o;
logic [63:0] wb_resid1_o;
logic [3:0] wb_bits0_o;
logic [3:0] wb_bits1_o;
IntDiv_registers r, rin;

divstage64 stage0 (
    .i_divident(r.divident_i),
    .i_divisor(wb_divisor0_i),
    .o_resid(wb_resid0_o),
    .o_bits(wb_bits0_o)
);


divstage64 stage1 (
    .i_divident(wb_resid0_o),
    .i_divisor(wb_divisor1_i),
    .o_resid(wb_resid1_o),
    .o_bits(wb_bits1_o)
);


always_comb
begin: comb_proc
    IntDiv_registers v;
    logic v_invert64;
    logic v_invert32;
    logic [63:0] vb_a1;
    logic [63:0] vb_a2;
    logic [63:0] vb_rem;
    logic [63:0] vb_div;
    logic v_a1_m0;                                          // a1 == -0ll
    logic v_a2_m1;                                          // a2 == -1ll
    logic v_ena;                                            // 1
    logic [119:0] t_divisor;

    v_invert64 = 0;
    v_invert32 = 0;
    vb_a1 = 0;
    vb_a2 = 0;
    vb_rem = 0;
    vb_div = 0;
    v_a1_m0 = 1'h0;
    v_a2_m1 = 1'h0;
    v_ena = 0;
    t_divisor = 0;

    v = r;

    if (i_rv32 == 1'b1) begin
        if ((i_unsigned == 1'b1) || (i_a1[31] == 1'b0)) begin
            vb_a1[31: 0] = i_a1[31: 0];
        end else begin
            vb_a1[31: 0] = ((~i_a1[31: 0]) + 1);
        end
        if ((i_unsigned == 1'b1) || (i_a2[31] == 1'b0)) begin
            vb_a2[31: 0] = i_a2[31: 0];
        end else begin
            vb_a2[31: 0] = ((~i_a2[31: 0]) + 1);
        end
    end else begin
        if ((i_unsigned == 1'b1) || (i_a1[63] == 1'b0)) begin
            vb_a1[63: 0] = i_a1;
        end else begin
            vb_a1[63: 0] = ((~i_a1) + 1);
        end
        if ((i_unsigned == 1'b1) || (i_a2[63] == 1'b0)) begin
            vb_a2[63: 0] = i_a2;
        end else begin
            vb_a2[63: 0] = ((~i_a2) + 1);
        end
    end

    if ((vb_a1[63] == 1'b1) && ((|vb_a1[62: 0]) == 1'b0)) begin
        v_a1_m0 = 1'b1;                                     // = (1ull << 63)
    end
    if ((&vb_a2) == 1'b1) begin
        v_a2_m1 = 1'b1;                                     // = -1ll
    end

    v_ena = (i_ena && (~r.busy));
    v.ena = {r.ena[8: 0], v_ena};

    if (r.invert == 1'b1) begin
        vb_rem = ((~r.divident_i) + 1);
    end else begin
        vb_rem = r.divident_i;
    end

    if (r.invert == 1'b1) begin
        vb_div = ((~r.bits_i) + 1);
    end else begin
        vb_div = r.bits_i;
    end

    // DIVW, DIVUW, REMW and REMUW sign-extended accordingly with
    // User Level ISA v2.2
    if (r.rv32 == 1'b1) begin
        vb_div[63: 32] = 32'h00000000;
        vb_rem[63: 32] = 32'h00000000;
        if (vb_div[31] == 1'b1) begin
            vb_div[63: 32] = '1;
        end
        if (vb_rem[31] == 1'b1) begin
            vb_rem[63: 32] = '1;
        end
    end

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.rv32 = i_rv32;
        v.resid = i_residual;

        v.divident_i = vb_a1;
        t_divisor[119: 56] = vb_a2;
        v.divisor_i = t_divisor;
        v_invert32 = ((~i_unsigned)
                && (((~i_residual) && (i_a1[31] ^ i_a2[31]))
                        || (i_residual && i_a1[31])));
        v_invert64 = ((~i_unsigned)
                && (((~i_residual) && (i_a1[63] ^ i_a2[63]))
                        || (i_residual && i_a1[63])));
        v.invert = (((~i_rv32) && v_invert64)
                || (i_rv32 && v_invert32));

        if (i_rv32 == 1'b1) begin
            if (i_unsigned == 1'b1) begin
                v.div_on_zero = 1'b0;
                if ((|i_a2[31: 0]) == 1'b0) begin
                    v.div_on_zero = 1'b1;
                end
                v.overflow = 1'b0;
            end else begin
                v.div_on_zero = 1'b0;
                if ((|i_a2[30: 0]) == 1'b0) begin
                    v.div_on_zero = 1'b1;
                end
                v.overflow = (v_a1_m0 && v_a2_m1);
            end
        end else begin
            if (i_unsigned == 1'b1) begin
                v.div_on_zero = 1'b0;
                if ((|i_a2[63: 0]) == 1'b0) begin
                    v.div_on_zero = 1'b1;
                end
                v.overflow = 1'b0;
            end else begin
                v.div_on_zero = 1'b0;
                if ((|i_a2[62: 0]) == 1'b0) begin
                    v.div_on_zero = 1'b1;
                end
                v.overflow = (v_a1_m0 && v_a2_m1);
            end
        end
        v.a1_dbg = i_a1;
        v.a2_dbg = i_a2;
        // v.reference_div = compute_reference(i_unsigned.read(), i_rv32.read(),
        //                              i_residual.read(),
        //                              i_a1.read(), i_a2.read());
    end else if (r.ena[8] == 1'b1) begin
        v.busy = 1'b0;
        if (r.resid == 1'b1) begin
            if (r.overflow == 1'b1) begin
                v.result = '0;
            end else begin
                v.result = vb_rem;
            end
        end else if (r.div_on_zero == 1'b1) begin
            v.result = '1;
        end else if (r.overflow == 1'b1) begin
            v.result = 64'h7fffffffffffffff;
        end else begin
            v.result = vb_div;
        end
    end else if (r.busy == 1'b1) begin
        v.divident_i = wb_resid1_o;
        v.divisor_i = {8'h00, r.divisor_i[119: 8]};
        v.bits_i = {r.bits_i, wb_bits0_o, wb_bits1_o};
    end
    wb_divisor0_i = {r.divisor_i, 4'h0};
    wb_divisor1_i = {4'h0, r.divisor_i};

    if (~async_reset && i_nrst == 1'b0) begin
        v = IntDiv_r_reset;
    end

    o_res = r.result;
    o_valid = r.ena[9];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= IntDiv_r_reset;
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

endmodule: IntDiv
