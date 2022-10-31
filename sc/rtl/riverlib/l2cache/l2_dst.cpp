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

#include "l2_dst.h"
#include "api_core.h"

namespace debugger {

L2Destination::L2Destination(sc_module_name name,
                             bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    i_resp_status("i_resp_status"),
    i_l1o("i_l1o", CFG_SLOT_L1_TOTAL),
    o_l1i("o_l1i", CFG_SLOT_L1_TOTAL),
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
    sensitive << i_nrst;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << i_resp_status;
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        sensitive << i_l1o[i];
    }
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
    sensitive << r.ac_valid;
    sensitive << r.cr_ready;
    sensitive << r.cd_ready;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void L2Destination::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, i_resp_status, i_resp_status.name());
        sc_trace(o_vcd, i_req_ready, i_req_ready.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_type, o_req_type.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_size, o_req_size.name());
        sc_trace(o_vcd, o_req_prot, o_req_prot.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, o_req_wstrb, o_req_wstrb.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.srcid, pn + ".r_srcid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_size, pn + ".r_req_size");
        sc_trace(o_vcd, r.req_prot, pn + ".r_req_prot");
        sc_trace(o_vcd, r.req_src, pn + ".r_req_src");
        sc_trace(o_vcd, r.req_type, pn + ".r_req_type");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.ac_valid, pn + ".r_ac_valid");
        sc_trace(o_vcd, r.cr_ready, pn + ".r_cr_ready");
        sc_trace(o_vcd, r.cd_ready, pn + ".r_cd_ready");
    }

}

