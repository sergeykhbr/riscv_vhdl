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

module sdctrl_wdog #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_ena,
    input logic [15:0] i_period,
    output logic o_trigger
);

import sdctrl_wdog_pkg::*;

sdctrl_wdog_registers r, rin;


always_comb
begin: comb_proc
    sdctrl_wdog_registers v;
    v = r;

    v.trigger = 1'b0;
    if (i_ena == 1'b0) begin
        v.cnt = i_period;
    end else if ((|r.cnt) == 1'b1) begin
        v.cnt = (r.cnt - 1);
    end else begin
        v.trigger = 1'b1;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_wdog_r_reset;
    end

    o_trigger = r.trigger;

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_wdog_r_reset;
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

endmodule: sdctrl_wdog
