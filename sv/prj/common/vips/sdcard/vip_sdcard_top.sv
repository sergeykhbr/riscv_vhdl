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

module vip_sdcard_top #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_nrst,                                     // To avoid undefined states of registers (xxx)
    input logic i_sclk,
    inout logic io_cmd,
    inout logic io_dat0,
    inout logic io_dat1,
    inout logic io_dat2,
    inout logic io_cd_dat3
);

import vip_sdcard_top_pkg::*;

logic w_clk;
logic [7:0] wb_rdata;
logic w_spi_mode;
logic w_cmd_in;
logic w_cmd_out;
logic w_cmd_dir;
logic w_dat0_in;
logic w_dat1_in;
logic w_dat2_in;
logic w_dat3_in;
logic w_dat0_out;
logic w_dat1_out;
logic w_dat2_out;
logic w_dat3_out;
logic w_dat0_dir;
logic w_dat1_dir;
logic w_dat2_dir;
logic w_dat3_dir;
logic w_cmd_req_valid;
logic [5:0] wb_cmd_req_cmd;
logic [31:0] wb_cmd_req_data;
logic w_cmd_req_ready;
logic w_cmd_resp_valid;
logic [31:0] wb_cmd_resp_data32;
logic w_cmd_resp_ready;
logic w_cmd_resp_r1b;
logic w_cmd_resp_r2;
logic w_cmd_resp_r3;
logic w_cmd_resp_r7;
logic w_cmdio_cmd_dir;
logic w_cmdio_cmd_out;
// Status signals:
logic w_stat_idle_state;
logic w_stat_erase_reset;
logic w_stat_illegal_cmd;
logic w_stat_err_erase_sequence;
logic w_stat_err_address;
logic w_stat_err_parameter;
logic w_stat_locked;
logic w_stat_wp_erase_skip;
logic w_stat_err;
logic w_stat_err_cc;
logic w_stat_ecc_failed;
logic w_stat_wp_violation;
logic w_stat_erase_param;
logic w_stat_out_of_range;
logic [40:0] wb_mem_addr;
logic [7:0] wb_mem_rdata;
logic w_crc15_clear;
logic w_crc15_next;
logic [15:0] wb_crc16;
logic w_dat_trans;
logic [3:0] wb_dat;
logic w_cmdio_busy;

iobuf_tech iobufcmd0 (
    .io(io_cmd),
    .o(w_cmd_in),
    .i(w_cmd_out),
    .t(w_cmd_dir)
);


iobuf_tech iobufdat0 (
    .io(io_dat0),
    .o(w_dat0_in),
    .i(w_dat0_out),
    .t(w_dat0_dir)
);


iobuf_tech iobufdat1 (
    .io(io_dat1),
    .o(w_dat1_in),
    .i(w_dat1_out),
    .t(w_dat1_dir)
);


iobuf_tech iobufdat2 (
    .io(io_dat2),
    .o(w_dat2_in),
    .i(w_dat2_out),
    .t(w_dat2_dir)
);


iobuf_tech iobufdat3 (
    .io(io_cd_dat3),
    .o(w_dat3_in),
    .i(w_dat3_out),
    .t(w_dat3_dir)
);


vip_sdcard_cmdio #(
    .async_reset(async_reset)
) cmdio0 (
    .i_nrst(i_nrst),
    .i_clk(i_sclk),
    .i_cs(w_dat3_in),
    .o_spi_mode(w_spi_mode),
    .i_cmd(w_cmd_in),
    .o_cmd(w_cmdio_cmd_out),
    .o_cmd_dir(w_cmdio_cmd_dir),
    .o_cmd_req_valid(w_cmd_req_valid),
    .o_cmd_req_cmd(wb_cmd_req_cmd),
    .o_cmd_req_data(wb_cmd_req_data),
    .i_cmd_req_ready(w_cmd_req_ready),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_data32(wb_cmd_resp_data32),
    .o_cmd_resp_ready(w_cmd_resp_ready),
    .i_cmd_resp_r1b(w_cmd_resp_r1b),
    .i_cmd_resp_r2(w_cmd_resp_r2),
    .i_cmd_resp_r3(w_cmd_resp_r3),
    .i_cmd_resp_r7(w_cmd_resp_r7),
    .i_stat_idle_state(w_stat_idle_state),
    .i_stat_erase_reset(w_stat_erase_reset),
    .i_stat_illegal_cmd(w_stat_illegal_cmd),
    .i_stat_err_erase_sequence(w_stat_err_erase_sequence),
    .i_stat_err_address(w_stat_err_address),
    .i_stat_err_parameter(w_stat_err_parameter),
    .i_stat_locked(w_stat_locked),
    .i_stat_wp_erase_skip(w_stat_wp_erase_skip),
    .i_stat_err(w_stat_err),
    .i_stat_err_cc(w_stat_err_cc),
    .i_stat_ecc_failed(w_stat_ecc_failed),
    .i_stat_wp_violation(w_stat_wp_violation),
    .i_stat_erase_param(w_stat_erase_param),
    .i_stat_out_of_range(w_stat_out_of_range),
    .o_busy(w_cmdio_busy)
);


