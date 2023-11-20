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

#include "sdctrl_wdog.h"
#include "api_core.h"

namespace debugger {

sdctrl_wdog::sdctrl_wdog(sc_module_name name,
                         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_period("i_period"),
    o_trigger("o_trigger") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_period;
    sensitive << r.cnt;
    sensitive << r.trigger;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_wdog::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_period, i_period.name());
        sc_trace(o_vcd, o_trigger, o_trigger.name());
        sc_trace(o_vcd, r.cnt, pn + ".r_cnt");
        sc_trace(o_vcd, r.trigger, pn + ".r_trigger");
    }

}

void sdctrl_wdog::comb() {
    v = r;

    v.trigger = 0;
    if (i_ena.read() == 0) {
        v.cnt = i_period;
    } else if (r.cnt.read().or_reduce() == 1) {
        v.cnt = (r.cnt.read() - 1);
    } else {
        v.trigger = 1;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_wdog_r_reset(v);
    }

    o_trigger = r.trigger;
}

void sdctrl_wdog::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_wdog_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

