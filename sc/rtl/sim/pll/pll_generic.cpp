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

#include "pll_generic.h"
#include "api_core.h"

namespace debugger {

pll_generic::pll_generic(sc_module_name name,
                         double period)
    : sc_module(name),
    o_clk("o_clk") {

    period_ = period;

    SC_THREAD(comb);
}

void pll_generic::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_clk, o_clk.name());
    }

}

void pll_generic::comb() {
    while (true) {
        wait(static_cast<int>((0.5 * period_)), SC_NS);
        o_clk = 0;
        wait(static_cast<int>((0.5 * period_)), SC_NS);
        o_clk = 1;
    }
}

}  // namespace debugger

