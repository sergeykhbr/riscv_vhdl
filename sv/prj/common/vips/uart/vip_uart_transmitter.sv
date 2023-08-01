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

module vip_uart_transmitter #(
    parameter bit async_reset = 1'b0,
    parameter int scaler = 8
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_we,
    input logic [7:0] i_wdata,
    output logic o_full,
    output logic o_tx
);

import vip_uart_transmitter_pkg::*;

localparam int scaler_max = ((2 * scaler) - 1);

vip_uart_transmitter_registers r, rin;

always_comb
begin: comb_proc
    vip_uart_transmitter_registers v;
    logic v_next;

    v_next = 0;

    v = r;

    if (i_we == 1'b1) begin
        if (r.txdata_rdy == 1'b1) begin
            v.overflow = 1'b1;
        end else begin
            v.txdata_rdy = 1'b1;
            v.txdata = i_wdata;
        end
    end

    if (r.sample == scaler_max) begin
        v.sample = '0;
        v_next = 1'b1;
    end else begin
        v.sample = (r.sample + 1);
    end

    if (v_next == 1'b1) begin
        case (r.state)
        idle: begin
            if (r.txdata_rdy == 1'b1) begin
                v.txdata_rdy = 1'b0;
                v.overflow = 1'b0;
                v.shiftreg = {r.txdata, 1'h0};
                v.state = startbit;
            end
        end
        startbit: begin
            v.state = data;
            v.bitpos = '0;
            v.shiftreg = {1'h1, r.shiftreg[8: 1]};
        end
        data: begin
            if (r.bitpos == 4'h7) begin
                v.state = stopbit;
                v.shiftreg = '1;
            end else begin
                v.shiftreg = {1'h1, r.shiftreg[8: 1]};
            end
            v.bitpos = (r.bitpos + 1);
        end
        stopbit: begin
            v.state = idle;
        end
        default: begin
        end
        endcase
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_uart_transmitter_r_reset;
    end

    o_full = r.txdata_rdy;
    o_tx = r.shiftreg[0];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_uart_transmitter_r_reset;
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

endmodule: vip_uart_transmitter
