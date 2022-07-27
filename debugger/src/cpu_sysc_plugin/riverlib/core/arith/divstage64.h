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

SC_MODULE(divstage64) {
 public:
    sc_in<sc_uint<64>> i_divident;                          // integer value
    sc_in<sc_biguint<124>> i_divisor;                       // integer value
    sc_out<sc_uint<64>> o_resid;                            // residual
    sc_out<sc_uint<4>> o_bits;                              // resulting bits

    void comb();

    SC_HAS_PROCESS(divstage64);

    divstage64(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
};

}  // namespace debugger

