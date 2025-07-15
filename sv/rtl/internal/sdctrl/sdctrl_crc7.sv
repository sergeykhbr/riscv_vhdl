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

module sdctrl_crc7 #(
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_clear,                                    // Clear CRC register;
    input logic i_next,                                     // Shift enable strob
    input logic i_dat,                                      // Input bit
    output logic [6:0] o_crc7                               // Computed value
);

import sdctrl_crc7_pkg::*;

sdctrl_crc7_registers r;
sdctrl_crc7_registers rin;


always_comb
begin: comb_proc
    sdctrl_crc7_registers v;
    logic v_inv7;
    logic [6:0] vb_crc7;

    v = r;
    v_inv7 = 1'b0;
    vb_crc7 = '0;

    // CRC7 = x^7 + x^3 + 1 (page 116)
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

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = sdctrl_crc7_r_reset;
    end

    o_crc7 = vb_crc7;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= sdctrl_crc7_r_reset;
            end else begin
                r <= rin;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r <= rin;
        end

    end: async_r_dis
endgenerate

endmodule: sdctrl_crc7
