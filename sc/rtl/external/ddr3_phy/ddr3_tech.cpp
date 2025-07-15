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

#include "ddr3_tech.h"
#include "api_core.h"

namespace debugger {

ddr3_tech::ddr3_tech(sc_module_name name)
    : sc_module(name),
    i_apb_nrst("i_apb_nrst"),
    i_apb_clk("i_apb_clk"),
    i_xslv_nrst("i_xslv_nrst"),
    i_xslv_clk("i_xslv_clk"),
    i_xmapinfo("i_xmapinfo"),
    o_xcfg("o_xcfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_pmapinfo("i_pmapinfo"),
    o_pcfg("o_pcfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_ui_nrst("o_ui_nrst"),
    o_ui_clk("o_ui_clk"),
    o_init_calib_done("o_init_calib_done") {

    clk0 = 0;
    pctrl0 = 0;
    sram0 = 0;

    clk0 = new pll_generic("clk0",
                            5.0);
    clk0->o_clk(w_ui_clk);

    pctrl0 = new apb_ddr("pctrl0",
                          0);
    pctrl0->i_clk(i_apb_clk);
    pctrl0->i_nrst(i_apb_nrst);
    pctrl0->i_mapinfo(i_pmapinfo);
    pctrl0->o_cfg(o_pcfg);
    pctrl0->i_apbi(i_apbi);
    pctrl0->o_apbo(o_apbo);
    pctrl0->i_pll_locked(w_ui_nrst);
    pctrl0->i_init_calib_done(w_init_calib_done);
    pctrl0->i_device_temp(wb_device_temp);
    pctrl0->i_sr_active(w_sr_active);
    pctrl0->i_ref_ack(w_ref_ack);
    pctrl0->i_zq_ack(w_zq_ack);

    sram0 = new axi_sram<12>("sram0",
                             0);
    sram0->i_clk(w_ui_clk);
    sram0->i_nrst(w_ui_nrst);
    sram0->i_mapinfo(i_xmapinfo);
    sram0->o_cfg(wb_xcfg_unused);
    sram0->i_xslvi(i_xslvi);
    sram0->o_xslvo(o_xslvo);

    SC_THREAD(init);

    SC_METHOD(comb);
    sensitive << i_apb_nrst;
    sensitive << i_apb_clk;
    sensitive << i_xslv_nrst;
    sensitive << i_xslv_clk;
    sensitive << i_xmapinfo;
    sensitive << i_xslvi;
    sensitive << i_pmapinfo;
    sensitive << i_apbi;
    sensitive << w_ui_nrst;
    sensitive << w_ui_clk;
    sensitive << w_init_calib_done;
    sensitive << wb_device_temp;
    sensitive << w_sr_active;
    sensitive << w_ref_ack;
    sensitive << w_zq_ack;
    sensitive << wb_xcfg_unused;
    sensitive << r.ddr_calib;

    SC_METHOD(registers);
    sensitive << i_xslv_nrst;
    sensitive << i_xslv_clk.pos();
}

ddr3_tech::~ddr3_tech() {
    if (clk0) {
        delete clk0;
    }
    if (pctrl0) {
        delete pctrl0;
    }
    if (sram0) {
        delete sram0;
    }
}

void ddr3_tech::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_apb_nrst, i_apb_nrst.name());
        sc_trace(o_vcd, i_apb_clk, i_apb_clk.name());
        sc_trace(o_vcd, i_xslv_nrst, i_xslv_nrst.name());
        sc_trace(o_vcd, i_xslv_clk, i_xslv_clk.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_ui_nrst, o_ui_nrst.name());
        sc_trace(o_vcd, o_ui_clk, o_ui_clk.name());
        sc_trace(o_vcd, o_init_calib_done, o_init_calib_done.name());
        sc_trace(o_vcd, r.ddr_calib, pn + ".r.ddr_calib");
    }

    if (clk0) {
        clk0->generateVCD(i_vcd, o_vcd);
    }
    if (pctrl0) {
        pctrl0->generateVCD(i_vcd, o_vcd);
    }
    if (sram0) {
        sram0->generateVCD(i_vcd, o_vcd);
    }
}

void ddr3_tech::init() {
    w_ui_nrst = 0;
    wait(static_cast<int>(250.0), SC_NS);
    w_ui_nrst = 1;
}

void ddr3_tech::comb() {
    v = r;

    if (r.ddr_calib.read()[7] == 0) {
        v.ddr_calib = (r.ddr_calib.read() + 1);
    }

    w_init_calib_done = r.ddr_calib.read()[7];
    o_ui_nrst = w_ui_nrst.read();
    o_ui_clk = w_ui_clk.read();
    o_init_calib_done = w_init_calib_done.read();
}

void ddr3_tech::registers() {
    if (i_xslv_nrst.read() == 0) {
        ddr3_tech_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

