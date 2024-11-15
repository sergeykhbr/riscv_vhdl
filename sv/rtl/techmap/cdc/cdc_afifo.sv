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
    parameter int dbits = 32                                // payload width
)
(
    input logic i_wclk,                                     // clock write
    input logic i_wrstn,                                    // write reset active LOW
    input logic i_wr,                                       // write enable strob
    input logic [dbits-1:0] i_wdata,                        // write data
    output logic o_wfull,                                   // fifo is full in wclk domain
    input logic i_rclk,                                     // read clock
    input logic i_rrstn,                                    // read reset active LOW
    input logic i_rd,                                       // read enable strob
    output logic [dbits-1:0] o_rdata,                       // fifo payload read
    output logic o_rempty                                   // fifo is empty it rclk domain
);

localparam int DEPTH = (2**abits);

typedef struct {
    logic [(abits + 1)-1:0] wgray;
    logic [(abits + 1)-1:0] wbin;
    logic [(abits + 1)-1:0] wq2_rgray;
    logic [(abits + 1)-1:0] wq1_rgray;
    logic wfull;
} cdc_afifo_registers;

const cdc_afifo_registers cdc_afifo_r_reset = '{
    4'd0,                               // wgray
    4'd0,                               // wbin
    4'd0,                               // wq2_rgray
    4'd0,                               // wq1_rgray
    1'b0                                // wfull
};

typedef struct {
    logic [(abits + 1)-1:0] rgray;
    logic [(abits + 1)-1:0] rbin;
    logic [(abits + 1)-1:0] rq2_wgray;
    logic [(abits + 1)-1:0] rq1_wgray;
    logic rempty;
} cdc_afifo_r2egisters;

const cdc_afifo_r2egisters cdc_afifo_r2_reset = '{
    4'd0,                               // rgray
    4'd0,                               // rbin
    4'd0,                               // rq2_wgray
    4'd0,                               // rq1_wgray
    1'b1                                // rempty
};

typedef struct {
    logic [dbits-1:0] mem[0: DEPTH - 1];
} cdc_afifo_rx2egisters;

cdc_afifo_registers r, rin;
cdc_afifo_r2egisters r2, r2in;
cdc_afifo_rx2egisters rx2, rx2in;


always_comb
begin: comb_proc
    cdc_afifo_registers v;
    cdc_afifo_r2egisters v2;
    cdc_afifo_rx2egisters vx2;
    logic [abits-1:0] vb_waddr;
    logic [abits-1:0] vb_raddr;
    logic v_wfull_next;
    logic v_rempty_next;
    logic [(abits + 1)-1:0] vb_wgraynext;
    logic [(abits + 1)-1:0] vb_wbinnext;
    logic [(abits + 1)-1:0] vb_rgraynext;
    logic [(abits + 1)-1:0] vb_rbinnext;

    vb_waddr = 3'd0;
    vb_raddr = 3'd0;
    v_wfull_next = 1'b0;
    v_rempty_next = 1'b0;
    vb_wgraynext = 4'd0;
    vb_wbinnext = 4'd0;
    vb_rgraynext = 4'd0;
    vb_rbinnext = 4'd0;

    v = r;
    v2 = r2;
    for (int i = 0; i < DEPTH; i++) begin
        vx2.mem[i] = rx2.mem[i];
    end

    // Cross the Gray pointer to write clock domain:
    v.wq1_rgray = r2.rgray;
    v.wq2_rgray = r.wq1_rgray;

    // Next write address and Gray write pointer
    vb_wbinnext = (r.wbin + {3'd0, (i_wr && (~r.wfull))});
    vb_wgraynext = ({'0, vb_wbinnext[(abits + 1) - 1: 1]} ^ vb_wbinnext);
    vb_waddr = r.wbin[(abits - 1): 0];
    v.wgray = vb_wgraynext;
    v.wbin = vb_wbinnext;

    if (vb_wgraynext == {(~r.wq2_rgray[abits: (abits - 1)]), r.wq2_rgray[(abits - 2): 0]}) begin
        v_wfull_next = 1'b1;
    end
    v.wfull = v_wfull_next;

    if ((i_wr && (~r.wfull)) == 1'b1) begin
        vx2.mem[int'(vb_waddr)] = i_wdata;
    end

    // Write Gray pointer into read clock domain
    v2.rq1_wgray = r.wgray;
    v2.rq2_wgray = r2.rq1_wgray;
    vb_rbinnext = (r2.rbin + {3'd0, (i_rd && (~r2.rempty))});
    vb_rgraynext = ({'0, vb_rbinnext[(abits + 1) - 1: 1]} ^ vb_rbinnext);
    v2.rgray = vb_rgraynext;
    v2.rbin = vb_rbinnext;
    vb_raddr = r2.rbin[(abits - 1): 0];

    if (vb_rgraynext == r2.rq2_wgray) begin
        v_rempty_next = 1'b1;
    end
    v2.rempty = v_rempty_next;

    o_wfull = r.wfull;
    o_rempty = r2.rempty;
    o_rdata = rx2.mem[int'(vb_raddr)];

    rin = v;
    r2in = v2;
    for (int i = 0; i < DEPTH; i++) begin
        rx2in.mem[i] = vx2.mem[i];
    end
end: comb_proc

always_ff @(posedge i_wclk, negedge i_wrstn) begin: rg_proc
    if (i_wrstn == 1'b0) begin
        r <= cdc_afifo_r_reset;
    end else begin
        r <= rin;
    end
end: rg_proc

always_ff @(posedge i_rclk, negedge i_rrstn) begin: r2g_proc
    if (i_rrstn == 1'b0) begin
        r2 <= cdc_afifo_r2_reset;
    end else begin
        r2 <= r2in;
    end
end: r2g_proc

always_ff @(posedge i_wclk) begin: rx2g_proc
    for (int i = 0; i < DEPTH; i++) begin
        rx2.mem[i] <= rx2in.mem[i];
    end
end: rx2g_proc

endmodule: cdc_afifo
