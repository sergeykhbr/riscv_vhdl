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

module imul53 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,                                      // enable pulse (1 clock)
    input logic [52:0] i_a,                                 // integer value
    input logic [52:0] i_b,                                 // integer value
    output logic [105:0] o_result,                          // resulting bits
    output logic [6:0] o_shift,                             // first non-zero bit index
    output logic o_rdy,                                     // delayed 'enable' signal
    output logic o_overflow                                 // overflow flag
);

import imul53_pkg::*;

logic [104:0] wb_sumInv;
logic [6:0] wb_lshift;
imul53_registers r, rin;

zeroenc #(
    .iwidth(105),
    .shiftwidth(7)
) enc0 (
    .i_value(wb_sumInv),
    .o_shift(wb_lshift)
);


always_comb
begin: comb_proc
    imul53_registers v;
    logic v_ena;
    logic [56:0] vb_mux[0: 17-1];
    logic [56:0] vb_sel;
    logic [6:0] vb_shift;
    logic [104:0] vb_sumInv;

    v_ena = 0;
    for (int i = 0; i < 17; i++) begin
        vb_mux[i] = 57'h000000000000000;
    end
    vb_sel = 0;
    vb_shift = 0;
    vb_sumInv = 0;

    v = r;


    vb_mux[0] = '0;
    vb_mux[1] = i_a;
    vb_mux[2] = {i_a, 1'h0};
    vb_mux[3] = (vb_mux[2] + vb_mux[1]);
    vb_mux[4] = {i_a, 2'h0};
    vb_mux[5] = (vb_mux[4] + vb_mux[1]);
    vb_mux[6] = (vb_mux[4] + vb_mux[2]);
    vb_mux[8] = {i_a, 3'h0};
    vb_mux[7] = (vb_mux[8] - vb_mux[1]);
    vb_mux[9] = (vb_mux[8] + vb_mux[1]);
    vb_mux[10] = (vb_mux[8] + vb_mux[2]);
    vb_mux[11] = (vb_mux[10] + vb_mux[1]);
    vb_mux[12] = (vb_mux[8] + vb_mux[4]);
    vb_mux[16] = {i_a, 4'h0};
    vb_mux[13] = (vb_mux[16] - vb_mux[3]);
    vb_mux[14] = (vb_mux[16] - vb_mux[2]);
    vb_mux[15] = (vb_mux[16] - vb_mux[1]);

    v_ena = i_ena;
    v.delay = {r.delay[14: 0], v_ena};

    if (i_ena == 1'b1) begin
        v.b = {3'h0, i_b};
        v.overflow = 1'b0;
        v.accum_ena = 1'b1;
        v.sum = '0;
        v.shift = '0;
    end else if (r.delay[13] == 1'b1) begin
        v.accum_ena = 1'b0;
    end

    case (r.b[55: 52])
    4'h1: begin
        vb_sel = vb_mux[1];
    end
    4'h2: begin
        vb_sel = vb_mux[2];
    end
    4'h3: begin
        vb_sel = vb_mux[3];
    end
    4'h4: begin
        vb_sel = vb_mux[4];
    end
    4'h5: begin
        vb_sel = vb_mux[5];
    end
    4'h6: begin
        vb_sel = vb_mux[6];
    end
    4'h7: begin
        vb_sel = vb_mux[7];
    end
    4'h8: begin
        vb_sel = vb_mux[8];
    end
    4'h9: begin
        vb_sel = vb_mux[9];
    end
    4'ha: begin
        vb_sel = vb_mux[10];
    end
    4'hb: begin
        vb_sel = vb_mux[11];
    end
    4'hc: begin
        vb_sel = vb_mux[12];
    end
    4'hd: begin
        vb_sel = vb_mux[13];
    end
    4'he: begin
        vb_sel = vb_mux[14];
    end
    4'hf: begin
        vb_sel = vb_mux[15];
    end
    default: begin
        vb_sel = '0;
    end
    endcase
    if (r.accum_ena == 1'b1) begin
        v.sum = ({r.sum, 4'h0} + vb_sel);
        v.b = {r.b, 4'h0};
    end

    // To avoid timing constrains violation try to implement parallel demux
    // for Xilinx Vivado
    for (int i = 0; i < 104; i++) begin
        vb_sumInv[(i + 1)] = r.sum[(103 - i)];
    end
    wb_sumInv = vb_sumInv;

    if (r.sum[105] == 1'b1) begin
        vb_shift = '1;
        v.overflow = 1'b1;
    end else if (r.sum[104] == 1'b1) begin
        vb_shift = '0;
    end else begin
        vb_shift = wb_lshift;
    end

    if (r.delay[14] == 1'b1) begin
        v.shift = vb_shift;
        v.overflow = 1'b0;
        if (vb_shift == 7'h7f) begin
            v.overflow = 1'b1;
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = imul53_r_reset;
    end

    o_result = r.sum;
    o_shift = r.shift;
    o_overflow = r.overflow;
    o_rdy = r.delay[15];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= imul53_r_reset;
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

endmodule: imul53
