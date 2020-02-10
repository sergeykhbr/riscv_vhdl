/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "l2_src.h"

namespace debugger {

L2Source::L2Source(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_coreo0("i_coreo0"),
    i_coreo1("i_coreo1"),
    i_coreo2("i_coreo2"),
    i_coreo3("i_coreo3"),
    i_acpo("i_acpo"),
    i_eos("i_eos"),
    i_req_ready("i_req_ready"),
    o_req_valid("o_req_valid"),
    o_req_src("o_req_src"),
    o_req_write("o_req_write"),
    o_req_cached("o_req_cached"),
    o_msg_src("o_msg_src"),
    o_msg_type("o_msg_type"),
    o_req_addr("o_req_addr"),
    o_req_size("o_req_size"),
    o_req_prot("o_req_prot"),
    o_req_wdata("o_req_wdata"),
    o_req_wstrb("o_req_wstrb") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_coreo0;
    sensitive << i_coreo1;
    sensitive << i_coreo2;
    sensitive << i_coreo3;
    sensitive << i_acpo;
    sensitive << i_eos;
    sensitive << i_req_ready;
    sensitive << r.state;
    sensitive << r.req_addr;
    sensitive << r.req_cached;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.req_src;
    sensitive << r.chansel;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

L2Source::~L2Source() {
}

void L2Source::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_coreo0, i_coreo0.name());
        sc_trace(o_vcd, i_req_ready, i_req_ready.name());
        sc_trace(o_vcd, i_eos, i_eos.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_src, o_req_src.name());
        sc_trace(o_vcd, o_req_write, o_req_write.name());
        sc_trace(o_vcd, o_req_cached, o_req_cached.name());
        sc_trace(o_vcd, o_msg_src, o_msg_src.name());
        sc_trace(o_vcd, o_msg_type, o_msg_type.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_size, o_req_size.name());
        sc_trace(o_vcd, o_req_prot, o_req_prot.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, o_req_wstrb, o_req_wstrb.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_cached, pn + ".r_req_cached");
        sc_trace(o_vcd, r.chansel, pn + ".r_chansel");
    }
}

