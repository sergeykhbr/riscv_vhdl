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

#pragma once

#include "api_core.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(JtagCDC) {

 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    // tck clock
    sc_in<bool> i_dmi_req_valid;
    sc_in<bool> i_dmi_req_write;
    sc_in<sc_uint<7>> i_dmi_req_addr;
    sc_in<sc_uint<32>> i_dmi_req_data;
    sc_in<bool> i_dmi_reset;
    sc_in<bool> i_dmi_hardreset;
    // system clock
    sc_in<bool> i_dmi_req_ready;
    sc_out<bool> o_dmi_req_valid;
    sc_out<bool> o_dmi_req_write;
    sc_out<sc_uint<7>> o_dmi_req_addr;
    sc_out<sc_uint<32>> o_dmi_req_data;
    sc_out<bool> o_dmi_reset;
    sc_out<bool> o_dmi_hardreset;

    void comb();
    void registers();

    SC_HAS_PROCESS(JtagCDC);

    JtagCDC(sc_module_name name, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    struct RegistersType {
        sc_signal<sc_uint<32+7+4>> l1;
        sc_signal<sc_uint<32+7+4>> l2;
        sc_signal<bool> req_valid;
        sc_signal<bool> req_accepted;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<7>> req_addr;
        sc_signal<sc_uint<32>> req_data;
        sc_signal<bool> req_reset;
        sc_signal<bool> req_hardreset;
    } r, v;


    void R_RESET(RegistersType &iv) {
        iv.l1 = 0;
        iv.l2 = 0;
        iv.req_valid = 0;
        iv.req_accepted = 0;
        iv.req_write = 0;
        iv.req_addr = 0;
        iv.req_data = 0;
        iv.req_reset = 0;
        iv.req_hardreset = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

