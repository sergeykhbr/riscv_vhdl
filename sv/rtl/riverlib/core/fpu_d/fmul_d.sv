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

module DoubleMul #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,
    input logic [63:0] i_a,                                 // Operand 1
    input logic [63:0] i_b,                                 // Operand 2
    output logic [63:0] o_res,                              // Result
    output logic o_illegal_op,
    output logic o_overflow,
    output logic o_valid,                                   // Result is valid
    output logic o_busy                                     // Multiclock instruction under processing
);

import fmul_d_pkg::*;

logic w_imul_ena;
logic [105:0] wb_imul_result;
logic [6:0] wb_imul_shift;
logic w_imul_rdy;
logic w_imul_overflow;
DoubleMul_registers r, rin;

imul53 #(
    .async_reset(async_reset)
) u_imul53 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(w_imul_ena),
    .i_a(r.mantA),
    .i_b(r.mantB),
    .o_result(wb_imul_result),
    .o_shift(wb_imul_shift),
    .o_rdy(w_imul_rdy),
    .o_overflow(w_imul_overflow)
);


always_comb
begin: comb_proc
    DoubleMul_registers v;
    logic [4:0] vb_ena;
    logic signA;
    logic signB;
    logic [52:0] mantA;
    logic [52:0] mantB;
    logic zeroA;
    logic zeroB;
    logic [11:0] expAB_t;
    logic [12:0] expAB;
    logic [104:0] mantAlign;
    logic [12:0] expAlign_t;
    logic [12:0] expAlign;
    logic [11:0] postShift;
    logic [104:0] mantPostScale;
    logic [52:0] mantShort;
    logic [51:0] tmpMant05;
    logic mantOnes;
    logic mantEven;
    logic mant05;
    logic rndBit;
    logic nanA;
    logic nanB;
    logic mantZeroA;
    logic mantZeroB;
    logic v_res_sign;
    logic [10:0] vb_res_exp;
    logic [51:0] vb_res_mant;

    vb_ena = 0;
    signA = 0;
    signB = 0;
    mantA = 0;
    mantB = 0;
    zeroA = 0;
    zeroB = 0;
    expAB_t = 0;
    expAB = 0;
    mantAlign = 0;
    expAlign_t = 0;
    expAlign = 0;
    postShift = 0;
    mantPostScale = 0;
    mantShort = 0;
    tmpMant05 = 0;
    mantOnes = 0;
    mantEven = 0;
    mant05 = 0;
    rndBit = 0;
    nanA = 0;
    nanB = 0;
    mantZeroA = 0;
    mantZeroB = 0;
    v_res_sign = 0;
    vb_res_exp = 0;
    vb_res_mant = 0;

    v = r;

    vb_ena[0] = (i_ena && (~r.busy));
    vb_ena[1] = r.ena[0];
    vb_ena[4: 2] = {r.ena[3: 2], w_imul_rdy};
    v.ena = vb_ena;

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.overflow = 1'b0;
        v.a = i_a;
        v.b = i_b;
    end

    signA = r.a[63];
    signB = r.b[63];

    if ((|r.a[62: 0]) == 1'b0) begin
        zeroA = 1'b1;
    end

    if ((|r.b[62: 0]) == 1'b0) begin
        zeroB = 1'b1;
    end

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

    // expA - expB + 1023
    expAB_t = ({1'h0, r.a[62: 52]} + {1'h0, r.b[62: 52]});
    expAB = ({1'h0, expAB_t} - 13'h03ff);

    if (r.ena[0] == 1'b1) begin
        v.expAB = expAB;
        v.zeroA = zeroA;
        v.zeroB = zeroB;
        v.mantA = mantA;
        v.mantB = mantB;
    end

    w_imul_ena = r.ena[1];

    // imul53 module:
    if (wb_imul_result[105] == 1'b1) begin
        mantAlign = wb_imul_result[105: 1];
    end else if (wb_imul_result[104] == 1'b1) begin
        mantAlign = wb_imul_result[104: 0];
    end else begin
        for (int i = 1; i < 105; i++) begin
            if (i == int'(wb_imul_shift)) begin
                mantAlign = (wb_imul_result << i);
            end
        end
    end

    expAlign_t = (r.expAB + 1);
    if (wb_imul_result[105] == 1'b1) begin
        expAlign = expAlign_t;
    end else if (((|r.a[62: 52]) == 1'b0) || ((|r.b[62: 52]) == 1'b0)) begin
        expAlign = (expAlign_t - {6'h00, wb_imul_shift});
    end else begin
        expAlign = (r.expAB - {6'h00, wb_imul_shift});
    end

    // IMPORTANT exception! new ZERO value
    if ((expAlign[12] == 1'b1) || ((|expAlign) == 1'b0)) begin
        if (((|wb_imul_shift) == 1'b0)
                || (wb_imul_result[105] == 1'b1)
                || ((|r.a[62: 52]) == 1'b0)
                || ((|r.b[62: 52]) == 1'b0)) begin
            postShift = ((~expAlign[11: 0]) + 12'h002);
        end else begin
            postShift = ((~expAlign[11: 0]) + 1);
        end
    end

    if (w_imul_rdy == 1'b1) begin
        v.expAlign = expAlign[11: 0];
        v.mantAlign = mantAlign;
        v.postShift = postShift;

        // Exceptions:
        v.nanA = 1'b0;
        if (r.a[62: 52] == 11'h7ff) begin
            v.nanA = 1'b1;
        end
        v.nanB = 1'b0;
        if (r.b[62: 52] == 11'h7ff) begin
            v.nanB = 1'b1;
        end
        v.overflow = 1'b0;
        if ((expAlign[12] == 1'b0) && (expAlign >= 13'h07ff)) begin
            v.overflow = 1'b1;
        end
    end

    // Prepare to mantissa post-scale
    if ((|r.postShift) == 1'b0) begin
        mantPostScale = r.mantAlign;
    end else if (r.postShift < 12'h069) begin
        for (int i = 1; i < 105; i++) begin
            if (i == int'(r.postShift)) begin
                mantPostScale = (r.mantAlign >> i);
            end
        end
    end
    if (r.ena[2] == 1'b1) begin
        v.mantPostScale = mantPostScale;
    end

    // Rounding bit
    mantShort = r.mantPostScale[104: 52];
    tmpMant05 = r.mantPostScale[51: 0];
    if (mantShort == 53'h1fffffffffffff) begin
        mantOnes = 1'b1;
    end
    mantEven = r.mantPostScale[52];
    if (tmpMant05 == 52'h8000000000000) begin
        mant05 = 1'b1;
    end
    rndBit = (r.mantPostScale[51] && (~(mant05 && (~mantEven))));

    // Check Borders
    if (r.a[62: 52] == 11'h7ff) begin
        nanA = 1'b1;
    end
    if (r.b[62: 52] == 11'h7ff) begin
        nanB = 1'b1;
    end
    if ((|r.a[51: 0]) == 1'b0) begin
        mantZeroA = 1'b1;
    end
    if ((|r.b[51: 0]) == 1'b0) begin
        mantZeroB = 1'b1;
    end

    // Result multiplexers:
    if ((nanA && mantZeroA && r.zeroB) || (nanB && mantZeroB && r.zeroA)) begin
        v_res_sign = 1'h1;
    end else if ((nanA && (~mantZeroA)) == 1'b1) begin
        // when both values are NaN, value B has higher priority if sign=1
        v_res_sign = (signA || (nanA && signB));
    end else if ((nanB && (~mantZeroB)) == 1'b1) begin
        v_res_sign = signB;
    end else begin
        v_res_sign = (r.a[63] ^ r.b[63]);
    end

    if (nanA == 1'b1) begin
        vb_res_exp = r.a[62: 52];
    end else if (nanB == 1'b1) begin
        vb_res_exp = r.b[62: 52];
    end else if ((r.expAlign[11] || r.zeroA || r.zeroB) == 1'b1) begin
        vb_res_exp = '0;
    end else if (r.overflow == 1'b1) begin
        vb_res_exp = '1;
    end else begin
        vb_res_exp = (r.expAlign[10: 0]
                + (mantOnes && rndBit && (~r.overflow)));
    end

    if ((nanA && mantZeroA && (~mantZeroB))
            || (nanB && mantZeroB && (~mantZeroA))
            || ((~nanA) && (~nanB) && r.overflow)) begin
        vb_res_mant = '0;
    end else if ((nanA && (~(nanB && signB))) == 1'b1) begin
        // when both values are NaN, value B has higher priority if sign=1
        vb_res_mant = {1'h1, r.a[50: 0]};
    end else if (nanB == 1'b1) begin
        vb_res_mant = {1'h1, r.b[50: 0]};
    end else begin
        vb_res_mant = (mantShort[51: 0] + rndBit);
    end

    if (r.ena[3] == 1'b1) begin
        v.result = {v_res_sign, vb_res_exp, vb_res_mant};
        v.illegal_op = (nanA || nanB);
        v.busy = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = DoubleMul_r_reset;
    end

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_overflow = r.overflow;
    o_valid = r.ena[4];
    o_busy = r.busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DoubleMul_r_reset;
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

endmodule: DoubleMul
