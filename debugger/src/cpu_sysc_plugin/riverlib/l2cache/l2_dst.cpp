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

#include "l2_dst.h"

namespace debugger {

L2Destination::L2Destination(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    i_resp_status("i_resp_status"),
    i_l1o0("i_l1o0"),
    o_l1i0("o_l1i0"),
    i_l1o1("i_l1o1"),
    o_l1i1("o_l1i1"),
    i_l1o2("i_l1o2"),
    o_l1i2("o_l1i2"),
    i_l1o3("i_l1o3"),
    o_l1i3("o_l1i3"),
    i_acpo("i_acpo"),
    o_acpi("o_acpi"),
    i_req_ready("i_req_ready"),
    o_req_valid("o_req_valid"),
    o_req_type("o_req_type"),
    o_req_addr("o_req_addr"),
    o_req_size("o_req_size"),
    o_req_prot("o_req_prot"),
    o_req_wdata("o_req_wdata"),
    o_req_wstrb("o_req_wstrb") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_l1o0;
    sensitive << i_l1o1;
    sensitive << i_l1o2;
    sensitive << i_l1o3;
    sensitive << i_acpo;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << i_resp_status;
    sensitive << i_req_ready;
    sensitive << r.state;
    sensitive << r.srcid;
    sensitive << r.req_addr;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.req_src;
    sensitive << r.req_type;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

L2Destination::~L2Destination() {
}

void L2Destination::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_ready, i_req_ready.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_type, o_req_type.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, i_resp_status, i_resp_status.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.srcid, pn + ".r_srcid");
    }
}

void L2Destination::comb() {
    axi4_l1_out_type vcoreo[SRC_MUX_WIDTH+1];
    axi4_l1_in_type vlxi[SRC_MUX_WIDTH];
    sc_uint<SRC_MUX_WIDTH> vb_src_aw;
    sc_uint<SRC_MUX_WIDTH> vb_src_ar;
    sc_uint<3> vb_srcid;
    sc_uint<4> vb_ar_type;
    sc_uint<3> vb_aw_type;
    bool v_req_valid;

    v = r;

    vcoreo[0] = i_acpo.read();
    vcoreo[1] = i_l1o0.read();
    vcoreo[2] = i_l1o1.read();
    vcoreo[3] = i_l1o2.read();
    vcoreo[4] = i_l1o3.read();
    vcoreo[5] = axi4_l1_out_none;

    v_req_valid = 0;
    vb_ar_type = AR_ReadNoSnoop;
    vb_aw_type = AW_WriteNoSnoop;
    vb_srcid = SRC_MUX_WIDTH;
    for (int i = 0; i < SRC_MUX_WIDTH; i++) {
        vlxi[i] = axi4_l1_in_none;

        vb_src_aw[i] = vcoreo[i].aw_valid;
        vb_src_ar[i] = vcoreo[i].ar_valid;
    }

    // select source (aw has higher priority):
    if (vb_src_aw.or_reduce() == 0) {
        for (int i = 0; i < SRC_MUX_WIDTH; i++) {
            if (vb_srcid == SRC_MUX_WIDTH && vb_src_ar[i] == 1) {
                vb_srcid = i;
                vb_ar_type = getArType(vcoreo[i]);
            }
        }
    } else {
        for (int i = 0; i < SRC_MUX_WIDTH; i++) {
            if (vb_srcid == SRC_MUX_WIDTH && vb_src_aw[i] == 1) {
                vb_srcid = i;
                vb_aw_type = getAwType(vcoreo[i]);
            }
        }
    }

    switch (r.state.read()) {
    case Idle:
        if (vb_src_aw.or_reduce() == 1) {
            v.state = CacheWriteReq;
            vlxi[vb_srcid.to_int()].aw_ready = 1;
            vlxi[vb_srcid.to_int()].w_ready = 1;        // Lite-interface

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[vb_srcid.to_int()].aw_bits.addr;
            v.req_size = vcoreo[vb_srcid.to_int()].aw_bits.size;
            v.req_prot = vcoreo[vb_srcid.to_int()].aw_bits.prot;
            if (vcoreo[vb_srcid.to_int()].aw_bits.cache[0] == 1) {
                v.req_type = L2_ReqCachedWrite;
            } else {
                v.req_type = L2_ReqUncachedWrite;
            }
        } else if (vb_src_ar.or_reduce() == 1) {
            v.state = CacheReadReq;
            vlxi[vb_srcid.to_int()].ar_ready = 1;

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[vb_srcid.to_int()].ar_bits.addr;
            v.req_size = vcoreo[vb_srcid.to_int()].ar_bits.size;
            v.req_prot = vcoreo[vb_srcid.to_int()].ar_bits.prot;
            if (vcoreo[vb_srcid.to_int()].ar_bits.cache[0] == 1) {
                v.req_type = L2_ReqCachedRead;
            } else {
                v.req_type = L2_ReqUncachedRead;
            }
        }
        // Lite-interface
        v.req_wdata = vcoreo[vb_srcid.to_int()].w_data;
        v.req_wstrb = vcoreo[vb_srcid.to_int()].w_strb;
        break;
    case CacheReadReq:
        v_req_valid = 1;
        if (i_req_ready.read() == 1) {
            v.state = ReadMem;
        }
        break;
    case CacheWriteReq:
        v_req_valid = 1;
        if (i_req_ready.read() == 1) {
            v.state = WriteMem;
        }
        break;
    case ReadMem:
        vlxi[r.srcid.read().to_int()].r_valid = i_resp_valid;
        vlxi[r.srcid.read().to_int()].r_last = i_resp_valid;    // Lite interface
        vlxi[r.srcid.read().to_int()].r_data = i_resp_rdata;
        if (i_resp_status.read() == 0) {
            vlxi[r.srcid.read().to_int()].r_resp = 0;
        } else {
            vlxi[r.srcid.read().to_int()].r_resp = 0x2;    // SLVERR
        }
        if (i_resp_valid.read() == 1) {
            v.state = Idle;    // Wouldn't implement wait to accept because L1 is always ready
        }
        break;
    case WriteMem:
        vlxi[r.srcid.read().to_int()].b_valid = i_resp_valid;
        if (i_resp_status.read() == 0) {
            vlxi[r.srcid.read().to_int()].b_resp = 0;
        } else {
            vlxi[r.srcid.read().to_int()].b_resp = 0x2;    // SLVERR
        }
        if (i_resp_valid.read() == 1) {
            v.state = Idle;    // Wouldn't implement wait to accept because L1 is always ready
        }
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_acpi = vlxi[0];
    o_l1i0 = vlxi[1];
    o_l1i1 = vlxi[2];
    o_l1i2 = vlxi[3];
    o_l1i3 = vlxi[4];

    o_req_valid = v_req_valid;
    o_req_type = r.req_type;
    o_req_addr = r.req_addr;
    o_req_size = r.req_size;
    o_req_prot = r.req_prot;
    o_req_wdata = r.req_wdata;
    o_req_wstrb = r.req_wstrb;
}

void L2Destination::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
