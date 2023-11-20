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

#include "sdctrl_err.h"
#include "api_core.h"

namespace debugger {

sdctrl_err::sdctrl_err(sc_module_name name,
                       bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_err_valid("i_err_valid"),
    i_err_code("i_err_code"),
    i_err_clear("i_err_clear"),
    o_err_code("o_err_code"),
    o_err_pending("o_err_pending") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_err_valid;
    sensitive << i_err_code;
    sensitive << i_err_clear;
    sensitive << r.code;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_err::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_err_valid, i_err_valid.name());
        sc_trace(o_vcd, i_err_code, i_err_code.name());
        sc_trace(o_vcd, i_err_clear, i_err_clear.name());
        sc_trace(o_vcd, o_err_code, o_err_code.name());
        sc_trace(o_vcd, o_err_pending, o_err_pending.name());
        sc_trace(o_vcd, r.code, pn + ".r_code");
    }

}

void sdctrl_err::comb() {
    v = r;

    if (i_err_clear.read() == 1) {
        v.code = CMDERR_NONE;
    } else if ((i_err_valid.read() == 1)
                && (r.code.read() == CMDERR_NONE)) {
        v.code = i_err_code;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_err_r_reset(v);
    }

    o_err_code = r.code;
    o_err_pending = r.code.read().or_reduce();
}

void sdctrl_err::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_err_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

