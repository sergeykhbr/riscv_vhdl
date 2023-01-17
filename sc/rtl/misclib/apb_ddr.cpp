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

#include "apb_ddr.h"
#include "api_core.h"

namespace debugger {

apb_ddr::apb_ddr(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    i_pll_locked("i_pll_locked"),
    i_init_calib_done("i_init_calib_done"),
    i_device_temp("i_device_temp"),
    i_sr_active("i_sr_active"),
    i_ref_ack("i_ref_ack"),
    i_zq_ack("i_zq_ack") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_DDRCTRL);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(i_nrst);
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
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_pll_locked;
    sensitive << i_init_calib_done;
    sensitive << i_device_temp;
    sensitive << i_sr_active;
    sensitive << i_ref_ack;
    sensitive << i_zq_ack;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.pll_locked;
    sensitive << r.init_calib_done;
    sensitive << r.device_temp;
    sensitive << r.sr_active;
    sensitive << r.ref_ack;
    sensitive << r.zq_ack;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

apb_ddr::~apb_ddr() {
    if (pslv0) {
        delete pslv0;
    }
}

void apb_ddr::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, i_pll_locked, i_pll_locked.name());
        sc_trace(o_vcd, i_init_calib_done, i_init_calib_done.name());
        sc_trace(o_vcd, i_device_temp, i_device_temp.name());
        sc_trace(o_vcd, i_sr_active, i_sr_active.name());
        sc_trace(o_vcd, i_ref_ack, i_ref_ack.name());
        sc_trace(o_vcd, i_zq_ack, i_zq_ack.name());
        sc_trace(o_vcd, r.pll_locked, pn + ".r_pll_locked");
        sc_trace(o_vcd, r.init_calib_done, pn + ".r_init_calib_done");
        sc_trace(o_vcd, r.device_temp, pn + ".r_device_temp");
        sc_trace(o_vcd, r.sr_active, pn + ".r_sr_active");
        sc_trace(o_vcd, r.ref_ack, pn + ".r_ref_ack");
        sc_trace(o_vcd, r.zq_ack, pn + ".r_zq_ack");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void apb_ddr::comb() {
    sc_uint<32> vb_rdata;

    vb_rdata = 0;

    v = r;

    v.pll_locked = i_pll_locked;
    v.init_calib_done = i_init_calib_done;
    v.device_temp = i_device_temp;
    v.sr_active = i_sr_active;
    v.ref_ack = i_ref_ack;
    v.zq_ack = i_zq_ack;

    v.resp_err = 0;
    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: clock status
        vb_rdata[0] = r.pll_locked.read();
        vb_rdata[1] = r.init_calib_done.read();
        break;
    case 1:                                                 // 0x04: temperature
        vb_rdata(11, 0) = r.device_temp;
        break;
    case 2:                                                 // 0x08: app bits
        vb_rdata[0] = r.sr_active.read();                   // [0] 
        vb_rdata[1] = r.ref_ack.read();                     // [1] 
        vb_rdata[2] = r.zq_ack.read();                      // [2] 
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;

    if (!async_reset_ && i_nrst.read() == 0) {
        apb_ddr_r_reset(v);
    }
}

void apb_ddr::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        apb_ddr_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

