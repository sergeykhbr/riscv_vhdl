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

module divstage53(
    input logic i_mux_ena,                                  // find first non-zero bit
    input logic [55:0] i_muxind,                            // bits indexes 8x7 bits bus
    input logic [60:0] i_divident,                          // integer value
    input logic [52:0] i_divisor,                           // integer value
    output logic [52:0] o_dif,                              // residual value
    output logic [7:0] o_bits,                              // resulting bits
    output logic [6:0] o_muxind,                            // first found non-zero bits
    output logic o_muxind_rdy                               // seeking was successfull
);

import divstage53_pkg::*;

logic [61:0] wb_thresh[0: 16 - 1];
logic [60:0] wb_dif[0: 4 - 1];

always_comb
begin: comb_proc
    logic [7:0] wb_bits;
    logic [54:0] wb_divx3;                                  // width 53+2
    logic [54:0] wb_divx2;                                  // width 53+2
    logic [6:0] wb_muxind;
    logic w_muxind_rdy;

    wb_bits = 0;
    wb_divx3 = 55'h00000000000000;
    wb_divx2 = 55'h00000000000000;
    wb_muxind = 0;
    w_muxind_rdy = 0;

    wb_divx2 = {i_divisor, 1'h0};
    wb_divx3 = (wb_divx2 + i_divisor);

    // stage 1 of 4
    wb_thresh[15] = ({1'h0, i_divident} - {1'h0, wb_divx3, 6'h00});
    wb_thresh[14] = ({1'h0, i_divident} - {1'h0, wb_divx2, 6'h00});
    wb_thresh[13] = ({1'h0, i_divident} - {3'h0, i_divisor, 6'h00});
    wb_thresh[12] = {1'h0, i_divident};

    if (wb_thresh[15][61] == 1'b0) begin
        wb_bits[7: 6] = 2'h3;
        wb_dif[0] = wb_thresh[15][60: 0];
    end else if (wb_thresh[14][61] == 1'b0) begin
        wb_bits[7: 6] = 2'h2;
        wb_dif[0] = wb_thresh[14][60: 0];
    end else if (wb_thresh[13][61] == 1'b0) begin
        wb_bits[7: 6] = 2'h1;
        wb_dif[0] = wb_thresh[13][60: 0];
    end else begin
        wb_bits[7: 6] = 2'h0;
        wb_dif[0] = wb_thresh[12][60: 0];
    end

    // stage 2 of 4
    wb_thresh[11] = ({1'h0, wb_dif[0]} - {3'h0, wb_divx3, 4'h0});
    wb_thresh[10] = ({1'h0, wb_dif[0]} - {3'h0, wb_divx2, 4'h0});
    wb_thresh[9] = ({1'h0, wb_dif[0]} - {5'h00, i_divisor, 4'h0});
    wb_thresh[8] = {1'h0, wb_dif[0]};

    if (wb_thresh[11][61] == 1'b0) begin
        wb_bits[5: 4] = 2'h3;
        wb_dif[1] = wb_thresh[11][60: 0];
    end else if (wb_thresh[10][61] == 1'b0) begin
        wb_bits[5: 4] = 2'h2;
        wb_dif[1] = wb_thresh[10][60: 0];
    end else if (wb_thresh[9][61] == 1'b0) begin
        wb_bits[5: 4] = 2'h1;
        wb_dif[1] = wb_thresh[9][60: 0];
    end else begin
        wb_bits[5: 4] = 2'h0;
        wb_dif[1] = wb_thresh[8][60: 0];
    end

    // stage 3 of 4
    wb_thresh[7] = ({1'h0, wb_dif[1]} - {5'h00, wb_divx3, 2'h0});
    wb_thresh[6] = ({1'h0, wb_dif[1]} - {5'h00, wb_divx2, 2'h0});
    wb_thresh[5] = ({1'h0, wb_dif[1]} - {7'h00, i_divisor, 2'h0});
    wb_thresh[4] = {1'h0, wb_dif[1]};
    if (wb_thresh[7][61] == 1'b0) begin
        wb_bits[3: 2] = 2'h3;
        wb_dif[2] = wb_thresh[7][60: 0];
    end else if (wb_thresh[6][61] == 1'b0) begin
        wb_bits[3: 2] = 2'h2;
        wb_dif[2] = wb_thresh[6][60: 0];
    end else if (wb_thresh[5][61] == 1'b0) begin
        wb_bits[3: 2] = 2'h1;
        wb_dif[2] = wb_thresh[5][60: 0];
    end else begin
        wb_bits[3: 2] = 2'h0;
        wb_dif[2] = wb_thresh[4][60: 0];
    end

    // stage 4 of 4
    wb_thresh[3] = ({1'h0, wb_dif[2]} - {7'h00, wb_divx3});
    wb_thresh[2] = ({1'h0, wb_dif[2]} - {7'h00, wb_divx2});
    wb_thresh[1] = ({1'h0, wb_dif[2]} - {9'h000, i_divisor});
    wb_thresh[0] = {1'h0, wb_dif[2]};
    if (wb_thresh[3][61] == 1'b0) begin
        wb_bits[1: 0] = 2'h3;
        wb_dif[3] = wb_thresh[3][60: 0];
    end else if (wb_thresh[2][61] == 1'b0) begin
        wb_bits[1: 0] = 2'h2;
        wb_dif[3] = wb_thresh[2][60: 0];
    end else if (wb_thresh[1][61] == 1'b0) begin
        wb_bits[1: 0] = 2'h1;
        wb_dif[3] = wb_thresh[1][60: 0];
    end else begin
        wb_bits[1: 0] = 2'h0;
        wb_dif[3] = wb_thresh[0][60: 0];
    end

    // Number multiplexor
    wb_muxind = '0;
    if (i_mux_ena == 1'b1) begin
        if (wb_thresh[15][61] == 1'b0) begin
            wb_muxind = i_muxind[55: 49];
        end else if (wb_thresh[14][61] == 1'b0) begin
            wb_muxind = i_muxind[55: 49];
        end else if (wb_thresh[13][61] == 1'b0) begin
            wb_muxind = i_muxind[48: 42];
        end else if (wb_thresh[11][61] == 1'b0) begin
            wb_muxind = i_muxind[41: 35];
        end else if (wb_thresh[10][61] == 1'b0) begin
            wb_muxind = i_muxind[41: 35];
        end else if (wb_thresh[9][61] == 1'b0) begin
            wb_muxind = i_muxind[34: 28];
        end else if (wb_thresh[7][61] == 1'b0) begin
            wb_muxind = i_muxind[27: 21];
        end else if (wb_thresh[6][61] == 1'b0) begin
            wb_muxind = i_muxind[27: 21];
        end else if (wb_thresh[5][61] == 1'b0) begin
            wb_muxind = i_muxind[20: 14];
        end else if (wb_thresh[3][61] == 1'b0) begin
            wb_muxind = i_muxind[13: 7];
        end else if (wb_thresh[2][61] == 1'b0) begin
            wb_muxind = i_muxind[13: 7];
        end else if (wb_thresh[1][61] == 1'b0) begin
            wb_muxind = i_muxind[6: 0];
        end else begin
            wb_muxind = i_muxind[6: 0];
        end
    end

    w_muxind_rdy = 1'b0;
    if ((i_mux_ena == 1'b1) && ((|wb_bits) == 1'b1)) begin
        w_muxind_rdy = 1'b1;
    end

    o_bits = wb_bits;
    o_dif = wb_dif[3][52: 0];
    o_muxind = wb_muxind;
    o_muxind_rdy = w_muxind_rdy;
end: comb_proc

endmodule: divstage53
