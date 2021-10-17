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
    i_dmi_rdata("i_dmi_rdata"),
    i_dmi_busy("i_dmi_busy"),
    o_dmi_hardreset("o_dmi_hardreset") {

    SC_METHOD(comb);
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << i_dmi_rdata;
    sensitive << i_dmi_busy;
    sensitive << r.state;
    sensitive << r.tck;
    sensitive << r.tms;
    sensitive << r.tdi;
    sensitive << r.dr_length;
    sensitive << r.dr;

    SC_METHOD(registers);
    sensitive << i_tck.pos();
    sensitive << i_trst;

    SC_METHOD(nregisters);
    sensitive << i_tck.neg();
    sensitive << i_trst;
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

    v = r;
    nv = nr;
    vb_dr = r.dr;

    int t = r.dr_length.read().to_int();
    unsigned t_ir = nr.ir.read();

    switch (r.state.read()) {
    case CAPTURE_DR:
        if (nr.ir.read() == IR_IDCODE) {
            vb_dr = idcode;
            v.dr_length = 32;
        } else if (nr.ir.read() == IR_DTMCONTROL) {
            vb_dr(31, 0) = 0;
            vb_dr(3, 0) = 0x1;      // version
            vb_dr(9, 4) = abits;    // the size of the address
            vb_dr(11, 10) = r.dmistat;
            v.dr_length = 32;
        } else if (nr.ir.read() == IR_DBUS) {
            vb_dr = i_dmi_rdata;        // Result of previous DMI transaction
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
            if (r.dr.read()[DTMCONTROL_DMIRESET] == 1) {
                v.dmistat = DMISTAT_SUCCESS;
            } if (r.dr.read()[DTMCONTROL_DMIHARDRESET]) {
                //reset();
            }
        } else if (nr.ir.read() == IR_BYPASS) {
            v.bypass = r.dr.read()[0];
        } else if (nr.ir.read() == IR_DBUS) {
            /*unsigned op = get_field(dr, DMI_OP);
            uint32_t data = get_field(dr, DMI_DATA);
            unsigned address = get_field(dr, DMI_ADDRESS);

            dmi = dr;

            bool success = true;
            if (op == DMI_OP_READ) {
              uint32_t value;
              if (dm->dmi_read(address, &value)) {
                dmi = set_field(dmi, DMI_DATA, value);
              } else {
                success = false;
              }
            } else if (op == DMI_OP_WRITE) {
              success = dm->dmi_write(address, data);
            }

            if (success) {
              dmi = set_field(dmi, DMI_OP, DMI_OP_STATUS_SUCCESS);
            } else {
              dmi = set_field(dmi, DMI_OP, DMI_OP_STATUS_FAILED);
            }
            D(fprintf(stderr, "dmi=0x%lx\n", dmi));

            rti_remaining = required_rti_cycles;*/
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

