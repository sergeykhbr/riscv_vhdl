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

#include "vip_clk.h"
#include "api_core.h"

namespace debugger {

vip_clk::vip_clk(sc_module_name name,
                 double period)
    : sc_module(name),
    o_clk("o_clk"),
    pll("pll", period, SC_SEC) {

    period_ = period;
    // initial
    // pll = 0;
    // end initial

    SC_METHOD(comb);
    sensitive << pll;
}

void vip_clk::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_clk, o_clk.name());
    }

}

void vip_clk::comb() {
    o_clk = pll;
}

}  // namespace debugger

