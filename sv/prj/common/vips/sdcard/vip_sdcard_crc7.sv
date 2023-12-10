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

module vip_sdcard_crc7 #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_clear,                                    // Clear CRC register;
    input logic i_next,                                     // Shift enable strob
    input logic i_dat,                                      // Input bit
    output logic [6:0] o_crc7                               // Computed value
);

import vip_sdcard_crc7_pkg::*;

vip_sdcard_crc7_registers r, rin;


always_comb
begin: comb_proc
    vip_sdcard_crc7_registers v;
    logic v_inv7;
    logic [6:0] vb_crc7;

    v_inv7 = 1'b0;
    vb_crc7 = '0;

    v = r;

    // CRC7 = x^7 + x^3 + 1
    // CMD0 -> 01 000000 0000..000000000000000000000000 1001010 1 -> 0x4A (0x95)
    // CMD17-> 01 010001 0000..000000000000000000000000 0101010 1 -> 0x2A (0x55)
    // CMD17<- 00 010001 0000..000000000010010000000000 0110011 1 -> 0x33 (0x67)
    v_inv7 = (r.crc7[6] ^ i_dat);
    vb_crc7[6] = r.crc7[5];
    vb_crc7[5] = r.crc7[4];
    vb_crc7[4] = r.crc7[3];
    vb_crc7[3] = (r.crc7[2] ^ v_inv7);
    vb_crc7[2] = r.crc7[1];
    vb_crc7[1] = r.crc7[0];
    vb_crc7[0] = v_inv7;

    if (i_clear == 1'b1) begin
        v.crc7 = 7'd0;
    end else if (i_next == 1'b1) begin
        v.crc7 = vb_crc7;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_sdcard_crc7_r_reset;
    end

    o_crc7 = vb_crc7;

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_sdcard_crc7_r_reset;
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

endmodule: vip_sdcard_crc7
