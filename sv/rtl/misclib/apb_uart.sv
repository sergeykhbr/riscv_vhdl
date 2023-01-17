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

module apb_uart #(
    parameter bit async_reset = 1'b0,
    parameter int log2_fifosz = 4,
    parameter int speedup_rate = 0                          // simulation speed-up: 0=no speed up, 1=2x, 2=4x, etc
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_amba_pkg::dev_config_type o_cfg,           // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    input logic i_rd,
    output logic o_td,
    output logic o_irq
);

import types_amba_pkg::*;
// Rx/Tx states
localparam bit [2:0] idle = 3'h0;
localparam bit [2:0] startbit = 3'h1;
localparam bit [2:0] data = 3'h2;
localparam bit [2:0] parity = 3'h3;
localparam bit [2:0] stopbit = 3'h4;

localparam int fifosz = (2**log2_fifosz);

typedef struct {
    logic [31:0] scaler;
    logic [31:0] scaler_cnt;
    logic level;
    logic err_parity;
    logic err_stopbit;
    logic [31:0] fwcpuid;
    logic [7:0] rx_fifo[0: fifosz - 1];
    logic [2:0] rx_state;
    logic rx_ena;
    logic rx_ie;
    logic rx_ip;
    logic rx_nstop;
    logic rx_par;
    logic [log2_fifosz-1:0] rx_wr_cnt;
    logic [log2_fifosz-1:0] rx_rd_cnt;
    logic [log2_fifosz-1:0] rx_byte_cnt;
    logic [log2_fifosz-1:0] rx_irq_thresh;
    logic [3:0] rx_frame_cnt;
    logic rx_stop_cnt;
    logic [10:0] rx_shift;
    logic [7:0] tx_fifo[0: fifosz - 1];
    logic [2:0] tx_state;
    logic tx_ena;
    logic tx_ie;
    logic tx_ip;
    logic tx_nstop;
    logic tx_par;
    logic [log2_fifosz-1:0] tx_wr_cnt;
    logic [log2_fifosz-1:0] tx_rd_cnt;
    logic [log2_fifosz-1:0] tx_byte_cnt;
    logic [log2_fifosz-1:0] tx_irq_thresh;
    logic [3:0] tx_frame_cnt;
    logic tx_stop_cnt;
    logic [10:0] tx_shift;
    logic tx_amo_guard;                                     // AMO operation read-modify-write often hit on full flag border
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
} apb_uart_registers;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_uart_registers r, rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_UART)
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


