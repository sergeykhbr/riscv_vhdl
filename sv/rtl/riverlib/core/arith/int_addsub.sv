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

module IntAddSub #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [6:0] i_mode,                               // [0]0=rv64;1=rv32;[1]0=sign;1=unsign[2]Add[3]Sub[4]less[5]min[6]max
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a1,       // Operand 1
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a2,       // Operand 2
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_res      // Result
);

import river_cfg_pkg::*;
import int_addsub_pkg::*;

IntAddSub_registers r, rin;

always_comb
begin: comb_proc
    IntAddSub_registers v;
    logic [RISCV_ARCH-1:0] vb_rdata1;
    logic [RISCV_ARCH-1:0] vb_rdata2;
    logic [RISCV_ARCH-1:0] vb_add;
    logic [RISCV_ARCH-1:0] vb_sub;
    logic [RISCV_ARCH-1:0] vb_res;

    vb_rdata1 = 0;
    vb_rdata2 = 0;
    vb_add = 0;
    vb_sub = 0;
    vb_res = 0;

    v = r;

    // To support 32-bits instruction transform 32-bits operands to 64 bits
    if (i_mode[0] == 1'b1) begin
        vb_rdata1[31: 0] = i_a1[31: 0];
        vb_rdata2[31: 0] = i_a2[31: 0];
        if (vb_rdata1[31] == 1'b1) begin
            vb_rdata1[63: 32] = '1;
        end
        if (vb_rdata2[31] == 1'b1) begin
            vb_rdata2[63: 32] = '1;
        end
    end else begin
        vb_rdata1 = i_a1;
        vb_rdata2 = i_a2;
    end

    vb_add = (vb_rdata1 + vb_rdata2);
    vb_sub = (vb_rdata1 - vb_rdata2);
    if (i_mode[2] == 1'b1) begin
        vb_res = vb_add;
    end else if (i_mode[3] == 1'b1) begin
        vb_res = vb_sub;
    end else if (i_mode[4] == 1'b1) begin
        if (i_mode[1] == 1'b1) begin
            // unsigned less
            if (vb_rdata1 < vb_rdata2) begin
                vb_res[0] = 1'b1;
            end
        end else begin
            // signed less
            vb_res[0] = vb_sub[63];
        end
    end else if (i_mode[5] == 1'b1) begin
        if (i_mode[1] == 1'b1) begin
            // unsigned min
            if (vb_rdata1 < vb_rdata2) begin
                vb_res = vb_rdata1;
            end else begin
                vb_res = vb_rdata2;
            end
        end else begin
            // signed min
            if (vb_sub[63] == 1'b1) begin
                vb_res = vb_rdata1;
            end else begin
                vb_res = vb_rdata2;
            end
        end
    end else if (i_mode[6] == 1'b1) begin
        if (i_mode[1] == 1'b1) begin
            // unsigned max
            if (vb_rdata1 < vb_rdata2) begin
                vb_res = vb_rdata2;
            end else begin
                vb_res = vb_rdata1;
            end
        end else begin
            // signed max
            if (vb_sub[63] == 1'b1) begin
                vb_res = vb_rdata2;
            end else begin
                vb_res = vb_rdata1;
            end
        end
    end
    if (i_mode[0] == 1'b1) begin
        if (vb_res[31] == 1'b1) begin
            vb_res[63: 32] = '1;
        end else begin
            vb_res[63: 32] = '0;
        end
    end

    v.res = vb_res;

    if (~async_reset && i_nrst == 1'b0) begin
        v = IntAddSub_r_reset;
    end

    o_res = r.res;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= IntAddSub_r_reset;
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

endmodule: IntAddSub