void L2Source::comb() {
    bool v_req_valid;
    bool v_req_write;
    sc_uint<3> vb_msg_type;
    sc_uint<5> vb_req_srcid;
    sc_biguint<L1CACHE_LINE_BITS> vb_req_wdata;
    sc_uint<L1CACHE_BYTES_PER_LINE> vb_req_wstrb;
    req_addr_type reqa[5];
    req_data_type reqd[5];

    v = r;

    v_req_valid = 0;
    v_req_write = 0;
    vb_req_wdata = 0;
    vb_req_wstrb = 0;
    vb_req_srcid = 0;
    vb_msg_type = L2_MSG_NONE;

    reqa[0].ar_valid = i_acpo.read().ar_valid;
    reqa[0].aw_valid = i_acpo.read().aw_valid;
    reqa[0].ar_cached = i_acpo.read().ar_bits.cache[0];
    reqa[0].aw_cached = i_acpo.read().aw_bits.cache[0];
    reqa[0].ar_addr = i_acpo.read().ar_bits.addr;
    reqa[0].aw_addr = i_acpo.read().aw_bits.addr;
    reqa[0].ar_size = i_acpo.read().ar_bits.size;
    reqa[0].aw_size = i_acpo.read().aw_bits.size;
    reqa[0].ar_prot = i_acpo.read().ar_bits.prot;
    reqa[0].aw_prot = i_acpo.read().aw_bits.prot;
    reqd[0].req_wdata = i_acpo.read().w_data;   // TODO demux
    reqd[0].req_wstrb = i_acpo.read().w_strb;

    reqa[1].ar_valid = i_coreo0.read().ar_valid;
    reqa[1].aw_valid = i_coreo0.read().aw_valid;
    reqa[1].ar_cached = i_coreo0.read().ar_bits.cache[0];
    reqa[1].aw_cached = i_coreo0.read().aw_bits.cache[0];
    reqa[1].ar_addr = i_coreo0.read().ar_bits.addr;
    reqa[1].aw_addr = i_coreo0.read().aw_bits.addr;
    reqa[1].ar_size = i_coreo0.read().ar_bits.size;
    reqa[1].aw_size = i_coreo0.read().aw_bits.size;
    reqa[1].ar_prot = i_coreo0.read().ar_bits.prot;
    reqa[1].aw_prot = i_coreo0.read().aw_bits.prot;
    reqd[1].req_wdata = i_coreo0.read().w_data;
    reqd[1].req_wstrb = i_coreo0.read().w_strb;

    reqa[2].ar_valid = i_coreo1.read().ar_valid;
    reqa[2].aw_valid = i_coreo1.read().aw_valid;
    reqa[2].ar_cached = i_coreo1.read().ar_bits.cache[0];
    reqa[2].aw_cached = i_coreo1.read().aw_bits.cache[0];
    reqa[2].ar_addr = i_coreo1.read().ar_bits.addr;
    reqa[2].aw_addr = i_coreo1.read().aw_bits.addr;
    reqa[2].ar_size = i_coreo1.read().ar_bits.size;
    reqa[2].aw_size = i_coreo1.read().aw_bits.size;
    reqa[2].ar_prot = i_coreo1.read().ar_bits.prot;
    reqa[2].aw_prot = i_coreo1.read().aw_bits.prot;
    reqd[2].req_wdata = i_coreo1.read().w_data;
    reqd[2].req_wstrb = i_coreo1.read().w_strb;

    reqa[3].ar_valid = i_coreo2.read().ar_valid;
    reqa[3].aw_valid = i_coreo2.read().aw_valid;
    reqa[3].ar_cached = i_coreo2.read().ar_bits.cache[0];
    reqa[3].aw_cached = i_coreo2.read().aw_bits.cache[0];
    reqa[3].ar_addr = i_coreo2.read().ar_bits.addr;
    reqa[3].aw_addr = i_coreo2.read().aw_bits.addr;
    reqa[3].ar_size = i_coreo2.read().ar_bits.size;
    reqa[3].aw_size = i_coreo2.read().aw_bits.size;
    reqa[3].ar_prot = i_coreo2.read().ar_bits.prot;
    reqa[3].aw_prot = i_coreo2.read().aw_bits.prot;
    reqd[3].req_wdata = i_coreo2.read().w_data;
    reqd[3].req_wstrb = i_coreo2.read().w_strb;

    reqa[4].ar_valid = i_coreo3.read().ar_valid;
    reqa[4].aw_valid = i_coreo3.read().aw_valid;
    reqa[4].ar_cached = i_coreo3.read().ar_bits.cache[0];
    reqa[4].aw_cached = i_coreo3.read().aw_bits.cache[0];
    reqa[4].ar_addr = i_coreo3.read().ar_bits.addr;
    reqa[4].aw_addr = i_coreo3.read().aw_bits.addr;
    reqa[4].ar_size = i_coreo3.read().ar_bits.size;
    reqa[4].aw_size = i_coreo3.read().aw_bits.size;
    reqa[4].ar_prot = i_coreo3.read().ar_bits.prot;
    reqa[4].aw_prot = i_coreo3.read().aw_bits.prot;
    reqd[4].req_wdata = i_coreo3.read().w_data;
    reqd[4].req_wstrb = i_coreo3.read().w_strb;

    switch (r.state.read()) {
    case State_Idle:
        for (int i = 0; i < 5; i++) {
            if (vb_req_srcid == 0) {
                if (reqa[i].aw_valid == 1) {
                    v.state = State_Write;
                    vb_msg_type = L2_MSG_AW_ACCEPT;
                    vb_req_srcid[i] = 1;
                    v.chansel = i;
                    v.req_cached = reqa[i].aw_cached;
                    v.req_addr = reqa[i].aw_addr;
                    v.req_size = reqa[i].aw_size;
                    v.req_prot = reqa[i].aw_prot;
                    v.req_src = vb_req_srcid;
                } else if (reqa[i].ar_valid == 1) {
                    v.state = State_Read;
                    vb_msg_type = L2_MSG_AR_ACCEPT;
                    vb_req_srcid[i] = 1;
                    v.chansel = i;
                    v.req_cached = reqa[i].ar_cached;
                    v.req_addr = reqa[i].ar_addr;
                    v.req_size = reqa[i].ar_size;
                    v.req_prot = reqa[i].ar_prot;
                    v.req_src = vb_req_srcid;
                }
            }
        }
        break;
    case State_Read:
        v_req_valid = 1;
        if (i_req_ready.read() == 1) {
            v.state = State_Busy;
        }
        break;
    case State_Write:
        v_req_valid = 1;
        v_req_write = 1;
        vb_req_wdata = reqd[r.chansel.read().to_int()].req_wdata;
        vb_req_wstrb = reqd[r.chansel.read().to_int()].req_wstrb;
        if (i_req_ready.read() == 1) {
            v.state = State_Busy;
        }
        break;
    case State_Busy:
        if (i_eos.read() == 1) {
            v.state = State_Idle;
        }
        break;
    default:;
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_req_valid = v_req_valid;
    o_req_src = r.req_src;
    o_req_write = v_req_write;
    o_req_cached = r.req_cached;
    o_msg_src = vb_req_srcid;
    o_msg_type = vb_msg_type;
    o_req_addr = r.req_addr;
    o_req_size = r.req_size;
    o_req_prot = r.req_prot;
    o_req_wdata = vb_req_wdata;
    o_req_wstrb = vb_req_wstrb;
}

void L2Source::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
