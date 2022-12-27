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

module apb_spi #(
    parameter bit async_reset = 1'b0,
    parameter int log2_fifosz = 9
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_amba_pkg::dev_config_type o_cfg,           // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_cs,
    output logic o_sclk,
    output logic o_miso,
    input logic i_mosi,
    input logic i_detected,
    input logic i_protect
);

import types_amba_pkg::*;
localparam int fifo_dbits = 8;
// SPI states
localparam bit [1:0] idle = 2'h0;
localparam bit [1:0] send_data = 2'h1;
localparam bit [1:0] receive_data = 2'h2;

typedef struct {
    logic [31:0] scaler;
    logic [31:0] scaler_cnt;
    logic generate_crc;
    logic level;
    logic cs;
    logic [1:0] state;
    logic [15:0] ena_byte_cnt;
    logic [2:0] bit_cnt;
    logic [7:0] tx_shift;
    logic [7:0] rx_shift;
    logic [7:0] spi_resp;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_spi_registers;

const apb_spi_registers apb_spi_r_reset = '{
    '0,                                 // scaler
    '0,                                 // scaler_cnt
    1'b0,                               // generate_crc
    1'h1,                               // level
    1'b0,                               // cs
    idle,                               // state
    '0,                                 // ena_byte_cnt
    '0,                                 // bit_cnt
    '1,                                 // tx_shift
    '0,                                 // rx_shift
    '0,                                 // spi_resp
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
// Rx FIFO signals:
logic [log2_fifosz-1:0] wb_rxfifo_thresh;
logic w_rxfifo_we;
logic [7:0] wb_rxfifo_wdata;
logic w_rxfifo_re;
logic [7:0] wb_rxfifo_rdata;
logic w_rxfifo_full;
logic w_rxfifo_empty;
logic w_rxfifo_less;
logic w_rxfifo_greater;
// Tx FIFO signals:
logic [log2_fifosz-1:0] wb_txfifo_thresh;
logic w_txfifo_we;
logic [7:0] wb_txfifo_wdata;
logic w_txfifo_re;
logic [7:0] wb_txfifo_rdata;
logic w_txfifo_full;
logic w_txfifo_empty;
logic w_txfifo_less;
logic w_txfifo_greater;
apb_spi_registers r, rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_SPI)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(r.resp_valid),
    .i_resp_rdata(r.resp_rdata),
    .i_resp_err(r.resp_err)
);


sfifo #(
    .async_reset(async_reset),
    .dbits(fifo_dbits),
    .log2_depth(9)
) rxfifo (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_thresh(wb_rxfifo_thresh),
    .i_we(w_rxfifo_we),
    .i_wdata(wb_rxfifo_wdata),
    .i_re(w_rxfifo_re),
    .o_rdata(wb_rxfifo_rdata),
    .o_full(w_rxfifo_full),
    .o_empty(w_rxfifo_empty),
    .o_less(w_rxfifo_less),
    .o_greater(w_rxfifo_greater)
);


sfifo #(
    .async_reset(async_reset),
    .dbits(fifo_dbits),
    .log2_depth(9)
) txfifo (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_thresh(wb_txfifo_thresh),
    .i_we(w_txfifo_we),
    .i_wdata(wb_txfifo_wdata),
    .i_re(w_txfifo_re),
    .o_rdata(wb_txfifo_rdata),
    .o_full(w_txfifo_full),
    .o_empty(w_txfifo_empty),
    .o_less(w_txfifo_less),
    .o_greater(w_txfifo_greater)
);


