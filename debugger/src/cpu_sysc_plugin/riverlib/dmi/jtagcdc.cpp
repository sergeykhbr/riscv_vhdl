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

#include "jtagcdc.h"

namespace debugger {

JtagCDC::JtagCDC(sc_module_name name, bool async_reset) : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_dmi_req_valid("i_dmi_req_valid"),
    i_dmi_req_write("i_dmi_req_write"),
    i_dmi_req_addr("i_dmi_req_addr"),
    i_dmi_req_data("i_dmi_req_data"),
    i_dmi_reset("i_dmi_reset"),
    i_dmi_hardreset("i_dmi_hardreset"),
    i_dmi_req_ready("i_dmi_req_ready"),
    o_dmi_req_valid("o_dmi_req_valid"),
    o_dmi_req_write("o_dmi_req_write"),
    o_dmi_req_addr("o_dmi_req_addr"),
    o_dmi_req_data("o_dmi_req_data"),
    o_dmi_reset("o_dmi_reset"),
    o_dmi_hardreset("o_dmi_hardreset") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dmi_req_valid;
    sensitive << i_dmi_req_write;
    sensitive << i_dmi_req_addr;
    sensitive << i_dmi_req_data;
    sensitive << i_dmi_reset;
    sensitive << i_dmi_hardreset;
    sensitive << i_dmi_req_ready;
    sensitive << r.l1;
    sensitive << r.l2;
    sensitive << r.req_valid;
    sensitive << r.req_accepted;
    sensitive << r.req_write;
    sensitive << r.req_addr;
    sensitive << r.req_data;
    sensitive << r.req_reset;
    sensitive << r.req_hardreset;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
    sensitive << i_nrst;
}

void JtagCDC::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
    }
}


void JtagCDC::comb() {
    sc_uint<32+7+4> vb_bus;

    v = r;

    vb_bus[0] = i_dmi_req_valid;
    vb_bus[1] = i_dmi_req_write;
    vb_bus(33, 2) = i_dmi_req_data;
    vb_bus(40, 34) = i_dmi_req_addr;
    vb_bus[41] = i_dmi_reset;
    vb_bus[42] = i_dmi_hardreset;

    v.l1 = vb_bus;
    v.l2 = r.l1;
    if (r.l2.read()[0] && !r.req_valid && !r.req_accepted) {
        // To avoid request repeading
        v.req_valid = 1;
        v.req_write = r.l2.read()[1];
        v.req_data = r.l2.read()(33, 2);
        v.req_addr = r.l2.read()(40, 34);
        v.req_reset = r.l2.read()[41];
        v.req_hardreset = r.l2.read()[42];
    } else if (i_dmi_req_ready) {
        v.req_valid = 0;
    }
    if (r.l2.read()[0] && r.req_valid && i_dmi_req_ready) {
        v.req_accepted = 1;
    } else if (!r.l2.read()[0]) {
        v.req_accepted = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_dmi_req_valid = r.req_valid;
    o_dmi_req_write = r.req_write;
    o_dmi_req_data = r.req_data;
    o_dmi_req_addr = r.req_addr;
    o_dmi_reset = r.req_reset;
    o_dmi_hardreset = r.req_hardreset;
}

void JtagCDC::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

