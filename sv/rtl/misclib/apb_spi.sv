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
    output logic o_mosi,
    input logic i_miso,
    input logic i_detected,
    input logic i_protect
);

import types_amba_pkg::*;
localparam int fifo_dbits = 8;
// SPI states
localparam bit [1:0] idle = 2'h0;
localparam bit [1:0] wait_edge = 2'h1;
localparam bit [1:0] send_data = 2'h2;
localparam bit [1:0] ending = 2'h3;

typedef struct {
    logic [31:0] scaler;
    logic [31:0] scaler_cnt;
    logic generate_crc;
    logic level;
    logic cs;
    logic [1:0] state;
    logic [15:0] ena_byte_cnt;
    logic [2:0] bit_cnt;
    logic [7:0] tx_val;
    logic [7:0] tx_shift;
    logic [7:0] rx_shift;
    logic rx_ready;
    logic [6:0] crc7;
    logic [15:0] crc16;
    logic [7:0] spi_resp;
    logic [log2_fifosz-1:0] txmark;
    logic [log2_fifosz-1:0] rxmark;
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
    '0,                                 // tx_val
    '1,                                 // tx_shift
    '0,                                 // rx_shift
    1'b0,                               // rx_ready
    '0,                                 // crc7
    '0,                                 // crc16
    '0,                                 // spi_resp
    '0,                                 // txmark
    '0,                                 // rxmark
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    1'b0                                // resp_err
};

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
// Rx FIFO signals:
logic w_rxfifo_we;
logic [7:0] wb_rxfifo_wdata;
logic w_rxfifo_re;
logic [7:0] wb_rxfifo_rdata;
logic [log2_fifosz:0] wb_rxfifo_count;
// Tx FIFO signals:
logic w_txfifo_we;
logic [7:0] wb_txfifo_wdata;
logic w_txfifo_re;
logic [7:0] wb_txfifo_rdata;
logic [log2_fifosz:0] wb_txfifo_count;
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
    .i_we(w_rxfifo_we),
    .i_wdata(wb_rxfifo_wdata),
    .i_re(w_rxfifo_re),
    .o_rdata(wb_rxfifo_rdata),
    .o_count(wb_rxfifo_count)
);


