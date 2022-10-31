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

SC_MODULE(divstage53) {
 public:
    sc_in<bool> i_mux_ena;                                  // find first non-zero bit
    sc_in<sc_uint<56>> i_muxind;                            // bits indexes 8x7 bits bus
    sc_in<sc_uint<61>> i_divident;                          // integer value
    sc_in<sc_uint<53>> i_divisor;                           // integer value
    sc_out<sc_uint<53>> o_dif;                              // residual value
    sc_out<sc_uint<8>> o_bits;                              // resulting bits
    sc_out<sc_uint<7>> o_muxind;                            // first found non-zero bits
    sc_out<bool> o_muxind_rdy;                              // seeking was successfull

    void comb();

    SC_HAS_PROCESS(divstage53);

    divstage53(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_uint<62> wb_thresh[16];
    sc_uint<61> wb_dif[4];


};

}  // namespace debugger

