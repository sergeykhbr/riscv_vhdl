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

module apb_i2c #(
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_pnp_pkg::dev_config_type o_cfg,            // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_scl,                                     // Clock output upto 400 kHz (default 100 kHz)
    output logic o_sda,                                     // Data output (tri-state buffer input)
    output logic o_sda_dir,                                 // Data to control tri-state buffer
    input logic i_sda,                                      // Tri-state buffer output
    output logic o_irq,                                     // Interrupt request
    output logic o_nreset                                   // I2C slave reset. PCA9548 I2C mux must be de-asserted.
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
import apb_i2c_pkg::*;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_i2c_registers r;
apb_i2c_registers rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_I2C)
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
    apb_i2c_registers v;
    logic v_change_data;
    logic v_latch_data;
    logic [31:0] vb_rdata;

    v = r;
    v_change_data = 1'b0;
    v_latch_data = 1'b0;
    vb_rdata = '0;

    // system bus clock scaler to baudrate:
    if ((|r.scaler) == 1'b1) begin
        if ((|r.state) == 1'b0) begin
            v.scaler_cnt = 16'd0;
            v.level = 1'b1;
        end else if (r.scaler_cnt == (r.scaler - 1)) begin
            v.scaler_cnt = 16'd0;
            v.level = (~r.level);
        end else begin
            // The data on the SDA line must remain stable during the
            // HIGH period of the clock pulse.
            v.scaler_cnt = (r.scaler_cnt + 1);
            if (r.scaler_cnt == r.setup_time) begin
                v_change_data = (~r.level);
                v_latch_data = r.level;
            end
        end
    end

    if (v_change_data == 1'b1) begin
        v.shiftreg = {r.shiftreg[17: 0], 1'b1};
    end
    if (v_latch_data == 1'b1) begin
        v.rxbyte = {r.rxbyte[6: 0], i_sda};
    end

    // Transmitter's state machine:
    case (r.state)
    STATE_IDLE: begin
        v.start = 1'b0;
        v.shiftreg = '1;
        v.sda_dir = PIN_DIR_OUTPUT;
        if (r.start == 1'b1) begin
            // Start condition SDA goes LOW while SCL is HIGH
            v.shiftreg = {1'b0,
                    r.addr,
                    r.R_nW,
                    1'b0,
                    r.payload[7: 0],
                    1'b1};
            v.payload = {8'd0, r.payload[31: 8]};
            v.state = STATE_START;
        end
    end

    STATE_START: begin
        if (v_change_data == 1'b1) begin
            v.bit_cnt = 3'd7;
            v.state = STATE_HEADER;
        end
    end

    STATE_HEADER: begin
        if (v_change_data == 1'b1) begin
            if ((|r.bit_cnt) == 1'b0) begin
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_ACK_HEADER;
            end else begin
                v.bit_cnt = (r.bit_cnt - 1);
            end
        end
    end

    STATE_ACK_HEADER: begin
        if (v_latch_data == 1'b1) begin
            v.ack = i_sda;
        end
        if (v_change_data == 1'b1) begin
            v.sda_dir = PIN_DIR_OUTPUT;
            if (r.ack == 1'b0) begin
                v.bit_cnt = 3'd7;
                if (r.R_nW == 1'b1) begin
                    v.state = STATE_RX_DATA;
                    v.sda_dir = PIN_DIR_INPUT;
                end else begin
                    v.state = STATE_TX_DATA;
                end
            end else begin
                v.err_ack_header = 1'b1;
                v.state = STATE_STOP;
            end
        end
    end

    STATE_RX_DATA: begin
        if (v_change_data == 1'b1) begin
            if ((|r.bit_cnt) == 1'b0) begin
                v.sda_dir = PIN_DIR_OUTPUT;
                v.byte_cnt = (r.byte_cnt - 1);
                v.payload = {r.payload[23: 0], r.rxbyte};
                // A master receiver must signal an end of data to the
                // transmitter by not generating ACK on the last byte
                if ((|r.byte_cnt[3: 1]) == 1'b1) begin
                    v.shiftreg = 19'd0;
                end else begin
                    v.shiftreg = '1;
                end
                v.state = STATE_ACK_DATA;
            end else begin
                v.bit_cnt = (r.bit_cnt - 1);
            end
        end
    end

    STATE_ACK_DATA: begin
        if (v_change_data == 1'b1) begin
            if ((|r.byte_cnt) == 1'b0) begin
                v.state = STATE_STOP;
            end else begin
                v.bit_cnt = 3'd7;
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_RX_DATA;
            end
        end
    end

    STATE_TX_DATA: begin
        if (v_change_data == 1'b1) begin
            if ((|r.bit_cnt) == 1'b0) begin
                v.shiftreg = 19'd0;                         // set LOW to generate STOP ocndition if last byte
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_WAIT_ACK_DATA;
                if ((|r.byte_cnt) == 1'b1) begin
                    v.byte_cnt = (r.byte_cnt - 1);
                end
            end else begin
                v.bit_cnt = (r.bit_cnt - 1);
            end
        end
    end

    STATE_WAIT_ACK_DATA: begin
        if (v_latch_data == 1'b1) begin
            v.ack = i_sda;
        end
        if (v_change_data == 1'b1) begin
            v.sda_dir = PIN_DIR_OUTPUT;
            if ((r.ack == 1'b1) || ((|r.byte_cnt) == 1'b0)) begin
                v.err_ack_data = r.ack;
                v.state = STATE_STOP;
            end else begin
                v.bit_cnt = 3'd7;
                v.shiftreg = {r.payload[7: 0], 11'h7FF};
                v.payload = {8'd0, r.payload[31: 8]};
                v.state = STATE_TX_DATA;
            end
        end
    end

    STATE_STOP: begin
        if (v_latch_data == 1'b1) begin
            v.shiftreg = '1;
            v.state = STATE_IDLE;
            v.irq = r.ie;
        end
    end

    default: begin
        v.state = STATE_IDLE;
    end
    endcase

    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // 0x00: scldiv
        vb_rdata = r.scaler;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scaler = wb_req_wdata[15: 0];
            v.setup_time = wb_req_wdata[31: 16];
            v.scaler_cnt = 16'd0;
        end
    end
    10'h001: begin                                          // 0x04: control and status
        vb_rdata[7: 0] = r.state;                           // [7:0] state machine
        vb_rdata[8] = i_sda;                                // [8] input SDA data bit
        vb_rdata[9] = r.err_ack_header;
        vb_rdata[10] = r.err_ack_data;
        vb_rdata[12] = r.ie;                                // [12] Interrupt enable bit: 1=enabled
        vb_rdata[13] = r.irq;                               // [13] Interrupt pending bit. Clear on read.
        vb_rdata[16] = r.nreset;                            // [16] 0=unchanged; 1=set HIGH nreset
        vb_rdata[17] = r.nreset;                            // [17] 0=unchanged; 1=set LOW nreset
        if (w_req_valid == 1'b1) begin
            v.irq = 1'b0;                                   // Reset irq on read
            if (w_req_write == 1'b1) begin
                v.err_ack_header = 1'b0;
                v.err_ack_data = 1'b0;
                v.ie = wb_req_wdata[12];
                v.irq = wb_req_wdata[13];
                if (wb_req_wdata[16] == 1'b1) begin
                    v.nreset = 1'b1;
                end else if (wb_req_wdata[17] == 1'b1) begin
                    v.nreset = 1'b0;
                end
            end
        end
    end
    10'h002: begin                                          // 0x8: Addr
        vb_rdata[31] = r.R_nW;
        vb_rdata[19: 16] = r.byte_cnt;
        vb_rdata[6: 0] = r.addr;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v.R_nW = wb_req_wdata[31];
                v.byte_cnt = wb_req_wdata[19: 16];
                v.addr = wb_req_wdata[6: 0];
                v.start = 1'b1;
            end
        end
    end
    10'h003: begin                                          // 0xC: Payload
        vb_rdata = r.payload;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                v.payload = wb_req_wdata;
            end
        end
    end
    default: begin
    end
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = apb_i2c_r_reset;
    end

    o_scl = r.level;
    o_sda = r.shiftreg[18];
    o_sda_dir = r.sda_dir;
    o_nreset = r.nreset;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= apb_i2c_r_reset;
            end else begin
                r <= rin;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r <= rin;
        end

    end: async_r_dis
endgenerate

endmodule: apb_i2c
