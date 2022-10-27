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

module DoubleAdd #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,
    input logic i_add,
    input logic i_sub,
    input logic i_eq,
    input logic i_lt,
    input logic i_le,
    input logic i_max,
    input logic i_min,
    input logic [63:0] i_a,                                 // Operand 1
    input logic [63:0] i_b,                                 // Operand 2
    output logic [63:0] o_res,                              // Result
    output logic o_illegal_op,                              // nanA | nanB
    output logic o_overflow,
    output logic o_valid,                                   // Result is valid
    output logic o_busy                                     // Multiclock instruction under processing
);

import fadd_d_pkg::*;

DoubleAdd_registers r, rin;

always_comb
begin: comb_proc
    DoubleAdd_registers v;
    logic v_ena;
    logic signOp;
    logic signA;
    logic signB;
    logic signOpB;
    logic [52:0] mantA;
    logic [52:0] mantB;
    logic [53:0] mantDif;
    logic [11:0] expDif;
    logic v_flMore;
    logic v_flEqual;
    logic v_flLess;
    logic [11:0] vb_preShift;
    logic v_signOpMore;
    logic [10:0] vb_expMore;
    logic [52:0] vb_mantMore;
    logic [52:0] vb_mantLess;
    logic [104:0] mantMoreScale;
    logic [104:0] mantLessScale;
    logic [105:0] vb_mantSum;
    logic [6:0] vb_lshift;
    logic [104:0] vb_mantAlign;
    logic [11:0] vb_expPostScale;
    logic [104:0] vb_mantPostScale;
    logic [104:0] vb_mantSumInv;
    logic [6:0] vb_lshift_p1;
    logic [6:0] vb_lshift_p2;
    logic [52:0] mantShort;
    logic [51:0] tmpMant05;
    logic mantOnes;
    logic mantEven;
    logic mant05;
    logic rndBit;
    logic mantZeroA;
    logic mantZeroB;
    logic allZero;
    logic sumZero;
    logic nanA;
    logic nanB;
    logic nanAB;
    logic overflow;
    logic [63:0] resAdd;
    logic [63:0] resEQ;
    logic [63:0] resLT;
    logic [63:0] resLE;
    logic [63:0] resMax;
    logic [63:0] resMin;

    v_ena = 0;
    signOp = 0;
    signA = 0;
    signB = 0;
    signOpB = 0;
    mantA = 0;
    mantB = 0;
    mantDif = 0;
    expDif = 0;
    v_flMore = 0;
    v_flEqual = 0;
    v_flLess = 0;
    vb_preShift = 0;
    v_signOpMore = 0;
    vb_expMore = 0;
    vb_mantMore = 0;
    vb_mantLess = 0;
    mantMoreScale = 0;
    mantLessScale = 0;
    vb_mantSum = 0;
    vb_lshift = 0;
    vb_mantAlign = 0;
    vb_expPostScale = 0;
    vb_mantPostScale = 0;
    vb_mantSumInv = 0;
    vb_lshift_p1 = 0;
    vb_lshift_p2 = 0;
    mantShort = 0;
    tmpMant05 = 0;
    mantOnes = 0;
    mantEven = 0;
    mant05 = 0;
    rndBit = 0;
    mantZeroA = 0;
    mantZeroB = 0;
    allZero = 0;
    sumZero = 0;
    nanA = 0;
    nanB = 0;
    nanAB = 0;
    overflow = 0;
    resAdd = 0;
    resEQ = 0;
    resLT = 0;
    resLE = 0;
    resMax = 0;
    resMin = 0;

    v = r;

    v_ena = (i_ena && (~r.busy));
    v.ena = {r.ena[6: 0], v_ena};

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.add = i_add;
        v.sub = i_sub;
        v.eq = i_eq;
        v.lt = i_lt;
        v.le = i_le;
        v.max = i_max;
        v.min = i_min;
        v.a = i_a;
        v.b = i_b;
        v.illegal_op = 1'b0;
        v.overflow = 1'b0;
    end

    signOp = (r.sub || r.le || r.lt);
    signA = r.a[63];
    signB = r.b[63];
    signOpB = (signB ^ signOp);

    mantA[51: 0] = r.a[51: 0];
    mantA[52] = 1'h0;
    if ((|r.a[62: 52]) == 1'b1) begin
        mantA[52] = 1'h1;
    end

    mantB[51: 0] = r.b[51: 0];
    mantB[52] = 1'h0;
    if ((|r.b[62: 52]) == 1'b1) begin
        mantB[52] = 1'h1;
    end

    if (((|r.a[62: 52]) == 1'b1) && ((|r.b[62: 52]) == 1'b0)) begin
        expDif = (r.a[62: 52] - 1);
    end else if (((|r.a[62: 52]) == 1'b0) && ((|r.b[62: 52]) == 1'b1)) begin
        expDif = (12'h001 - r.b[62: 52]);
    end else begin
        expDif = (r.a[62: 52] - r.b[62: 52]);
    end

    mantDif = ({1'h0, mantA} - {1'h0, mantB});
    if ((|expDif) == 1'b0) begin
        vb_preShift = expDif;
        if ((|mantDif) == 1'b0) begin
            v_flMore = ((~signA) && (signA ^ signB));
            v_flEqual = (~(signA ^ signB));
            v_flLess = (signA && (signA ^ signB));

            v_signOpMore = signA;
            vb_expMore = r.a[62: 52];
            vb_mantMore = mantA;
            vb_mantLess = mantB;
        end else if (mantDif[53] == 1'b0) begin             // A > B
            v_flMore = (~signA);
            v_flEqual = 1'b0;
            v_flLess = signA;

            v_signOpMore = signA;
            vb_expMore = r.a[62: 52];
            vb_mantMore = mantA;
            vb_mantLess = mantB;
        end else begin
            v_flMore = signB;
            v_flEqual = 1'b0;
            v_flLess = (~signB);

            v_signOpMore = signOpB;
            vb_expMore = r.b[62: 52];
            vb_mantMore = mantB;
            vb_mantLess = mantA;
        end
    end else if (expDif[11] == 1'b0) begin
        v_flMore = (~signA);
        v_flEqual = 1'b0;
        v_flLess = signA;

        vb_preShift = expDif;
        v_signOpMore = signA;
        vb_expMore = r.a[62: 52];
        vb_mantMore = mantA;
        vb_mantLess = mantB;
    end else begin
        v_flMore = signB;
        v_flEqual = 1'b0;
        v_flLess = (~signB);

        vb_preShift = ((~expDif) + 1);
        v_signOpMore = signOpB;
        vb_expMore = r.b[62: 52];
        vb_mantMore = mantB;
        vb_mantLess = mantA;
    end
    if (r.ena[0] == 1'b1) begin
        v.flMore = v_flMore;
        v.flEqual = v_flEqual;
        v.flLess = v_flLess;
        v.preShift = vb_preShift;
        v.signOpMore = v_signOpMore;
        v.expMore = vb_expMore;
        v.mantMore = vb_mantMore;
        v.mantLess = vb_mantLess;
    end

    // Pre-scale 105-bits mantissa if preShift < 105:
    // M = {mantM, 52'd0}
    mantLessScale = r.mantLess;
    mantLessScale = {mantLessScale, {52{1'b0}}};
    if (r.ena[1] == 1'b1) begin
        v.mantLessScale = '0;
        for (int i = 0; i < 105; i++) begin
            if (i == r.preShift) begin
                v.mantLessScale = (mantLessScale >> i);
            end
        end
    end

    mantMoreScale = r.mantMore;
    mantMoreScale = {mantMoreScale, {52{1'b0}}};

    // 106-bits adder/subtractor
    if ((signA ^ signOpB) == 1'b1) begin
        vb_mantSum = (mantMoreScale - r.mantLessScale);
    end else begin
        vb_mantSum = (mantMoreScale + r.mantLessScale);
    end

    if (r.ena[2] == 1'b1) begin
        v.mantSum = vb_mantSum;
    end

    // To avoid timing constrains violation try to implement parallel demux
    // for Xilinx Vivado
    for (int i = 0; i < 104; i++) begin
        vb_mantSumInv[(i + 1)] = r.mantSum[(103 - i)];
    end

    for (int i = 0; i < 64; i++) begin
        if (((|vb_lshift_p1) == 1'b0) && (vb_mantSumInv[i] == 1'b1)) begin
            vb_lshift_p1 = i;
        end
    end

    for (int i = 0; i < 41; i++) begin
        if (((|vb_lshift_p2) == 1'b0) && (vb_mantSumInv[(64 + i)] == 1'b1)) begin
            vb_lshift_p2 = i;
            vb_lshift_p2[6] = 1'h1;
        end
    end

    // multiplexer
    if (r.mantSum[105] == 1'b1) begin
        // shift right
        vb_lshift = '1;
    end else if (r.mantSum[104] == 1'b1) begin
        vb_lshift = '0;
    end else if ((|vb_lshift_p1) == 1'b1) begin
        vb_lshift = vb_lshift_p1;
    end else begin
        vb_lshift = vb_lshift_p2;
    end
    if (r.ena[3] == 1'b1) begin
        v.lshift = vb_lshift;
    end

    // Prepare to mantissa post-scale
    if (r.lshift == 7'h7f) begin
        vb_mantAlign = {'0, r.mantSum[106 - 1: 1]};
    end else if ((|r.lshift) == 1'b0) begin
        vb_mantAlign = r.mantSum;
    end else begin
        for (int i = 1; i < 105; i++) begin
            if (i == r.lshift) begin
                vb_mantAlign = (r.mantSum << i);
            end
        end
    end
    if (r.lshift == 7'h7f) begin
        if (r.expMore == 11'h7ff) begin
            vb_expPostScale = {1'h0, r.expMore};
        end else begin
            vb_expPostScale = {1'h0, (r.expMore + 1)};
        end
    end else begin
        if (((|r.expMore) == 1'b0) && ((|r.lshift) == 1'b0)) begin
            vb_expPostScale = 12'h001;
        end else begin
            vb_expPostScale = ({1'h0, r.expMore} - {1'h0, r.lshift});
        end
    end
    if ((signA ^ signOpB) == 1'b1) begin
        // subtractor only: result value becomes with exp=0
        if (((|r.expMore) == 1'b1)
                && ((vb_expPostScale[11] == 1'b1) || ((|vb_expPostScale) == 1'b0))) begin
            vb_expPostScale = (vb_expPostScale - 1);
        end
    end
    if (r.ena[4] == 1'b1) begin
        v.mantAlign = vb_mantAlign;
        v.expPostScale = vb_expPostScale;
        v.expPostScaleInv = ((~vb_expPostScale) + 1);
    end

    // Mantissa post-scale:
    //    Scaled = SumScale>>(-ExpSum) only if ExpSum < 0;
    vb_mantPostScale = r.mantAlign;
    if (r.expPostScale[11] == 1'b1) begin
        for (int i = 1; i < 105; i++) begin
            if (i == int'(r.expPostScaleInv)) begin
                vb_mantPostScale = (r.mantAlign >> i);
            end
        end
    end
    if (r.ena[5] == 1'b1) begin
        v.mantPostScale = vb_mantPostScale;
    end

    // Rounding bit
    mantShort = r.mantPostScale[104: 52];
    tmpMant05 = r.mantPostScale[51: 0];
    mantOnes = 1'b0;
    if (mantShort == 53'h1fffffffffffff) begin
        mantOnes = 1'b1;
    end
    mantEven = r.mantPostScale[52];
    if (tmpMant05 == 52'h8000000000000) begin
        mant05 = 1'b1;
    end
    rndBit = (r.mantPostScale[51] && (~(mant05 && (~mantEven))));

    // Check borders
    if ((|r.a[51: 0]) == 1'b0) begin
        mantZeroA = 1'b1;
    end
    if ((|r.b[51: 0]) == 1'b0) begin
        mantZeroB = 1'b1;
    end

    // Exceptions
    if (((|r.a[62: 0]) == 1'b0) && ((|r.b[62: 0]) == 1'b0)) begin
        allZero = 1'b1;
    end
    if ((|r.mantPostScale) == 1'b0) begin
        sumZero = 1'b1;
    end
    if (r.a[62: 52] == 11'h7ff) begin
        nanA = 1'b1;
    end
    if (r.b[62: 52] == 11'h7ff) begin
        nanB = 1'b1;
    end
    nanAB = (nanA && mantZeroA && nanB && mantZeroB);
    if (r.expPostScale == 12'h7ff) begin                    // positive
        overflow = 1'b1;
    end

    // Result multiplexers:
    if ((nanAB && signOp) == 1'b1) begin
        resAdd[63] = (signA ^ signOpB);
    end else if (nanA == 1'b1) begin
        // when both values are NaN, value B has higher priority if sign=1
        resAdd[63] = (signA || (nanB && signOpB));
    end else if (nanB == 1'b1) begin
        resAdd[63] = (signOpB ^ (signOp && (~mantZeroB)));
    end else if (allZero == 1'b1) begin
        resAdd[63] = (signA && signOpB);
    end else if (sumZero == 1'b1) begin
        resAdd[63] = 1'h0;
    end else begin
        resAdd[63] = r.signOpMore;
    end

    if ((nanA || nanB) == 1'b1) begin
        resAdd[62: 52] = '1;
    end else if ((r.expPostScale[11] == 1'b1) || (sumZero == 1'b1)) begin
        resAdd[62: 52] = '0;
    end else begin
        resAdd[62: 52] = (r.expPostScale + (mantOnes && rndBit && (~overflow)));
    end

    if ((nanA && mantZeroA && nanB && mantZeroB) == 1'b1) begin
        resAdd[51] = signOp;
        resAdd[50: 0] = '0;
    end else if ((nanA && (~(nanB && signOpB))) == 1'b1) begin
        // when both values are NaN, value B has higher priority if sign=1
        resAdd[51] = 1'h1;
        resAdd[50: 0] = r.a[50: 0];
    end else if (nanB == 1'b1) begin
        resAdd[51] = 1'h1;
        resAdd[50: 0] = r.b[50: 0];
    end else if (overflow == 1'b1) begin
        resAdd[51: 0] = '0;
    end else begin
        resAdd[51: 0] = (mantShort[51: 0] + rndBit);
    end

    resEQ[63: 1] = '0;
    resEQ[0] = r.flEqual;

    resLT[63: 1] = '0;
    resLT[0] = r.flLess;

    resLE[63: 1] = '0;
    resLE[0] = (r.flLess || r.flEqual);

    if ((nanA || nanB) == 1'b1) begin
        resMax = r.b;
    end else if (r.flMore == 1'b1) begin
        resMax = r.a;
    end else begin
        resMax = r.b;
    end

    if ((nanA || nanB) == 1'b1) begin
        resMin = r.b;
    end else if (r.flLess == 1'b1) begin
        resMin = r.a;
    end else begin
        resMin = r.b;
    end

    if (r.ena[6] == 1'b1) begin
        if (r.eq == 1'b1) begin
            v.result = resEQ;
        end else if (r.lt == 1'b1) begin
            v.result = resLT;
        end else if (r.le == 1'b1) begin
            v.result = resLE;
        end else if (r.max == 1'b1) begin
            v.result = resMax;
        end else if (r.min == 1'b1) begin
            v.result = resMin;
        end else begin
            v.result = resAdd;
        end

        v.illegal_op = (nanA || nanB);
        v.overflow = overflow;

        v.busy = 1'b0;
        v.add = 1'b0;
        v.sub = 1'b0;
        v.eq = 1'b0;
        v.lt = 1'b0;
        v.le = 1'b0;
        v.max = 1'b0;
        v.min = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = DoubleAdd_r_reset;
    end

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_overflow = r.overflow;
    o_valid = r.ena[7];
    o_busy = r.busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DoubleAdd_r_reset;
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

endmodule: DoubleAdd
