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

module vip_spi_transmitter #(
    parameter bit async_reset = 1'b0,
    parameter int scaler = 8
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_csn,
    input logic i_sclk,
    input logic i_mosi,
    output logic o_miso,
    output logic o_req_valid,
    output logic o_req_write,
    output logic [31:0] o_req_addr,
    output logic [31:0] o_req_wdata,
    input logic i_req_ready,
    input logic i_resp_valid,
    input logic [31:0] i_resp_rdata,
    output logic o_resp_ready
);

import vip_spi_transmitter_pkg::*;

localparam int scaler_max = ((2 * scaler) - 1);
localparam int scaler_mid = scaler;

vip_spi_transmitter_registers r, rin;


always_comb
begin: comb_proc
    vip_spi_transmitter_registers v;
    logic v_pos;
    logic v_neg;
    logic v_resp_ready;

    v_pos = 1'b0;
    v_neg = 1'b0;
    v_resp_ready = 1'b0;

    v = r;

    v.byterdy = 1'b0;
    if ((r.req_valid == 1'b1) && (i_req_ready == 1'b1)) begin
        v.req_valid = 1'b0;
    end

    v.sclk = i_sclk;
    v_pos = ((~r.sclk) && i_sclk);
    v_neg = (r.sclk && (~i_sclk));

    if ((i_csn == 1'b0) && (v_pos == 1'b1)) begin
        v.rxshift = {r.rxshift[31: 0], i_mosi};
        v.bitcnt = (r.bitcnt + 1);
        if (r.bitcnt == 4'h7) begin
            v.byterdy = 1'b1;
        end
    end else if (i_csn == 1'b1) begin
        v.bitcnt = 4'd0;
    end

    if ((i_csn == 1'b0) && (v_neg == 1'b1)) begin
        v_resp_ready = 1'b1;
        if (i_resp_valid == 1'b1) begin
            // There's one negedge before CSn goes high:
            v.txshift = i_resp_rdata;
        end else begin
            v.txshift = {r.txshift[31: 0], 1'b1};
        end
    end

    if (r.byterdy == 1'b1) begin
        case (r.state)
        state_cmd: begin
            v.bytecnt = 3'd0;
            if (r.rxshift[7: 0] == 8'h41) begin
                v.state = state_addr;
                v.req_write = 1'b0;                         // Read request
            end else if (r.rxshift[7: 0] == 8'h42) begin
                v.state = state_addr;
                v.req_write = 1'b1;                         // Write request
            end
        end
        state_addr: begin
            v.bytecnt = (r.bytecnt + 1);
            if (r.bytecnt == 3'd3) begin
                v.bytecnt = 3'd0;
                v.state = state_data;
                v.req_addr = r.rxshift;
                v.req_valid = (~r.req_write);
            end
        end
        state_data: begin
            v.bytecnt = (r.bytecnt + 1);
            if (r.bytecnt == 3'd3) begin
                v.bytecnt = 3'd0;
                v.state = state_cmd;
                v.req_wdata = r.rxshift;
                v.req_valid = r.req_write;
            end
        end
        default: begin
            v.state = state_cmd;
        end
        endcase
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_spi_transmitter_r_reset;
    end

    o_req_valid = r.req_valid;
    o_req_write = r.req_write;
    o_req_addr = r.req_addr;
    o_req_wdata = r.req_wdata;
    o_resp_ready = v_resp_ready;
    o_miso = r.txshift[31];

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_spi_transmitter_r_reset;
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

endmodule: vip_spi_transmitter
