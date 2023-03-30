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

#include "jtagcdc.h"
#include "api_core.h"

namespace debugger {

jtagcdc::jtagcdc(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_dmi_req_valid("i_dmi_req_valid"),
    i_dmi_req_write("i_dmi_req_write"),
    i_dmi_req_addr("i_dmi_req_addr"),
    i_dmi_req_data("i_dmi_req_data"),
    i_dmi_hardreset("i_dmi_hardreset"),
    i_dmi_req_ready("i_dmi_req_ready"),
    o_dmi_req_valid("o_dmi_req_valid"),
    o_dmi_req_write("o_dmi_req_write"),
    o_dmi_req_addr("o_dmi_req_addr"),
    o_dmi_req_data("o_dmi_req_data"),
    o_dmi_hardreset("o_dmi_hardreset") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dmi_req_valid;
    sensitive << i_dmi_req_write;
    sensitive << i_dmi_req_addr;
    sensitive << i_dmi_req_data;
    sensitive << i_dmi_hardreset;
    sensitive << i_dmi_req_ready;
    sensitive << r.l1;
    sensitive << r.l2;
    sensitive << r.req_valid;
    sensitive << r.req_accepted;
    sensitive << r.req_write;
    sensitive << r.req_addr;
    sensitive << r.req_data;
    sensitive << r.req_hardreset;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void jtagcdc::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_dmi_req_valid, i_dmi_req_valid.name());
        sc_trace(o_vcd, i_dmi_req_write, i_dmi_req_write.name());
        sc_trace(o_vcd, i_dmi_req_addr, i_dmi_req_addr.name());
        sc_trace(o_vcd, i_dmi_req_data, i_dmi_req_data.name());
        sc_trace(o_vcd, i_dmi_hardreset, i_dmi_hardreset.name());
        sc_trace(o_vcd, i_dmi_req_ready, i_dmi_req_ready.name());
        sc_trace(o_vcd, o_dmi_req_valid, o_dmi_req_valid.name());
        sc_trace(o_vcd, o_dmi_req_write, o_dmi_req_write.name());
        sc_trace(o_vcd, o_dmi_req_addr, o_dmi_req_addr.name());
        sc_trace(o_vcd, o_dmi_req_data, o_dmi_req_data.name());
        sc_trace(o_vcd, o_dmi_hardreset, o_dmi_hardreset.name());
        sc_trace(o_vcd, r.l1, pn + ".r_l1");
        sc_trace(o_vcd, r.l2, pn + ".r_l2");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_accepted, pn + ".r_req_accepted");
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_data, pn + ".r_req_data");
        sc_trace(o_vcd, r.req_hardreset, pn + ".r_req_hardreset");
    }

}

void jtagcdc::comb() {
    sc_uint<CDC_REG_WIDTH> vb_bus;

    vb_bus = 0;

    v = r;

    vb_bus = (i_dmi_hardreset,
            i_dmi_req_addr,
            i_dmi_req_data,
            i_dmi_req_write,
            i_dmi_req_valid);

    v.l1 = vb_bus;
    v.l2 = r.l1;
    if ((r.l2.read()[0] && (!r.req_valid) && (!r.req_accepted)) == 1) {
        // To avoid request repeading
        v.req_valid = 1;
        v.req_write = r.l2.read()[1];
        v.req_data = r.l2.read()(33, 2);
        v.req_addr = r.l2.read()(40, 34);
        v.req_hardreset = r.l2.read()[41];
    } else if (i_dmi_req_ready.read() == 1) {
        v.req_valid = 0;
    }
    if ((r.l2.read()[0] && r.req_valid && i_dmi_req_ready) == 1) {
        v.req_accepted = 1;
    } else if (r.l2.read()[0] == 0) {
        v.req_accepted = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        jtagcdc_r_reset(v);
    }

    o_dmi_req_valid = r.req_valid;
    o_dmi_req_write = r.req_write;
    o_dmi_req_data = r.req_data;
    o_dmi_req_addr = r.req_addr;
    o_dmi_hardreset = r.req_hardreset;
}

void jtagcdc::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        jtagcdc_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

