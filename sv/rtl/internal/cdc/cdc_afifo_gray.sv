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

module cdc_afifo_gray #(
    parameter int abits = 3
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_ena,
    input logic [(abits + 1)-1:0] i_q2_gray,
    output logic [abits-1:0] o_addr,
    output logic [(abits + 1)-1:0] o_gray,
    output logic o_empty,
    output logic o_full
);

logic [(abits + 1)-1:0] wb_bin_next;
logic [(abits + 1)-1:0] wb_gray_next;
logic [(abits + 1)-1:0] bin;
logic [(abits + 1)-1:0] gray;
logic empty;
logic full;


assign wb_bin_next = (bin + {'0, i_ena});
assign wb_gray_next = ({'0, wb_bin_next[(abits + 1) - 1: 1]} ^ wb_bin_next);
assign o_addr = bin[(abits - 1): 0];
assign o_gray = gray;
assign o_empty = empty;
assign o_full = full;


always_ff @(posedge i_clk, negedge i_nrst) begin: proc_ff_proc
    if (i_nrst == 1'b0) begin
        bin <= '0;
        gray <= '0;
        empty <= 1'b1;
        full <= 1'b0;
    end else begin
        bin <= wb_bin_next;
        gray <= wb_gray_next;
        empty <= (wb_gray_next == i_q2_gray);
        // Optimized version of 3 conditions:
        //     wb_gray_next[abits] != i_q2_ptr[abits]
        //     wb_gray_next[abits-1] != i_q2_ptr[abits-1]
        //     wb_gray_next[abits-2:0] == i_q2_ptr[abits-2:0]
        full <= ((wb_gray_next[abits] ^ i_q2_gray[abits])
                & (wb_gray_next[(abits - 1)] ^ i_q2_gray[(abits - 1)])
                & (wb_gray_next[(abits - 2): 0] == i_q2_gray[(abits - 2): 0]));
    end
end: proc_ff_proc

endmodule: cdc_afifo_gray
