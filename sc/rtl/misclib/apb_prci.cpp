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
    pslv0->i_nrst(r.sys_nrst);
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
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.sys_rst;
    sensitive << r.sys_nrst;
    sensitive << r.dbg_nrst;
    sensitive << r.sys_locked;
    sensitive << r.ddr_locked;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
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
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, r.sys_rst, pn + ".r_sys_rst");
        sc_trace(o_vcd, r.sys_nrst, pn + ".r_sys_nrst");
        sc_trace(o_vcd, r.dbg_nrst, pn + ".r_dbg_nrst");
        sc_trace(o_vcd, r.sys_locked, pn + ".r_sys_locked");
        sc_trace(o_vcd, r.ddr_locked, pn + ".r_ddr_locked");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void apb_prci::comb() {
    sc_uint<32> vb_rdata;

    vb_rdata = 0;

    v = r;

    v.sys_rst = (i_pwrreset || (!i_sys_locked) || i_dmireset);
    v.sys_nrst = (!(i_pwrreset || (!i_sys_locked) || i_dmireset));
    v.dbg_nrst = (!(i_pwrreset || (!i_sys_locked)));

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: pll statuses
        vb_rdata[0] = i_sys_locked.read();
        vb_rdata[1] = i_ddr_locked.read();
        break;
    case 1:                                                 // 0x04: reset status
        vb_rdata[0] = r.sys_nrst.read();
        vb_rdata[1] = r.dbg_nrst.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                // todo:
            }
        }
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if (!async_reset_ && i_pwrreset.read() == 1) {
        apb_prci_r_reset(v);
    }

    o_sys_rst = r.sys_rst;
    o_sys_nrst = r.sys_nrst;
    o_dbg_nrst = r.dbg_nrst;
}

void apb_prci::registers() {
    if (async_reset_ && i_pwrreset.read() == 1) {
        apb_prci_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

