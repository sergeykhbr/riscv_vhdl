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
    o_sys_rst("o_sys_rst"),
    o_sys_nrst("o_sys_nrst"),
    o_dbg_nrst("o_dbg_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_PRCI);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(rh.sys_nrst);
    pslv0->i_mapinfo(i_mapinfo);
    pslv0->o_cfg(o_cfg);
    pslv0->i_apbi(i_apbi);
    pslv0->o_apbo(o_apbo);
    pslv0->o_req_valid(w_req_valid);
    pslv0->o_req_addr(wb_req_addr);
    pslv0->o_req_write(w_req_write);
    pslv0->o_req_wdata(wb_req_wdata);
    pslv0->i_resp_valid(rh.resp_valid);
    pslv0->i_resp_rdata(rh.resp_rdata);
    pslv0->i_resp_err(rh.resp_err);

    SC_METHOD(comb);
    sensitive << i_pwrreset;
    sensitive << i_dmireset;
    sensitive << i_sys_locked;
    sensitive << i_ddr_locked;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << rh.sys_rst;
    sensitive << rh.sys_nrst;
    sensitive << rh.dbg_nrst;
    sensitive << rh.sys_locked;
    sensitive << rh.ddr_locked;
    sensitive << rh.resp_valid;
    sensitive << rh.resp_rdata;
    sensitive << rh.resp_err;

    SC_METHOD(rhegisters);
    sensitive << i_pwrreset;
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
        sc_trace(o_vcd, o_sys_rst, o_sys_rst.name());
        sc_trace(o_vcd, o_sys_nrst, o_sys_nrst.name());
        sc_trace(o_vcd, o_dbg_nrst, o_dbg_nrst.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, rh.sys_rst, pn + ".rh_sys_rst");
        sc_trace(o_vcd, rh.sys_nrst, pn + ".rh_sys_nrst");
        sc_trace(o_vcd, rh.dbg_nrst, pn + ".rh_dbg_nrst");
        sc_trace(o_vcd, rh.sys_locked, pn + ".rh_sys_locked");
        sc_trace(o_vcd, rh.ddr_locked, pn + ".rh_ddr_locked");
        sc_trace(o_vcd, rh.resp_valid, pn + ".rh_resp_valid");
        sc_trace(o_vcd, rh.resp_rdata, pn + ".rh_resp_rdata");
        sc_trace(o_vcd, rh.resp_err, pn + ".rh_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void apb_prci::comb() {
    sc_uint<32> vb_rdata;

    vb_rdata = 0;

    vh = rh;

    vh.sys_rst = (i_pwrreset.read() || (!i_sys_locked.read()) || i_dmireset.read());
    vh.sys_nrst = (!(i_pwrreset.read() || (!i_sys_locked.read()) || i_dmireset.read()));
    vh.dbg_nrst = (!(i_pwrreset.read() || (!i_sys_locked.read())));

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: pll statuses
        vb_rdata[0] = i_sys_locked;
        vb_rdata[1] = i_ddr_locked;
        break;
    case 1:                                                 // 0x04: reset status
        vb_rdata[0] = rh.sys_nrst;
        vb_rdata[1] = rh.dbg_nrst;
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                // todo:
            }
        }
        break;
    default:
        break;
    }

    vh.resp_valid = w_req_valid;
    vh.resp_rdata = vb_rdata;
    vh.resp_err = 0;

    if (!async_reset_ && i_pwrreset.read() == 1) {
        apb_prci_rh_reset(vh);
    }

    o_sys_rst = rh.sys_rst;
    o_sys_nrst = rh.sys_nrst;
    o_dbg_nrst = rh.dbg_nrst;
}

void apb_prci::rhegisters() {
    if (async_reset_ && i_pwrreset.read() == 1) {
        apb_prci_rh_reset(rh);
    } else {
        rh = vh;
    }
}

}  // namespace debugger

