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

module AluLogic #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [2:0] i_mode,                               // operation type: [0]AND;[1]=OR;[2]XOR
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a1,       // Operand 1
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_a2,       // Operand 2
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_res      // Result
);

import river_cfg_pkg::*;
import alu_logic_pkg::*;

AluLogic_registers r, rin;

always_comb
begin: comb_proc
    AluLogic_registers v;
    v = r;

    if (i_mode[1] == 1'b1) begin
        v.res = (i_a1 | i_a2);
    end else if (i_mode[2] == 1'b1) begin
        v.res = (i_a1 ^ i_a2);
    end else begin
        v.res = (i_a1 & i_a2);
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = AluLogic_r_reset;
    end

    o_res = r.res;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= AluLogic_r_reset;
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

endmodule: AluLogic
