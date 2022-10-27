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

module idiv53 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,                                      // divider enable pulse (1 clock)
    input logic [52:0] i_divident,                          // integer value
    input logic [52:0] i_divisor,                           // integer value
    output logic [104:0] o_result,                          // resulting bits
    output logic [6:0] o_lshift,                            // first non-zero bit index
    output logic o_rdy,                                     // delayed 'enable' signal
    output logic o_overflow,                                // overflow flag
    output logic o_zero_resid                               // reasidual is zero flag
);

import idiv53_pkg::*;

logic w_mux_ena_i;
logic [55:0] wb_muxind_i;
logic [60:0] wb_divident_i;
logic [52:0] wb_divisor_i;
logic [52:0] wb_dif_o;
logic [7:0] wb_bits_o;
logic [6:0] wb_muxind_o;
logic w_muxind_rdy_o;
idiv53_registers r, rin;

divstage53 divstage0 (
    .i_mux_ena(w_mux_ena_i),
    .i_muxind(wb_muxind_i),
    .i_divident(wb_divident_i),
    .i_divisor(wb_divisor_i),
    .o_dif(wb_dif_o),
    .o_bits(wb_bits_o),
    .o_muxind(wb_muxind_o),
    .o_muxind_rdy(w_muxind_rdy_o)
);


