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

#include "dummycpu.h"
#include "api_core.h"

namespace debugger {

DummyCpu::DummyCpu(sc_module_name name)
    : sc_module(name),
    o_msto("o_msto"),
    o_dport("o_dport"),
    o_flush_l2("o_flush_l2"),
    o_halted("o_halted"),
    o_available("o_available") {


    SC_METHOD(comb);
}

void DummyCpu::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, o_dport, o_dport.name());
        sc_trace(o_vcd, o_flush_l2, o_flush_l2.name());
        sc_trace(o_vcd, o_halted, o_halted.name());
        sc_trace(o_vcd, o_available, o_available.name());
    }

}

void DummyCpu::comb() {
    o_msto = axi4_l1_out_none;
    o_dport = dport_out_none;
    o_flush_l2 = 0;
    o_halted = 0;
    o_available = 0;
}

}  // namespace debugger

