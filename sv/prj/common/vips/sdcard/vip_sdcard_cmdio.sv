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

module vip_sdcard_cmdio #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_cs,                                       // dat3 in SPI mode.
    output logic o_spi_mode,                                // Detected SPI mode on CMD0
    input logic i_cmd,
    output logic o_cmd,
    output logic o_cmd_dir,
    output logic o_cmd_req_valid,
    output logic [5:0] o_cmd_req_cmd,
    output logic [31:0] o_cmd_req_data,
    input logic i_cmd_req_ready,
    input logic i_cmd_resp_valid,
    input logic [31:0] i_cmd_resp_data32,
    output logic o_cmd_resp_ready,
    input logic i_cmd_resp_r1b,                             // Same as R1 with zero line (any number of bits) card is busy, non-zero is ready
    input logic i_cmd_resp_r2,                              // 2-Bytes status
    input logic i_cmd_resp_r3,                              // Read OCR 32 bits
    input logic i_cmd_resp_r7,                              // CMD8 interface condition response
    input logic i_stat_idle_state,                          // Card in idle state and running initialization process
    input logic i_stat_erase_reset,                         // Erase sequence was cleared before executing
    input logic i_stat_illegal_cmd,                         // Illegal command was detected
    input logic i_stat_err_erase_sequence,                  // An error ini the sequence of erase commands occured
    input logic i_stat_err_address,                         // A misaligned adddres that didnot mathc block length
    input logic i_stat_err_parameter,                       // The command argument was otuside the allows range
    input logic i_stat_locked,                              // Is set when card is locked by the user
    input logic i_stat_wp_erase_skip,                       // Erase wp-sector or password error during card lock/unlock
    input logic i_stat_err,                                 // A general or an unknown error occured during operation
    input logic i_stat_err_cc,                              // Internal card controller error
    input logic i_stat_ecc_failed,                          // Card internal ECC eas applied but failed to correct data
    input logic i_stat_wp_violation,                        // The command tried to write wp block
    input logic i_stat_erase_param,                         // An invalid selection for erase, sectors or groups
    input logic i_stat_out_of_range,
    output logic o_busy
);

import vip_sdcard_cmdio_pkg::*;

logic w_cmd_out;
logic w_crc7_clear;
logic w_crc7_next;
logic w_crc7_dat;
logic [6:0] wb_crc7;
vip_sdcard_cmdio_registers r, rin;

vip_sdcard_crc7 #(
    .async_reset(async_reset)
) crccmd0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc7_clear),
    .i_next(w_crc7_next),
    .i_dat(w_crc7_dat),
    .o_crc7(wb_crc7)
);


