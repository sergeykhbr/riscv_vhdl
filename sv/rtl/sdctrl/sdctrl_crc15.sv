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

module sdctrl_crc15 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_clear,                                    // Clear CRC register;
    input logic i_next,                                     // Shift enable strob
    input logic i_dat,                                      // Input bit
    output logic [14:0] o_crc15                             // Computed value
);

import sdctrl_crc15_pkg::*;

sdctrl_crc15_registers r, rin;

always_comb
begin: comb_proc
    sdctrl_crc15_registers v;
    logic v_inv15_0;
    logic [14:0] vb_crc15_0;

    v_inv15_0 = 0;
    vb_crc15_0 = 0;

    v = r;

    // CRC15 = x^16 + x^12 + x^5 + 1
    v_inv15_0 = (r.crc15[14] ^ i_dat);
    vb_crc15_0[14] = r.crc15[13];
    vb_crc15_0[13] = r.crc15[12];
    vb_crc15_0[12] = (r.crc15[11] ^ v_inv15_0);
    vb_crc15_0[11] = r.crc15[10];
    vb_crc15_0[10] = r.crc15[9];
    vb_crc15_0[9] = r.crc15[8];
    vb_crc15_0[8] = r.crc15[7];
    vb_crc15_0[7] = r.crc15[6];
    vb_crc15_0[6] = r.crc15[5];
    vb_crc15_0[5] = (r.crc15[4] ^ v_inv15_0);
    vb_crc15_0[4] = r.crc15[3];
    vb_crc15_0[3] = r.crc15[2];
    vb_crc15_0[2] = r.crc15[1];
    vb_crc15_0[1] = r.crc15[0];
    vb_crc15_0[0] = v_inv15_0;

    if (i_clear == 1'b1) begin
        v.crc15 = '0;
    end else if (i_next == 1'b1) begin
        v.crc15 = vb_crc15_0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_crc15_r_reset;
    end

    o_crc15 = r.crc15;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_crc15_r_reset;
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

endmodule: sdctrl_crc15
