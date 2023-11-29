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

module sdctrl_err #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_err_valid,
    input logic [3:0] i_err_code,
    input logic i_err_clear,
    output logic [3:0] o_err_code,
    output logic o_err_pending
);

import sdctrl_cfg_pkg::*;
import sdctrl_err_pkg::*;

sdctrl_err_registers r, rin;


always_comb
begin: comb_proc
    sdctrl_err_registers v;
    v = r;

    if (i_err_clear == 1'b1) begin
        v.code = CMDERR_NONE;
    end else if ((i_err_valid == 1'b1)
                && (r.code == CMDERR_NONE)) begin
        v.code = i_err_code;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_err_r_reset;
    end

    o_err_code = r.code;
    o_err_pending = (|r.code);

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_err_r_reset;
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

endmodule: sdctrl_err
