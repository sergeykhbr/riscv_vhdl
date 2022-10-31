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

SC_MODULE(DoubleAdd) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;
    sc_in<bool> i_add;
    sc_in<bool> i_sub;
    sc_in<bool> i_eq;
    sc_in<bool> i_lt;
    sc_in<bool> i_le;
    sc_in<bool> i_max;
    sc_in<bool> i_min;
    sc_in<sc_uint<64>> i_a;                                 // Operand 1
    sc_in<sc_uint<64>> i_b;                                 // Operand 2
    sc_out<sc_uint<64>> o_res;                              // Result
    sc_out<bool> o_illegal_op;                              // nanA | nanB
    sc_out<bool> o_overflow;
    sc_out<bool> o_valid;                                   // Result is valid
    sc_out<bool> o_busy;                                    // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(DoubleAdd);

    DoubleAdd(sc_module_name name,
              bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct DoubleAdd_registers {
        sc_signal<bool> busy;
        sc_signal<sc_uint<8>> ena;
        sc_signal<sc_uint<64>> a;
        sc_signal<sc_uint<64>> b;
        sc_signal<sc_uint<64>> result;
        sc_signal<bool> illegal_op;
        sc_signal<bool> overflow;
        sc_signal<bool> add;
        sc_signal<bool> sub;
        sc_signal<bool> eq;
        sc_signal<bool> lt;
        sc_signal<bool> le;
        sc_signal<bool> max;
        sc_signal<bool> min;
        sc_signal<bool> flMore;
        sc_signal<bool> flEqual;
        sc_signal<bool> flLess;
        sc_signal<sc_uint<12>> preShift;
        sc_signal<bool> signOpMore;
        sc_signal<sc_uint<11>> expMore;
        sc_signal<sc_uint<53>> mantMore;
        sc_signal<sc_uint<53>> mantLess;
        sc_signal<sc_biguint<105>> mantLessScale;
        sc_signal<sc_biguint<106>> mantSum;
        sc_signal<sc_uint<7>> lshift;
        sc_signal<sc_biguint<105>> mantAlign;
        sc_signal<sc_uint<12>> expPostScale;
        sc_signal<sc_uint<12>> expPostScaleInv;
        sc_signal<sc_biguint<105>> mantPostScale;
    } v, r;

    void DoubleAdd_r_reset(DoubleAdd_registers &iv) {
        iv.busy = 0;
        iv.ena = 0;
        iv.a = 0ull;
        iv.b = 0ull;
        iv.result = 0ull;
        iv.illegal_op = 0;
        iv.overflow = 0;
        iv.add = 0;
        iv.sub = 0;
        iv.eq = 0;
        iv.lt = 0;
        iv.le = 0;
        iv.max = 0;
        iv.min = 0;
        iv.flMore = 0;
        iv.flEqual = 0;
        iv.flLess = 0;
        iv.preShift = 0;
        iv.signOpMore = 0;
        iv.expMore = 0;
        iv.mantMore = 0ull;
        iv.mantLess = 0ull;
        iv.mantLessScale = 0ull;
        iv.mantSum = 0ull;
        iv.lshift = 0;
        iv.mantAlign = 0ull;
        iv.expPostScale = 0;
        iv.expPostScaleInv = 0;
        iv.mantPostScale = 0ull;
    }

};

}  // namespace debugger

