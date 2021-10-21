/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "jtagtap.h"

namespace debugger {

JtagTap::JtagTap(sc_module_name name) : sc_module(name),
    i_nrst("i_nrst"),
    i_trst("i_trst"),
    i_tck("i_tck"),
    i_tms("i_tms"),
    i_tdi("i_tdi"),
    o_tdo("o_tdo"),
    o_dmi_req_valid("o_dmi_req_valid"),
    o_dmi_req_write("o_dmi_req_write"),
    o_dmi_req_addr("o_dmi_req_addr"),
    o_dmi_req_data("o_dmi_req_data"),
    i_dmi_resp_data("i_dmi_resp_data"),
    i_dmi_busy("i_dmi_busy"),
    i_dmi_error("i_dmi_error"),
    o_dmi_reset("o_dmi_reset"),
    o_dmi_hardreset("o_dmi_hardreset") {

    SC_METHOD(comb);
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << i_dmi_resp_data;
    sensitive << i_dmi_busy;
    sensitive << i_dmi_error;
    sensitive << r.state;
    sensitive << r.tck;
    sensitive << r.tms;
    sensitive << r.tdi;
    sensitive << r.dr_length;
    sensitive << r.dr;
    sensitive << nr.ir;
    sensitive << nr.dmi_addr;

    SC_METHOD(registers);
    sensitive << i_tck.pos();
    sensitive << i_trst;
    sensitive << i_nrst;

    SC_METHOD(nregisters);
    sensitive << i_tck.neg();
    sensitive << i_trst;
    sensitive << i_nrst;
}

void JtagTap::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_trst, i_trst.name());
        sc_trace(o_vcd, i_tck, i_tck.name());
        sc_trace(o_vcd, i_tms, i_tms.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, nr.ir, pn + ".nr_ir");
        sc_trace(o_vcd, r.dr, pn + ".r_dr");
        sc_trace(o_vcd, r.datacnt, pn + ".r_datacnt");
    }
}


void JtagTap::comb() {
    sc_uint<drlen> vb_dr;
    sc_uint<2> vb_stat;
    bool v_dmi_req_valid;
    bool v_dmi_req_write;
    sc_uint<32> vb_dmi_req_data;
    sc_uint<abits> vb_dmi_req_addr;
    bool v_dmi_reset;
    bool v_dmi_hardreset;

    v = r;
    nv = nr;
    vb_dr = r.dr;
    v_dmi_req_valid = 0;
    v_dmi_req_write = 0;
    vb_dmi_req_data = 0;
    vb_dmi_req_addr = 0;
    v_dmi_reset = 0;
    v_dmi_hardreset = 0;


    if (i_dmi_busy.read()) {
        vb_stat = DMISTAT_BUSY;
    } else if (i_dmi_error.read()) {
        vb_stat = DMISTAT_FAILED;
    } else {
        vb_stat = DMISTAT_SUCCESS;
    }

    switch (r.state.read()) {
    case CAPTURE_DR:
        if (nr.ir.read() == IR_IDCODE) {
            vb_dr = idcode;
            v.dr_length = 32;
        } else if (nr.ir.read() == IR_DTMCONTROL) {
            vb_dr(31, 0) = 0;
            vb_dr(3, 0) = 0x1;      // version
            vb_dr(9, 4) = abits;    // the size of the address
            vb_dr(11, 10) = vb_stat;
            v.dr_length = 32;
        } else if (nr.ir.read() == IR_DBUS) {
            vb_dr(1, 0) = vb_stat;
            vb_dr(33, 2) = i_dmi_resp_data;
            vb_dr(34 + abits - 1, 34) = nr.dmi_addr;
            v.dr_length = abits + 34;
        } else if (nr.ir.read() == IR_BYPASS) {
            vb_dr[0] = r.bypass;
            v.dr_length = 1;
        }
        v.datacnt = 0;
        break;
    case SHIFT_DR:
        vb_dr[r.dr_length.read().to_uint()-1] = i_tdi;
        if (r.dr_length.read() > 1) {
            // For the bypass dr_length = 1
            vb_dr(r.dr_length.read().to_uint() - 2, 0)
                = r.dr.read()(r.dr_length.read().to_uint() - 1, 1);
        }
        v.datacnt = r.datacnt + 1;  // debug counter no need in rtl
        break;
    case UPDATE_DR:
        if (nr.ir.read() == IR_DTMCONTROL) {
            v_dmi_reset = r.dr.read()[DTMCONTROL_DMIRESET];
            v_dmi_hardreset = r.dr.read()[DTMCONTROL_DMIHARDRESET];
        } else if (nr.ir.read() == IR_BYPASS) {
            v.bypass = r.dr.read()[0];
        } else if (nr.ir.read() == IR_DBUS) {
            v_dmi_req_valid = r.dr.read()(1, 0).or_reduce();
            v_dmi_req_write = r.dr.read()[1];
            vb_dmi_req_data = r.dr.read()(33, 2);
            vb_dmi_req_addr = r.dr.read()(34 + abits - 1, 34);

            nv.dmi_addr = r.dr.read()(34 + abits - 1, 34);
        }
        break;
    case CAPTURE_IR:
        vb_dr(irlen-1, 2) = nr.ir.read()(irlen-1, 2);
        vb_dr(1, 0) = 0x1;
        break;
    case SHIFT_IR:
        vb_dr[irlen - 1] = i_tdi;
        vb_dr(irlen - 2, 0) = r.dr.read()(irlen - 1, 1);
        break;
    case UPDATE_IR:
        nv.ir = r.dr.read()(irlen-1, 0);
        break;
    default:;
    }
    v.dr = vb_dr;
    v.state = next[r.state.read()][i_tms.read()];

    o_tdo = r.dr.read()[0];
    o_dmi_req_valid = v_dmi_req_valid;
    o_dmi_req_write = v_dmi_req_write;
    o_dmi_req_data = vb_dmi_req_data;
    o_dmi_req_addr = vb_dmi_req_addr;
    o_dmi_reset = v_dmi_reset;
    o_dmi_hardreset = v_dmi_hardreset;
}

void JtagTap::registers() {
    if (!i_nrst.read()) {
        R_RESET(r);
    } else {
        r = v;
    }
}

void JtagTap::nregisters() {
    if (!i_nrst.read()) {
        NR_RESET(nr);
    } else {
        nr = nv;
    }
}

}  // namespace debugger

