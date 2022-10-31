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

SC_MODULE(Double2Long) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;
    sc_in<bool> i_signed;
    sc_in<bool> i_w32;
    sc_in<sc_uint<64>> i_a;                                 // Operand 1
    sc_out<sc_uint<64>> o_res;                              // Result
    sc_out<bool> o_overflow;
    sc_out<bool> o_underflow;
    sc_out<bool> o_valid;                                   // Result is valid
    sc_out<bool> o_busy;                                    // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(Double2Long);

    Double2Long(sc_module_name name,
                bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct Double2Long_registers {
        sc_signal<bool> busy;
        sc_signal<sc_uint<3>> ena;
        sc_signal<bool> signA;
        sc_signal<sc_uint<11>> expA;
        sc_signal<sc_uint<53>> mantA;
        sc_signal<sc_uint<64>> result;
        sc_signal<bool> op_signed;
        sc_signal<bool> w32;
        sc_signal<sc_uint<64>> mantPostScale;
        sc_signal<bool> overflow;
        sc_signal<bool> underflow;
    } v, r;

    void Double2Long_r_reset(Double2Long_registers &iv) {
        iv.busy = 0;
        iv.ena = 0;
        iv.signA = 0;
        iv.expA = 0;
        iv.mantA = 0ull;
        iv.result = 0ull;
        iv.op_signed = 0;
        iv.w32 = 0;
        iv.mantPostScale = 0ull;
        iv.overflow = 0;
        iv.underflow = 0;
    }

};

}  // namespace debugger