always_comb
begin: comb_proc
    idiv53_registers v;
    logic v_ena;
    logic [55:0] vb_muxind;
    logic [104:0] vb_bits;
    logic v_mux_ena_i;

    v_ena = 0;
    vb_muxind = 0;
    vb_bits = 0;
    v_mux_ena_i = 0;

    v = r;

    vb_bits = r.bits;

    v_ena = i_ena;
    v.delay = {r.delay[13: 0], v_ena};
    if (i_ena == 1'b1) begin
        v.divident = {8'h00, i_divident};
        v.divisor = i_divisor;
        v.lshift_rdy = 1'b0;
        v.overflow = 1'b0;
        v.zero_resid = 1'b0;
    end else if (r.delay[0] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_bits[104] = (~wb_dif_o[52]);
    end else if (r.delay[1] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 1;
        vb_muxind[48: 42] = 2;
        vb_muxind[41: 35] = 3;
        vb_muxind[34: 28] = 4;
        vb_muxind[27: 21] = 5;
        vb_muxind[20: 14] = 6;
        vb_muxind[13: 7] = 7;
        vb_muxind[6: 0] = 8;
        vb_bits[103: 96] = wb_bits_o;
    end else if (r.delay[2] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 9;
        vb_muxind[48: 42] = 10;
        vb_muxind[41: 35] = 11;
        vb_muxind[34: 28] = 12;
        vb_muxind[27: 21] = 13;
        vb_muxind[20: 14] = 14;
        vb_muxind[13: 7] = 15;
        vb_muxind[6: 0] = 16;
        vb_bits[95: 88] = wb_bits_o;
    end else if (r.delay[3] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 17;
        vb_muxind[48: 42] = 18;
        vb_muxind[41: 35] = 19;
        vb_muxind[34: 28] = 20;
        vb_muxind[27: 21] = 21;
        vb_muxind[20: 14] = 22;
        vb_muxind[13: 7] = 23;
        vb_muxind[6: 0] = 24;
        vb_bits[87: 80] = wb_bits_o;
    end else if (r.delay[4] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 25;
        vb_muxind[48: 42] = 26;
        vb_muxind[41: 35] = 27;
        vb_muxind[34: 28] = 28;
        vb_muxind[27: 21] = 29;
        vb_muxind[20: 14] = 30;
        vb_muxind[13: 7] = 31;
        vb_muxind[6: 0] = 32;
        vb_bits[79: 72] = wb_bits_o;
    end else if (r.delay[5] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 33;
        vb_muxind[48: 42] = 34;
        vb_muxind[41: 35] = 35;
        vb_muxind[34: 28] = 36;
        vb_muxind[27: 21] = 37;
        vb_muxind[20: 14] = 38;
        vb_muxind[13: 7] = 39;
        vb_muxind[6: 0] = 40;
        vb_bits[71: 64] = wb_bits_o;
    end else if (r.delay[6] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 41;
        vb_muxind[48: 42] = 42;
        vb_muxind[41: 35] = 43;
        vb_muxind[34: 28] = 44;
        vb_muxind[27: 21] = 45;
        vb_muxind[20: 14] = 46;
        vb_muxind[13: 7] = 47;
        vb_muxind[6: 0] = 48;
        vb_bits[63: 56] = wb_bits_o;
    end else if (r.delay[7] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 49;
        vb_muxind[48: 42] = 50;
        vb_muxind[41: 35] = 51;
        vb_muxind[34: 28] = 52;
        vb_muxind[27: 21] = 53;
        vb_muxind[20: 14] = 54;
        vb_muxind[13: 7] = 55;
        vb_muxind[6: 0] = 56;
        vb_bits[55: 48] = wb_bits_o;
    end else if (r.delay[8] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 57;
        vb_muxind[48: 42] = 58;
        vb_muxind[41: 35] = 59;
        vb_muxind[34: 28] = 60;
        vb_muxind[27: 21] = 61;
        vb_muxind[20: 14] = 62;
        vb_muxind[13: 7] = 63;
        vb_muxind[6: 0] = 64;
        vb_bits[47: 40] = wb_bits_o;
    end else if (r.delay[9] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 65;
        vb_muxind[48: 42] = 66;
        vb_muxind[41: 35] = 67;
        vb_muxind[34: 28] = 68;
        vb_muxind[27: 21] = 69;
        vb_muxind[20: 14] = 70;
        vb_muxind[13: 7] = 71;
        vb_muxind[6: 0] = 72;
        vb_bits[39: 32] = wb_bits_o;
    end else if (r.delay[10] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 73;
        vb_muxind[48: 42] = 74;
        vb_muxind[41: 35] = 75;
        vb_muxind[34: 28] = 76;
        vb_muxind[27: 21] = 77;
        vb_muxind[20: 14] = 78;
        vb_muxind[13: 7] = 79;
        vb_muxind[6: 0] = 80;
        vb_bits[31: 24] = wb_bits_o;
    end else if (r.delay[11] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 81;
        vb_muxind[48: 42] = 82;
        vb_muxind[41: 35] = 83;
        vb_muxind[34: 28] = 84;
        vb_muxind[27: 21] = 85;
        vb_muxind[20: 14] = 86;
        vb_muxind[13: 7] = 87;
        vb_muxind[6: 0] = 88;
        vb_bits[23: 16] = wb_bits_o;
    end else if (r.delay[12] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 89;
        vb_muxind[48: 42] = 90;
        vb_muxind[41: 35] = 91;
        vb_muxind[34: 28] = 92;
        vb_muxind[27: 21] = 93;
        vb_muxind[20: 14] = 94;
        vb_muxind[13: 7] = 95;
        vb_muxind[6: 0] = 96;
        vb_bits[15: 8] = wb_bits_o;
    end else if (r.delay[13] == 1'b1) begin
        v_mux_ena_i = (~r.lshift_rdy);
        v.divident = {wb_dif_o, 8'h00};
        vb_muxind[55: 49] = 97;
        vb_muxind[48: 42] = 98;
        vb_muxind[41: 35] = 99;
        vb_muxind[34: 28] = 100;
        vb_muxind[27: 21] = 101;
        vb_muxind[20: 14] = 102;
        vb_muxind[13: 7] = 103;
        vb_muxind[6: 0] = 104;
        vb_bits[7: 0] = wb_bits_o;

        if ((|wb_dif_o) == 1'b0) begin
            v.zero_resid = 1'b1;
        end
        if (r.lshift == 7'h7f) begin
            v.overflow = 1'b1;
        end
    end

    if (r.lshift_rdy == 1'b0) begin
        if (w_muxind_rdy_o == 1'b1) begin
            v.lshift_rdy = 1'b1;
            v.lshift = wb_muxind_o;
        end else if (r.delay[13] == 1'b1) begin
            v.lshift_rdy = 1'b1;
            v.lshift = 7'h68;
        end
    end

    w_mux_ena_i = v_mux_ena_i;
    wb_divident_i = r.divident;
    wb_divisor_i = r.divisor;
    wb_muxind_i = vb_muxind;
    v.bits = vb_bits;

    if (~async_reset && i_nrst == 1'b0) begin
        v = idiv53_r_reset;
    end

    o_result = r.bits;
    o_lshift = r.lshift;
    o_overflow = r.overflow;
    o_zero_resid = r.zero_resid;
    o_rdy = r.delay[14];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= idiv53_r_reset;
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

endmodule: idiv53
