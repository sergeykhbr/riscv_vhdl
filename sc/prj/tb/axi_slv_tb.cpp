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

#include "axi_slv_tb.h"
#include "api_core.h"

namespace debugger {

axi_slv_tb::axi_slv_tb(sc_module_name name)
    : sc_module(name) {

    clk = 0;
    slv0 = 0;

    clk = new pll_generic("clk",
                           25.0);
    clk->o_clk(w_clk);

    slv0 = new axi_slv("slv0",
                        0,
                        0,
                        0);
    slv0->i_clk(w_clk);
    slv0->i_nrst(w_nrst);
    slv0->i_mapinfo(wb_slv_mapinfo);
    slv0->o_cfg(wb_slv_o_cfg);
    slv0->i_xslvi(wb_xslvi);
    slv0->o_xslvo(wb_xslvo);
    slv0->o_req_valid(w_slv_o_req_valid);
    slv0->o_req_addr(wb_slv_o_req_addr);
    slv0->o_req_size(wb_slv_o_req_size);
    slv0->o_req_write(w_slv_o_req_write);
    slv0->o_req_wdata(wb_slv_o_req_wdata);
    slv0->o_req_wstrb(wb_slv_o_req_wstrb);
    slv0->o_req_last(w_slv_o_req_last);
    slv0->i_req_ready(w_slv_i_req_ready);
    slv0->i_resp_valid(w_slv_i_resp_valid);
    slv0->i_resp_rdata(wb_slv_i_resp_rdata);
    slv0->i_resp_err(w_slv_i_resp_err);

    SC_THREAD(init);

    SC_METHOD(comb);
    sensitive << w_nrst;
    sensitive << w_clk;
    sensitive << wb_mst_o_cfg;
    sensitive << wb_xslvi;
    sensitive << wb_xslvo;
    sensitive << wb_slv_o_cfg;
    sensitive << wb_slv_mapinfo;
    sensitive << w_slv_o_req_valid;
    sensitive << wb_slv_o_req_addr;
    sensitive << wb_slv_o_req_size;
    sensitive << w_slv_o_req_write;
    sensitive << wb_slv_o_req_wdata;
    sensitive << wb_slv_o_req_wstrb;
    sensitive << w_slv_o_req_last;
    sensitive << w_slv_i_req_ready;
    sensitive << w_slv_i_resp_valid;
    sensitive << wb_slv_i_resp_rdata;
    sensitive << w_slv_i_resp_err;
    sensitive << r.wb_clk_cnt;
    sensitive << r.test_cnt;

    SC_METHOD(test_clk);
    sensitive << w_clk.posedge_event();

    SC_METHOD(registers);
    sensitive << w_nrst;
    sensitive << w_clk.posedge_event();
}

axi_slv_tb::~axi_slv_tb() {
    if (clk) {
        delete clk;
    }
    if (slv0) {
        delete slv0;
    }
}

void axi_slv_tb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, r.wb_clk_cnt, pn + ".r.wb_clk_cnt");
        sc_trace(o_vcd, r.test_cnt, pn + ".r.test_cnt");
    }

    if (clk) {
        clk->generateVCD(i_vcd, o_vcd);
    }
    if (slv0) {
        slv0->generateVCD(i_vcd, o_vcd);
    }
}

void axi_slv_tb::init() {
    w_nrst = 0;
    wait(static_cast<int>(1000.0), SC_NS);
    w_nrst = 1;
}

void axi_slv_tb::comb() {
    mapinfo_type vb_mapinfo;
    axi4_slave_in_type vb_xslvi;
    axi4_master_in_type vb_xmsti;

    v = r;

    vb_mapinfo.addr_start = 0x000080000000;
    vb_mapinfo.addr_end = 0x000080001000;
    wb_slv_mapinfo = vb_mapinfo;

    v.wb_clk_cnt = (r.wb_clk_cnt.read() + 1);

    if (r.test_cnt.read() == 0) {
        vb_xslvi.ar_valid = 1;
        vb_xslvi.ar_bits.addr = 0x000000000010;
        vb_xslvi.ar_bits.len = 0;
        vb_xslvi.aw_valid = 1;
        vb_xslvi.aw_bits.addr = 0x000000000010;
        vb_xslvi.aw_bits.len = 0;
        vb_xslvi.w_valid = 1;
        vb_xslvi.w_data = 1;
        vb_xslvi.w_last = 1;
    } else {
    }

    vb_xslvi.ar_bits.size = 3;
    vb_xslvi.ar_bits.burst = 1;
    vb_xslvi.ar_bits.lock = 0;
    vb_xslvi.ar_bits.cache = 0;
    vb_xslvi.ar_bits.prot = 0;
    vb_xslvi.ar_bits.qos = 0;
    vb_xslvi.ar_bits.region = 0;
    vb_xslvi.aw_bits.size = 3;
    vb_xslvi.aw_bits.burst = 1;
    vb_xslvi.aw_bits.lock = 0;
    vb_xslvi.aw_bits.cache = 0;
    vb_xslvi.aw_bits.prot = 0;
    vb_xslvi.aw_bits.qos = 0;
    vb_xslvi.aw_bits.region = 0;
    vb_xslvi.aw_id = 0;
    vb_xslvi.aw_user = 0;
    vb_xslvi.w_strb = ~0ull;
    vb_xslvi.w_user = 0;
    vb_xslvi.b_ready = 1;
    vb_xslvi.ar_id = 0;
    vb_xslvi.ar_user = 0;
    vb_xslvi.r_ready = 1;

    wb_xslvi = vb_xslvi;
}

void axi_slv_tb::test_clk() {
    axi4_slave_in_type vb_xslvi;

    if (w_nrst.read() == 0) {
        v.test_cnt = 0;
        rd_valid = 0;
        req_ready = 0;
        rd_addr = 0;
        rd_data = 0;
        v_busy = 0;
    } else {
        v_busy = 0;
        if ((w_slv_o_req_write.read() == 1) && (w_slv_o_req_valid.read() == 1)) {
            mem[wb_slv_o_req_addr.read()(5, 2).to_int()] = wb_slv_o_req_wdata.read();
        }
        rd_addr = wb_slv_o_req_addr.read()(5, 2);
        if ((w_slv_o_req_valid.read() & (!v_busy)) == 1) {
            rd_data = wb_slv_o_req_addr.read();
        }
        rd_valid = (rd_valid(1, 0), (w_slv_o_req_valid.read() & (!v_busy)));
    }
    wb_slv_i_resp_rdata = rd_data;
    w_slv_i_resp_valid = rd_valid[0];
    w_slv_i_req_ready = 1;
    w_slv_i_resp_err = 0;
}

void axi_slv_tb::registers() {
    if (w_nrst.read() == 0) {
        axi_slv_tb_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