always_comb
begin: comb_proc
    vip_sdcard_cmdio_registers v;
    logic [47:0] vb_cmd_txshift;
    logic v_crc7_clear;
    logic v_crc7_next;
    logic v_crc7_in;
    logic v_busy;

    vb_cmd_txshift = '0;
    v_crc7_clear = 1'b0;
    v_crc7_next = 1'b0;
    v_crc7_in = 1'b0;
    v_busy = 1'b1;

    v = r;

    vb_cmd_txshift = {r.cmd_txshift[46: 0], 1'b1};
    v_crc7_in = i_cmd;

    if (i_cmd_req_ready == 1'b1) begin
        v.cmd_req_valid = 1'b0;
    end
    v.clkcnt = (r.clkcnt + 1);

    case (r.cmd_state)
    CMDSTATE_INIT: begin
        v.spi_mode = 1'b0;
        v.cmd_dir = 1'b1;
        v_crc7_clear = 1'b1;
        v.cmd_req_crc_err = 1'b0;
        // Wait several (72) clocks to switch into idle state
        if (r.clkcnt == 70) begin
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
        end
    end
    CMDSTATE_REQ_STARTBIT: begin
        v_busy = 1'b0;
        v_crc7_clear = 1'b1;
        if ((r.spi_mode && i_cs) == 1'b1) begin
            // Do nothing
        end else if ((r.cmdz == 1'b1) && (i_cmd == 1'b0)) begin
            v.cs = i_cs;
            v.cmd_state = CMDSTATE_REQ_TXBIT;
        end
    end
    CMDSTATE_REQ_TXBIT: begin
        v.cmd_state = CMDSTATE_REQ_CMD;
        v.bitcnt = 6'd5;
        v_crc7_next = 1'b1;
        v.txbit = i_cmd;
    end
    CMDSTATE_REQ_CMD: begin
        v_crc7_next = 1'b1;
        if ((|r.bitcnt) == 1'b0) begin
            v.bitcnt = 6'd31;
            v.cmd_state = CMDSTATE_REQ_ARG;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_REQ_ARG: begin
        v_crc7_next = 1'b1;
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_REQ_CRC7;
            v.bitcnt = 6'd6;
            v.crc_calc = wb_crc7;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_REQ_CRC7: begin
        v.crc_rx = {r.crc_rx[5: 0], i_cmd};
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_REQ_STOPBIT;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_REQ_STOPBIT: begin
        v.cmd_state = CMDSTATE_REQ_VALID;
        v.cmd_dir = 1'b0;
        v_crc7_clear = 1'b1;
    end
    CMDSTATE_REQ_VALID: begin
        if (r.crc_calc != r.crc_rx) begin
            v.cmd_req_crc_err = 1'b1;
        end
        if (r.txbit == 1'b1) begin
            v.cmd_state = CMDSTATE_WAIT_RESP;
            v.cmd_req_valid = 1'b1;
            if (((|r.cmd_rxshift[45: 40]) == 1'b0) && (r.cs == 1'b0)) begin
                // CMD0 with CS = 0 (CD_DAT3)
                v.spi_mode = 1'b1;
            end
        end else begin
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
            v.cmd_dir = 1'b1;
        end
        v.cmd_req_cmd = r.cmd_rxshift[45: 40];
        v.cmd_req_data = r.cmd_rxshift[39: 8];
    end
    CMDSTATE_WAIT_RESP: begin
        v.cmd_resp_ready = 1'b1;
        if (i_cmd_resp_valid == 1'b1) begin
            v.cmd_resp_ready = 1'b0;
            v.cmd_state = CMDSTATE_RESP;
            if (r.spi_mode == 1'b0) begin
                v.bitcnt = 6'd39;
                vb_cmd_txshift = 48'd0;
                vb_cmd_txshift[45: 40] = r.cmd_rxshift[45: 40];
                vb_cmd_txshift[39: 8] = i_cmd_resp_data32;
                vb_cmd_txshift[7: 0] = 8'hff;
            end else begin
                // Default R1 response in SPI mode:
                v.bitcnt = 6'd7;
                vb_cmd_txshift = '1;
                vb_cmd_txshift[47] = 1'b0;
                vb_cmd_txshift[46] = i_stat_err_parameter;
                vb_cmd_txshift[45] = i_stat_err_address;
                vb_cmd_txshift[44] = i_stat_err_erase_sequence;
                vb_cmd_txshift[43] = r.cmd_req_crc_err;
                vb_cmd_txshift[42] = i_stat_illegal_cmd;
                vb_cmd_txshift[41] = i_stat_erase_reset;
                vb_cmd_txshift[40] = i_stat_idle_state;
                if (i_cmd_resp_r2 == 1'b1) begin
                    v.bitcnt = 6'd15;
                    vb_cmd_txshift[39] = i_stat_out_of_range;
                    vb_cmd_txshift[38] = i_stat_erase_param;
                    vb_cmd_txshift[37] = i_stat_wp_violation;
                    vb_cmd_txshift[36] = i_stat_ecc_failed;
                    vb_cmd_txshift[35] = i_stat_err_cc;
                    vb_cmd_txshift[34] = i_stat_err;
                    vb_cmd_txshift[33] = i_stat_wp_erase_skip;
                    vb_cmd_txshift[32] = i_stat_locked;
                end else if ((i_cmd_resp_r3 == 1'b1) || (i_cmd_resp_r7 == 1'b1)) begin
                    v.bitcnt = 6'd39;
                    vb_cmd_txshift[39: 8] = i_cmd_resp_data32;
                end
            end
        end
    end
    CMDSTATE_RESP: begin
        v_crc7_in = r.cmd_txshift[47];
        if ((|r.bitcnt) == 1'b0) begin
            if (r.spi_mode == 1'b0) begin
                v.bitcnt = 6'd6;
                v.cmd_state = CMDSTATE_RESP_CRC7;
                vb_cmd_txshift[47: 40] = {wb_crc7, 1'b1};
                v.crc_calc = wb_crc7;
            end else begin
                v.cmd_state = CMDSTATE_RESP_STOPBIT;
            end
        end else begin
            v_crc7_next = 1'b1;
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_RESP_CRC7: begin
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_RESP_STOPBIT;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_RESP_STOPBIT: begin
        v.cmd_state = CMDSTATE_REQ_STARTBIT;
        v.cmd_dir = 1'b1;
        v_crc7_clear = 1'b1;
    end
    default: begin
    end
    endcase

    if (r.cmd_state < CMDSTATE_REQ_VALID) begin
        v.cmd_rxshift = {r.cmd_rxshift[46: 0], i_cmd};
        v.cmd_txshift = '1;
    end else begin
        if ((r.cmd_state == CMDSTATE_RESP_STOPBIT) && ((|r.bitcnt) == 1'b0)) begin
            v.cmd_rxshift = '1;
        end
        v.cmd_txshift = vb_cmd_txshift;
    end

    if (r.cmd_dir == 1'b0) begin
        // Output:
        v.cmdz = r.cmd_txshift[47];
    end else begin
        // Input:
        v.cmdz = i_cmd;
    end

    w_crc7_clear = v_crc7_clear;
    w_crc7_next = v_crc7_next;
    w_crc7_dat = v_crc7_in;
    o_cmd = r.cmd_txshift[47];
    o_cmd_dir = r.cmd_dir;
    o_cmd_req_valid = r.cmd_req_valid;
    o_cmd_req_cmd = r.cmd_req_cmd;
    o_cmd_req_data = r.cmd_req_data;
    o_cmd_resp_ready = r.cmd_resp_ready;
    o_spi_mode = r.spi_mode;
    o_busy = v_busy;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_sdcard_cmdio_r_reset;
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

endmodule: vip_sdcard_cmdio