void L2Destination::comb() {
    axi4_l1_out_type vcoreo[(CFG_SLOT_L1_TOTAL + 1)];
    axi4_l1_in_type vlxi[CFG_SLOT_L1_TOTAL];
    sc_uint<CFG_SLOT_L1_TOTAL> vb_src_aw;
    sc_uint<CFG_SLOT_L1_TOTAL> vb_src_ar;
    sc_uint<(CFG_SLOT_L1_TOTAL + 1)> vb_broadband_mask_full;
    sc_uint<(CFG_SLOT_L1_TOTAL + 1)> vb_broadband_mask;
    sc_uint<(CFG_SLOT_L1_TOTAL + 1)> vb_ac_valid;
    sc_uint<(CFG_SLOT_L1_TOTAL + 1)> vb_cr_ready;
    sc_uint<(CFG_SLOT_L1_TOTAL + 1)> vb_cd_ready;
    sc_uint<3> vb_srcid;
    bool v_req_valid;
    sc_uint<L2_REQ_TYPE_BITS> vb_req_type;

    for (int i = 0; i < (CFG_SLOT_L1_TOTAL + 1); i++) {
        vcoreo[i] = axi4_l1_out_none;
    }
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        vlxi[i] = axi4_l1_in_none;
    }
    vb_src_aw = 0;
    vb_src_ar = 0;
    vb_broadband_mask_full = 0;
    vb_broadband_mask = 0;
    vb_ac_valid = 0;
    vb_cr_ready = 0;
    vb_cd_ready = 0;
    vb_srcid = 0;
    v_req_valid = 0;
    vb_req_type = 0;

    v = r;

    vb_req_type = r.req_type;

    vb_srcid = CFG_SLOT_L1_TOTAL;
    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        vcoreo[i] = i_l1o[i];                               // Cannot read vector item from port in systemc
        vlxi[i] = axi4_l1_in_none;

        vb_src_aw[i] = vcoreo[i].aw_valid;
        vb_src_ar[i] = vcoreo[i].ar_valid;
    }
    vcoreo[CFG_SLOT_L1_TOTAL] = axi4_l1_out_none;

    // select source (aw has higher priority):
    if (vb_src_aw.or_reduce() == 0) {
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            if ((vb_srcid == CFG_SLOT_L1_TOTAL) && (vb_src_ar[i] == 1)) {
                vb_srcid = i;
            }
        }
    } else {
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            if ((vb_srcid == CFG_SLOT_L1_TOTAL) && (vb_src_aw[i] == 1)) {
                vb_srcid = i;
            }
        }
    }

    vb_ac_valid = r.ac_valid;
    vb_cr_ready = r.cr_ready;
    vb_cd_ready = r.cd_ready;

    vb_broadband_mask_full = ~0ull;
    vb_broadband_mask_full[CFG_SLOT_L1_TOTAL] = 0;          // exclude empty slot
    vb_broadband_mask = vb_broadband_mask_full;
    vb_broadband_mask[vb_srcid.to_int()] = 0;               // exclude source

    switch (r.state.read()) {
    case Idle:
        vb_req_type = 0;
        if (vb_src_aw.or_reduce() == 1) {
            v.state = CacheWriteReq;
            vlxi[vb_srcid.to_int()].aw_ready = 1;
            vlxi[vb_srcid.to_int()].w_ready = 1;            // Lite-interface

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[vb_srcid.to_int()].aw_bits.addr;
            v.req_size = vcoreo[vb_srcid.to_int()].aw_bits.size;
            v.req_prot = vcoreo[vb_srcid.to_int()].aw_bits.prot;
            vb_req_type[L2_REQ_TYPE_WRITE] = 1;
            if (vcoreo[vb_srcid.to_int()].aw_bits.cache[0] == 1) {
                vb_req_type[L2_REQ_TYPE_CACHED] = 1;
                if (vcoreo[vb_srcid.to_int()].aw_snoop == AWSNOOP_WRITE_LINE_UNIQUE) {
                    vb_req_type[L2_REQ_TYPE_UNIQUE] = 1;
                    v.ac_valid = vb_broadband_mask;
                    v.cr_ready = 0;
                    v.cd_ready = 0;
                    v.state = snoop_ac;
                }
            }
        } else if (vb_src_ar.or_reduce() == 1) {
            v.state = CacheReadReq;
            vlxi[vb_srcid.to_int()].ar_ready = 1;

            v.srcid = vb_srcid;
            v.req_addr = vcoreo[vb_srcid.to_int()].ar_bits.addr;
            v.req_size = vcoreo[vb_srcid.to_int()].ar_bits.size;
            v.req_prot = vcoreo[vb_srcid.to_int()].ar_bits.prot;
            if (vcoreo[vb_srcid.to_int()].ar_bits.cache[0] == 1) {
                vb_req_type[L2_REQ_TYPE_CACHED] = 1;
                if (vcoreo[vb_srcid.to_int()].ar_snoop == ARSNOOP_READ_MAKE_UNIQUE) {
                    vb_req_type[L2_REQ_TYPE_UNIQUE] = 1;
                }
                // prot[2]: 0=Data, 1=Instr.
                // If source is I$ then request D$ of the same CPU
                if (vcoreo[vb_srcid.to_int()].ar_bits.prot[2] == 1) {
                    v.ac_valid = vb_broadband_mask_full;
                } else {
                    v.ac_valid = vb_broadband_mask;
                }
                v.cr_ready = 0;
                v.cd_ready = 0;
                v.state = snoop_ac;
            }
        }
        v.req_type = vb_req_type;
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
        vlxi[r.srcid.read().to_int()].r_last = i_resp_valid;// Lite interface
        if (r.req_type.read()[L2_REQ_TYPE_SNOOP] == 1) {
            vlxi[r.srcid.read().to_int()].r_data = r.req_wdata;
        } else {
            vlxi[r.srcid.read().to_int()].r_data = i_resp_rdata;
        }
        if (i_resp_status.read().or_reduce() == 0) {
            vlxi[r.srcid.read().to_int()].r_resp = AXI_RESP_OKAY;
        } else {
            vlxi[r.srcid.read().to_int()].r_resp = AXI_RESP_SLVERR;
        }
        if (i_resp_valid.read() == 1) {
            v.state = Idle;                                 // Wouldn't implement wait to accept because L1 is always ready
        }
        break;
    case WriteMem:
        vlxi[r.srcid.read().to_int()].b_valid = i_resp_valid;
        if (i_resp_status.read().or_reduce() == 0) {
            vlxi[r.srcid.read().to_int()].b_resp = AXI_RESP_OKAY;
        } else {
            vlxi[r.srcid.read().to_int()].b_resp = AXI_RESP_SLVERR;
        }
        if (i_resp_valid.read() == 1) {
            v.state = Idle;                                 // Wouldn't implement wait to accept because L1 is always ready
        }
        break;
    case snoop_ac:
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            vlxi[i].ac_valid = r.ac_valid.read()[i];
            vlxi[i].ac_addr = r.req_addr;
            if (r.req_type.read()[L2_REQ_TYPE_UNIQUE] == 1) {
                vlxi[i].ac_snoop = AC_SNOOP_READ_UNIQUE;
            } else {
                vlxi[i].ac_snoop = 0;
            }
            if ((r.ac_valid.read()[i] == 1) && (vcoreo[i].ac_ready == 1)) {
                vb_ac_valid[i] = 0;
                vb_cr_ready[i] = 1;
            }
        }
        v.ac_valid = vb_ac_valid;
        v.cr_ready = vb_cr_ready;
        if (vb_ac_valid.or_reduce() == 0) {
            v.state = snoop_cr;
        }
        break;
    case snoop_cr:
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            vlxi[i].cr_ready = r.cr_ready.read()[i];
            if ((r.cr_ready.read()[i] == 1) && (vcoreo[i].cr_valid == 1)) {
                vb_cr_ready[i] = 0;
                if (vcoreo[i].cr_resp[0] == 1) {            // data transaction flag ACE spec
                    vb_cd_ready[i] = 1;
                }
            }
        }
        v.cr_ready = vb_cr_ready;
        v.cd_ready = vb_cd_ready;
        if (vb_cr_ready.or_reduce() == 0) {
            if (vb_cd_ready.or_reduce() == 1) {
                v.state = snoop_cd;
            } else if (r.req_type.read()[L2_REQ_TYPE_WRITE] == 1) {
                v.state = CacheWriteReq;
            } else {
                v.state = CacheReadReq;
            }
        }
        break;
    case snoop_cd:
        // Here only to read Unique data from L1 and write to L2
        for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
            vlxi[i].cd_ready = r.cd_ready.read()[i];
            if ((r.cd_ready.read()[i] == 1) && (vcoreo[i].cd_valid == 1)) {
                vb_cd_ready[i] = 0;
                v.req_wdata = vcoreo[i].cd_data;
            }
        }
        v.cd_ready = vb_cd_ready;
        if (vb_cd_ready.or_reduce() == 0) {
            if (r.req_type.read()[L2_REQ_TYPE_WRITE] == 1) {
                v.state = CacheWriteReq;
            } else {
                v.state = CacheReadReq;
                v.req_wstrb = ~0ull;
            }
            // write to L2 for Read and Write requests
            vb_req_type[L2_REQ_TYPE_WRITE] = 1;
            vb_req_type[L2_REQ_TYPE_SNOOP] = 1;
            v.req_type = vb_req_type;
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        L2Destination_r_reset(v);
    }

    for (int i = 0; i < CFG_SLOT_L1_TOTAL; i++) {
        o_l1i[i] = vlxi[i];                                 // vector should be assigned in cycle in systemc
    }

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
        L2Destination_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

