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

#include "apb_prci.h"
#include "api_core.h"

namespace debugger {

apb_prci::apb_prci(sc_module_name name,
                   bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_pwrreset("i_pwrreset"),
    i_dmireset("i_dmireset"),
    i_sys_locked("i_sys_locked"),
    i_ddr_locked("i_ddr_locked"),
    i_pcie_phy_rst("i_pcie_phy_rst"),
    i_pcie_phy_clk("i_pcie_phy_clk"),
    i_pcie_phy_lnk_up("i_pcie_phy_lnk_up"),
    o_sys_rst("o_sys_rst"),
    o_sys_nrst("o_sys_nrst"),
    o_dbg_nrst("o_dbg_nrst"),
    o_pcie_nrst("o_pcie_nrst"),
    o_hdmi_nrst("o_hdmi_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_PRCI);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(r_sys_nrst);
    pslv0->i_mapinfo(i_mapinfo);
    pslv0->o_cfg(o_cfg);
    pslv0->i_apbi(i_apbi);
    pslv0->o_apbo(o_apbo);
    pslv0->o_req_valid(w_req_valid);
    pslv0->o_req_addr(wb_req_addr);
    pslv0->o_req_write(w_req_write);
    pslv0->o_req_wdata(wb_req_wdata);
    pslv0->i_resp_valid(r.resp_valid);
    pslv0->i_resp_rdata(r.resp_rdata);
    pslv0->i_resp_err(r.resp_err);

    SC_METHOD(comb);
    sensitive << i_pwrreset;
    sensitive << i_dmireset;
    sensitive << i_sys_locked;
    sensitive << i_ddr_locked;
    sensitive << i_pcie_phy_rst;
    sensitive << i_pcie_phy_clk;
    sensitive << i_pcie_phy_lnk_up;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r_sys_nrst;
    sensitive << r_dbg_nrst;
    sensitive << rb_pcie_nrst;
    sensitive << rb_hdmi_nrst;
    sensitive << r_sys_locked;
    sensitive << rb_ddr_locked;
    sensitive << rb_pcie_lnk_up;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(reqff);
    sensitive << i_pwrreset;
    sensitive << i_clk.pos();

    SC_METHOD(registers);
    sensitive << r_sys_nrst;
    sensitive << i_clk.pos();
}

apb_prci::~apb_prci() {
    if (pslv0) {
        delete pslv0;
    }
}

void apb_prci::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_pwrreset, i_pwrreset.name());
        sc_trace(o_vcd, i_dmireset, i_dmireset.name());
        sc_trace(o_vcd, i_sys_locked, i_sys_locked.name());
        sc_trace(o_vcd, i_ddr_locked, i_ddr_locked.name());
        sc_trace(o_vcd, i_pcie_phy_rst, i_pcie_phy_rst.name());
        sc_trace(o_vcd, i_pcie_phy_clk, i_pcie_phy_clk.name());
        sc_trace(o_vcd, i_pcie_phy_lnk_up, i_pcie_phy_lnk_up.name());
        sc_trace(o_vcd, o_sys_rst, o_sys_rst.name());
        sc_trace(o_vcd, o_sys_nrst, o_sys_nrst.name());
        sc_trace(o_vcd, o_dbg_nrst, o_dbg_nrst.name());
        sc_trace(o_vcd, o_pcie_nrst, o_pcie_nrst.name());
        sc_trace(o_vcd, o_hdmi_nrst, o_hdmi_nrst.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r.resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r.resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void apb_prci::comb() {
    sc_uint<32> vb_rdata;

    v = r;
    vb_rdata = 0;


    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: pll statuses
        vb_rdata[0] = r_sys_locked.read();
        vb_rdata[1] = rb_ddr_locked.read()[1];
        vb_rdata[2] = rb_pcie_lnk_up.read()[1];
        break;
    case 1:                                                 // 0x04: reset status
        vb_rdata[0] = r_sys_nrst.read();
        vb_rdata[1] = r_dbg_nrst.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                // todo:
            }
        }
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid.read();
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if ((!async_reset_) && (r_sys_nrst.read() == 0)) {
        apb_prci_r_reset(v);
    }

    o_sys_rst = r_sys_rst;
    o_sys_nrst = r_sys_nrst.read();
    o_dbg_nrst = r_dbg_nrst.read();
    o_pcie_nrst = rb_pcie_nrst.read()[1];
    o_hdmi_nrst = rb_hdmi_nrst.read()[1];
}

void apb_prci::reqff() {
    if (i_pwrreset.read() == 1) {
        r_sys_locked = 0;
        rb_ddr_locked = 0;
        rb_pcie_lnk_up = 0;
        r_sys_rst = 1;
        r_sys_nrst = 0;
        r_dbg_nrst = 0;
        rb_pcie_nrst = 0;
        rb_hdmi_nrst = 0;
    } else {
        r_sys_locked = i_sys_locked.read();
        rb_ddr_locked = (rb_ddr_locked.read()[0], i_ddr_locked.read());
        rb_pcie_lnk_up = (rb_pcie_lnk_up.read()[0], i_pcie_phy_lnk_up.read());
        r_sys_rst = ((!i_sys_locked.read()) || i_dmireset.read());
        r_sys_nrst = (i_sys_locked.read() & (!i_dmireset.read()));
        r_dbg_nrst = i_sys_locked.read();
        rb_pcie_nrst = (rb_pcie_nrst.read()[0], (i_pcie_phy_lnk_up.read() & (!i_pcie_phy_rst.read())));
        rb_hdmi_nrst = (rb_hdmi_nrst.read()[0], (rb_ddr_locked.read()[1] & r_sys_nrst.read()));
    }
}

void apb_prci::registers() {
    if ((async_reset_ == 1) && (r_sys_nrst.read() == 0)) {
        apb_prci_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