always_comb
begin: comb_proc
    apb_uart_registers v;
    logic [31:0] vb_rdata;
    logic [log2_fifosz-1:0] vb_tx_wr_cnt_next;
    logic v_tx_fifo_full;
    logic v_tx_fifo_empty;
    logic [7:0] vb_tx_fifo_rdata;
    logic v_tx_fifo_we;
    logic [log2_fifosz-1:0] vb_rx_wr_cnt_next;
    logic v_rx_fifo_full;
    logic v_rx_fifo_empty;
    logic [7:0] vb_rx_fifo_rdata;
    logic v_rx_fifo_we;
    logic v_rx_fifo_re;
    logic v_negedge_flag;
    logic v_posedge_flag;
    logic par;

    vb_rdata = 0;
    vb_tx_wr_cnt_next = 0;
    v_tx_fifo_full = 0;
    v_tx_fifo_empty = 0;
    vb_tx_fifo_rdata = 0;
    v_tx_fifo_we = 0;
    vb_rx_wr_cnt_next = 0;
    v_rx_fifo_full = 0;
    v_rx_fifo_empty = 0;
    vb_rx_fifo_rdata = 0;
    v_rx_fifo_we = 0;
    v_rx_fifo_re = 0;
    v_negedge_flag = 0;
    v_posedge_flag = 0;
    par = 0;

    v.scaler = r.scaler;
    v.scaler_cnt = r.scaler_cnt;
    v.level = r.level;
    v.err_parity = r.err_parity;
    v.err_stopbit = r.err_stopbit;
    v.fwcpuid = r.fwcpuid;
    for (int i = 0; i < fifosz; i++) begin
        v.rx_fifo[i] = r.rx_fifo[i];
    end
    v.rx_state = r.rx_state;
    v.rx_ena = r.rx_ena;
    v.rx_ie = r.rx_ie;
    v.rx_ip = r.rx_ip;
    v.rx_nstop = r.rx_nstop;
    v.rx_par = r.rx_par;
    v.rx_wr_cnt = r.rx_wr_cnt;
    v.rx_rd_cnt = r.rx_rd_cnt;
    v.rx_byte_cnt = r.rx_byte_cnt;
    v.rx_irq_thresh = r.rx_irq_thresh;
    v.rx_frame_cnt = r.rx_frame_cnt;
    v.rx_stop_cnt = r.rx_stop_cnt;
    v.rx_shift = r.rx_shift;
    for (int i = 0; i < fifosz; i++) begin
        v.tx_fifo[i] = r.tx_fifo[i];
    end
    v.tx_state = r.tx_state;
    v.tx_ena = r.tx_ena;
    v.tx_ie = r.tx_ie;
    v.tx_ip = r.tx_ip;
    v.tx_nstop = r.tx_nstop;
    v.tx_par = r.tx_par;
    v.tx_wr_cnt = r.tx_wr_cnt;
    v.tx_rd_cnt = r.tx_rd_cnt;
    v.tx_byte_cnt = r.tx_byte_cnt;
    v.tx_irq_thresh = r.tx_irq_thresh;
    v.tx_frame_cnt = r.tx_frame_cnt;
    v.tx_stop_cnt = r.tx_stop_cnt;
    v.tx_shift = r.tx_shift;
    v.tx_amo_guard = r.tx_amo_guard;
    v.resp_valid = r.resp_valid;
    v.resp_rdata = r.resp_rdata;
    v.resp_err = r.resp_err;

    vb_rx_fifo_rdata = r.rx_fifo[int'(r.rx_rd_cnt)];
    vb_tx_fifo_rdata = r.tx_fifo[int'(r.tx_rd_cnt)];

    // Check FIFOs counters with thresholds:
    if (r.tx_byte_cnt < r.tx_irq_thresh) begin
        v.tx_ip = r.tx_ie;
    end

    if (r.rx_byte_cnt > r.rx_irq_thresh) begin
        v.rx_ip = r.rx_ie;
    end

    // Transmitter's FIFO:
    vb_tx_wr_cnt_next = (r.tx_wr_cnt + 1);
    if (vb_tx_wr_cnt_next == r.tx_rd_cnt) begin
        v_tx_fifo_full = 1'b1;
    end

    if (r.tx_rd_cnt == r.tx_wr_cnt) begin
        v_tx_fifo_empty = 1'b1;
        v.tx_byte_cnt = '0;
    end
    // Receiver's FIFO:
    vb_rx_wr_cnt_next = (r.rx_wr_cnt + 1);
    if (vb_rx_wr_cnt_next == r.rx_rd_cnt) begin
        v_rx_fifo_full = 1'b1;
    end

    if (r.rx_rd_cnt == r.rx_wr_cnt) begin
        v_rx_fifo_empty = 1'b1;
        v.rx_byte_cnt = '0;
    end

    // system bus clock scaler to baudrate:
    if ((|r.scaler) == 1'b1) begin
        if (r.scaler_cnt == (r.scaler - 1)) begin
            v.scaler_cnt = '0;
            v.level = (~r.level);
            v_posedge_flag = (~r.level);
            v_negedge_flag = r.level;
        end else begin
            v.scaler_cnt = (r.scaler_cnt + 1);
        end

        if (((r.rx_state == idle) && (i_rd == 1'b1))
                && ((r.tx_state == idle) && (v_tx_fifo_empty == 1'b1))) begin
            v.scaler_cnt = '0;
            v.level = 1'b1;
        end
    end

    // Transmitter's state machine:
    if (v_posedge_flag == 1'b1) begin
        case (r.tx_state)
        idle: begin
            if ((v_tx_fifo_empty == 1'b0) && (r.tx_ena == 1'b1)) begin
                // stopbit=1,parity=xor,data[7:0],startbit=0
                if (r.tx_par == 1'b1) begin
                    par = (vb_tx_fifo_rdata[7]
                            ^ vb_tx_fifo_rdata[6]
                            ^ vb_tx_fifo_rdata[5]
                            ^ vb_tx_fifo_rdata[4]
                            ^ vb_tx_fifo_rdata[3]
                            ^ vb_tx_fifo_rdata[2]
                            ^ vb_tx_fifo_rdata[1]
                            ^ vb_tx_fifo_rdata[0]);
                    v.tx_shift = {1'h1, par, vb_tx_fifo_rdata, 1'h0};
                end else begin
                    v.tx_shift = {2'h3, vb_tx_fifo_rdata, 1'h0};
                end

                v.tx_state = startbit;
                v.tx_rd_cnt = (r.tx_rd_cnt + 1);
                v.tx_byte_cnt = (r.tx_byte_cnt - 1);
                v.tx_frame_cnt = '0;
            end else begin
                v.tx_shift = '1;
            end
        end
        startbit: begin
            v.tx_state = data;
        end
        data: begin
            if (r.tx_frame_cnt == 8) begin
                if (r.tx_par == 1'b1) begin
                    v.tx_state = parity;
                end else begin
                    v.tx_state = stopbit;
                    v.tx_stop_cnt = r.tx_nstop;
                end
            end
        end
        parity: begin
            v.tx_state = stopbit;
        end
        stopbit: begin
            if (r.tx_stop_cnt == 1'b0) begin
                v.tx_state = idle;
            end else begin
                v.tx_stop_cnt = 1'b0;
            end
        end
        default: begin
        end
        endcase

        if (r.tx_state != idle) begin
            v.tx_frame_cnt = (r.tx_frame_cnt + 1);
            v.tx_shift = {1'h1, r.tx_shift[10: 1]};
        end
    end

    // Receiver's state machine:
    if (v_negedge_flag == 1'b1) begin
        case (r.rx_state)
        idle: begin
            if ((i_rd == 1'b0) && (r.rx_ena == 1'b1)) begin
                v.rx_state = data;
                v.rx_shift = '0;
                v.rx_frame_cnt = '0;
            end
        end
        data: begin
            v.rx_shift = {i_rd, r.rx_shift[7: 1]};
            if (r.rx_frame_cnt == 4'h7) begin
                if (r.rx_par == 1'b1) begin
                    v.rx_state = parity;
                end else begin
                    v.rx_state = stopbit;
                    v.rx_stop_cnt = r.rx_nstop;
                end
            end else begin
                v.rx_frame_cnt = (r.rx_frame_cnt + 1);
            end
        end
        parity: begin
            par = (r.rx_shift[7]
                    ^ r.rx_shift[6]
                    ^ r.rx_shift[5]
                    ^ r.rx_shift[4]
                    ^ r.rx_shift[3]
                    ^ r.rx_shift[2]
                    ^ r.rx_shift[1]
                    ^ r.rx_shift[0]);
            if (par == i_rd) begin
                v.err_parity = 1'b0;
            end else begin
                v.err_parity = 1'b1;
            end

            v.rx_state = stopbit;
        end
        stopbit: begin
            if (i_rd == 1'b0) begin
                v.err_stopbit = 1'b1;
            end else begin
                v.err_stopbit = 1'b0;
            end

            if (r.rx_stop_cnt == 1'b0) begin
                if (v_rx_fifo_full == 1'b0) begin
                    v_rx_fifo_we = 1'b1;
                end
                v.rx_state = idle;
            end else begin
                v.rx_stop_cnt = 1'b0;
            end
        end
        default: begin
        end
        endcase
    end

    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // 0x00: txdata
        vb_rdata[31] = v_tx_fifo_full;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v_tx_fifo_we = ((~v_tx_fifo_full) & (~r.tx_amo_guard));
            end else begin
                v.tx_amo_guard = v_tx_fifo_full;            // skip next write
            end
        end
    end
    10'h001: begin                                          // 0x04: rxdata
        vb_rdata[31] = v_rx_fifo_empty;
        vb_rdata[7: 0] = vb_rx_fifo_rdata;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                // do nothing:
            end else begin
                v_rx_fifo_re = (~v_rx_fifo_empty);
            end
        end
    end
    10'h002: begin                                          // 0x08: txctrl
        vb_rdata[0] = r.tx_ena;                             // [0] tx ena
        vb_rdata[1] = r.tx_nstop;                           // [1] Number of stop bits
        vb_rdata[2] = r.tx_par;                             // [2] parity bit enable
        vb_rdata[18: 16] = r.tx_irq_thresh[2: 0];           // [18:16] FIFO threshold to raise irq
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.tx_ena = wb_req_wdata[0];
            v.tx_nstop = wb_req_wdata[1];
            v.tx_par = wb_req_wdata[2];
            v.tx_irq_thresh = wb_req_wdata[18: 16];
        end
    end
    10'h003: begin                                          // 0x0C: rxctrl
        vb_rdata[0] = r.rx_ena;                             // [0] txena
        vb_rdata[1] = r.rx_nstop;                           // [1] Number of stop bits
        vb_rdata[2] = r.rx_par;
        vb_rdata[18: 16] = r.rx_irq_thresh[2: 0];
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.rx_ena = wb_req_wdata[0];
            v.rx_nstop = wb_req_wdata[1];
            v.rx_par = wb_req_wdata[2];
            v.rx_irq_thresh = wb_req_wdata[18: 16];
        end
    end
    10'h004: begin                                          // 0x10: ie
        vb_rdata[0] = r.tx_ie;
        vb_rdata[1] = r.rx_ie;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.tx_ie = wb_req_wdata[0];
            v.rx_ie = wb_req_wdata[1];
        end
    end
    10'h005: begin                                          // 0x14: ip
        vb_rdata[0] = r.tx_ip;
        vb_rdata[1] = r.rx_ip;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.tx_ip = wb_req_wdata[0];
            v.rx_ip = wb_req_wdata[1];
        end
    end
    10'h006: begin                                          // 0x18: scaler
        vb_rdata = r.scaler;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scaler = wb_req_wdata[30: speedup_rate];
            v.scaler_cnt = '0;
        end
    end
    10'h007: begin                                          // 0x1C: fwcpuid
        vb_rdata = r.fwcpuid;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            if (((|r.fwcpuid) == 1'b0) || ((|wb_req_wdata) == 1'b0)) begin
                v.fwcpuid = wb_req_wdata;
            end
        end
    end
    default: begin
    end
    endcase

    if (v_rx_fifo_we == 1'b1) begin
        v.rx_wr_cnt = (r.rx_wr_cnt + 1);
        v.rx_byte_cnt = (r.rx_byte_cnt + 1);
        v.rx_fifo[int'(r.rx_wr_cnt)] = r.rx_shift[7: 0];
    end else if (v_rx_fifo_re == 1'b1) begin
        v.rx_rd_cnt = (r.rx_rd_cnt + 1);
        v.rx_byte_cnt = (r.rx_byte_cnt - 1);
    end
    if (v_tx_fifo_we == 1'b1) begin
        v.tx_wr_cnt = (r.tx_wr_cnt + 1);
        v.tx_byte_cnt = (r.tx_byte_cnt + 1);
        v.tx_fifo[int'(r.tx_wr_cnt)] = wb_req_wdata[7: 0];
    end

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if (~async_reset && i_nrst == 1'b0) begin
        v.scaler = 32'h00000000;
        v.scaler_cnt = 32'h00000000;
        v.level = 1'h1;
        v.err_parity = 1'h0;
        v.err_stopbit = 1'h0;
        v.fwcpuid = 32'h00000000;
        for (int i = 0; i < fifosz; i++) begin
            v.rx_fifo[i] = 8'h00;
        end
        v.rx_state = idle;
        v.rx_ena = 1'h0;
        v.rx_ie = 1'h0;
        v.rx_ip = 1'h0;
        v.rx_nstop = 1'h0;
        v.rx_par = 1'h0;
        v.rx_wr_cnt = 4'h0;
        v.rx_rd_cnt = 4'h0;
        v.rx_byte_cnt = 4'h0;
        v.rx_irq_thresh = 4'h0;
        v.rx_frame_cnt = 4'h0;
        v.rx_stop_cnt = 1'h0;
        v.rx_shift = 11'h000;
        for (int i = 0; i < fifosz; i++) begin
            v.tx_fifo[i] = 8'h00;
        end
        v.tx_state = idle;
        v.tx_ena = 1'h0;
        v.tx_ie = 1'h0;
        v.tx_ip = 1'h0;
        v.tx_nstop = 1'h0;
        v.tx_par = 1'h0;
        v.tx_wr_cnt = 4'h0;
        v.tx_rd_cnt = 4'h0;
        v.tx_byte_cnt = 4'h0;
        v.tx_irq_thresh = 4'h0;
        v.tx_frame_cnt = 4'h0;
        v.tx_stop_cnt = 1'h0;
        v.tx_shift = '1;
        v.tx_amo_guard = 1'h0;
        v.resp_valid = 1'h0;
        v.resp_rdata = 32'h00000000;
        v.resp_err = 1'h0;
    end

    o_td = r.tx_shift[0];
    o_irq = (r.tx_ip | r.rx_ip);

    rin.scaler = v.scaler;
    rin.scaler_cnt = v.scaler_cnt;
    rin.level = v.level;
    rin.err_parity = v.err_parity;
    rin.err_stopbit = v.err_stopbit;
    rin.fwcpuid = v.fwcpuid;
    for (int i = 0; i < fifosz; i++) begin
        rin.rx_fifo[i] = v.rx_fifo[i];
    end
    rin.rx_state = v.rx_state;
    rin.rx_ena = v.rx_ena;
    rin.rx_ie = v.rx_ie;
    rin.rx_ip = v.rx_ip;
    rin.rx_nstop = v.rx_nstop;
    rin.rx_par = v.rx_par;
    rin.rx_wr_cnt = v.rx_wr_cnt;
    rin.rx_rd_cnt = v.rx_rd_cnt;
    rin.rx_byte_cnt = v.rx_byte_cnt;
    rin.rx_irq_thresh = v.rx_irq_thresh;
    rin.rx_frame_cnt = v.rx_frame_cnt;
    rin.rx_stop_cnt = v.rx_stop_cnt;
    rin.rx_shift = v.rx_shift;
    for (int i = 0; i < fifosz; i++) begin
        rin.tx_fifo[i] = v.tx_fifo[i];
    end
    rin.tx_state = v.tx_state;
    rin.tx_ena = v.tx_ena;
    rin.tx_ie = v.tx_ie;
    rin.tx_ip = v.tx_ip;
    rin.tx_nstop = v.tx_nstop;
    rin.tx_par = v.tx_par;
    rin.tx_wr_cnt = v.tx_wr_cnt;
    rin.tx_rd_cnt = v.tx_rd_cnt;
    rin.tx_byte_cnt = v.tx_byte_cnt;
    rin.tx_irq_thresh = v.tx_irq_thresh;
    rin.tx_frame_cnt = v.tx_frame_cnt;
    rin.tx_stop_cnt = v.tx_stop_cnt;
    rin.tx_shift = v.tx_shift;
    rin.tx_amo_guard = v.tx_amo_guard;
    rin.resp_valid = v.resp_valid;
    rin.resp_rdata = v.resp_rdata;
    rin.resp_err = v.resp_err;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r.scaler <= 32'h00000000;
                r.scaler_cnt <= 32'h00000000;
                r.level <= 1'h1;
                r.err_parity <= 1'h0;
                r.err_stopbit <= 1'h0;
                r.fwcpuid <= 32'h00000000;
                for (int i = 0; i < fifosz; i++) begin
                    r.rx_fifo[i] <= 8'h00;
                end
                r.rx_state <= idle;
                r.rx_ena <= 1'h0;
                r.rx_ie <= 1'h0;
                r.rx_ip <= 1'h0;
                r.rx_nstop <= 1'h0;
                r.rx_par <= 1'h0;
                r.rx_wr_cnt <= 4'h0;
                r.rx_rd_cnt <= 4'h0;
                r.rx_byte_cnt <= 4'h0;
                r.rx_irq_thresh <= 4'h0;
                r.rx_frame_cnt <= 4'h0;
                r.rx_stop_cnt <= 1'h0;
                r.rx_shift <= 11'h000;
                for (int i = 0; i < fifosz; i++) begin
                    r.tx_fifo[i] <= 8'h00;
                end
                r.tx_state <= idle;
                r.tx_ena <= 1'h0;
                r.tx_ie <= 1'h0;
                r.tx_ip <= 1'h0;
                r.tx_nstop <= 1'h0;
                r.tx_par <= 1'h0;
                r.tx_wr_cnt <= 4'h0;
                r.tx_rd_cnt <= 4'h0;
                r.tx_byte_cnt <= 4'h0;
                r.tx_irq_thresh <= 4'h0;
                r.tx_frame_cnt <= 4'h0;
                r.tx_stop_cnt <= 1'h0;
                r.tx_shift <= '1;
                r.tx_amo_guard <= 1'h0;
                r.resp_valid <= 1'h0;
                r.resp_rdata <= 32'h00000000;
                r.resp_err <= 1'h0;
            end else begin
                r.scaler <= rin.scaler;
                r.scaler_cnt <= rin.scaler_cnt;
                r.level <= rin.level;
                r.err_parity <= rin.err_parity;
                r.err_stopbit <= rin.err_stopbit;
                r.fwcpuid <= rin.fwcpuid;
                for (int i = 0; i < fifosz; i++) begin
                    r.rx_fifo[i] <= rin.rx_fifo[i];
                end
                r.rx_state <= rin.rx_state;
                r.rx_ena <= rin.rx_ena;
                r.rx_ie <= rin.rx_ie;
                r.rx_ip <= rin.rx_ip;
                r.rx_nstop <= rin.rx_nstop;
                r.rx_par <= rin.rx_par;
                r.rx_wr_cnt <= rin.rx_wr_cnt;
                r.rx_rd_cnt <= rin.rx_rd_cnt;
                r.rx_byte_cnt <= rin.rx_byte_cnt;
                r.rx_irq_thresh <= rin.rx_irq_thresh;
                r.rx_frame_cnt <= rin.rx_frame_cnt;
                r.rx_stop_cnt <= rin.rx_stop_cnt;
                r.rx_shift <= rin.rx_shift;
                for (int i = 0; i < fifosz; i++) begin
                    r.tx_fifo[i] <= rin.tx_fifo[i];
                end
                r.tx_state <= rin.tx_state;
                r.tx_ena <= rin.tx_ena;
                r.tx_ie <= rin.tx_ie;
                r.tx_ip <= rin.tx_ip;
                r.tx_nstop <= rin.tx_nstop;
                r.tx_par <= rin.tx_par;
                r.tx_wr_cnt <= rin.tx_wr_cnt;
                r.tx_rd_cnt <= rin.tx_rd_cnt;
                r.tx_byte_cnt <= rin.tx_byte_cnt;
                r.tx_irq_thresh <= rin.tx_irq_thresh;
                r.tx_frame_cnt <= rin.tx_frame_cnt;
                r.tx_stop_cnt <= rin.tx_stop_cnt;
                r.tx_shift <= rin.tx_shift;
                r.tx_amo_guard <= rin.tx_amo_guard;
                r.resp_valid <= rin.resp_valid;
                r.resp_rdata <= rin.resp_rdata;
                r.resp_err <= rin.resp_err;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r.scaler <= rin.scaler;
            r.scaler_cnt <= rin.scaler_cnt;
            r.level <= rin.level;
            r.err_parity <= rin.err_parity;
            r.err_stopbit <= rin.err_stopbit;
            r.fwcpuid <= rin.fwcpuid;
            for (int i = 0; i < fifosz; i++) begin
                r.rx_fifo[i] <= rin.rx_fifo[i];
            end
            r.rx_state <= rin.rx_state;
            r.rx_ena <= rin.rx_ena;
            r.rx_ie <= rin.rx_ie;
            r.rx_ip <= rin.rx_ip;
            r.rx_nstop <= rin.rx_nstop;
            r.rx_par <= rin.rx_par;
            r.rx_wr_cnt <= rin.rx_wr_cnt;
            r.rx_rd_cnt <= rin.rx_rd_cnt;
            r.rx_byte_cnt <= rin.rx_byte_cnt;
            r.rx_irq_thresh <= rin.rx_irq_thresh;
            r.rx_frame_cnt <= rin.rx_frame_cnt;
            r.rx_stop_cnt <= rin.rx_stop_cnt;
            r.rx_shift <= rin.rx_shift;
            for (int i = 0; i < fifosz; i++) begin
                r.tx_fifo[i] <= rin.tx_fifo[i];
            end
            r.tx_state <= rin.tx_state;
            r.tx_ena <= rin.tx_ena;
            r.tx_ie <= rin.tx_ie;
            r.tx_ip <= rin.tx_ip;
            r.tx_nstop <= rin.tx_nstop;
            r.tx_par <= rin.tx_par;
            r.tx_wr_cnt <= rin.tx_wr_cnt;
            r.tx_rd_cnt <= rin.tx_rd_cnt;
            r.tx_byte_cnt <= rin.tx_byte_cnt;
            r.tx_irq_thresh <= rin.tx_irq_thresh;
            r.tx_frame_cnt <= rin.tx_frame_cnt;
            r.tx_stop_cnt <= rin.tx_stop_cnt;
            r.tx_shift <= rin.tx_shift;
            r.tx_amo_guard <= rin.tx_amo_guard;
            r.resp_valid <= rin.resp_valid;
            r.resp_rdata <= rin.resp_rdata;
            r.resp_err <= rin.resp_err;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: apb_uart
