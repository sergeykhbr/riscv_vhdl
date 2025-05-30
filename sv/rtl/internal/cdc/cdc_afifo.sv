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

module cdc_afifo #(
    parameter int abits = 3,                                // fifo log2(depth)
    parameter int dbits = 65                                // payload width
)
(
    input logic i_nrst,                                     // reset active LOW
    input logic i_wclk,                                     // clock write
    input logic i_wr,                                       // write enable strob
    input logic [dbits-1:0] i_wdata,                        // write data
    output logic o_wready,                                  // ready to accept (fifo is not full) in wclk domain
    input logic i_rclk,                                     // read clock
    input logic i_rd,                                       // read enable strob
    output logic [dbits-1:0] o_rdata,                       // fifo payload read
    output logic o_rvalid                                   // new valid data (fifo is not empty) in rclk domain
);

logic w_wr_ena;
logic [abits-1:0] wb_wgray_addr;
logic [(abits + 1)-1:0] wgray;
logic [(abits + 1)-1:0] q1_wgray;
logic [(abits + 1)-1:0] q2_wgray;
logic w_wgray_full;
logic w_wgray_empty_unused;
logic w_rd_ena;
logic [abits-1:0] wb_rgray_addr;
logic [(abits + 1)-1:0] rgray;
logic [(abits + 1)-1:0] q1_rgray;
logic [(abits + 1)-1:0] q2_rgray;
logic w_rgray_full_unused;
logic w_rgray_empty;

cdc_afifo_gray #(
    .abits(abits)
) wgray0 (
    .i_nrst(i_nrst),
    .i_clk(i_wclk),
    .i_ena(w_wr_ena),
    .i_q2_gray(q2_rgray),
    .o_addr(wb_wgray_addr),
    .o_gray(wgray),
    .o_empty(w_wgray_empty_unused),
    .o_full(w_wgray_full)
);

cdc_afifo_gray #(
    .abits(abits)
) rgray0 (
    .i_nrst(i_nrst),
    .i_clk(i_rclk),
    .i_ena(w_rd_ena),
    .i_q2_gray(q2_wgray),
    .o_addr(wb_rgray_addr),
    .o_gray(rgray),
    .o_empty(w_rgray_empty),
    .o_full(w_rgray_full_unused)
);

cdc_dp_mem #(
    .abits(abits),
    .dbits(dbits)
) mem0 (
    .i_wclk(i_wclk),
    .i_wena(w_wr_ena),
    .i_addr(wb_wgray_addr),
    .i_wdata(i_wdata),
    .i_rclk(i_rclk),
    .i_raddr(wb_rgray_addr),
    .o_rdata(o_rdata)
);

assign w_wr_ena = (i_wr & (~w_wgray_full));
assign w_rd_ena = (i_rd & (~w_rgray_empty));
assign o_wready = (~w_wgray_full);
assign o_rvalid = (~w_rgray_empty);


always_ff @(posedge i_wclk, negedge i_nrst) begin: proc_wff_proc
    if (i_nrst == 1'b0) begin
        q1_wgray <= '0;
        q2_wgray <= '0;
    end else begin
        q1_wgray <= wgray;
        q2_wgray <= q1_wgray;
    end
end: proc_wff_proc


always_ff @(posedge i_rclk, negedge i_nrst) begin: proc_rff_proc
    if (i_nrst == 1'b0) begin
        q1_rgray <= '0;
        q2_rgray <= '0;
    end else begin
        q1_rgray <= rgray;
        q2_rgray <= q1_rgray;
    end
end: proc_rff_proc

endmodule: cdc_afifo
