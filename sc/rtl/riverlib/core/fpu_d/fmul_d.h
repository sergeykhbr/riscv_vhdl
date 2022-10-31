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
#include "imul53.h"

namespace debugger {

SC_MODULE(DoubleMul) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;
    sc_in<sc_uint<64>> i_a;                                 // Operand 1
    sc_in<sc_uint<64>> i_b;                                 // Operand 2
    sc_out<sc_uint<64>> o_res;                              // Result
    sc_out<bool> o_illegal_op;
    sc_out<bool> o_overflow;
    sc_out<bool> o_valid;                                   // Result is valid
    sc_out<bool> o_busy;                                    // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(DoubleMul);

    DoubleMul(sc_module_name name,
              bool async_reset);
    virtual ~DoubleMul();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct DoubleMul_registers {
        sc_signal<bool> busy;
        sc_signal<sc_uint<5>> ena;
        sc_signal<sc_uint<64>> a;
        sc_signal<sc_uint<64>> b;
        sc_signal<sc_uint<64>> result;
        sc_signal<bool> zeroA;
        sc_signal<bool> zeroB;
        sc_signal<sc_uint<53>> mantA;
        sc_signal<sc_uint<53>> mantB;
        sc_signal<sc_uint<13>> expAB;
        sc_signal<sc_uint<12>> expAlign;
        sc_signal<sc_biguint<105>> mantAlign;
        sc_signal<sc_uint<12>> postShift;
        sc_signal<sc_biguint<105>> mantPostScale;
        sc_signal<bool> nanA;
        sc_signal<bool> nanB;
        sc_signal<bool> overflow;
        sc_signal<bool> illegal_op;
    } v, r;

    void DoubleMul_r_reset(DoubleMul_registers &iv) {
        iv.busy = 0;
        iv.ena = 0;
        iv.a = 0ull;
        iv.b = 0ull;
        iv.result = 0ull;
        iv.zeroA = 0;
        iv.zeroB = 0;
        iv.mantA = 0ull;
        iv.mantB = 0ull;
        iv.expAB = 0;
        iv.expAlign = 0;
        iv.mantAlign = 0ull;
        iv.postShift = 0;
        iv.mantPostScale = 0ull;
        iv.nanA = 0;
        iv.nanB = 0;
        iv.overflow = 0;
        iv.illegal_op = 0;
    }

    sc_signal<bool> w_imul_ena;
    sc_signal<sc_biguint<106>> wb_imul_result;
    sc_signal<sc_uint<7>> wb_imul_shift;
    sc_signal<bool> w_imul_rdy;
    sc_signal<bool> w_imul_overflow;

    imul53 *u_imul53;

};

}  // namespace debugger