sfifo #(
    .async_reset(async_reset),
    .dbits(fifo_dbits),
    .log2_depth(9)
) txfifo (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_we(w_txfifo_we),
    .i_wdata(wb_txfifo_wdata),
    .i_re(w_txfifo_re),
    .o_rdata(wb_txfifo_rdata),
    .o_count(wb_txfifo_count)
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
    logic v_inv7;
    logic [6:0] vb_crc7;
    logic v_inv16;
    logic [15:0] vb_crc16;
    logic [31:0] vb_rdata;

    v_posedge = 0;
    v_negedge = 0;
    v_txfifo_re = 0;
    v_txfifo_we = 0;
    vb_txfifo_wdata = 0;
    v_rxfifo_re = 0;
    v_rxfifo_we = 0;
    vb_rxfifo_wdata = 0;
    v_inv7 = 0;
    vb_crc7 = 0;
    v_inv16 = 0;
    vb_crc16 = 0;
    vb_rdata = 0;

    v = r;

    // CRC7 = x^7 + x^3 + 1
    v_inv7 = (r.crc7[6] ^ r.tx_shift[7]);
    vb_crc7[6] = r.crc7[5];
    vb_crc7[5] = r.crc7[4];
    vb_crc7[4] = r.crc7[3];
    vb_crc7[3] = (r.crc7[2] ^ v_inv7);
    vb_crc7[2] = r.crc7[1];
    vb_crc7[1] = r.crc7[0];
    vb_crc7[0] = v_inv7;
    // CRC16 = x^16 + x^12 + x^5 + 1
    v_inv16 = (r.crc16[15] ^ i_miso);
    vb_crc16[15] = r.crc16[14];
    vb_crc16[14] = r.crc16[13];
    vb_crc16[13] = r.crc16[12];
    vb_crc16[12] = (r.crc16[11] ^ v_inv16);
    vb_crc16[11] = r.crc16[10];
    vb_crc16[10] = r.crc16[9];
    vb_crc16[9] = r.crc16[8];
    vb_crc16[8] = r.crc16[7];
    vb_crc16[7] = r.crc16[6];
    vb_crc16[6] = r.crc16[5];
    vb_crc16[5] = (r.crc16[4] ^ v_inv16);
    vb_crc16[4] = r.crc16[3];
    vb_crc16[3] = r.crc16[2];
    vb_crc16[2] = r.crc16[1];
    vb_crc16[1] = r.crc16[0];
    vb_crc16[0] = v_inv16;

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

    if ((v_negedge == 1'b1) && (r.cs == 1'b1)) begin
        v.tx_shift = {r.tx_shift[6: 0], 1'h1};
        if ((|r.bit_cnt) == 1'b1) begin
            v.bit_cnt = (r.bit_cnt - 1);
        end else begin
            v.cs = 1'b0;
        end
    end

    if (v_posedge == 1'b1) begin
        if (r.rx_ready == 1'b1) begin
            v.rx_ready = 1'b0;
            v_rxfifo_we = 1'b1;
            vb_rxfifo_wdata = r.rx_shift;
            v.rx_shift = '0;
        end

        if (r.cs == 1'b1) begin
            v.rx_shift = {r.rx_shift[6: 0], i_miso};
            v.crc7 = vb_crc7;
            v.crc16 = vb_crc16;
        end
    end

    // Transmitter's state machine:
    case (r.state)
    idle: begin
        if ((|r.ena_byte_cnt) == 1'b1) begin
            v_txfifo_re = 1'b1;
            if ((|wb_txfifo_count) == 1'b0) begin
                // FIFO is empty:
                v.tx_val = '1;
            end else begin
                v.tx_val = wb_txfifo_rdata;
            end
            v.state = wait_edge;
            v.ena_byte_cnt = (r.ena_byte_cnt - 1);
            v.crc7 = '0;
        end else begin
            v.tx_val = '1;
        end
    end
    wait_edge: begin
        if (v_negedge == 1'b1) begin
            v.cs = 1'b1;
            v.bit_cnt = 7;
            v.tx_shift = r.tx_val;
            v.state = send_data;
        end
    end
    send_data: begin
        if (((|r.bit_cnt) == 1'b0) && (v_posedge == 1'b1)) begin
            if ((|r.ena_byte_cnt) == 1'b1) begin
                v_txfifo_re = 1'b1;
                if ((|wb_txfifo_count) == 1'b0) begin
                    // FIFO is empty:
                    v.tx_val = '1;
                end else begin
                    v.tx_val = wb_txfifo_rdata;
                end
                v.state = wait_edge;
                v.ena_byte_cnt = (r.ena_byte_cnt - 1);
            end else if (r.generate_crc == 1'b1) begin
                v.tx_val = {vb_crc7, 1'h1};
                v.generate_crc = 1'b0;
                v.state = wait_edge;
            end else begin
                v.state = ending;
            end
            v.rx_ready = 1'b1;
        end
    end
    ending: begin
        if (r.cs == 1'b0) begin
            v.state = idle;
        end
    end
    default: begin
    end
    endcase

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
        vb_rdata[2] = i_miso;                               // [2] miso data bit
        vb_rdata[5: 4] = r.state;                           // [5:4] state machine
        vb_rdata[7] = r.generate_crc;                       // [7] Compute and generate CRC as the last Tx byte
        vb_rdata[31: 16] = r.ena_byte_cnt;                  // [31:16] Number of bytes to transmit
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.generate_crc = wb_req_wdata[7];
            v.ena_byte_cnt = wb_req_wdata[31: 16];
        end
    end
    10'h012: begin                                          // 0x48: Tx FIFO Data
        vb_rdata[31] = (&wb_txfifo_count);
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v_txfifo_we = 1'h1;
                vb_txfifo_wdata = wb_req_wdata[7: 0];
            end
        end
    end
    10'h013: begin                                          // 0x4C: Rx FIFO Data
        vb_rdata[7: 0] = wb_rxfifo_rdata;
        vb_rdata[31] = (~(|wb_rxfifo_count));
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                // do nothing:
            end else begin
                v_rxfifo_re = 1'h1;
            end
        end
    end
    10'h014: begin                                          // 0x50: Tx FIFO Watermark
        vb_rdata[(log2_fifosz - 1): 0] = r.txmark;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v.txmark = wb_req_wdata[(log2_fifosz - 1): 0];
            end
        end
    end
    10'h015: begin                                          // 0x54: Rx FIFO Watermark
        vb_rdata[(log2_fifosz - 1): 0] = r.rxmark;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v.rxmark = wb_req_wdata[(log2_fifosz - 1): 0];
            end
        end
    end
    10'h016: begin                                          // 0x58: CRC16 value (reserved FU740)
        vb_rdata[15: 0] = r.crc16;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v.crc16 = wb_req_wdata[15: 0];
            end
        end
    end
    default: begin
    end
    endcase

    w_rxfifo_we = v_rxfifo_we;
    wb_rxfifo_wdata = vb_rxfifo_wdata;
    w_rxfifo_re = v_rxfifo_re;

    w_txfifo_we = v_txfifo_we;
    wb_txfifo_wdata = vb_txfifo_wdata;
    w_txfifo_re = v_txfifo_re;

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if (~async_reset && i_nrst == 1'b0) begin
        v = apb_spi_r_reset;
    end

    o_sclk = (r.level & r.cs);
    o_mosi = r.tx_shift[7];
    o_cs = (~r.cs);

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
