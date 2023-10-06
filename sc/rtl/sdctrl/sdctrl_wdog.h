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

SC_MODULE(sdctrl_wdog) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;
    sc_in<sc_uint<16>> i_period;
    sc_out<bool> o_trigger;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_wdog);

    sdctrl_wdog(sc_module_name name,
                bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct sdctrl_wdog_registers {
        sc_signal<sc_uint<16>> cnt;
        sc_signal<bool> trigger;
    } v, r;

    void sdctrl_wdog_r_reset(sdctrl_wdog_registers &iv) {
        iv.cnt = 0;
        iv.trigger = 0;
    }

};

}  // namespace debugger

