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

template<int iwidth = 105,                                  // Input bus width
         int shiftwidth = 7>                                // Encoded value width
SC_MODULE(zeroenc) {
 public:
    sc_in<sc_biguint<iwidth>> i_value;                      // Input value to encode
    sc_out<sc_uint<shiftwidth>> o_shift;                    // First non-zero bit

    void gen0();

    SC_HAS_PROCESS(zeroenc);

    zeroenc(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<sc_uint<shiftwidth>> wb_muxind[(iwidth + 1)];


};

template<int iwidth, int shiftwidth>
zeroenc<iwidth, shiftwidth>::zeroenc(sc_module_name name)
    : sc_module(name),
    i_value("i_value"),
    o_shift("o_shift") {


    SC_METHOD(gen0);
    sensitive << i_value;
    for (int i = 0; i < (iwidth + 1); i++) {
        sensitive << wb_muxind[i];
    }
}

template<int iwidth, int shiftwidth>
void zeroenc<iwidth, shiftwidth>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_value, i_value.name());
        sc_trace(o_vcd, o_shift, o_shift.name());
    }

}

template<int iwidth, int shiftwidth>
void zeroenc<iwidth, shiftwidth>::gen0() {
    wb_muxind[iwidth] = 0;
    for (int i = (iwidth - 1); i >= 0; i--) {
        wb_muxind[i] = (i_value.read()[i] == 1) ? i : wb_muxind[(i + 1)].read();
    }
    o_shift = wb_muxind[0];
}

}  // namespace debugger

