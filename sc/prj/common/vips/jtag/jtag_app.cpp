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

#include "jtag_app.h"
#include "api_core.h"

namespace debugger {

jtag_app::jtag_app(sc_module_name name)
    : sc_module(name),
    o_trst("o_trst"),
    o_tck("o_tck"),
    o_tms("o_tms"),
    o_tdo("o_tdo"),
    i_tdi("i_tdi") {

    clk1 = 0;
    tap = 0;

    clk1 = new pll_generic("clk1",
                            80.0);
    clk1->o_clk(w_tck);

    tap = new vip_jtag_tap("tap");
    tap->i_nrst(w_nrst);
    tap->i_tck(w_tck);
    tap->i_req_valid(w_req_valid);
    tap->i_req_irlen(wb_req_irlen);
    tap->i_req_ir(wb_req_ir);
    tap->i_req_drlen(wb_req_drlen);
    tap->i_req_dr(wb_req_dr);
    tap->o_resp_valid(w_resp_valid);
    tap->o_resp_data(wb_resp_data);
    tap->o_trst(o_trst);
    tap->o_tck(o_tck);
    tap->o_tms(o_tms);
    tap->o_tdo(o_tdo);
    tap->i_tdi(i_tdi);

    SC_THREAD(init);

    SC_METHOD(comb);
    sensitive << i_tdi;
    sensitive << w_nrst;
    sensitive << w_tck;
    sensitive << w_req_valid;
    sensitive << wb_req_irlen;
    sensitive << wb_req_ir;
    sensitive << wb_req_drlen;
    sensitive << wb_req_dr;
    sensitive << w_resp_valid;
    sensitive << wb_resp_data;

    SC_METHOD(test_clk1);
    sensitive << w_tck.posedge_event();
}

jtag_app::~jtag_app() {
    if (clk1) {
        delete clk1;
    }
    if (tap) {
        delete tap;
    }
}

void jtag_app::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_trst, o_trst.name());
        sc_trace(o_vcd, o_tck, o_tck.name());
        sc_trace(o_vcd, o_tms, o_tms.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
    }

    if (clk1) {
        clk1->generateVCD(i_vcd, o_vcd);
    }
    if (tap) {
        tap->generateVCD(i_vcd, o_vcd);
    }
}

void jtag_app::init() {
    w_nrst = 0;
    wait(static_cast<int>(800.0), SC_NS);
    w_nrst = 1;
}

void jtag_app::comb() {
}

void jtag_app::test_clk1() {
    if (w_nrst.read() == 0) {
        wb_clk1_cnt = 0;
    } else {
        wb_clk1_cnt = (wb_clk1_cnt + 1);
    }

    if (wb_clk1_cnt == 500) {
        w_req_valid = 1;
        wb_req_irlen = 5;
        wb_req_ir = 0x01;
        wb_req_drlen = 32;
        wb_req_dr = 0;
    } else if (wb_clk1_cnt == 750) {
        w_req_valid = 1;
        wb_req_irlen = 5;
        wb_req_ir = 0x10;                                   // 0x10 DTM_CONTROL
        wb_req_drlen = 32;
        wb_req_dr = 0x12345678;
    } else {
        w_req_valid = 0;
    }
}

}  // namespace debugger

