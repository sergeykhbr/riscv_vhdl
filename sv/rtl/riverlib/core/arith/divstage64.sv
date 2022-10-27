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

module divstage64(
    input logic [63:0] i_divident,                          // integer value
    input logic [123:0] i_divisor,                          // integer value
    output logic [63:0] o_resid,                            // residual
    output logic [3:0] o_bits                               // resulting bits
);

import divstage64_pkg::*;

always_comb
begin: comb_proc
    logic [3:0] wb_bits;
    logic [63:0] wb_dif;
    logic [64:0] wb_divx1;
    logic [64:0] wb_divx2;
    logic [64:0] wb_divx3;
    logic [64:0] wb_divx4;
    logic [64:0] wb_divx5;
    logic [64:0] wb_divx6;
    logic [64:0] wb_divx7;
    logic [64:0] wb_divx8;
    logic [64:0] wb_divx9;
    logic [64:0] wb_divx10;
    logic [64:0] wb_divx11;
    logic [64:0] wb_divx12;
    logic [64:0] wb_divx13;
    logic [64:0] wb_divx14;
    logic [64:0] wb_divx15;
    logic [64:0] wb_divx16;
    logic [64:0] wb_divident;
    logic [123:0] wb_divisor;
    logic [65:0] wb_thresh[0: 16-1];

    wb_bits = 0;
    wb_dif = 0;
    wb_divx1 = 0;
    wb_divx2 = 0;
    wb_divx3 = 0;
    wb_divx4 = 0;
    wb_divx5 = 0;
    wb_divx6 = 0;
    wb_divx7 = 0;
    wb_divx8 = 0;
    wb_divx9 = 0;
    wb_divx10 = 0;
    wb_divx11 = 0;
    wb_divx12 = 0;
    wb_divx13 = 0;
    wb_divx14 = 0;
    wb_divx15 = 0;
    wb_divx16 = 0;
    wb_divident = 0;
    wb_divisor = 0;
    for (int i = 0; i < 16; i++) begin
        wb_thresh[i] = '0;
    end

    wb_divident = i_divident;
    wb_divisor = i_divisor;

    wb_divx1[63: 0] = wb_divisor[63: 0];
    if ((|wb_divisor[123: 64]) == 1'b1) begin
        wb_divx1[64] = 1'b1;
    end else begin
        wb_divx1[64] = 1'b0;
    end

    wb_divx2[0] = 1'b0;
    wb_divx2[63: 1] = wb_divisor[62: 0];
    if ((|wb_divisor[123: 63]) == 1'b1) begin
        wb_divx2[64] = 1'b1;
    end else begin
        wb_divx2[64] = 1'b0;
    end

    wb_divx3[64: 0] = ({1'h0, wb_divx2[63: 0]} + {1'h0, wb_divx1[63: 0]});
    wb_divx3[64] = (wb_divx3[64] || wb_divx2[64]);

    wb_divx4[1: 0] = 2'h0;
    wb_divx4[63: 2] = wb_divisor[61: 0];
    if ((|wb_divisor[123: 62]) == 1'b1) begin
        wb_divx4[64] = 1'b1;
    end else begin
        wb_divx4[64] = 1'b0;
    end

    wb_divx5[64: 0] = ({1'h0, wb_divx4[63: 0]} + {1'h0, wb_divx1[63: 0]});
    wb_divx5[64] = (wb_divx5[64] || wb_divx4[64]);

    wb_divx6[0] = 1'b0;
    wb_divx6[63: 1] = wb_divx3[62: 0];
    wb_divx6[64] = (wb_divx3[64] || wb_divx3[63]);

    wb_divx8[2: 0] = 3'h0;
    wb_divx8[63: 3] = wb_divisor[60: 0];
    if ((|wb_divisor[123: 61]) == 1'b1) begin
        wb_divx8[64] = 1'b1;
    end else begin
        wb_divx8[64] = 1'b0;
    end

    // 7 = 8 - 1
    wb_divx7[64: 0] = (wb_divx8[64: 0] - {1'h0, wb_divx1[63: 0]});
    if ((wb_divx7[64] == 1'b1) || ((|wb_divisor[123: 62]) == 1'b1)) begin
        wb_divx7[64] = 1'b1;
    end else begin
        wb_divx7[64] = 1'b0;
    end

    // 9 = 8 + 1
    wb_divx9[64: 0] = ({1'h0, wb_divx8[63: 0]} + {1'h0, wb_divx1[63: 0]});
    if ((wb_divx9[64] == 1'b1) || ((|wb_divisor[123: 61]) == 1'b1)) begin
        wb_divx9[64] = 1'b1;
    end else begin
        wb_divx9[64] = 1'b0;
    end

    // 10 = 8 + 2
    wb_divx10[64: 0] = ({1'h0, wb_divx8[63: 0]} + {1'h0, wb_divx2[63: 0]});
    if ((wb_divx10[64] == 1'b1) || ((|wb_divisor[123: 61]) == 1'b1)) begin
        wb_divx10[64] = 1'b1;
    end else begin
        wb_divx10[64] = 1'b0;
    end

    // 11 = 8 + 3
    wb_divx11[64: 0] = ({1'h0, wb_divx8[63: 0]} + {1'h0, wb_divx3[63: 0]});
    if ((wb_divx11[64] == 1'b1) || ((|wb_divisor[123: 61]) == 1'b1)) begin
        wb_divx11[64] = 1'b1;
    end else begin
        wb_divx11[64] = 1'b0;
    end

    // 12 = 3 << 2
    wb_divx12[1: 0] = 2'h0;
    wb_divx12[63: 2] = wb_divx3[61: 0];
    wb_divx12[64] = (wb_divx3[64] || wb_divx3[63] || wb_divx3[62]);

    // 16 = divisor << 4
    wb_divx16[3: 0] = 4'h0;
    wb_divx16[63: 4] = wb_divisor[59: 0];
    if ((|wb_divisor[123: 60]) == 1'b1) begin
        wb_divx16[64] = 1'b1;
    end else begin
        wb_divx16[64] = 1'b0;
    end

    // 13 = 16 - 3
    wb_divx13[64: 0] = (wb_divx16[64: 0] - {1'h0, wb_divx3[63: 0]});
    if ((wb_divx13[64] == 1'b1) || ((|wb_divisor[123: 61]) == 1'b1)) begin
        wb_divx13[64] = 1'b1;
    end else begin
        wb_divx13[64] = 1'b0;
    end

    // 14 = 7 << 1
    wb_divx14[0] = 1'b0;
    wb_divx14[63: 1] = wb_divx7[62: 0];
    wb_divx14[64] = (wb_divx7[64] || wb_divx7[63]);

    // 15 = 16 - 1
    wb_divx15[64: 0] = (wb_divx16[64: 0] - {1'h0, wb_divx1[63: 0]});
    if ((wb_divx15[64] == 1'b1) || ((|wb_divisor[123: 61]) == 1'b1)) begin
        wb_divx15[64] = 1'b1;
    end else begin
        wb_divx15[64] = 1'b0;
    end

    wb_thresh[15] = ({1'h0, wb_divident} - {1'h0, wb_divx15});
    wb_thresh[14] = ({1'h0, wb_divident} - {1'h0, wb_divx14});
    wb_thresh[13] = ({1'h0, wb_divident} - {1'h0, wb_divx13});
    wb_thresh[12] = ({1'h0, wb_divident} - {1'h0, wb_divx12});
    wb_thresh[11] = ({1'h0, wb_divident} - {1'h0, wb_divx11});
    wb_thresh[10] = ({1'h0, wb_divident} - {1'h0, wb_divx10});
    wb_thresh[9] = ({1'h0, wb_divident} - {1'h0, wb_divx9});
    wb_thresh[8] = ({1'h0, wb_divident} - {1'h0, wb_divx8});
    wb_thresh[7] = ({1'h0, wb_divident} - {1'h0, wb_divx7});
    wb_thresh[6] = ({1'h0, wb_divident} - {1'h0, wb_divx6});
    wb_thresh[5] = ({1'h0, wb_divident} - {1'h0, wb_divx5});
    wb_thresh[4] = ({1'h0, wb_divident} - {1'h0, wb_divx4});
    wb_thresh[3] = ({1'h0, wb_divident} - {1'h0, wb_divx3});
    wb_thresh[2] = ({1'h0, wb_divident} - {1'h0, wb_divx2});
    wb_thresh[1] = ({1'h0, wb_divident} - {1'h0, wb_divx1});
    wb_thresh[0] = {1'h0, wb_divident};

    if (wb_thresh[15][65] == 1'b0) begin
        wb_bits = 4'hf;
        wb_dif = wb_thresh[15][63: 0];
    end else if (wb_thresh[14][65] == 1'b0) begin
        wb_bits = 4'he;
        wb_dif = wb_thresh[14][63: 0];
    end else if (wb_thresh[13][65] == 1'b0) begin
        wb_bits = 4'hd;
        wb_dif = wb_thresh[13][63: 0];
    end else if (wb_thresh[12][65] == 1'b0) begin
        wb_bits = 4'hc;
        wb_dif = wb_thresh[12][63: 0];
    end else if (wb_thresh[11][65] == 1'b0) begin
        wb_bits = 4'hb;
        wb_dif = wb_thresh[11][63: 0];
    end else if (wb_thresh[10][65] == 1'b0) begin
        wb_bits = 4'ha;
        wb_dif = wb_thresh[10][63: 0];
    end else if (wb_thresh[9][65] == 1'b0) begin
        wb_bits = 4'h9;
        wb_dif = wb_thresh[9][63: 0];
    end else if (wb_thresh[8][65] == 1'b0) begin
        wb_bits = 4'h8;
        wb_dif = wb_thresh[8][63: 0];
    end else if (wb_thresh[7][65] == 1'b0) begin
        wb_bits = 4'h7;
        wb_dif = wb_thresh[7][63: 0];
    end else if (wb_thresh[6][65] == 1'b0) begin
        wb_bits = 4'h6;
        wb_dif = wb_thresh[6][63: 0];
    end else if (wb_thresh[5][65] == 1'b0) begin
        wb_bits = 4'h5;
        wb_dif = wb_thresh[5][63: 0];
    end else if (wb_thresh[4][65] == 1'b0) begin
        wb_bits = 4'h4;
        wb_dif = wb_thresh[4][63: 0];
    end else if (wb_thresh[3][65] == 1'b0) begin
        wb_bits = 4'h3;
        wb_dif = wb_thresh[3][63: 0];
    end else if (wb_thresh[2][65] == 1'b0) begin
        wb_bits = 4'h2;
        wb_dif = wb_thresh[2][63: 0];
    end else if (wb_thresh[1][65] == 1'b0) begin
        wb_bits = 4'h1;
        wb_dif = wb_thresh[1][63: 0];
    end else begin
        wb_bits = 4'h0;
        wb_dif = wb_thresh[0][63: 0];
    end

    o_bits = wb_bits;
    o_resid = wb_dif;
end: comb_proc

endmodule: divstage64
