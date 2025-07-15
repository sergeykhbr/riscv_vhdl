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

#include "apb_i2c_tb.h"
#include "api_core.h"

namespace debugger {

apb_i2c_tb::apb_i2c_tb(sc_module_name name)
    : sc_module(name) {

    clk0 = 0;
    hdmi = 0;
    tt = 0;

    clk0 = new pll_generic("clk0",
                            25.0);
    clk0->o_clk(i_clk);

    hdmi = new vip_i2c_s("hdmi",
                          0);
    hdmi->i_clk(i_clk);
    hdmi->i_nrst(i_nrst);
    hdmi->i_scl(w_o_scl);
    hdmi->i_sda(w_o_sda);
    hdmi->o_sda(w_i_sda);
    hdmi->o_sda_dir(w_hdmi_sda_dir);

    tt = new apb_i2c("tt",
                      0);
    tt->i_clk(i_clk);
    tt->i_nrst(i_nrst);
    tt->i_mapinfo(wb_i_mapinfo);
    tt->o_cfg(wb_o_cfg);
    tt->i_apbi(wb_i_apbi);
    tt->o_apbo(wb_o_apbo);
    tt->o_scl(w_o_scl);
    tt->o_sda(w_o_sda);
    tt->o_sda_dir(w_o_sda_dir);
    tt->i_sda(w_i_sda);
    tt->o_irq(w_o_irq);
    tt->o_nreset(w_o_nreset);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << wb_i_mapinfo;
    sensitive << wb_o_cfg;
    sensitive << wb_i_apbi;
    sensitive << wb_o_apbo;
    sensitive << w_o_scl;
    sensitive << w_o_sda;
    sensitive << w_o_sda_dir;
    sensitive << w_i_sda;
    sensitive << w_o_irq;
    sensitive << w_o_nreset;
    sensitive << w_hdmi_sda_dir;

    SC_METHOD(test);
    sensitive << i_clk.posedge_event();
}

apb_i2c_tb::~apb_i2c_tb() {
    if (clk0) {
        delete clk0;
    }
    if (hdmi) {
        delete hdmi;
    }
    if (tt) {
        delete tt;
    }
}

void apb_i2c_tb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
    }

    if (clk0) {
        clk0->generateVCD(i_vcd, o_vcd);
    }
    if (hdmi) {
        hdmi->generateVCD(i_vcd, o_vcd);
    }
    if (tt) {
        tt->generateVCD(i_vcd, o_vcd);
    }
}

void apb_i2c_tb::comb() {
    mapinfo_type vb_mapinfo;

    vb_mapinfo.addr_start = 0x000008000000;
    vb_mapinfo.addr_end = 0x000008001000;
    wb_i_mapinfo = vb_mapinfo;
}

void apb_i2c_tb::test() {
    apb_in_type vb_pslvi;

    wb_clk_cnt = (wb_clk_cnt + 1);
    if (wb_clk_cnt < 10) {
        i_nrst = 0;
    } else {
        i_nrst = 1;
    }

    vb_pslvi = apb_in_none;
    vb_pslvi.penable = wb_i_apbi.read().penable;
    if (wb_o_apbo.read().pready == 1) {
        vb_pslvi.penable = 0;
    }

    switch (wb_clk_cnt) {
    case 20:
        vb_pslvi.paddr = 0x00000000;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 1;
        vb_pslvi.pwdata = 0x00320064;
        vb_pslvi.pstrb = 0xF;
        break;
    case 30:                                                // De-assert nreset signal
        vb_pslvi.paddr = 0x00000004;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 1;
        vb_pslvi.pwdata = 0x00010000;                       // [16] set HIGH nreset
        vb_pslvi.pstrb = 0xF;
        break;
        // Write I2C payload
    case 40:
        vb_pslvi.paddr = 0x0000000C;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 1;
        vb_pslvi.pwdata = 0x0000C020;                       // [7:0]Select channel 5 (HDMI)
        vb_pslvi.pstrb = 0xF;
        break;
        // Start write sequence
    case 50:
        vb_pslvi.paddr = 0x00000008;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 1;
        vb_pslvi.pwdata = 0x00020074;                       // [31]0=write, [19:16]byte_cnt,[6:0]addr
        vb_pslvi.pstrb = 0xF;
        break;
        // Start Read sequence
    case 10000:
        vb_pslvi.paddr = 0x00000008;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 1;
        vb_pslvi.pwdata = 0x80020074;                       // [31]1=read, [19:16]byte_cnt,[6:0]addr
        vb_pslvi.pstrb = 0xF;
        break;
    case 18000:                                             // Read payload through APB
        vb_pslvi.paddr = 0x0000000C;
        vb_pslvi.pprot = 0;
        vb_pslvi.pselx = 1;
        vb_pslvi.penable = 1;
        vb_pslvi.pwrite = 0;
        vb_pslvi.pwdata = 0;
        vb_pslvi.pstrb = 0;
        break;

    default:
        break;
    }

    wb_i_apbi = vb_pslvi;
}

}  // namespace debugger

