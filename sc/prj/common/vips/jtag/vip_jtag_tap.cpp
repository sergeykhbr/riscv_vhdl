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

#include "vip_jtag_tap.h"
#include "api_core.h"

namespace debugger {

vip_jtag_tap::vip_jtag_tap(sc_module_name name)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_tck("i_tck"),
    i_req_valid("i_req_valid"),
    i_req_irlen("i_req_irlen"),
    i_req_ir("i_req_ir"),
    i_req_drlen("i_req_drlen"),
    i_req_dr("i_req_dr"),
    o_resp_valid("o_resp_valid"),
    o_resp_data("o_resp_data"),
    o_trst("o_trst"),
    o_tck("o_tck"),
    o_tms("o_tms"),
    o_tdo("o_tdo"),
    i_tdi("i_tdi") {


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_tck;
    sensitive << i_req_valid;
    sensitive << i_req_irlen;
    sensitive << i_req_ir;
    sensitive << i_req_drlen;
    sensitive << i_req_dr;
    sensitive << i_tdi;
    sensitive << w_tck;
    sensitive << r.req_valid;
    sensitive << r.req_irlen;
    sensitive << r.req_drlen;
    sensitive << r.req_ir;
    sensitive << r.req_dr;
    sensitive << r.state;
    sensitive << r.dr_length;
    sensitive << r.dr;
    sensitive << r.bypass;
    sensitive << r.datacnt;
    sensitive << r.shiftreg;
    sensitive << r.resp_valid;
    sensitive << r.resp_data;
    sensitive << r.ir;
    sensitive << rn.trst;
    sensitive << rn.tms;
    sensitive << rn.tdo;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_tck.pos();

    SC_METHOD(rnegisters);
    sensitive << i_nrst;
    sensitive << i_tck.neg();
}

void vip_jtag_tap::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_tck, i_tck.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_irlen, i_req_irlen.name());
        sc_trace(o_vcd, i_req_ir, i_req_ir.name());
        sc_trace(o_vcd, i_req_drlen, i_req_drlen.name());
        sc_trace(o_vcd, i_req_dr, i_req_dr.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_trst, o_trst.name());
        sc_trace(o_vcd, o_tck, o_tck.name());
        sc_trace(o_vcd, o_tms, o_tms.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
        sc_trace(o_vcd, r.req_valid, pn + ".r.req_valid");
        sc_trace(o_vcd, r.req_irlen, pn + ".r.req_irlen");
        sc_trace(o_vcd, r.req_drlen, pn + ".r.req_drlen");
        sc_trace(o_vcd, r.req_ir, pn + ".r.req_ir");
        sc_trace(o_vcd, r.req_dr, pn + ".r.req_dr");
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.dr_length, pn + ".r.dr_length");
        sc_trace(o_vcd, r.dr, pn + ".r.dr");
        sc_trace(o_vcd, r.bypass, pn + ".r.bypass");
        sc_trace(o_vcd, r.datacnt, pn + ".r.datacnt");
        sc_trace(o_vcd, r.shiftreg, pn + ".r.shiftreg");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_data, pn + ".r.resp_data");
        sc_trace(o_vcd, r.ir, pn + ".r.ir");
        sc_trace(o_vcd, rn.trst, pn + ".rn.trst");
        sc_trace(o_vcd, rn.tms, pn + ".rn.tms");
        sc_trace(o_vcd, rn.tdo, pn + ".rn.tdo");
    }

}

void vip_jtag_tap::comb() {
    sc_uint<64> vb_shiftreg;
    int vi_dr_idx;

    vn = rn;
    v = r;
    vb_shiftreg = r.shiftreg.read();
    vi_dr_idx = 0;

    vn.tms = 0;
    v.resp_valid = 0;
    if (r.req_drlen.read().or_reduce() == 1) {
        vi_dr_idx = (r.req_drlen.read().to_int() - 1);
    } else {
        vi_dr_idx = 0;
    }

    if (i_req_valid.read() == 1) {
        v.req_valid = 1;
        v.req_irlen = i_req_irlen.read();
        v.req_ir = i_req_ir.read();
        v.req_drlen = i_req_drlen.read();
        v.req_dr = i_req_dr.read();
    }

    switch (r.state.read()) {
    case INIT_RESET:
        vn.trst = 1;
        vn.tms = 1;
        // 127 clocks to reset JTAG state machine without TRST
        if (r.dr_length.read().or_reduce() == 1) {
            v.dr_length = (r.dr_length.read() - 1);
        } else {
            v.state = RESET_TAP;
        }
        break;
    case RESET_TAP:
        vn.trst = 1;
        v.ir = ~0ull;
        v.state = IDLE;
        break;
    case IDLE:
        vn.trst = 0;
        v.resp_data = 0;
        if (r.req_valid.read() == 1) {
            v.req_valid = 0;
            vn.tms = 1;
            v.state = SELECT_DR_SCAN1;
        }
        break;
    case SELECT_DR_SCAN1:
        vn.tms = 1;
        v.state = SELECT_IR_SCAN;
        break;
    case SELECT_IR_SCAN:
        v.state = CAPTURE_IR;
        v.ir = r.req_ir.read();
        break;
    case CAPTURE_IR:
        v.state = SHIFT_IR;
        vb_shiftreg = r.ir.read();
        v.datacnt = r.req_irlen.read();
        break;
    case SHIFT_IR:
        if (r.datacnt.read() <= 1) {
            vn.tms = 1;
            v.state = EXIT1_IR;
        } else {
            vb_shiftreg = (0, r.shiftreg.read()(63, 1));
            v.datacnt = (r.datacnt.read() - 1);
        }
        break;
    case EXIT1_IR:
        vn.tms = 1;
        v.state = UPDATE_IR;
        break;
    case UPDATE_IR:
        vn.tms = 1;
        v.state = SELECT_DR_SCAN;
        break;
    case SELECT_DR_SCAN:
        v.state = CAPTURE_DR;
        break;
    case CAPTURE_DR:
        vb_shiftreg = r.req_dr.read();
        v.datacnt = r.req_drlen.read();
        v.state = SHIFT_DR;
        break;
    case SHIFT_DR:
        vb_shiftreg = (0, r.shiftreg.read()(63, 1));
        if (r.datacnt.read() <= 1) {
            vn.tms = 1;
            v.state = EXIT1_DR;
        } else {
            v.datacnt = (r.datacnt.read() - 1);
        }
        break;
    case EXIT1_DR:
        vn.tms = 1;
        v.state = UPDATE_DR;
        break;
    case UPDATE_DR:
        v.resp_valid = 1;
        v.resp_data = r.shiftreg.read();
        v.state = IDLE;
        break;
    default:
        v.state = IDLE;
        break;
    }

    vn.tdo = r.shiftreg.read()[0];
    vb_shiftreg[vi_dr_idx] = i_tdi.read();
    v.shiftreg = vb_shiftreg;
    o_trst = rn.trst.read();
    o_tck = i_tck.read();
    o_tms = rn.tms.read();
    o_tdo = rn.tdo.read();
    o_resp_valid = r.resp_valid.read();
    o_resp_data = r.resp_data.read();
}

void vip_jtag_tap::registers() {
    if (i_nrst.read() == 0) {
        vip_jtag_tap_r_reset(r);
    } else {
        r = v;
    }
}

void vip_jtag_tap::rnegisters() {
    if (i_nrst.read() == 0) {
        vip_jtag_tap_rn_reset(rn);
    } else {
        rn = vn;
    }
}

}  // namespace debugger