vip_sdcard_ctrl #(
    .async_reset(async_reset),
    .CFG_SDCARD_POWERUP_DONE_DELAY(CFG_SDCARD_POWERUP_DONE_DELAY),
    .CFG_SDCARD_HCS(CFG_SDCARD_HCS),
    .CFG_SDCARD_VHS(CFG_SDCARD_VHS),
    .CFG_SDCARD_PCIE_1_2V(CFG_SDCARD_PCIE_1_2V),
    .CFG_SDCARD_PCIE_AVAIL(CFG_SDCARD_PCIE_AVAIL),
    .CFG_SDCARD_VDD_VOLTAGE_WINDOW(CFG_SDCARD_VDD_VOLTAGE_WINDOW)
) ctrl0 (
    .i_nrst(i_nrst),
    .i_clk(i_sclk),
    .i_spi_mode(w_spi_mode),
    .i_cs(w_dat3_in),
    .i_cmd_req_valid(w_cmd_req_valid),
    .i_cmd_req_cmd(wb_cmd_req_cmd),
    .i_cmd_req_data(wb_cmd_req_data),
    .o_cmd_req_ready(w_cmd_req_ready),
    .o_cmd_resp_valid(w_cmd_resp_valid),
    .o_cmd_resp_data32(wb_cmd_resp_data32),
    .i_cmd_resp_ready(w_cmd_resp_ready),
    .o_cmd_resp_r1b(w_cmd_resp_r1b),
    .o_cmd_resp_r2(w_cmd_resp_r2),
    .o_cmd_resp_r3(w_cmd_resp_r3),
    .o_cmd_resp_r7(w_cmd_resp_r7),
    .o_stat_idle_state(w_stat_idle_state),
    .o_stat_illegal_cmd(w_stat_illegal_cmd),
    .o_mem_addr(wb_mem_addr),
    .i_mem_rdata(wb_mem_rdata),
    .o_crc16_clear(w_crc15_clear),
    .o_crc16_next(w_crc15_next),
    .i_crc16(wb_crc16),
    .o_dat_trans(w_dat_trans),
    .o_dat(wb_dat),
    .i_cmdio_busy(w_cmdio_busy)
);


always_comb
begin: comb_proc
    logic [47:0] vb_cmd_txshift;
    logic v_crc7_clear;
    logic v_crc7_next;
    logic v_crc7_in;

    vb_cmd_txshift = 0;
    v_crc7_clear = 0;
    v_crc7_next = 0;
    v_crc7_in = 0;

    if (w_spi_mode == 1'b1) begin
        w_cmd_dir = 1'b1;                                   // in: din
        w_dat0_dir = 1'b0;                                  // out: dout
        w_dat1_dir = 1'b1;                                  // in: reserved
        w_dat2_dir = 1'b1;                                  // in: reserved
        w_dat3_dir = 1'b1;                                  // in: cs

        w_dat0_out = (((~w_dat_trans) & w_cmdio_cmd_out)
                | (w_dat_trans & wb_dat[3]));
        w_dat1_out = 1'b1;
        w_dat2_out = 1'b1;
        w_dat3_out = 1'b1;
    end else begin
        w_cmd_dir = w_cmdio_cmd_dir;
        w_dat0_dir = 1'b1;                                  // in:
        w_dat1_dir = 1'b1;                                  // in:
        w_dat2_dir = 1'b1;                                  // in:
        w_dat3_dir = 1'b1;                                  // in:

        w_cmd_out = w_cmdio_cmd_out;
        w_dat0_out = wb_dat[0];
        w_dat1_out = wb_dat[1];
        w_dat2_out = wb_dat[2];
        w_dat3_out = wb_dat[3];
    end

    // Not implemented yet:
    w_stat_erase_reset = 1'b0;
    w_stat_err_erase_sequence = 1'b0;
    w_stat_err_address = 1'b0;
    w_stat_err_parameter = 1'b0;
    w_stat_locked = 1'b0;
    w_stat_wp_erase_skip = 1'b0;
    w_stat_err = 1'b0;
    w_stat_err_cc = 1'b0;
    w_stat_ecc_failed = 1'b0;
    w_stat_wp_violation = 1'b0;
    w_stat_erase_param = 1'b0;
    w_stat_out_of_range = 1'b0;
    wb_mem_rdata = 8'hff;
    wb_crc16 = 16'h7fa1;
end: comb_proc

endmodule: vip_sdcard_top
