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

module Double2Long #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,
    input logic i_signed,
    input logic i_w32,
    input logic [63:0] i_a,                                 // Operand 1
    output logic [63:0] o_res,                              // Result
    output logic o_overflow,
    output logic o_underflow,
    output logic o_valid,                                   // Result is valid
    output logic o_busy                                     // Multiclock instruction under processing
);

import d2l_d_pkg::*;

Double2Long_registers r, rin;

always_comb
begin: comb_proc
    Double2Long_registers v;
    logic v_ena;
    logic [52:0] mantA;
    logic expDif_gr;                                        // greater than 1023 + 63
    logic expDif_lt;                                        // less than 1023
    logic overflow;
    logic underflow;
    logic [11:0] expDif;
    logic [63:0] mantPreScale;
    logic [63:0] mantPostScale;
    logic [10:0] expMax;
    logic [5:0] expShift;
    logic resSign;
    logic [63:0] resMant;
    logic [63:0] res;

    v_ena = 0;
    mantA = 0;
    expDif_gr = 1'h0;
    expDif_lt = 1'h0;
    overflow = 0;
    underflow = 0;
    expDif = 0;
    mantPreScale = 0;
    mantPostScale = 0;
    expMax = 0;
    expShift = 0;
    resSign = 0;
    resMant = 0;
    res = 0;

    v = r;

    v_ena = (i_ena && (~r.busy));
    v.ena = {r.ena[1: 0], v_ena};

    mantA[51: 0] = i_a[51: 0];
    mantA[52] = 1'h0;
    if ((|i_a[62: 52]) == 1'b1) begin
        mantA[52] = 1'h1;
    end

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.signA = i_a[63];
        v.expA = i_a[62: 52];
        v.mantA = mantA;
        v.op_signed = i_signed;
        v.w32 = i_w32;
        v.overflow = 1'b0;
        v.underflow = 1'b0;
    end

    // (1086 - expA)[5:0]
    expShift = (6'h3e - r.expA[5: 0]);
    if (r.w32 == 1'b1) begin
        if (r.op_signed == 1'b1) begin
            expMax = 11'h41d;
        end else begin
            expMax = 11'h43d;
        end
    end else begin
        if ((r.op_signed || r.signA) == 1'b1) begin
            expMax = 11'h43d;
        end else begin
            expMax = 11'h43e;
        end
    end
    expDif = ({1'h0, expMax} - {1'h0, r.expA});

    expDif_gr = expDif[11];
    expDif_lt = 1'b0;
    if ((r.expA != 11'h3ff) && (r.expA[10] == 1'b0)) begin
        expDif_lt = 1'b1;
    end

    mantPreScale = {r.mantA, 11'h000};

    mantPostScale = '0;
    if (expDif_gr == 1'b1) begin
        overflow = 1'b1;
        underflow = 1'b0;
    end else if (expDif_lt == 1'b1) begin
        overflow = 1'b0;
        underflow = 1'b1;
    end else begin
        overflow = 1'b0;
        underflow = 1'b0;
        // Multiplexer, probably switch case in rtl
        for (int i = 0; i < 64; i++) begin
            if (expShift == i) begin
                mantPostScale = (mantPreScale >> i);
            end
        end
    end

    if (r.ena[0] == 1'b1) begin
        v.overflow = overflow;
        v.underflow = underflow;
        v.mantPostScale = mantPostScale;
    end

    // Result multiplexers:
    resSign = ((r.signA || r.overflow) && (~r.underflow));
    if (r.signA == 1'b1) begin
        resMant = ((~r.mantPostScale) + 1);
    end else begin
        resMant = r.mantPostScale;
    end

    res = resMant;
    if (r.op_signed == 1'b1) begin
        if (resSign == 1'b1) begin
            if (r.w32 == 1'b1) begin
                res[63: 31] = '1;
            end else begin
                res[63] = 1'h1;
            end
        end
    end else begin
        if (r.w32 == 1'b1) begin
            res[63: 32] = '0;
        end else if (r.overflow == 1'b1) begin
            res[63] = 1'h1;
        end
    end

    if (r.ena[1] == 1'b1) begin
        v.result = res;
        v.busy = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = Double2Long_r_reset;
    end

    o_res = r.result;
    o_overflow = r.overflow;
    o_underflow = r.underflow;
    o_valid = r.ena[2];
    o_busy = r.busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= Double2Long_r_reset;
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

endmodule: Double2Long
