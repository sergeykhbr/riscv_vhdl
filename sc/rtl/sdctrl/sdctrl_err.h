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
#include "sdctrl_cfg.h"

namespace debugger {

SC_MODULE(sdctrl_err) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_err_valid;
    sc_in<sc_uint<4>> i_err_code;
    sc_in<bool> i_err_clear;
    sc_out<sc_uint<4>> o_err_code;
    sc_out<bool> o_err_pending;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_err);

    sdctrl_err(sc_module_name name,
               bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct sdctrl_err_registers {
        sc_signal<sc_uint<4>> code;
    } v, r;

    void sdctrl_err_r_reset(sdctrl_err_registers &iv) {
        iv.code = CMDERR_NONE;
    }

};

}  // namespace debugger

