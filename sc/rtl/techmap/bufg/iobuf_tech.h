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

SC_MODULE(iobuf_tech) {
 public:
    sc_inout<bool> io;                                      // bi-drectional port
    sc_out<bool> o;                                         // Output signal is valid when t=0
    sc_in<bool> i;                                          // Input signal is valid when t=1
    sc_in<bool> t;                                          // Direction bit: 0=output; 1=input

    void comb();

    SC_HAS_PROCESS(iobuf_tech);

    iobuf_tech(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
};

}  // namespace debugger

