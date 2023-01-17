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

#include "axi2apb.h"
#include "api_core.h"

namespace debugger {

axi2apb::axi2apb(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_apbo("i_apbo", CFG_BUS1_PSLV_TOTAL),
    o_apbi("o_apbi", CFG_BUS1_PSLV_TOTAL),
    o_mapinfo("o_mapinfo", CFG_BUS1_PSLV_TOTAL) {

    async_reset_ = async_reset;
    axi0 = 0;

    axi0 = new axi_slv("axi0", async_reset,
                        VENDOR_OPTIMITECH,
                        OPTIMITECH_AXI2APB_BRIDGE);
    axi0->i_clk(i_clk);
    axi0->i_nrst(i_nrst);
    axi0->i_mapinfo(i_mapinfo);
    axi0->o_cfg(o_cfg);
    axi0->i_xslvi(i_xslvi);
    axi0->o_xslvo(o_xslvo);
    axi0->o_req_valid(w_req_valid);
    axi0->o_req_addr(wb_req_addr);
    axi0->o_req_size(wb_req_size);
    axi0->o_req_write(w_req_write);
    axi0->o_req_wdata(wb_req_wdata);
    axi0->o_req_wstrb(wb_req_wstrb);
    axi0->o_req_last(w_req_last);
    axi0->i_req_ready(w_req_ready);
    axi0->i_resp_valid(r.pvalid);
    axi0->i_resp_rdata(r.prdata);
    axi0->i_resp_err(r.pslverr);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_xslvi;
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << i_apbo[i];
    }
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << wb_req_size;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << wb_req_wstrb;
    sensitive << w_req_last;
    sensitive << w_req_ready;
    sensitive << r.state;
    sensitive << r.selidx;
    sensitive << r.pvalid;
    sensitive << r.paddr;
    sensitive << r.pwdata;
    sensitive << r.prdata;
    sensitive << r.pwrite;
    sensitive << r.pstrb;
    sensitive << r.pprot;
    sensitive << r.pselx;
    sensitive << r.penable;
    sensitive << r.pslverr;
    sensitive << r.size;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

axi2apb::~axi2apb() {
    if (axi0) {
        delete axi0;
    }
}

void axi2apb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.selidx, pn + ".r_selidx");
        sc_trace(o_vcd, r.pvalid, pn + ".r_pvalid");
        sc_trace(o_vcd, r.paddr, pn + ".r_paddr");
        sc_trace(o_vcd, r.pwdata, pn + ".r_pwdata");
        sc_trace(o_vcd, r.prdata, pn + ".r_prdata");
        sc_trace(o_vcd, r.pwrite, pn + ".r_pwrite");
        sc_trace(o_vcd, r.pstrb, pn + ".r_pstrb");
        sc_trace(o_vcd, r.pprot, pn + ".r_pprot");
        sc_trace(o_vcd, r.pselx, pn + ".r_pselx");
        sc_trace(o_vcd, r.penable, pn + ".r_penable");
        sc_trace(o_vcd, r.pslverr, pn + ".r_pslverr");
        sc_trace(o_vcd, r.size, pn + ".r_size");
    }

    if (axi0) {
        axi0->generateVCD(i_vcd, o_vcd);
    }
}

void axi2apb::comb() {
    int iselidx;
    apb_in_type vapbi[CFG_BUS1_PSLV_TOTAL];
    apb_out_type vapbo[(CFG_BUS1_PSLV_TOTAL + 1)];

    iselidx = 0;
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        vapbi[i] = apb_in_none;
    }
    for (int i = 0; i < (CFG_BUS1_PSLV_TOTAL + 1); i++) {
        vapbo[i] = apb_out_none;
    }

    v = r;

    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        vapbo[i] = i_apbo[i];                               // Cannot read vector item from port in systemc
    }
    // Unmapped default slave:
    vapbo[CFG_BUS1_PSLV_TOTAL].pready = 1;
    vapbo[CFG_BUS1_PSLV_TOTAL].pslverr = 1;
    vapbo[CFG_BUS1_PSLV_TOTAL].prdata = ~0ull;
    w_req_ready = 0;
    v.pvalid = 0;
    iselidx = r.selidx.read().to_int();

    switch (r.state.read()) {
    case State_Idle:
        w_req_ready = 1;
        v.pslverr = 0;
        v.penable = 0;
        v.pselx = 0;
        v.selidx = CFG_BUS1_PSLV_TOTAL;
        for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
            if ((wb_req_addr.read() >= CFG_BUS1_MAP[i].addr_start)
                    && (wb_req_addr.read() < CFG_BUS1_MAP[i].addr_end)) {
                v.selidx = i;
            }
        }
        if (w_req_valid.read() == 1) {
            v.pwrite = w_req_write;
            v.pselx = 1;
            v.paddr = (wb_req_addr.read()(31, 2) << 2);
            v.pprot = 0;
            if (wb_req_addr.read()[2] == 1) {
                v.pwdata = (0, wb_req_wdata.read()(63, 32));
                v.pstrb = (0, wb_req_wstrb.read()(7, 4));
            } else {
                v.pwdata = wb_req_wdata;
                v.pstrb = wb_req_wstrb;
            }
            v.state = State_setup;
            v.size = wb_req_size;
            if (w_req_last.read() == 0) {
                v.state = State_out;                        // Burst is not supported
                v.pselx = 0;
                v.pslverr = 1;
                v.prdata = ~0ull;
            }
        }
        break;
    case State_setup:
        v.penable = 1;
        v.state = State_access;
        break;
    case State_access:
        v.pslverr = vapbo[iselidx].pslverr;
        if (vapbo[iselidx].pready == 1) {
            v.penable = 0;
            if (r.paddr.read()[2] == 0) {
                v.prdata = (r.prdata.read()(63, 32), vapbo[iselidx].prdata);
            } else {
                v.prdata = (vapbo[iselidx].prdata, r.prdata.read()(31, 0));
            }
            if (r.size.read() > 4) {
                v.size = (r.size.read() - 4);
                v.paddr = (r.paddr.read() + 4);
                v.pwdata = (0, wb_req_wdata.read()(63, 32));
                v.pstrb = (0, wb_req_wstrb.read()(7, 4));
                v.state = State_setup;
            } else {
                v.pvalid = 1;
                v.state = State_out;
                v.pselx = 0;
                v.pwrite = 0;
            }
        }
        break;
    case State_out:
        v.pvalid = 0;
        v.pslverr = 0;
        v.state = State_Idle;
        break;
    default:
        break;
    }

    vapbi[iselidx].paddr = r.paddr;
    vapbi[iselidx].pwrite = r.pwrite;
    vapbi[iselidx].pwdata = r.pwdata.read()(31, 0);
    vapbi[iselidx].pstrb = r.pstrb.read()(3, 0);
    vapbi[iselidx].pselx = r.pselx;
    vapbi[iselidx].penable = r.penable;
    vapbi[iselidx].pprot = r.pprot;

    if (!async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(v);
    }

    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        o_apbi[i] = vapbi[i];
        o_mapinfo[i] = CFG_BUS1_MAP[i];
    }
}

void axi2apb::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

