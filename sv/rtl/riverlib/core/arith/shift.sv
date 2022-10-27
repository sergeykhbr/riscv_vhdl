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

module Shifter #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [3:0] i_mode,                               // operation type: [0]0=rv64;1=rv32;[1]=sll;[2]=srl;[3]=sra
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a1,       // Operand 1
    input logic [5:0] i_a2,                                 // Operand 2
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_res      // Result
);

import river_cfg_pkg::*;
import shift_pkg::*;

Shifter_registers r, rin;

always_comb
begin: comb_proc
    Shifter_registers v;
    logic [63:0] wb_sll;
    logic [63:0] wb_sllw;
    logic [63:0] wb_srl;
    logic [63:0] wb_sra;
    logic [63:0] wb_srlw;
    logic [63:0] wb_sraw;
    logic [63:0] v64;
    logic [31:0] v32;
    logic [63:0] msk64;
    logic [63:0] msk32;

    wb_sll = 0;
    wb_sllw = 0;
    wb_srl = 0;
    wb_sra = 0;
    wb_srlw = 0;
    wb_sraw = 0;
    v64 = 0;
    v32 = 0;
    msk64 = 0;
    msk32 = 0;

    v = r;

    v64 = i_a1;
    v32 = i_a1[31: 0];

    if (i_a1[63] == 1'b1) begin
        msk64 = '1;
    end else begin
        msk64 = '0;
    end
    if (i_a1[31] == 1'b1) begin
        msk32 = '1;
    end else begin
        msk32 = '0;
    end

    case (i_a2)
    6'h00: begin
        wb_sll = v64;
        wb_srl = v64;
        wb_sra = v64;
    end
    6'h01: begin
        wb_sll = {v64, 1'h0};
        wb_srl = v64[63: 1];
        wb_sra = {msk64[63: 63], v64[63: 1]};
    end
    6'h02: begin
        wb_sll = {v64, 2'h0};
        wb_srl = v64[63: 2];
        wb_sra = {msk64[63: 62], v64[63: 2]};
    end
    6'h03: begin
        wb_sll = {v64, 3'h0};
        wb_srl = v64[63: 3];
        wb_sra = {msk64[63: 61], v64[63: 3]};
    end
    6'h04: begin
        wb_sll = {v64, 4'h0};
        wb_srl = v64[63: 4];
        wb_sra = {msk64[63: 60], v64[63: 4]};
    end
    6'h05: begin
        wb_sll = {v64, 5'h00};
        wb_srl = v64[63: 5];
        wb_sra = {msk64[63: 59], v64[63: 5]};
    end
    6'h06: begin
        wb_sll = {v64, 6'h00};
        wb_srl = v64[63: 6];
        wb_sra = {msk64[63: 58], v64[63: 6]};
    end
    6'h07: begin
        wb_sll = {v64, 7'h00};
        wb_srl = v64[63: 7];
        wb_sra = {msk64[63: 57], v64[63: 7]};
    end
    6'h08: begin
        wb_sll = {v64, 8'h00};
        wb_srl = v64[63: 8];
        wb_sra = {msk64[63: 56], v64[63: 8]};
    end
    6'h09: begin
        wb_sll = {v64, 9'h000};
        wb_srl = v64[63: 9];
        wb_sra = {msk64[63: 55], v64[63: 9]};
    end
    6'h0a: begin
        wb_sll = {v64, 10'h000};
        wb_srl = v64[63: 10];
        wb_sra = {msk64[63: 54], v64[63: 10]};
    end
    6'h0b: begin
        wb_sll = {v64, 11'h000};
        wb_srl = v64[63: 11];
        wb_sra = {msk64[63: 53], v64[63: 11]};
    end
    6'h0c: begin
        wb_sll = {v64, 12'h000};
        wb_srl = v64[63: 12];
        wb_sra = {msk64[63: 52], v64[63: 12]};
    end
    6'h0d: begin
        wb_sll = {v64, 13'h0000};
        wb_srl = v64[63: 13];
        wb_sra = {msk64[63: 51], v64[63: 13]};
    end
    6'h0e: begin
        wb_sll = {v64, 14'h0000};
        wb_srl = v64[63: 14];
        wb_sra = {msk64[63: 50], v64[63: 14]};
    end
    6'h0f: begin
        wb_sll = {v64, 15'h0000};
        wb_srl = v64[63: 15];
        wb_sra = {msk64[63: 49], v64[63: 15]};
    end
    6'h10: begin
        wb_sll = {v64, 16'h0000};
        wb_srl = v64[63: 16];
        wb_sra = {msk64[63: 48], v64[63: 16]};
    end
    6'h11: begin
        wb_sll = {v64, 17'h00000};
        wb_srl = v64[63: 17];
        wb_sra = {msk64[63: 47], v64[63: 17]};
    end
    6'h12: begin
        wb_sll = {v64, 18'h00000};
        wb_srl = v64[63: 18];
        wb_sra = {msk64[63: 46], v64[63: 18]};
    end
    6'h13: begin
        wb_sll = {v64, 19'h00000};
        wb_srl = v64[63: 19];
        wb_sra = {msk64[63: 45], v64[63: 19]};
    end
    6'h14: begin
        wb_sll = {v64, 20'h00000};
        wb_srl = v64[63: 20];
        wb_sra = {msk64[63: 44], v64[63: 20]};
    end
    6'h15: begin
        wb_sll = {v64, 21'h000000};
        wb_srl = v64[63: 21];
        wb_sra = {msk64[63: 43], v64[63: 21]};
    end
    6'h16: begin
        wb_sll = {v64, 22'h000000};
        wb_srl = v64[63: 22];
        wb_sra = {msk64[63: 42], v64[63: 22]};
    end
    6'h17: begin
        wb_sll = {v64, 23'h000000};
        wb_srl = v64[63: 23];
        wb_sra = {msk64[63: 41], v64[63: 23]};
    end
    6'h18: begin
        wb_sll = {v64, 24'h000000};
        wb_srl = v64[63: 24];
        wb_sra = {msk64[63: 40], v64[63: 24]};
    end
    6'h19: begin
        wb_sll = {v64, 25'h0000000};
        wb_srl = v64[63: 25];
        wb_sra = {msk64[63: 39], v64[63: 25]};
    end
    6'h1a: begin
        wb_sll = {v64, 26'h0000000};
        wb_srl = v64[63: 26];
        wb_sra = {msk64[63: 38], v64[63: 26]};
    end
    6'h1b: begin
        wb_sll = {v64, 27'h0000000};
        wb_srl = v64[63: 27];
        wb_sra = {msk64[63: 37], v64[63: 27]};
    end
    6'h1c: begin
        wb_sll = {v64, 28'h0000000};
        wb_srl = v64[63: 28];
        wb_sra = {msk64[63: 36], v64[63: 28]};
    end
    6'h1d: begin
        wb_sll = {v64, 29'h00000000};
        wb_srl = v64[63: 29];
        wb_sra = {msk64[63: 35], v64[63: 29]};
    end
    6'h1e: begin
        wb_sll = {v64, 30'h00000000};
        wb_srl = v64[63: 30];
        wb_sra = {msk64[63: 34], v64[63: 30]};
    end
    6'h1f: begin
        wb_sll = {v64, 31'h00000000};
        wb_srl = v64[63: 31];
        wb_sra = {msk64[63: 33], v64[63: 31]};
    end
    6'h20: begin
        wb_sll = {v64, 32'h00000000};
        wb_srl = v64[63: 32];
        wb_sra = {msk64[63: 32], v64[63: 32]};
    end
    6'h21: begin
        wb_sll = {v64, 33'h000000000};
        wb_srl = v64[63: 33];
        wb_sra = {msk64[63: 31], v64[63: 33]};
    end
    6'h22: begin
        wb_sll = {v64, 34'h000000000};
        wb_srl = v64[63: 34];
        wb_sra = {msk64[63: 30], v64[63: 34]};
    end
    6'h23: begin
        wb_sll = {v64, 35'h000000000};
        wb_srl = v64[63: 35];
        wb_sra = {msk64[63: 29], v64[63: 35]};
    end
    6'h24: begin
        wb_sll = {v64, 36'h000000000};
        wb_srl = v64[63: 36];
        wb_sra = {msk64[63: 28], v64[63: 36]};
    end
    6'h25: begin
        wb_sll = {v64, 37'h0000000000};
        wb_srl = v64[63: 37];
        wb_sra = {msk64[63: 27], v64[63: 37]};
    end
    6'h26: begin
        wb_sll = {v64, 38'h0000000000};
        wb_srl = v64[63: 38];
        wb_sra = {msk64[63: 26], v64[63: 38]};
    end
    6'h27: begin
        wb_sll = {v64, 39'h0000000000};
        wb_srl = v64[63: 39];
        wb_sra = {msk64[63: 25], v64[63: 39]};
    end
    6'h28: begin
        wb_sll = {v64, 40'h0000000000};
        wb_srl = v64[63: 40];
        wb_sra = {msk64[63: 24], v64[63: 40]};
    end
    6'h29: begin
        wb_sll = {v64, 41'h00000000000};
        wb_srl = v64[63: 41];
        wb_sra = {msk64[63: 23], v64[63: 41]};
    end
    6'h2a: begin
        wb_sll = {v64, 42'h00000000000};
        wb_srl = v64[63: 42];
        wb_sra = {msk64[63: 22], v64[63: 42]};
    end
    6'h2b: begin
        wb_sll = {v64, 43'h00000000000};
        wb_srl = v64[63: 43];
        wb_sra = {msk64[63: 21], v64[63: 43]};
    end
    6'h2c: begin
        wb_sll = {v64, 44'h00000000000};
        wb_srl = v64[63: 44];
        wb_sra = {msk64[63: 20], v64[63: 44]};
    end
    6'h2d: begin
        wb_sll = {v64, 45'h000000000000};
        wb_srl = v64[63: 45];
        wb_sra = {msk64[63: 19], v64[63: 45]};
    end
    6'h2e: begin
        wb_sll = {v64, 46'h000000000000};
        wb_srl = v64[63: 46];
        wb_sra = {msk64[63: 18], v64[63: 46]};
    end
    6'h2f: begin
        wb_sll = {v64, 47'h000000000000};
        wb_srl = v64[63: 47];
        wb_sra = {msk64[63: 17], v64[63: 47]};
    end
    6'h30: begin
        wb_sll = {v64, 48'h000000000000};
        wb_srl = v64[63: 48];
        wb_sra = {msk64[63: 16], v64[63: 48]};
    end
    6'h31: begin
        wb_sll = {v64, 49'h0000000000000};
        wb_srl = v64[63: 49];
        wb_sra = {msk64[63: 15], v64[63: 49]};
    end
    6'h32: begin
        wb_sll = {v64, 50'h0000000000000};
        wb_srl = v64[63: 50];
        wb_sra = {msk64[63: 14], v64[63: 50]};
    end
    6'h33: begin
        wb_sll = {v64, 51'h0000000000000};
        wb_srl = v64[63: 51];
        wb_sra = {msk64[63: 13], v64[63: 51]};
    end
    6'h34: begin
        wb_sll = {v64, 52'h0000000000000};
        wb_srl = v64[63: 52];
        wb_sra = {msk64[63: 12], v64[63: 52]};
    end
    6'h35: begin
        wb_sll = {v64, 53'h00000000000000};
        wb_srl = v64[63: 53];
        wb_sra = {msk64[63: 11], v64[63: 53]};
    end
    6'h36: begin
        wb_sll = {v64, 54'h00000000000000};
        wb_srl = v64[63: 54];
        wb_sra = {msk64[63: 10], v64[63: 54]};
    end
    6'h37: begin
        wb_sll = {v64, 55'h00000000000000};
        wb_srl = v64[63: 55];
        wb_sra = {msk64[63: 9], v64[63: 55]};
    end
    6'h38: begin
        wb_sll = {v64, 56'h00000000000000};
        wb_srl = v64[63: 56];
        wb_sra = {msk64[63: 8], v64[63: 56]};
    end
    6'h39: begin
        wb_sll = {v64, 57'h000000000000000};
        wb_srl = v64[63: 57];
        wb_sra = {msk64[63: 7], v64[63: 57]};
    end
    6'h3a: begin
        wb_sll = {v64, 58'h000000000000000};
        wb_srl = v64[63: 58];
        wb_sra = {msk64[63: 6], v64[63: 58]};
    end
    6'h3b: begin
        wb_sll = {v64, 59'h000000000000000};
        wb_srl = v64[63: 59];
        wb_sra = {msk64[63: 5], v64[63: 59]};
    end
    6'h3c: begin
        wb_sll = {v64, 60'h000000000000000};
        wb_srl = v64[63: 60];
        wb_sra = {msk64[63: 4], v64[63: 60]};
    end
    6'h3d: begin
        wb_sll = {v64, 61'h0000000000000000};
        wb_srl = v64[63: 61];
        wb_sra = {msk64[63: 3], v64[63: 61]};
    end
    6'h3e: begin
        wb_sll = {v64, 62'h0000000000000000};
        wb_srl = v64[63: 62];
        wb_sra = {msk64[63: 2], v64[63: 62]};
    end
    6'h3f: begin
        wb_sll = {v64, 63'h0000000000000000};
        wb_srl = v64[63: 63];
        wb_sra = {msk64[63: 1], v64[63: 63]};
    end
    default: begin
    end
    endcase

    case (i_a2[4: 0])
    5'h00: begin
        wb_sllw = v32;
        wb_srlw = v32;
        wb_sraw = {msk32[63: 32], v32};
    end
    5'h01: begin
        wb_sllw = {v32, 1'h0};
        wb_srlw = v32[31: 1];
        wb_sraw = {msk32[63: 31], v32[31: 1]};
    end
    5'h02: begin
        wb_sllw = {v32, 2'h0};
        wb_srlw = v32[31: 2];
        wb_sraw = {msk32[63: 30], v32[31: 2]};
    end
    5'h03: begin
        wb_sllw = {v32, 3'h0};
        wb_srlw = v32[31: 3];
        wb_sraw = {msk32[63: 29], v32[31: 3]};
    end
    5'h04: begin
        wb_sllw = {v32, 4'h0};
        wb_srlw = v32[31: 4];
        wb_sraw = {msk32[63: 28], v32[31: 4]};
    end
    5'h05: begin
        wb_sllw = {v32, 5'h00};
        wb_srlw = v32[31: 5];
        wb_sraw = {msk32[63: 27], v32[31: 5]};
    end
    5'h06: begin
        wb_sllw = {v32, 6'h00};
        wb_srlw = v32[31: 6];
        wb_sraw = {msk32[63: 26], v32[31: 6]};
    end
    5'h07: begin
        wb_sllw = {v32, 7'h00};
        wb_srlw = v32[31: 7];
        wb_sraw = {msk32[63: 25], v32[31: 7]};
    end
    5'h08: begin
        wb_sllw = {v32, 8'h00};
        wb_srlw = v32[31: 8];
        wb_sraw = {msk32[63: 24], v32[31: 8]};
    end
    5'h09: begin
        wb_sllw = {v32, 9'h000};
        wb_srlw = v32[31: 9];
        wb_sraw = {msk32[63: 23], v32[31: 9]};
    end
    5'h0a: begin
        wb_sllw = {v32, 10'h000};
        wb_srlw = v32[31: 10];
        wb_sraw = {msk32[63: 22], v32[31: 10]};
    end
    5'h0b: begin
        wb_sllw = {v32, 11'h000};
        wb_srlw = v32[31: 11];
        wb_sraw = {msk32[63: 21], v32[31: 11]};
    end
    5'h0c: begin
        wb_sllw = {v32, 12'h000};
        wb_srlw = v32[31: 12];
        wb_sraw = {msk32[63: 20], v32[31: 12]};
    end
    5'h0d: begin
        wb_sllw = {v32, 13'h0000};
        wb_srlw = v32[31: 13];
        wb_sraw = {msk32[63: 19], v32[31: 13]};
    end
    5'h0e: begin
        wb_sllw = {v32, 14'h0000};
        wb_srlw = v32[31: 14];
        wb_sraw = {msk32[63: 18], v32[31: 14]};
    end
    5'h0f: begin
        wb_sllw = {v32, 15'h0000};
        wb_srlw = v32[31: 15];
        wb_sraw = {msk32[63: 17], v32[31: 15]};
    end
    5'h10: begin
        wb_sllw = {v32, 16'h0000};
        wb_srlw = v32[31: 16];
        wb_sraw = {msk32[63: 16], v32[31: 16]};
    end
    5'h11: begin
        wb_sllw = {v32, 17'h00000};
        wb_srlw = v32[31: 17];
        wb_sraw = {msk32[63: 15], v32[31: 17]};
    end
    5'h12: begin
        wb_sllw = {v32, 18'h00000};
        wb_srlw = v32[31: 18];
        wb_sraw = {msk32[63: 14], v32[31: 18]};
    end
    5'h13: begin
        wb_sllw = {v32, 19'h00000};
        wb_srlw = v32[31: 19];
        wb_sraw = {msk32[63: 13], v32[31: 19]};
    end
    5'h14: begin
        wb_sllw = {v32, 20'h00000};
        wb_srlw = v32[31: 20];
        wb_sraw = {msk32[63: 12], v32[31: 20]};
    end
    5'h15: begin
        wb_sllw = {v32, 21'h000000};
        wb_srlw = v32[31: 21];
        wb_sraw = {msk32[63: 11], v32[31: 21]};
    end
    5'h16: begin
        wb_sllw = {v32, 22'h000000};
        wb_srlw = v32[31: 22];
        wb_sraw = {msk32[63: 10], v32[31: 22]};
    end
    5'h17: begin
        wb_sllw = {v32, 23'h000000};
        wb_srlw = v32[31: 23];
        wb_sraw = {msk32[63: 9], v32[31: 23]};
    end
    5'h18: begin
        wb_sllw = {v32, 24'h000000};
        wb_srlw = v32[31: 24];
        wb_sraw = {msk32[63: 8], v32[31: 24]};
    end
    5'h19: begin
        wb_sllw = {v32, 25'h0000000};
        wb_srlw = v32[31: 25];
        wb_sraw = {msk32[63: 7], v32[31: 25]};
    end
    5'h1a: begin
        wb_sllw = {v32, 26'h0000000};
        wb_srlw = v32[31: 26];
        wb_sraw = {msk32[63: 6], v32[31: 26]};
    end
    5'h1b: begin
        wb_sllw = {v32, 27'h0000000};
        wb_srlw = v32[31: 27];
        wb_sraw = {msk32[63: 5], v32[31: 27]};
    end
    5'h1c: begin
        wb_sllw = {v32, 28'h0000000};
        wb_srlw = v32[31: 28];
        wb_sraw = {msk32[63: 4], v32[31: 28]};
    end
    5'h1d: begin
        wb_sllw = {v32, 29'h00000000};
        wb_srlw = v32[31: 29];
        wb_sraw = {msk32[63: 3], v32[31: 29]};
    end
    5'h1e: begin
        wb_sllw = {v32, 30'h00000000};
        wb_srlw = v32[31: 30];
        wb_sraw = {msk32[63: 2], v32[31: 30]};
    end
    5'h1f: begin
        wb_sllw = {v32, 31'h00000000};
        wb_srlw = v32[31: 31];
        wb_sraw = {msk32[63: 1], v32[31: 31]};
    end
    default: begin
    end
    endcase

    if (wb_sllw[31] == 1'b1) begin
        wb_sllw[63: 32] = '1;
    end else begin
        wb_sllw[63: 32] = '0;
    end

    if (wb_srlw[31] == 1'b1) begin
        // when shift right == 0 and a1[31] = 1
        wb_srlw[63: 32] = '1;
    end

    if (i_mode[0] == 1'b1) begin
        if (i_mode[1] == 1'b1) begin
            v.res = wb_sllw;
        end else if (i_mode[2] == 1'b1) begin
            v.res = wb_srlw;
        end else begin
            v.res = wb_sraw;
        end
    end else begin
        if (i_mode[1] == 1'b1) begin
            v.res = wb_sll;
        end else if (i_mode[2] == 1'b1) begin
            v.res = wb_srl;
        end else begin
            v.res = wb_sra;
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = Shifter_r_reset;
    end

    o_res = r.res;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= Shifter_r_reset;
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

endmodule: Shifter
