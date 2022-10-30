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
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    o_apbi("o_apbi"),
    i_apbo("i_apbo") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xslvi;
    sensitive << i_apbo;
    sensitive << r.state;
    sensitive << r.paddr;
    sensitive << r.pdata;
    sensitive << r.pwrite;
    sensitive << r.pstrb;
    sensitive << r.pprot;
    sensitive << r.pselx;
    sensitive << r.penable;
    sensitive << r.pslverr;
    sensitive << r.xsize;
    sensitive << r.req_id;
    sensitive << r.req_user;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void axi2apb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, o_apbi, o_apbi.name());
        sc_trace(o_vcd, i_apbo, i_apbo.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.paddr, pn + ".r_paddr");
        sc_trace(o_vcd, r.pdata, pn + ".r_pdata");
        sc_trace(o_vcd, r.pwrite, pn + ".r_pwrite");
        sc_trace(o_vcd, r.pstrb, pn + ".r_pstrb");
        sc_trace(o_vcd, r.pprot, pn + ".r_pprot");
        sc_trace(o_vcd, r.pselx, pn + ".r_pselx");
        sc_trace(o_vcd, r.penable, pn + ".r_penable");
        sc_trace(o_vcd, r.pslverr, pn + ".r_pslverr");
        sc_trace(o_vcd, r.xsize, pn + ".r_xsize");
        sc_trace(o_vcd, r.req_id, pn + ".r_req_id");
        sc_trace(o_vcd, r.req_user, pn + ".r_req_user");
    }

}

