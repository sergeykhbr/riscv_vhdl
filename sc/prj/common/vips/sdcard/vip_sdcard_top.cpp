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

#include "vip_sdcard_top.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_top::vip_sdcard_top(sc_module_name name,
                               bool async_reset)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_sclk("i_sclk"),
    io_cmd("io_cmd"),
    io_dat0("io_dat0"),
    io_dat1("io_dat1"),
    io_dat2("io_dat2"),
    io_cd_dat3("io_cd_dat3") {

    async_reset_ = async_reset;
    iobufcmd0 = 0;
    iobufdat0 = 0;
    iobufdat1 = 0;
    iobufdat2 = 0;
    iobufdat3 = 0;
    cmdio0 = 0;
    ctrl0 = 0;

    iobufcmd0 = new iobuf_tech("iobufcmd0");
    iobufcmd0->io(io_cmd);
    iobufcmd0->o(w_cmd_in);
    iobufcmd0->i(w_cmd_out);
    iobufcmd0->t(w_cmd_dir);


    iobufdat0 = new iobuf_tech("iobufdat0");
    iobufdat0->io(io_dat0);
    iobufdat0->o(w_dat0_in);
    iobufdat0->i(w_dat0_out);
    iobufdat0->t(w_dat0_dir);


    iobufdat1 = new iobuf_tech("iobufdat1");
    iobufdat1->io(io_dat1);
    iobufdat1->o(w_dat1_in);
    iobufdat1->i(w_dat1_out);
    iobufdat1->t(w_dat1_dir);


    iobufdat2 = new iobuf_tech("iobufdat2");
    iobufdat2->io(io_dat2);
    iobufdat2->o(w_dat2_in);
    iobufdat2->i(w_dat2_out);
    iobufdat2->t(w_dat2_dir);


    iobufdat3 = new iobuf_tech("iobufdat3");
    iobufdat3->io(io_cd_dat3);
    iobufdat3->o(w_dat3_in);
    iobufdat3->i(w_dat3_out);
    iobufdat3->t(w_dat3_dir);


    cmdio0 = new vip_sdcard_cmdio("cmdio0", async_reset);
    cmdio0->i_nrst(i_nrst);
    cmdio0->i_clk(i_sclk);
    cmdio0->i_cmd(w_cmd_in);
    cmdio0->o_cmd(w_cmd_out);
    cmdio0->o_cmd_dir(w_cmd_dir);
    cmdio0->o_cmd_req_valid(w_cmd_req_valid);
    cmdio0->o_cmd_req_cmd(wb_cmd_req_cmd);
    cmdio0->o_cmd_req_data(wb_cmd_req_data);
    cmdio0->i_cmd_req_ready(w_cmd_req_ready);
    cmdio0->i_cmd_resp_valid(w_cmd_resp_valid);
    cmdio0->i_cmd_resp_data32(wb_cmd_resp_data32);
    cmdio0->o_cmd_resp_ready(w_cmd_resp_ready);


    ctrl0 = new vip_sdcard_ctrl("ctrl0", async_reset,
                                 CFG_SDCARD_POWERUP_DONE_DELAY,
                                 CFG_SDCARD_VHS,
                                 CFG_SDCARD_PCIE_1_2V,
                                 CFG_SDCARD_PCIE_AVAIL,
                                 CFG_SDCARD_VDD_VOLTAGE_WINDOW);
    ctrl0->i_nrst(i_nrst);
    ctrl0->i_clk(i_sclk);
    ctrl0->i_cmd_req_valid(w_cmd_req_valid);
    ctrl0->i_cmd_req_cmd(wb_cmd_req_cmd);
    ctrl0->i_cmd_req_data(wb_cmd_req_data);
    ctrl0->o_cmd_req_ready(w_cmd_req_ready);
    ctrl0->o_cmd_resp_valid(w_cmd_resp_valid);
    ctrl0->o_cmd_resp_data32(wb_cmd_resp_data32);
    ctrl0->i_cmd_resp_ready(w_cmd_resp_ready);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sclk;
    sensitive << io_cmd;
    sensitive << io_dat0;
    sensitive << io_dat1;
    sensitive << io_dat2;
    sensitive << io_cd_dat3;
    sensitive << w_clk;
    sensitive << wb_rdata;
    sensitive << w_cmd_in;
    sensitive << w_cmd_out;
    sensitive << w_cmd_dir;
    sensitive << w_dat0_in;
    sensitive << w_dat1_in;
    sensitive << w_dat2_in;
    sensitive << w_dat3_in;
    sensitive << w_dat0_out;
    sensitive << w_dat1_out;
    sensitive << w_dat2_out;
    sensitive << w_dat3_out;
    sensitive << w_dat0_dir;
    sensitive << w_dat1_dir;
    sensitive << w_dat2_dir;
    sensitive << w_dat3_dir;
    sensitive << w_cmd_req_valid;
    sensitive << wb_cmd_req_cmd;
    sensitive << wb_cmd_req_data;
    sensitive << w_cmd_req_ready;
    sensitive << w_cmd_resp_valid;
    sensitive << wb_cmd_resp_data32;
    sensitive << w_cmd_resp_ready;
}

vip_sdcard_top::~vip_sdcard_top() {
    if (iobufcmd0) {
        delete iobufcmd0;
    }
    if (iobufdat0) {
        delete iobufdat0;
    }
    if (iobufdat1) {
        delete iobufdat1;
    }
    if (iobufdat2) {
        delete iobufdat2;
    }
    if (iobufdat3) {
        delete iobufdat3;
    }
    if (cmdio0) {
        delete cmdio0;
    }
    if (ctrl0) {
        delete ctrl0;
    }
}

void vip_sdcard_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_sclk, i_sclk.name());
        sc_trace(o_vcd, io_cmd, io_cmd.name());
        sc_trace(o_vcd, io_dat0, io_dat0.name());
        sc_trace(o_vcd, io_dat1, io_dat1.name());
        sc_trace(o_vcd, io_dat2, io_dat2.name());
        sc_trace(o_vcd, io_cd_dat3, io_cd_dat3.name());
    }

    if (iobufcmd0) {
        iobufcmd0->generateVCD(i_vcd, o_vcd);
    }
    if (iobufdat0) {
        iobufdat0->generateVCD(i_vcd, o_vcd);
    }
    if (iobufdat1) {
        iobufdat1->generateVCD(i_vcd, o_vcd);
    }
    if (iobufdat2) {
        iobufdat2->generateVCD(i_vcd, o_vcd);
    }
    if (iobufdat3) {
        iobufdat3->generateVCD(i_vcd, o_vcd);
    }
    if (cmdio0) {
        cmdio0->generateVCD(i_vcd, o_vcd);
    }
    if (ctrl0) {
        ctrl0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_sdcard_top::comb() {
    sc_uint<48> vb_cmd_txshift;
    bool v_crc7_clear;
    bool v_crc7_next;
    bool v_crc7_in;

    vb_cmd_txshift = 0;
    v_crc7_clear = 0;
    v_crc7_next = 0;
    v_crc7_in = 0;

    w_dat0_dir = 1;                                         // in:
    w_dat1_dir = 1;                                         // in:
    w_dat2_dir = 1;                                         // in:
    w_dat3_dir = 0;                                         // out: Emulate pull-up CardDetect value

    w_dat0_out = 1;
    w_dat1_out = 1;
    w_dat2_out = 1;
    w_dat3_out = 1;
}

}  // namespace debugger

