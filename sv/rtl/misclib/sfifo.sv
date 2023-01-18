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

module sfifo #(
    parameter bit async_reset = 1'b0,
    parameter int dbits = 8,                                // Data width bits
    parameter int log2_depth = 4                            // Fifo depth
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_we,
    input logic [dbits-1:0] i_wdata,
    input logic i_re,
    output logic [dbits-1:0] o_rdata,
    output logic [log2_depth:0] o_count                     // Number of words in FIFO
);

localparam int DEPTH = (2**log2_depth);

typedef struct {
    logic [dbits-1:0] databuf[0: DEPTH - 1];
    logic [log2_depth-1:0] wr_cnt;
    logic [log2_depth-1:0] rd_cnt;
    logic [log2_depth:0] total_cnt;
} sfifo_registers;

sfifo_registers r, rin;

always_comb
begin: comb_proc
    sfifo_registers v;
    logic v_full;
    logic v_empty;

    v_full = 0;
    v_empty = 0;

    for (int i = 0; i < DEPTH; i++) begin
        v.databuf[i] = r.databuf[i];
    end
    v.wr_cnt = r.wr_cnt;
    v.rd_cnt = r.rd_cnt;
    v.total_cnt = r.total_cnt;


    // Check FIFO counter:

    if ((|r.total_cnt) == 1'b0) begin
        v_empty = 1'b1;
    end

    v_full = r.total_cnt[log2_depth];

    if ((i_we == 1'b1) && ((v_full == 1'b0) || (i_re == 1'b1))) begin
        v.wr_cnt = (r.wr_cnt + 1);
        v.databuf[int'(r.wr_cnt)] = i_wdata;
        if (i_re == 1'b0) begin
            v.total_cnt = (r.total_cnt + 1);
        end
    end

    if ((i_re == 1'b1) && (v_empty == 1'b0)) begin
        v.rd_cnt = (r.rd_cnt + 1);
        if (i_we == 1'b0) begin
            v.total_cnt = (r.total_cnt - 1);
        end
    end

    if (~async_reset && i_nrst == 1'b0) begin
        for (int i = 0; i < DEPTH; i++) begin
            v.databuf[i] = 8'h00;
        end
        v.wr_cnt = 4'h0;
        v.rd_cnt = 4'h0;
        v.total_cnt = 5'h0;
    end

    o_rdata = r.databuf[int'(r.rd_cnt)];
    o_count = r.total_cnt;

    for (int i = 0; i < DEPTH; i++) begin
        rin.databuf[i] = v.databuf[i];
    end
    rin.wr_cnt = v.wr_cnt;
    rin.rd_cnt = v.rd_cnt;
    rin.total_cnt = v.total_cnt;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < DEPTH; i++) begin
                    r.databuf[i] <= 8'h00;
                end
                r.wr_cnt <= 4'h0;
                r.rd_cnt <= 4'h0;
                r.total_cnt <= 4'h0;
            end else begin
                for (int i = 0; i < DEPTH; i++) begin
                    r.databuf[i] <= rin.databuf[i];
                end
                r.wr_cnt <= rin.wr_cnt;
                r.rd_cnt <= rin.rd_cnt;
                r.total_cnt <= rin.total_cnt;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < DEPTH; i++) begin
                r.databuf[i] <= rin.databuf[i];
            end
            r.wr_cnt <= rin.wr_cnt;
            r.rd_cnt <= rin.rd_cnt;
            r.total_cnt <= rin.total_cnt;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: sfifo
