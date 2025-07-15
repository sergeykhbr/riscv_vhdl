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

module sdctrl_cmd_transmitter #(
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_sclk_posedge,
    input logic i_sclk_negedge,
    input logic i_cmd,
    output logic o_cmd,
    output logic o_cmd_dir,
    output logic o_cmd_cs,
    input logic i_spi_mode,                                 // SPI mode was selected by FW
    input logic [3:0] i_err_code,
    input logic i_wdog_trigger,                             // Event from wdog timer
    input logic i_cmd_set_low,                              // Set forcibly o_cmd output to LOW
    input logic i_req_valid,
    input logic [5:0] i_req_cmd,
    input logic [31:0] i_req_arg,
    input logic [2:0] i_req_rn,                             // R1, R3,R6 or R2
    output logic o_req_ready,
    output logic o_resp_valid,
    output logic [5:0] o_resp_cmd,                          // Mirrored command
    output logic [31:0] o_resp_reg,                         // Card Status, OCR register (R3) or RCA register (R6)
    output logic [6:0] o_resp_crc7_rx,                      // Received CRC7
    output logic [6:0] o_resp_crc7_calc,                    // Calculated CRC7
    output logic [14:0] o_resp_spistatus,                   // {R1,R2} response valid only in SPI mode
    input logic i_resp_ready,
    output logic o_wdog_ena,
    output logic o_err_valid,
    output logic [3:0] o_err_setcode
);

import sdctrl_cfg_pkg::*;
import sdctrl_cmd_transmitter_pkg::*;

logic [6:0] wb_crc7;
logic w_crc7_next;
logic w_crc7_dat;
sdctrl_cmd_transmitter_registers r;
sdctrl_cmd_transmitter_registers rin;

sdctrl_crc7 #(
    .async_reset(async_reset)
) crc0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc7_clear),
    .i_next(w_crc7_next),
    .i_dat(w_crc7_dat),
    .o_crc7(wb_crc7)
);

