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

#include "ids_tech.h"
#include "api_core.h"

namespace debugger {

ids_tech::ids_tech(sc_module_name name)
    : sc_module(name),
    i_clk_p("i_clk_p"),
    i_clk_n("i_clk_n"),
    o_clk("o_clk") {


    SC_METHOD(comb);
    sensitive << i_clk_p;
    sensitive << i_clk_n;
}

void ids_tech::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_clk_p, i_clk_p.name());
        sc_trace(o_vcd, i_clk_n, i_clk_n.name());
        sc_trace(o_vcd, o_clk, o_clk.name());
    }

}

void ids_tech::comb() {
    o_clk = i_clk_p;
}

}  // namespace debugger

