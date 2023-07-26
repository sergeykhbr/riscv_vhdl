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

#include "sdctrl_regs.h"
#include "api_core.h"

namespace debugger {

sdctrl_regs::sdctrl_regs(sc_module_name name,
                         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_pmapinfo("i_pmapinfo"),
    o_pcfg("o_pcfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_sck("o_sck"),
    o_sck_posedge("o_sck_posedge"),
    o_sck_negedge("o_sck_negedge") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_SDCTRL_REG);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(i_nrst);
    pslv0->i_mapinfo(i_pmapinfo);
    pslv0->o_cfg(o_pcfg);
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
    sensitive << i_pmapinfo;
    sensitive << i_apbi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.scaler;
    sensitive << r.scaler_cnt;
    sensitive << r.wdog;
    sensitive << r.wdog_cnt;
    sensitive << r.level;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

sdctrl_regs::~sdctrl_regs() {
    if (pslv0) {
        delete pslv0;
    }
}

void sdctrl_regs::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_pmapinfo, i_pmapinfo.name());
        sc_trace(o_vcd, o_pcfg, o_pcfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_sck, o_sck.name());
        sc_trace(o_vcd, o_sck_posedge, o_sck_posedge.name());
        sc_trace(o_vcd, o_sck_negedge, o_sck_negedge.name());
        sc_trace(o_vcd, r.scaler, pn + ".r_scaler");
        sc_trace(o_vcd, r.scaler_cnt, pn + ".r_scaler_cnt");
        sc_trace(o_vcd, r.wdog, pn + ".r_wdog");
        sc_trace(o_vcd, r.wdog_cnt, pn + ".r_wdog_cnt");
        sc_trace(o_vcd, r.level, pn + ".r_level");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void sdctrl_regs::comb() {
    bool v_posedge;
    bool v_negedge;
    sc_uint<32> vb_rdata;

    v_posedge = 0;
    v_negedge = 0;
    vb_rdata = 0;

    v = r;

    // system bus clock scaler to baudrate:
    if (r.scaler.read().or_reduce() == 1) {
        if (r.scaler_cnt.read() == (r.scaler.read() - 1)) {
            v.scaler_cnt = 0;
            v.level = (!r.level);
            v_posedge = (!r.level);
            v_negedge = r.level;
        } else {
            v.scaler_cnt = (r.scaler_cnt.read() + 1);
        }
    }
    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0x0:                                               // 0x00: sckdiv
        vb_rdata = r.scaler;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scaler = wb_req_wdata.read()(30, 0);
            v.scaler_cnt = 0;
        }
        break;
    case 0x2:                                               // 0x08: reserved (watchdog)
        vb_rdata(15, 0) = r.wdog;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.wdog = wb_req_wdata.read()(15, 0);
        }
        break;
    case 0x11:                                              // 0x44: reserved 4 (txctrl)
        break;
    case 0x12:                                              // 0x48: Tx FIFO Data
        break;
    case 0x13:                                              // 0x4C: Rx FIFO Data
        break;
    case 0x14:                                              // 0x50: Tx FIFO Watermark
        break;
    case 0x15:                                              // 0x54: Rx FIFO Watermark
        break;
    case 0x16:                                              // 0x58: CRC16 value (reserved FU740)
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_regs_r_reset(v);
    }

    o_sck = r.level;
    o_sck_posedge = v_posedge;
    o_sck_negedge = v_negedge;
}

void sdctrl_regs::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_regs_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

