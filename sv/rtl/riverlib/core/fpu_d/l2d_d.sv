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

module Long2Double #(
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
    output logic o_valid,                                   // Result is valid
    output logic o_busy                                     // Multiclock instruction under processing
);

import l2d_d_pkg::*;

Long2Double_registers r, rin;

always_comb
begin: comb_proc
    Long2Double_registers v;
    logic v_ena;
    logic [63:0] mantAlign;
    logic [5:0] lshift;
    logic [10:0] expAlign;
    logic mantEven;
    logic mant05;
    logic mantOnes;
    logic rndBit;
    logic v_signA;
    logic [63:0] vb_A;
    logic [63:0] res;

    v_ena = 0;
    mantAlign = 0;
    lshift = 0;
    expAlign = 0;
    mantEven = 0;
    mant05 = 0;
    mantOnes = 0;
    rndBit = 0;
    v_signA = 0;
    vb_A = 0;
    res = 0;

    v = r;

    v_ena = (i_ena && (~r.busy));
    v.ena = {r.ena[1: 0], v_ena};
    if (i_w32 == 1'b0) begin
        v_signA = i_a[63];
        vb_A = i_a;
    end else if ((i_signed && i_a[31]) == 1'b1) begin
        v_signA = 1'b1;
        vb_A[63: 32] = '1;
        vb_A[31: 0] = i_a[31: 0];
    end else begin
        v_signA = 1'b0;
        vb_A[31: 0] = i_a[31: 0];
        vb_A[63: 32] = '0;
    end

    if (i_ena == 1'b1) begin
        v.busy = 1'b1;
        if ((i_signed && v_signA) == 1'b1) begin
            v.signA = 1'b1;
            v.absA = ((~vb_A) + 1);
        end else begin
            v.signA = 1'b0;
            v.absA = vb_A;
        end
        v.op_signed = i_signed;
    end

    // multiplexer, probably if/elsif in rtl:
    lshift = 6'h3f;
    if (r.absA[63] == 1'b1) begin
        mantAlign = r.absA;
    end else begin
        for (int i = 1; i < 64; i++) begin
            if ((lshift == 6'h3f) && (r.absA[(63 - i)] == 1'b1)) begin
                mantAlign = (r.absA << i);
                lshift = i;
            end
        end
    end

    if (r.ena[0] == 1'b1) begin
        v.mantAlign = mantAlign;
        v.lshift = lshift;
    end

    if ((|r.absA) == 1'b0) begin
        expAlign = '0;
    end else begin
        expAlign = (11'h43e - r.lshift);
    end

    mantEven = r.mantAlign[11];
    if (r.mantAlign[10: 0] == 11'h7ff) begin
        mant05 = 1'b1;
    end
    rndBit = (r.mantAlign[10] && (~(mant05 && mantEven)));
    if (r.mantAlign[63: 11] == 52'hfffffffffffff) begin
        mantOnes = 1'b1;
    end

    // Result multiplexers:
    res[63] = (r.signA && r.op_signed);
    res[62: 52] = (expAlign + (mantOnes && rndBit));
    res[51: 0] = (r.mantAlign[62: 11] + rndBit);

    if (r.ena[1] == 1'b1) begin
        v.result = res;
        v.busy = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = Long2Double_r_reset;
    end

    o_res = r.result;
    o_valid = r.ena[2];
    o_busy = r.busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= Long2Double_r_reset;
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

endmodule: Long2Double
