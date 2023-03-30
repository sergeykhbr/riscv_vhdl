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

namespace debugger {

SC_MODULE(jtagcdc) {
 public:
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                                     // full reset including dmi (usually via reset button)
    // tck clock
    sc_in<bool> i_dmi_req_valid;
    sc_in<bool> i_dmi_req_write;
    sc_in<sc_uint<7>> i_dmi_req_addr;
    sc_in<sc_uint<32>> i_dmi_req_data;
    sc_in<bool> i_dmi_hardreset;
    // system clock
    sc_in<bool> i_dmi_req_ready;
    sc_out<bool> o_dmi_req_valid;
    sc_out<bool> o_dmi_req_write;
    sc_out<sc_uint<7>> o_dmi_req_addr;
    sc_out<sc_uint<32>> o_dmi_req_data;
    sc_out<bool> o_dmi_hardreset;

    void comb();
    void registers();

    SC_HAS_PROCESS(jtagcdc);

    jtagcdc(sc_module_name name,
            bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int CDC_REG_WIDTH = (1  // i_dmi_hardreset
            + 7  // i_dmi_req_addr
            + 32  // i_dmi_req_data
            + 1  // i_dmi_req_write
            + 1  // i_dmi_req_valid
    );

    struct jtagcdc_registers {
        sc_signal<sc_uint<CDC_REG_WIDTH>> l1;
        sc_signal<sc_uint<CDC_REG_WIDTH>> l2;
        sc_signal<bool> req_valid;
        sc_signal<bool> req_accepted;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<7>> req_addr;
        sc_signal<sc_uint<32>> req_data;
        sc_signal<bool> req_hardreset;
    } v, r;

    void jtagcdc_r_reset(jtagcdc_registers &iv) {
        iv.l1 = ~0ull;
        iv.l2 = 0ull;
        iv.req_valid = 0;
        iv.req_accepted = 0;
        iv.req_write = 0;
        iv.req_addr = 0;
        iv.req_data = 0;
        iv.req_hardreset = 0;
    }

};

}  // namespace debugger

