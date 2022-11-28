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

#include "apb_slv.h"
#include "api_core.h"

namespace debugger {

apb_slv::apb_slv(sc_module_name name,
                 bool async_reset,
                 uint32_t vid,
                 uint32_t did)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_req_valid("o_req_valid"),
    o_req_addr("o_req_addr"),
    o_req_write("o_req_write"),
    o_req_wdata("o_req_wdata"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    i_resp_err("i_resp_err") {

    async_reset_ = async_reset;
    vid_ = vid;
    did_ = did;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << i_resp_err;
    sensitive << r.state;
    sensitive << r.req_valid;
    sensitive << r.req_addr;
    sensitive << r.req_write;
    sensitive << r.req_wdata;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void apb_slv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_write, o_req_write.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, i_resp_err, i_resp_err.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

}

void apb_slv::comb() {
    sc_uint<32> vb_rdata;
    dev_config_type vcfg;
    apb_out_type vapbo;

    vb_rdata = 0;
    vcfg = dev_config_none;
    vapbo = apb_out_none;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.read().addr_start;
    vcfg.addr_end = i_mapinfo.read().addr_end;
    vcfg.vid = vid_;
    vcfg.did = did_;

    v.req_valid = 0;

    switch (r.state.read()) {
    case State_Idle:
        v.resp_valid = 0;
        v.resp_err = 0;
        if (i_apbi.read().pselx == 1) {
            v.state = State_Request;
            v.req_valid = 1;
            v.req_addr = i_apbi.read().paddr;
            v.req_write = i_apbi.read().pwrite;
            v.req_wdata = i_apbi.read().pwdata;
        }
        break;
    case State_Request:
        // One clock wait state:
        v.state = State_WaitResp;
        break;
    case State_WaitResp:
        v.resp_valid = i_resp_valid;
        if (i_resp_valid.read() == 1) {
            v.resp_rdata = i_resp_rdata;
            v.resp_err = i_resp_err;
            v.state = State_Resp;
        }
        break;
    case State_Resp:
        if (i_apbi.read().penable == 1) {
            v.state = State_Idle;
            v.resp_valid = 0;
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        apb_slv_r_reset(v);
    }

    o_req_valid = r.req_valid;
    o_req_addr = r.req_addr;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;

    vapbo.pready = r.resp_valid;
    vapbo.prdata = r.resp_rdata;
    vapbo.pslverr = r.resp_err;
    o_apbo = vapbo;
    o_cfg = vcfg;
}

void apb_slv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        apb_slv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

