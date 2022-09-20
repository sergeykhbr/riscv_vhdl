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

#include "ic_dport.h"
#include "api_core.h"

namespace debugger {

ic_dport::ic_dport(sc_module_name name,
                   bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_hartsel("i_hartsel"),
    i_haltreq("i_haltreq"),
    i_resumereq("i_resumereq"),
    i_resethaltreq("i_resethaltreq"),
    i_hartreset("i_hartreset"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_req_type("i_dport_req_type"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    i_dport_size("i_dport_size"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_resp_error("o_dport_resp_error"),
    o_dport_rdata("o_dport_rdata"),
    o_dporti("o_dporti", CFG_CPU_MAX),
    i_dporto("i_dporto", CFG_CPU_MAX) {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_hartsel;
    sensitive << i_haltreq;
    sensitive << i_resumereq;
    sensitive << i_resethaltreq;
    sensitive << i_hartreset;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_req_type;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_dport_size;
    sensitive << i_dport_resp_ready;
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        sensitive << i_dporto[i];
    }
    sensitive << r.hartsel;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void ic_dport::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_hartsel, i_hartsel.name());
        sc_trace(o_vcd, i_haltreq, i_haltreq.name());
        sc_trace(o_vcd, i_resumereq, i_resumereq.name());
        sc_trace(o_vcd, i_resethaltreq, i_resethaltreq.name());
        sc_trace(o_vcd, i_hartreset, i_hartreset.name());
        sc_trace(o_vcd, i_dport_req_valid, i_dport_req_valid.name());
        sc_trace(o_vcd, i_dport_req_type, i_dport_req_type.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, i_dport_size, i_dport_size.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, i_dport_resp_ready, i_dport_resp_ready.name());
        sc_trace(o_vcd, o_dport_resp_valid, o_dport_resp_valid.name());
        sc_trace(o_vcd, o_dport_resp_error, o_dport_resp_error.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, r.hartsel, pn + ".r_hartsel");
    }

}

void ic_dport::comb() {
    sc_uint<CFG_LOG2_CPU_MAX> vb_hartsel;
    sc_uint<CFG_CPU_MAX> vb_cpu_mask;
    sc_uint<CFG_CPU_MAX> vb_req_ready_mask;
    sc_uint<CFG_CPU_MAX> vb_req_valid_mask;
    sc_uint<CFG_CPU_MAX> vb_haltreq;
    sc_uint<CFG_CPU_MAX> vb_resumereq;
    sc_uint<CFG_CPU_MAX> vb_resethaltreq;
    sc_uint<CFG_CPU_MAX> vb_hartreset;
    sc_uint<CFG_CPU_MAX> vb_req_valid;
    sc_uint<CFG_CPU_MAX> vb_req_ready;
    dport_in_type vb_dporti[CFG_CPU_MAX];
    dport_out_type vb_dporto[CFG_CPU_MAX];
    bool v_req_accepted;

    vb_hartsel = 0;
    vb_cpu_mask = 0;
    vb_req_ready_mask = 0;
    vb_req_valid_mask = 0;
    vb_haltreq = 0;
    vb_resumereq = 0;
    vb_resethaltreq = 0;
    vb_hartreset = 0;
    vb_req_valid = 0;
    vb_req_ready = 0;
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        vb_dporti[i] = dport_in_none;
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        vb_dporto[i] = dport_out_none;
    }
    v_req_accepted = 0;

    v = r;

    vb_cpu_mask[i_hartsel.read().to_int()] = 1;
    if (i_haltreq.read() == 1) {
        vb_haltreq = ALL_CPU_MASK;
    }
    if (i_resumereq.read() == 1) {
        vb_resumereq = ALL_CPU_MASK;
    }
    if (i_resethaltreq.read() == 1) {
        vb_resethaltreq = ALL_CPU_MASK;
    }
    if (i_hartreset.read() == 1) {
        vb_hartreset = ALL_CPU_MASK;
    }
    if (i_dport_req_valid.read() == 1) {
        vb_req_valid = ALL_CPU_MASK;
    }
    for (int i = 0; i < CFG_CPU_MAX; i++) {
        vb_req_ready[i] = i_dporto[i].read().req_ready;
    }

    vb_req_ready_mask = (vb_req_ready & vb_cpu_mask);
    vb_req_valid_mask = (vb_req_valid & vb_req_ready & vb_cpu_mask);
    v_req_accepted = vb_req_valid_mask.or_reduce();

    for (int i = 0; i < CFG_CPU_MAX; i++) {
        vb_dporti[i].haltreq = (vb_haltreq[i] && vb_cpu_mask[i]);
        vb_dporti[i].resumereq = (vb_resumereq[i] && vb_cpu_mask[i]);
        vb_dporti[i].resethaltreq = (vb_resethaltreq[i] && vb_cpu_mask[i]);
        vb_dporti[i].hartreset = (vb_hartreset[i] && vb_cpu_mask[i]);
        vb_dporti[i].req_valid = (vb_req_valid[i] && vb_cpu_mask[i]);
        vb_dporti[i].dtype = i_dport_req_type;
        vb_dporti[i].addr = i_dport_addr;
        vb_dporti[i].wdata = i_dport_wdata;
        vb_dporti[i].size = i_dport_size;
        vb_dporti[i].resp_ready = i_dport_resp_ready;
    }

    if (v_req_accepted == 1) {
        vb_hartsel = i_hartsel;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        ic_dport_r_reset(v);
    }

    for (int i = 0; i < CFG_CPU_MAX; i++) {
        o_dporti[i] = vb_dporti[i];
    }

    o_dport_req_ready = vb_req_ready_mask.or_reduce();
    o_dport_resp_valid = i_dporto[r.hartsel.read().to_int()].read().resp_valid;
    o_dport_resp_error = i_dporto[r.hartsel.read().to_int()].read().resp_error;
    o_dport_rdata = i_dporto[r.hartsel.read().to_int()].read().rdata;
}

void ic_dport::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        ic_dport_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

