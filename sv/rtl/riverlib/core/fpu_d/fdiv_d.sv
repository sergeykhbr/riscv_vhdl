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

module DoubleDiv #(
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
    output logic o_divbyzero,
    output logic o_overflow,
    output logic o_underflow,
    output logic o_valid,                                   // Result is valid
    output logic o_busy                                     // Multiclock instruction under processing
);

import fdiv_d_pkg::*;

logic w_idiv_ena;
logic [52:0] wb_divident;
logic [52:0] wb_divisor;
logic [104:0] wb_idiv_result;
logic [6:0] wb_idiv_lshift;
logic w_idiv_rdy;
logic w_idiv_overflow;
logic w_idiv_zeroresid;
DoubleDiv_registers r, rin;

idiv53 #(
    .async_reset(async_reset)
) u_idiv53 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(w_idiv_ena),
    .i_divident(wb_divident),
    .i_divisor(wb_divisor),
    .o_result(wb_idiv_result),
    .o_lshift(wb_idiv_lshift),
    .o_rdy(w_idiv_rdy),
    .o_overflow(w_idiv_overflow),
    .o_zero_resid(w_idiv_zeroresid)
);


always_comb
begin: comb_proc
    DoubleDiv_registers v;
    logic [4:0] vb_ena;
    logic signA;
    logic signB;
    logic [52:0] mantA;
    logic [52:0] mantB;
    logic zeroA;
    logic zeroB;
    logic [52:0] divisor;
    logic [5:0] preShift;
    logic [11:0] expAB_t;
    logic [12:0] expAB;
    logic [104:0] mantAlign;
    logic [11:0] expShift;
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
    logic [63:0] res;

    vb_ena = 0;
    signA = 0;
    signB = 0;
    mantA = 0;
    mantB = 0;
    zeroA = 0;
    zeroB = 0;
    divisor = 0;
    preShift = 0;
    expAB_t = 0;
    expAB = 0;
    mantAlign = 0;
    expShift = 0;
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
    res = 0;

    v = r;

    vb_ena[0] = (i_ena && (~r.busy));
    vb_ena[1] = r.ena[0];
    vb_ena[4: 2] = {r.ena[3: 2], w_idiv_rdy};
    v.ena = vb_ena;

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        v.overflow = 1'b0;
        v.underflow = 1'b0;
        v.illegal_op = 1'b0;
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
        divisor = mantB;
    end else begin
        // multiplexer for operation with zero expanent
        divisor = mantB;
        for (int i = 1; i < 53; i++) begin
            if (((|preShift) == 1'b0) && (mantB[(52 - i)] == 1'b1)) begin
                divisor = (mantB << i);
                preShift = i;
            end
        end
    end

    // expA - expB + 1023
    expAB_t = ({1'h0, r.a[62: 52]} + 12'h3ff);
    expAB = ({1'h0, expAB_t} - {2'h0, r.b[62: 52]});        // signed value

    if (r.ena[0] == 1'b1) begin
        v.divisor = divisor;
        v.preShift = preShift;
        v.expAB = expAB;
        v.zeroA = zeroA;
        v.zeroB = zeroB;
    end

    w_idiv_ena = r.ena[1];
    wb_divident = mantA;
    wb_divisor = r.divisor;

    // idiv53 module:
    for (int i = 0; i < 105; i++) begin
        if (i == wb_idiv_lshift) begin
            mantAlign = (wb_idiv_result << i);
        end
    end

    expShift = ({6'h00, r.preShift} - {5'h00, wb_idiv_lshift});
    if (((|r.b[62: 52]) == 1'b0) && ((|r.a[62: 52]) == 1'b1)) begin
        expShift = (expShift - 1);
    end else if (((|r.b[62: 52]) == 1'b1) && ((|r.a[62: 52]) == 1'b0)) begin
        expShift = (expShift + 1);
    end

    expAlign = (r.expAB + {expShift[11], expShift});
    if (expAlign[12] == 1'b1) begin
        postShift = ((~expAlign[11: 0]) + 12'h002);
    end else begin
        postShift = '0;
    end

    if (w_idiv_rdy == 1'b1) begin
        v.expAlign = expAlign[11: 0];
        v.mantAlign = mantAlign;
        v.postShift = postShift;

        // Exceptions:
        v.nanRes = 1'b0;
        if (expAlign == 13'h07ff) begin
            v.nanRes = 1'b1;
        end
        v.overflow = ((~expAlign[12]) && expAlign[11]);
        v.underflow = (expAlign[12] && expAlign[11]);
    end

    // Prepare to mantissa post-scale
    if ((|r.postShift) == 1'b0) begin
        mantPostScale = r.mantAlign;
    end else if (r.postShift < 12'h069) begin
        for (int i = 0; i < 105; i++) begin
            if (i == r.postShift) begin
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
    if (nanA && mantZeroA && nanB && mantZeroB) begin
        res[63] = 1'h1;
    end else if (nanA && (~mantZeroA)) begin
        res[63] = signA;
    end else if (nanB && (~mantZeroB)) begin
        res[63] = signB;
    end else if (r.zeroA && r.zeroB) begin
        res[63] = 1'h1;
    end else begin
        res[63] = (r.a[63] ^ r.b[63]);
    end

    if ((nanB && (~mantZeroB)) == 1'b1) begin
        res[62: 52] = r.b[62: 52];
    end else if (((r.underflow || r.zeroA) && (~r.zeroB)) == 1'b1) begin
        res[62: 52] = '0;
    end else if ((r.overflow || r.zeroB) == 1'b1) begin
        res[62: 52] = '1;
    end else if (nanA == 1'b1) begin
        res[62: 52] = r.a[62: 52];
    end else if (((nanB && mantZeroB) || r.expAlign[11]) == 1'b1) begin
        res[62: 52] = '0;
    end else begin
        res[62: 52] = (r.expAlign[10: 0] + (mantOnes && rndBit && (~r.overflow)));
    end

    if (((r.zeroA && r.zeroB)
            || (nanA && mantZeroA && nanB && mantZeroB)) == 1'b1) begin
        res[51] = 1'h1;
        res[50: 0] = '0;
    end else if ((nanA && (~mantZeroA)) == 1'b1) begin
        res[51] = 1'h1;
        res[50: 0] = r.a[50: 0];
    end else if ((nanB && (~mantZeroB)) == 1'b1) begin
        res[51] = 1'h1;
        res[50: 0] = r.b[50: 0];
    end else if ((r.overflow
                || r.nanRes
                || (nanA && mantZeroA)
                || (nanB && mantZeroB)) == 1'b1) begin
        res[51: 0] = '0;
    end else begin
        res[51: 0] = (mantShort[51: 0] + rndBit);
    end

    if (r.ena[3] == 1'b1) begin
        v.result = res;
        v.illegal_op = (nanA || nanB);
        v.busy = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = DoubleDiv_r_reset;
    end

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_divbyzero = r.zeroB;
    o_overflow = r.overflow;
    o_underflow = r.underflow;
    o_valid = r.ena[4];
    o_busy = r.busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DoubleDiv_r_reset;
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

endmodule: DoubleDiv