always_comb
begin: comb_proc
    apb_spi_registers v;
    logic v_posedge;
    logic v_negedge;
    logic v_txfifo_re;
    logic v_txfifo_we;
    logic [7:0] vb_txfifo_wdata;
    logic v_rxfifo_re;
    logic v_rxfifo_we;
    logic [7:0] vb_rxfifo_wdata;
    logic [31:0] vb_rdata;

    v_posedge = 0;
    v_negedge = 0;
    v_txfifo_re = 0;
    v_txfifo_we = 0;
    vb_txfifo_wdata = 0;
    v_rxfifo_re = 0;
    v_rxfifo_we = 0;
    vb_rxfifo_wdata = 0;
    vb_rdata = 0;

    v = r;


    // system bus clock scaler to baudrate:
    if ((|r.scaler) == 1'b1) begin
        if (r.scaler_cnt == (r.scaler - 1)) begin
            v.scaler_cnt = '0;
            v.level = (~r.level);
            v_posedge = (~r.level);
            v_negedge = r.level;
        end else begin
            v.scaler_cnt = (r.scaler_cnt + 1);
        end
    end

    // Transmitter's state machine:
    if (v_posedge == 1'b1) begin
        case (r.state)
        idle: begin
            if ((|r.ena_byte_cnt) == 1'b1) begin
                v_txfifo_re = 1'b1;
                v.tx_shift = wb_txfifo_rdata;
                v.state = send_data;
                v.bit_cnt = 7;
                v.ena_byte_cnt = (r.ena_byte_cnt - 1);
                v.cs = 1'b1;
            end else begin
                v.tx_shift = '1;
            end
        end
        send_data: begin
            if ((|r.bit_cnt) == 1'b0) begin
                v.state = receive_data;
                v.bit_cnt = 7;
            end
        end
        receive_data: begin
            if ((|r.bit_cnt[2: 0]) == 1'b0) begin
                v.state = idle;
                v.spi_resp = r.rx_shift;
            end
        end
        default: begin
        end
        endcase

        if (r.state != idle) begin
            v.bit_cnt = (r.bit_cnt + 1);
            v.tx_shift = {r.tx_shift[7: 1], 1'h1};
        end
    end

    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // 0x00: sckdiv
        vb_rdata = r.scaler;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scaler = wb_req_wdata[30: 0];
            v.scaler_cnt = '0;
        end
    end
    10'h011: begin                                          // 0x44: reserved 4 (txctrl)
        vb_rdata[0] = i_detected;                           // [0] sd card inserted
        vb_rdata[1] = i_protect;                            // [1] write protect
        vb_rdata[5: 4] = r.state;                           // [5:4] state machine
        vb_rdata[7] = r.generate_crc;                       // [7] Compute and generate CRC as the last Tx byte
        vb_rdata[31: 16] = r.ena_byte_cnt;                  // [31:16] Number of bytes to transmit
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.generate_crc = wb_req_wdata[7];
            v.ena_byte_cnt = wb_req_wdata[31: 16];
        end
    end
    10'h012: begin                                          // 0x48: Tx FIFO Data
        vb_rdata[31] = w_txfifo_full;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v_txfifo_we = 1'h1;
                vb_txfifo_wdata = wb_req_wdata[7: 0];
            end
        end
    end
    10'h013: begin                                          // 0x4C: Rx FIFO Data
        vb_rdata[7: 0] = wb_rxfifo_rdata;
        vb_rdata[31] = w_rxfifo_empty;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                // do nothing:
            end else begin
                v_rxfifo_re = 1'h1;
            end
        end
    end
    default: begin
    end
    endcase

    wb_rxfifo_thresh = 0;
    w_rxfifo_we = v_rxfifo_we;
    wb_rxfifo_wdata = vb_rxfifo_wdata;
    w_rxfifo_re = v_rxfifo_re;

    wb_txfifo_thresh = 0;
    w_txfifo_we = v_txfifo_we;
    wb_txfifo_wdata = vb_txfifo_wdata;
    w_txfifo_re = v_txfifo_re;

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if (~async_reset && i_nrst == 1'b0) begin
        v = apb_spi_r_reset;
    end

    o_sclk = r.level;
    o_miso = r.tx_shift[7];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= apb_spi_r_reset;
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

endmodule: apb_spi
