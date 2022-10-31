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

#include "l2dummy.h"
#include "api_core.h"

namespace debugger {

L2Dummy::L2Dummy(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_l1o("i_l1o", CFG_SLOT_L1_TOTAL),
    o_l1i("o_l1i", CFG_SLOT_L1_TOTAL),
    i_l2i("i_l2i"),
    o_l2o("o_l2o"),
    i_flush_valid("i_flush_valid") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        sensitive << i_l1o[i];
    }
    sensitive << i_l2i;
    sensitive << i_flush_valid;
    sensitive << r.state;
    sensitive << r.srcid;
    sensitive << r.req_addr;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.req_lock;
    sensitive << r.req_id;
    sensitive << r.req_user;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.rdata;
    sensitive << r.resp;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void L2Dummy::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_l2i, i_l2i.name());
        sc_trace(o_vcd, o_l2o, o_l2o.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.srcid, pn + ".r_srcid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_size, pn + ".r_req_size");
        sc_trace(o_vcd, r.req_prot, pn + ".r_req_prot");
        sc_trace(o_vcd, r.req_lock, pn + ".r_req_lock");
        sc_trace(o_vcd, r.req_id, pn + ".r_req_id");
        sc_trace(o_vcd, r.req_user, pn + ".r_req_user");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.rdata, pn + ".r_rdata");
        sc_trace(o_vcd, r.resp, pn + ".r_resp");
    }

}

