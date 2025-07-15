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
#include "api_core.h"

namespace debugger {

template<int width = 8>                                     // Bus width
SC_MODULE(obuf_arr_tech) {
 public:
    sc_in<sc_uint<width>> i;                                // Input signal
    sc_out<sc_uint<width>> o;                               // Output signal

    void comb();

    obuf_arr_tech(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
};

template<int width>
obuf_arr_tech<width>::obuf_arr_tech(sc_module_name name)
    : sc_module(name),
    i("i"),
    o("o") {


    SC_METHOD(comb);
    sensitive << i;
}

template<int width>
void obuf_arr_tech<width>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i, i.name());
        sc_trace(o_vcd, o, o.name());
    }

}

template<int width>
void obuf_arr_tech<width>::comb() {
    o = i.read();
}

}  // namespace debugger