always_comb
begin: comb_proc
    sdctrl_cmd_transmitter_registers v;
    logic v_req_ready;
    logic [47:0] vb_cmdshift;
    logic [14:0] vb_resp_spistatus;
    logic v_crc7_dat;
    logic v_crc7_next;

    v = r;
    v_req_ready = 1'b0;
    vb_cmdshift = '0;
    vb_resp_spistatus = '0;
    v_crc7_dat = 1'b0;
    v_crc7_next = 1'b0;

    vb_cmdshift = r.cmdshift;
    vb_resp_spistatus = r.resp_spistatus;
    v.err_valid = 1'b0;
    v.err_setcode = 4'd0;
    if (i_resp_ready == 1'b1) begin
        v.resp_valid = 1'b0;
    end

    // command state:
    if (i_sclk_negedge == 1'b1) begin
        // CMD Request:
        if (r.cmdstate == CMDSTATE_IDLE) begin
            if (i_cmd_set_low == 1'b1) begin
                // Used during p-init state (power-up)
                vb_cmdshift = 48'd0;
            end else begin
                vb_cmdshift = '1;
            end
            v.wdog_ena = 1'b0;
            v.cmd_cs = 1'b1;
            v.cmd_dir = DIR_OUTPUT;
            v.crc7_clear = 1'b1;
            v_req_ready = 1'b1;
            if (i_err_code != CMDERR_NONE) begin
                v_req_ready = 1'b0;
            end else if (i_req_valid == 1'b1) begin
                v.cmd_cs = 1'b0;
                v.req_cmd = i_req_cmd;
                v.req_rn = i_req_rn;
                vb_cmdshift = {2'h1, i_req_cmd, i_req_arg};
                v.cmdbitcnt = 7'd39;
                v.crc7_clear = 1'b0;
                v.cmdstate = CMDSTATE_REQ_CONTENT;
            end
        end else if (r.cmdstate == CMDSTATE_REQ_CONTENT) begin
            v_crc7_next = 1'b1;
            vb_cmdshift = {r.cmdshift[38: 0], 1'b1};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_REQ_CRC7;
                v.crc_calc = wb_crc7;
                vb_cmdshift[39: 32] = {wb_crc7, 1'b1};
                v.cmdbitcnt = 7'd6;
                v.crc7_clear = 1'b1;
            end
        end else if (r.cmdstate == CMDSTATE_REQ_CRC7) begin
            vb_cmdshift = {r.cmdshift[38: 0], 1'b1};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_REQ_STOPBIT;
            end
        end else if (r.cmdstate == CMDSTATE_REQ_STOPBIT) begin
            v.cmdstate = CMDSTATE_RESP_WAIT;
            v.cmd_dir = DIR_INPUT;
            v.wdog_ena = 1'b1;
            v.crc7_clear = 1'b0;
        end else if (r.cmdstate == CMDSTATE_PAUSE) begin
            v.crc7_clear = 1'b1;
            v.cmd_cs = 1'b1;
            v.cmd_dir = DIR_OUTPUT;
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_IDLE;
            end
        end
    end else if (i_sclk_posedge == 1'b1) begin
        // CMD Response (see page 140. '4.9 Responses'):

        if (r.cmdstate == CMDSTATE_RESP_WAIT) begin
            // [47] start bit; [135] for R2
            if (i_cmd == 1'b0) begin
                if (i_spi_mode == 1'b0) begin
                    v_crc7_next = 1'b1;
                    v.cmdstate = CMDSTATE_RESP_TRANSBIT;
                end else begin
                    // Response in SPI mode:
                    v.cmdstate = CMDSTATE_RESP_SPI_R1;
                    v.cmdbitcnt = 7'd6;
                    v.cmdmirror = r.req_cmd;
                    v.resp_spistatus = 15'd0;
                    v.regshift = 32'd0;
                end
            end else if (i_wdog_trigger == 1'b1) begin
                v.wdog_ena = 1'b0;
                v.err_valid = 1'b1;
                v.err_setcode = CMDERR_NO_RESPONSE;
                v.cmdstate = CMDSTATE_IDLE;
                v.resp_valid = 1'b1;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_TRANSBIT) begin
            // [46](134) transmission bit (R2);
            v_crc7_next = 1'b1;
            if (i_cmd == 1'b0) begin
                v.cmdstate = CMDSTATE_RESP_CMD_MIRROR;
                v.cmdmirror = 6'd0;
                v.cmdbitcnt = 7'd5;
            end else begin
                v.err_valid = 1'b1;
                v.err_setcode = CMDERR_WRONG_RESP_STARTBIT;
                v.cmdstate = CMDSTATE_IDLE;
                v.resp_valid = 1'b1;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_CMD_MIRROR) begin
            // [45:40] [133:128] command index mirrored: 111111 for R2 and R3 (OCR)
            v_crc7_next = 1'b1;
            v.cmdmirror = {r.cmdmirror[4: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                if (r.req_rn == R2) begin
                    v.cmdbitcnt = 7'd119;
                    v.cmdstate = CMDSTATE_RESP_CID_CSD;
                end else begin
                    v.cmdbitcnt = 7'd31;
                    v.cmdstate = CMDSTATE_RESP_REG;
                end
            end
        end else if (r.cmdstate == CMDSTATE_RESP_REG) begin
            // [39:8] Card status (R1), OCR (R3) or RCA (R6) register
            v_crc7_next = 1'b1;
            v.regshift = {r.regshift[30: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.crc_calc = wb_crc7;
                v.cmdbitcnt = 7'd6;
                v.cmdstate = CMDSTATE_RESP_CRC7;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_CID_CSD) begin
            // [127:8] CID or CSD register incl. internal CRC7 R2 response on CMD2 and CMD10 (CID) or CMD9 (CSD)
            v.cidshift = {r.cidshift[118: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.crc_calc = wb_crc7;
                v.cmdbitcnt = 7'd6;
                v.cmdstate = CMDSTATE_RESP_CRC7;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_CRC7) begin
            // [7:1] CRC7: 1111111 for R3 (OCR) no proteection
            v.crc_rx = {r.crc_rx[5: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_RESP_STOPBIT;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_STOPBIT) begin
            // [7:1] End bit
            if (i_cmd == 1'b0) begin
                v.err_valid = 1'b1;
                v.err_setcode = CMDERR_WRONG_RESP_STOPBIT;
            end
            v.cmdstate = CMDSTATE_PAUSE;
            v.cmdbitcnt = 7'd2;
            v.resp_valid = 1'b1;
        end else if (r.cmdstate == CMDSTATE_RESP_SPI_R1) begin
            vb_resp_spistatus[14: 8] = {r.resp_spistatus[13: 8], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                if (r.req_rn == R2) begin
                    v.cmdbitcnt = 7'd7;
                    v.cmdstate = CMDSTATE_RESP_SPI_R2;
                end else if ((r.req_rn == R3) || (r.req_rn == R7)) begin
                    v.cmdbitcnt = 7'd31;
                    v.cmdstate = CMDSTATE_RESP_SPI_DATA;
                end else begin
                    v.cmdstate = CMDSTATE_PAUSE;
                    v.cmdbitcnt = 7'd2;
                    v.resp_valid = 1'b1;
                end
            end
        end else if (r.cmdstate == CMDSTATE_RESP_SPI_R2) begin
            vb_resp_spistatus[7: 0] = {r.resp_spistatus[6: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_PAUSE;
                v.cmdbitcnt = 7'd2;
                v.resp_valid = 1'b1;
            end
        end else if (r.cmdstate == CMDSTATE_RESP_SPI_DATA) begin
            v.regshift = {r.regshift[30: 0], i_cmd};
            if ((|r.cmdbitcnt) == 1'b1) begin
                v.cmdbitcnt = (r.cmdbitcnt - 1);
            end else begin
                v.cmdstate = CMDSTATE_PAUSE;
                v.cmdbitcnt = 7'd2;
                v.resp_valid = 1'b1;
            end
        end
    end
    v.cmdshift = vb_cmdshift;
    v.resp_spistatus = vb_resp_spistatus;

    if ((r.cmdstate < CMDSTATE_RESP_WAIT)
            || (r.cmdstate == CMDSTATE_PAUSE)) begin
        v_crc7_dat = r.cmdshift[39];
    end else begin
        v_crc7_dat = i_cmd;
    end

    if ((~async_reset) && (i_nrst == 1'b0)) begin
        v = sdctrl_cmd_transmitter_r_reset;
    end

    w_crc7_next = v_crc7_next;
    w_crc7_dat = v_crc7_dat;
    o_cmd = r.cmdshift[39];
    o_cmd_dir = r.cmd_dir;
    o_cmd_cs = r.cmd_cs;
    o_req_ready = v_req_ready;
    o_resp_valid = r.resp_valid;
    o_resp_cmd = r.cmdmirror;
    o_resp_reg = r.regshift;
    o_resp_crc7_rx = r.crc_rx;
    o_resp_crc7_calc = r.crc_calc;
    o_resp_spistatus = r.resp_spistatus;
    o_wdog_ena = r.wdog_ena;
    o_err_valid = r.err_valid;
    o_err_setcode = r.err_setcode;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge i_nrst) begin
            if (i_nrst == 1'b0) begin
                r <= sdctrl_cmd_transmitter_r_reset;
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

endmodule: sdctrl_cmd_transmitter
