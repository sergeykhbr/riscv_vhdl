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
    .i_cmd(w_cmd_in),
    .o_cmd(w_cmd_out),
    .o_cmd_dir(w_cmd_dir),
    .o_cmd_req_valid(w_cmd_req_valid),
    .o_cmd_req_cmd(wb_cmd_req_cmd),
    .o_cmd_req_data(wb_cmd_req_data),
    .i_cmd_req_ready(w_cmd_req_ready),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_data32(wb_cmd_resp_data32),
    .o_cmd_resp_ready(w_cmd_resp_ready)
);


vip_sdcard_ctrl #(
    .async_reset(async_reset),
    .CFG_SDCARD_POWERUP_DONE_DELAY(CFG_SDCARD_POWERUP_DONE_DELAY),
    .CFG_SDCARD_VHS(CFG_SDCARD_VHS),
    .CFG_SDCARD_PCIE_1_2V(CFG_SDCARD_PCIE_1_2V),
    .CFG_SDCARD_PCIE_AVAIL(CFG_SDCARD_PCIE_AVAIL),
    .CFG_SDCARD_VDD_VOLTAGE_WINDOW(CFG_SDCARD_VDD_VOLTAGE_WINDOW)
) ctrl0 (
    .i_nrst(i_nrst),
    .i_clk(i_sclk),
    .i_cmd_req_valid(w_cmd_req_valid),
    .i_cmd_req_cmd(wb_cmd_req_cmd),
    .i_cmd_req_data(wb_cmd_req_data),
    .o_cmd_req_ready(w_cmd_req_ready),
    .o_cmd_resp_valid(w_cmd_resp_valid),
    .o_cmd_resp_data32(wb_cmd_resp_data32),
    .i_cmd_resp_ready(w_cmd_resp_ready)
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

    w_dat0_dir = 1'b1;                                      // in:
    w_dat1_dir = 1'b1;                                      // in:
    w_dat2_dir = 1'b1;                                      // in:
    w_dat3_dir = 1'b0;                                      // out: Emulate pull-up CardDetect value

    w_dat0_out = 1'b1;
    w_dat1_out = 1'b1;
    w_dat2_out = 1'b1;
    w_dat3_out = 1'b1;
end: comb_proc

endmodule: vip_sdcard_top
