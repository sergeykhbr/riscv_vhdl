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

module vip_spi_top #(
    parameter bit async_reset = 1'b0,
    parameter int instnum = 0,
    parameter int baudrate = 2000000,
    parameter int scaler = 8
)
(
    input logic i_nrst,
    input logic i_csn,
    input logic i_sclk,
    input logic i_mosi,
    output logic o_miso,
    output logic o_vip_uart_loopback_ena,
    inout logic [15:0] io_vip_gpio
);

import vip_spi_top_pkg::*;

localparam realtime pll_period = (1.0 / ((2 * scaler) * baudrate));

logic w_clk;
logic w_req_valid;
logic w_req_write;
logic [31:0] wb_req_addr;
logic [31:0] wb_req_wdata;
logic w_req_ready;
logic w_resp_valid;
logic [31:0] wb_resp_rdata;
logic w_resp_ready;
logic [15:0] wb_gpio_in;
vip_spi_top_registers r, rin;

vip_clk #(
    .period(pll_period)
) clk0 (
    .o_clk(w_clk)
);


vip_spi_transmitter #(
    .async_reset(async_reset),
    .scaler(scaler)
) tx0 (
    .i_nrst(i_nrst),
    .i_clk(w_clk),
    .i_csn(i_csn),
    .i_sclk(i_sclk),
    .i_mosi(i_mosi),
    .o_miso(o_miso),
    .o_req_valid(w_req_valid),
    .o_req_write(w_req_write),
    .o_req_addr(wb_req_addr),
    .o_req_wdata(wb_req_wdata),
    .i_req_ready(w_req_ready),
    .i_resp_valid(w_resp_valid),
    .i_resp_rdata(wb_resp_rdata),
    .o_resp_ready(w_resp_ready)
);


always_comb
begin: comb_proc
    vip_spi_top_registers v;
    logic [31:0] rdata;

    rdata = '0;

    v = r;

    rdata = r.resp_rdata;

    if ((r.resp_valid == 1'b1) && (w_resp_ready == 1'b1)) begin
        v.resp_valid = 1'b0;
    end else if (w_req_valid == 1'b1) begin
        v.resp_valid = 1'b1;
    end

    case (wb_req_addr[7: 2])
    6'h00: begin                                            // [0x00] hwid
        rdata = 32'hcafecafe;
    end
    6'h01: begin                                            // [0x04] scratch0
        rdata = r.scratch0;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scratch0 = wb_req_wdata;
        end
    end
    6'h02: begin                                            // [0x08] scratch1
        rdata = r.scratch1;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scratch1 = wb_req_wdata;
        end
    end
    6'h03: begin                                            // [0x0C] scratch2
        rdata = r.scratch2;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scratch2 = wb_req_wdata;
        end
    end
    6'h04: begin                                            // [0x10] uart control
        rdata[0] = r.uart_loopback;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.uart_loopback = wb_req_wdata[0];
        end
    end
    6'h05: begin                                            // [0x14] gpio in
        rdata[15: 0] = wb_gpio_in;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.gpio_out = wb_req_wdata[15: 0];
        end
    end
    6'h06: begin                                            // [0x18] gpio direction
        rdata[15: 0] = r.gpio_dir;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.gpio_dir = wb_req_wdata[15: 0];
        end
    end
    default: begin
    end
    endcase
    v.resp_rdata = rdata;

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_spi_top_r_reset;
    end

    w_req_ready = 1'b1;
    w_resp_valid = r.resp_valid;
    wb_resp_rdata = r.resp_rdata;
    o_vip_uart_loopback_ena = r.uart_loopback;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge w_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_spi_top_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge w_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: vip_spi_top
