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

module vip_uart_receiver #(
    parameter bit async_reset = 1'b0,
    parameter int scaler = 8
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_rx,
    output logic o_rdy,
    input logic i_rdy_clr,
    output logic [7:0] o_data
);

import vip_uart_receiver_pkg::*;

localparam int scaler_max = ((2 * scaler) - 1);
localparam int scaler_mid = scaler;

vip_uart_receiver_registers r, rin;

always_comb
begin: comb_proc
    vip_uart_receiver_registers v;
    logic v_rx_pos;
    logic v_rx_neg;

    v_rx_pos = 0;
    v_rx_neg = 0;

    v = r;

    v.rx = i_rx;
    v_rx_pos = ((~r.rx) && i_rx);
    v_rx_neg = (r.rx && (~i_rx));
    if (i_rdy_clr == 1'b1) begin
        v.rdy = 1'b0;
    end

    case (r.state)
    startbit: begin

        // Start counting from the first low sample, once we've
        // sampled a full bit, start collecting data bits.

        if ((i_rx == 1'b0) || ((|r.sample) == 1'b1)) begin
            v.sample = (r.sample + 1);
        end

        if ((r.sample == scaler_max) || (v_rx_pos == 1'b1)) begin
            v.state = data;
            v.bitpos = '0;
            v.sample = '0;
            v.scratch = '0;
            v.rx_err = 1'b0;
        end
    end
    data: begin
        if ((r.sample == scaler_max)
                || ((r.sample > scaler_mid) && ((v_rx_neg == 1'b1) || (v_rx_pos == 1'b1)))) begin
            v.sample = '0;
            if (r.bitpos == 8'h08) begin
                v.state = stopbit;
            end
        end else begin
            v.sample = (r.sample + 1);
        end

        if (r.sample == scaler_mid) begin
            v.scratch = {i_rx, r.scratch[7: 1]};
            v.bitpos = (r.bitpos + 1);
        end
    end
    stopbit: begin
        if (r.sample == scaler_mid) begin
            v.rdata = r.scratch;
            v.rdy = 1'b1;
            if (i_rx == 1'b0) begin
                v.rx_err = 1'b1;
            end else begin
            end
        end
        if (r.sample == scaler_max) begin
            v.state = dummy;
            v.sample = '0;
        end else begin
            v.sample = (r.sample + 1);
        end
    end
    dummy: begin
        // Idle state in UART generates additional byte and it works
        // even if rx=0 on real device:
        if (r.sample >= scaler_mid) begin
            v.state = startbit;
            v.sample = '0;
        end else begin
            v.sample = (r.sample + 1);
        end
    end
    default: begin
        v.state = startbit;
    end
    endcase

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_uart_receiver_r_reset;
    end

    o_rdy = r.rdy;
    o_data = r.rdata;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_uart_receiver_r_reset;
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

endmodule: vip_uart_receiver
