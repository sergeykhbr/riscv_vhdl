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
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(IntAddSub) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<7>> i_mode;                               // [0]0=rv64;1=rv32;[1]0=sign;1=unsign[2]Add[3]Sub[4]less[5]min[6]max
    sc_in<sc_uint<RISCV_ARCH>> i_a1;                        // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;                        // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;                      // Result

    void comb();
    void registers();

    SC_HAS_PROCESS(IntAddSub);

    IntAddSub(sc_module_name name,
              bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct IntAddSub_registers {
        sc_signal<sc_uint<RISCV_ARCH>> res;
    } v, r;

    void IntAddSub_r_reset(IntAddSub_registers &iv) {
        iv.res = 0ull;
    }

};

}  // namespace debugger