sc_uint<8> axi2apb::size2len(sc_uint<3> size) {
    sc_uint<8> ret;

    switch (size) {
    case 4:                                                 // 16 Bytes
        ret = 1;
        break;
    case 5:                                                 // 32 Bytes
        ret = 3;
        break;
    case 6:                                                 // 64 Bytes
        ret = 7;
        break;
    case 7:                                                 // 128 Bytes
        ret = 15;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}

sc_uint<3> axi2apb::size2size(sc_uint<3> size) {
    sc_uint<3> ret;

    if (size >= 3) {
        ret = 3;
    } else {
        ret = size;
    }
    return ret;
}

void axi2apb::comb() {
    bool v_req_mem_ready;
    bool v_r_valid;
    bool v_w_valid;
    bool v_w_last;
    bool v_w_ready;
    sc_uint<8> vb_len;
    sc_uint<3> vb_size;
    axi4_slave_out_type vslvo;
    apb_in_type vapbi;

    v_req_mem_ready = 0;
    v_r_valid = 0;
    v_w_valid = 0;
    v_w_last = 0;
    v_w_ready = 0;
    vb_len = 0;
    vb_size = 0;

    v = r;

    switch (r.state.read()) {
    case State_Idle:
        v.pslverr = 0;
        v.penable = 0;
        v.pselx = 0;
        v.xsize = 0;
        vslvo.aw_ready = 1;
        vslvo.w_ready = 1;                                  // AXILite support
        vslvo.ar_ready = (!i_xslvi.read().aw_valid);
        if (i_xslvi.read().aw_valid == 1) {
            v.pwrite = 1;
            v.paddr = (i_xslvi.read().aw_bits.addr(31, 2) << 2);
            v.pprot = i_xslvi.read().aw_bits.prot;
            v.req_id = i_xslvi.read().aw_id;
            v.req_user = i_xslvi.read().aw_user;
            if (i_xslvi.read().aw_bits.size >= 3) {
                v.xsize = 1;
                v.pdata = i_xslvi.read().w_data;
                v.pstrb = i_xslvi.read().w_strb;
            } else if (i_xslvi.read().aw_bits.addr[2] == 0) {
                v.pdata = (0, i_xslvi.read().w_data(31, 0));
                v.pstrb = (0, i_xslvi.read().w_strb(3, 0));
            } else {
                v.pdata = (0, i_xslvi.read().w_data(63, 32));
                v.pstrb = (0, i_xslvi.read().w_strb(7, 4));
            }
            if (i_xslvi.read().aw_bits.len.or_reduce() == 1) {
                v.state = State_err;                        // Burst is not supported
            } else if (i_xslvi.read().w_valid == 1) {
                // AXILite support
                v.state = State_setup;
                v.pselx = 1;
            } else {
                v.state = State_w;
            }
        } else if (i_xslvi.read().ar_valid == 1) {
            v.pwrite = 0;
            v.pselx = 1;
            v.paddr = (i_xslvi.read().ar_bits.addr(31, 2) << 2);
            v.pprot = i_xslvi.read().ar_bits.prot;
            v.req_id = i_xslvi.read().ar_id;
            v.req_user = i_xslvi.read().ar_user;
            if (i_xslvi.read().ar_bits.size >= 3) {
                v.xsize = 1;
            }
            if (i_xslvi.read().ar_bits.len.or_reduce() == 1) {
                v.state = State_err;                        // Burst is not supported
            } else {
                v.state = State_setup;
            }
        }
        break;
    case State_w:
        vslvo.w_ready = 1;
        v.pselx = 1;
        if (r.xsize.read().or_reduce() == 1) {
            v.pdata = i_xslvi.read().w_data;
            v.pstrb = i_xslvi.read().w_strb;
        } else if (r.paddr.read()[2] == 0) {
            v.pdata = (0, i_xslvi.read().w_data(31, 0));
            v.pstrb = (0, i_xslvi.read().w_strb(3, 0));
        } else {
            v.pdata = (0, i_xslvi.read().w_data(63, 32));
            v.pstrb = (0, i_xslvi.read().w_strb(7, 4));
        }
        if (i_xslvi.read().w_valid == 1) {
            v.state = State_setup;
        }
        break;
    case State_b:
        vslvo.b_valid = 1;
        if (i_xslvi.read().b_ready == 1) {
            v.state = State_Idle;
        }
        break;
    case State_r:
        vslvo.r_valid = 1;
        if (i_xslvi.read().r_ready == 1) {
            v.state = State_Idle;
        }
        break;
    case State_setup:
        v.penable = 1;
        v.state = State_access;
        break;
    case State_access:
        if (r.pwrite.read() == 0) {
            if (r.xsize.read().or_reduce() == 1) {
                v.pdata(63, 32) = i_apbo.read().prdata;
            } else {
                v.pdata(31, 0) = i_apbo.read().prdata;
            }
        }
        v.pslverr = i_apbo.read().pslverr;
        if (i_apbo.read().pready == 1) {
            v.penable = 0;
            if (r.xsize.read().or_reduce() == 1) {
                v.xsize = (r.xsize.read() - 1);
                v.paddr = (r.paddr.read() + 4);
                v.pstrb = (0, r.pstrb.read()(7, 4));
                v.pdata = (0, r.pdata.read()(63, 32));
                v.state = State_setup;
            } else if (r.pwrite.read() == 1) {
                v.state = State_b;
            } else {
                v.state = State_r;
            }
        }
        break;
    default:
        // Burst transactions are not supported:
        v.pdata = ~0ull;
        v.pslverr = 1;
        if (r.pwrite.read() == 1) {
            v.state = State_b;
        } else {
            v.state = State_r;
        }
        break;
    }

    vapbi.paddr = r.paddr;
    vapbi.pwrite = r.pwrite;
    vapbi.pwdata = r.pdata.read()(31, 0);
    vapbi.pstrb = r.pstrb.read()(3, 0);
    vapbi.pselx = r.pselx;
    vapbi.penable = r.penable;
    vapbi.pprot = r.pprot;

    vslvo.r_data = r.pdata;
    vslvo.r_resp = (r.pslverr.read() << 1);
    vslvo.r_id = r.req_id;
    vslvo.r_user = r.req_user;
    vslvo.r_last = 1;

    vslvo.b_resp = (r.pslverr.read() << 1);
    vslvo.b_id = r.req_id;
    vslvo.b_user = r.req_user;

    if (!async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(v);
    }

    o_xslvo = vslvo;
    o_apbi = vapbi;
}

void axi2apb::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

