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
#pragma once

#include <systemc.h>
#include "../../../../rtl/sim/pll/pll_generic.h"
#include "vip_jtag_tap.h"

namespace debugger {

SC_MODULE(jtag_app) {
 public:
    sc_out<bool> o_trst;                                    // Must be open-train, pullup
    sc_out<bool> o_tck;
    sc_out<bool> o_tms;
    sc_out<bool> o_tdo;
    sc_in<bool> i_tdi;

    void init();
    void comb();
    void test_clk1();

    jtag_app(sc_module_name name);
    virtual ~jtag_app();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<bool> w_nrst;
    sc_signal<bool> w_tck;
    sc_uint<32> wb_clk1_cnt;
    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<4>> wb_req_irlen;
    sc_signal<sc_uint<16>> wb_req_ir;
    sc_signal<sc_uint<7>> wb_req_drlen;
    sc_signal<sc_uint<64>> wb_req_dr;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<64>> wb_resp_data;

    pll_generic *clk1;
    vip_jtag_tap *tap;

};

}  // namespace debugger

