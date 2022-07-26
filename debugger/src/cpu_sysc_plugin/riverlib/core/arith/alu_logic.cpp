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

#include "alu_logic.h"
#include "api_core.h"

namespace debugger {

AluLogic::AluLogic(sc_module_name name,
                   bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mode("i_mode"),
    i_a1("i_a1"),
    i_a2("i_a2"),
    o_res("o_res") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mode;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.res;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void AluLogic::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mode, i_mode.name());
        sc_trace(o_vcd, i_a1, i_a1.name());
        sc_trace(o_vcd, i_a2, i_a2.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, r.res, pn + ".r_res");
    }

}

void AluLogic::comb() {
    v = r;

    if (i_mode.read()[1] == 1) {
        v.res = (i_a1.read() | i_a2.read());
    } else if (i_mode.read()[2] == 1) {
        v.res = (i_a1.read() ^ i_a2.read());
    } else {
        v.res = (i_a1.read() & i_a2.read());
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        AluLogic_r_reset(v);
    }

    o_res = r.res;
}

void AluLogic::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        AluLogic_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

