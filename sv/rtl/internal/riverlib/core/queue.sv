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

module Queue #(
    parameter bit async_reset = 1'b0,
    parameter int abits = 6,
    parameter int dbits = 128
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_re,
    input logic i_we,
    input logic [dbits-1:0] i_wdata,
    output logic [dbits-1:0] o_rdata,
    output logic o_full,
    output logic o_nempty
);

localparam int DEPTH = (2**abits);

typedef struct {
    logic [(abits + 1)-1:0] wcnt;
} Queue_registers;

const Queue_registers Queue_r_reset = '{
    7'd0                                // wcnt
};

typedef struct {
    logic [dbits-1:0] mem[0: DEPTH - 1];
} Queue_rxegisters;

Queue_registers r, rin;
Queue_rxegisters rx, rxin;


always_comb
begin: comb_proc
    Queue_registers v;
    Queue_rxegisters vx;
    logic nempty;
    logic [dbits-1:0] vb_data_o;
    logic full;
    logic show_full;

    nempty = 1'b0;
    vb_data_o = '0;
    full = 1'b0;
    show_full = 1'b0;

    v = r;
    for (int i = 0; i < DEPTH; i++) begin
        vx.mem[i] = rx.mem[i];
    end

    if (r.wcnt == DEPTH) begin
        full = 1'b1;
    end
    if (r.wcnt >= (DEPTH - 1)) begin
        show_full = 1'b1;
    end

    if ((i_re == 1'b1) && (i_we == 1'b1)) begin
        for (int i = 1; i < DEPTH; i++) begin
            vx.mem[(i - 1)] = rx.mem[i];
        end
        if ((|r.wcnt) == 1'b1) begin
            vx.mem[(int'(r.wcnt) - 1)] = i_wdata;
        end else begin
            // do nothing, it will directly pass to output
        end
    end else if ((i_re == 1'b0) && (i_we == 1'b1)) begin
        if (full == 1'b0) begin
            v.wcnt = (r.wcnt + 1);
            vx.mem[int'(r.wcnt)] = i_wdata;
        end
    end else if ((i_re == 1'b1) && (i_we == 1'b0)) begin
        if ((|r.wcnt) == 1'b1) begin
            v.wcnt = (r.wcnt - 1);
        end
        for (int i = 1; i < DEPTH; i++) begin
            vx.mem[(i - 1)] = rx.mem[i];
        end
    end

    if ((|r.wcnt) == 1'b0) begin
        vb_data_o = i_wdata;
    end else begin
        vb_data_o = rx.mem[0];
    end

    if ((i_we == 1'b1) || ((|r.wcnt) == 1'b1)) begin
        nempty = 1'b1;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = Queue_r_reset;
    end

    o_nempty = nempty;
    o_full = show_full;
    o_rdata = vb_data_o;

    rin = v;
    for (int i = 0; i < DEPTH; i++) begin
        rxin.mem[i] = vx.mem[i];
    end
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= Queue_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc

        always_ff @(posedge i_clk) begin: rxg_proc
            for (int i = 0; i < DEPTH; i++) begin
                rx.mem[i] <= rxin.mem[i];
            end
        end: rxg_proc

    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

        always_ff @(posedge i_clk) begin: rxg_proc
            for (int i = 0; i < DEPTH; i++) begin
                rx.mem[i] <= rxin.mem[i];
            end
        end: rxg_proc

    end: no_rst_gen
endgenerate

endmodule: Queue