void L2Dummy::comb() {
    axi4_l1_out_type vl1o[CFG_SLOT_L1_TOTAL];
    axi4_l1_in_type vlxi[CFG_SLOT_L1_TOTAL];
    axi4_l2_out_type vl2o;
    sc_uint<CFG_SLOT_L1_TOTAL> vb_src_aw;
    sc_uint<CFG_SLOT_L1_TOTAL> vb_src_ar;
    int vb_srcid;
    bool v_selected;

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        vl1o[i] = axi4_l1_out_none;
    }
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        vlxi[i] = axi4_l1_in_none;
    }
    vl2o = axi4_l2_out_none;
    vb_src_aw = 0;
    vb_src_ar = 0;
    vb_srcid = 0;
    v_selected = 0;

    v = r;

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        vl1o[i] = i_l1o[i];
        vlxi[i] = axi4_l1_in_none;

        vb_src_aw[i] = vl1o[i].aw_valid;
        vb_src_ar[i] = vl1o[i].ar_valid;
    }
    vl2o = axi4_l2_out_none;

    // select source (aw has higher priority):
    if (vb_src_aw.or_reduce() == 0) {
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            if ((v_selected == 0) && (vb_src_ar[i] == 1)) {
                vb_srcid = i;
                v_selected = 1;
            }
        }
    } else {
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            if ((v_selected == 0) && (vb_src_aw[i] == 1)) {
                vb_srcid = i;
                v_selected = 1;
            }
        }
    }
    switch (r.state.read()) {
    case Idle:
        if (vb_src_aw.or_reduce() == 1) {
            v.state = state_aw;
            vlxi[vb_srcid].aw_ready = 1;
            vlxi[vb_srcid].w_ready = 1;                     // AXI-Lite-interface

            v.srcid = vb_srcid;
            v.req_addr = vl1o[vb_srcid].aw_bits.addr;
            v.req_size = vl1o[vb_srcid].aw_bits.size;
            v.req_lock = vl1o[vb_srcid].aw_bits.lock;
            v.req_prot = vl1o[vb_srcid].aw_bits.prot;
            v.req_id = vl1o[vb_srcid].aw_id;
            v.req_user = vl1o[vb_srcid].aw_user;
            // AXI-Lite-interface
            v.req_wdata = vl1o[vb_srcid].w_data;
            v.req_wstrb = vl1o[vb_srcid].w_strb;
        } else if (vb_src_ar.or_reduce() == 1) {
            v.state = state_ar;
            vlxi[vb_srcid].ar_ready = 1;

            v.srcid = vb_srcid;
            v.req_addr = vl1o[vb_srcid].ar_bits.addr;
            v.req_size = vl1o[vb_srcid].ar_bits.size;
            v.req_lock = vl1o[vb_srcid].ar_bits.lock;
            v.req_prot = vl1o[vb_srcid].ar_bits.prot;
            v.req_id = vl1o[vb_srcid].ar_id;
            v.req_user = vl1o[vb_srcid].ar_user;
        }
        break;
    case state_ar:
        vl2o.ar_valid = 1;
        vl2o.ar_bits.addr = r.req_addr;
        vl2o.ar_bits.size = r.req_size;
        vl2o.ar_bits.lock = r.req_lock;
        vl2o.ar_bits.prot = r.req_prot;
        vl2o.ar_id = r.req_id;
        vl2o.ar_user = r.req_user;
        if (i_l2i.read().ar_ready == 1) {
            v.state = state_r;
        }
        break;
    case state_r:
        vl2o.r_ready = 1;
        if (i_l2i.read().r_valid == 1) {
            v.rdata = i_l2i.read().r_data;
            v.resp = i_l2i.read().r_resp;
            v.state = l1_r_resp;
        }
        break;
    case l1_r_resp:
        vlxi[r.srcid.read().to_int()].r_valid = 1;
        vlxi[r.srcid.read().to_int()].r_last = 1;
        vlxi[r.srcid.read().to_int()].r_data = r.rdata;
        vlxi[r.srcid.read().to_int()].r_resp = (0, r.resp.read());
        vlxi[r.srcid.read().to_int()].r_id = r.req_id;
        vlxi[r.srcid.read().to_int()].r_user = r.req_user;
        if (vl1o[r.srcid.read().to_int()].r_ready == 1) {
            v.state = Idle;
        }
        break;
    case state_aw:
        vl2o.aw_valid = 1;
        vl2o.aw_bits.addr = r.req_addr;
        vl2o.aw_bits.size = r.req_size;
        vl2o.aw_bits.lock = r.req_lock;
        vl2o.aw_bits.prot = r.req_prot;
        vl2o.aw_id = r.req_id;
        vl2o.aw_user = r.req_user;
        vl2o.w_valid = 1;                                   // AXI-Lite request
        vl2o.w_last = 1;
        vl2o.w_data = r.req_wdata;
        vl2o.w_strb = r.req_wstrb;
        vl2o.w_user = r.req_user;
        if (i_l2i.read().aw_ready == 1) {
            if (i_l2i.read().w_ready == 1) {
                v.state = state_b;
            } else {
                v.state = state_w;
            }
        }
        break;
    case state_w:
        vl2o.w_valid = 1;
        vl2o.w_last = 1;
        vl2o.w_data = r.req_wdata;
        vl2o.w_strb = r.req_wstrb;
        vl2o.w_user = r.req_user;
        if (i_l2i.read().w_ready == 1) {
            v.state = state_b;
        }
        break;
    case state_b:
        vl2o.b_ready = 1;
        if (i_l2i.read().b_valid == 1) {
            v.resp = i_l2i.read().b_resp;
            v.state = l1_w_resp;
        }
        break;
    case l1_w_resp:
        vlxi[r.srcid.read().to_int()].b_valid = 1;
        vlxi[r.srcid.read().to_int()].b_resp = r.resp;
        vlxi[r.srcid.read().to_int()].b_id = r.req_id;
        vlxi[r.srcid.read().to_int()].b_user = r.req_user;
        if (vl1o[r.srcid.read().to_int()].b_ready == 1) {
            v.state = Idle;
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        L2Dummy_r_reset(v);
    }

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        o_l1i[i] = vlxi[i];
    }
    o_l2o = vl2o;
}

void L2Dummy::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        L2Dummy_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

