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

SC_MODULE(IntMul) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;                                      // Enable bit
    sc_in<bool> i_unsigned;                                 // Unsigned operands
    sc_in<bool> i_hsu;                                      // MULHSU instruction: signed * unsigned
    sc_in<bool> i_high;                                     // High multiplied bits [127:64]
    sc_in<bool> i_rv32;                                     // 32-bits operands enabled
    sc_in<sc_uint<RISCV_ARCH>> i_a1;                        // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;                        // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;                      // Result
    sc_out<bool> o_valid;                                   // Result is valid

    void comb();
    void registers();

    IntMul(sc_module_name name,
           bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct IntMul_registers {
        sc_signal<bool> busy;
        sc_signal<sc_uint<4>> ena;
        sc_signal<sc_uint<RISCV_ARCH>> a1;
        sc_signal<sc_uint<RISCV_ARCH>> a2;
        sc_signal<bool> unsign;
        sc_signal<bool> high;
        sc_signal<bool> rv32;
        sc_signal<bool> zero;
        sc_signal<bool> inv;
        sc_signal<sc_biguint<128>> result;
        sc_signal<sc_uint<RISCV_ARCH>> a1_dbg;
        sc_signal<sc_uint<RISCV_ARCH>> a2_dbg;
        sc_signal<sc_uint<RISCV_ARCH>> reference_mul;       // Used for run-time comparision
        sc_signal<sc_biguint<69>> lvl1[16];
        sc_signal<sc_biguint<83>> lvl3[4];
    };

    void IntMul_r_reset(IntMul_registers& iv) {
        iv.busy = 0;
        iv.ena = 0;
        iv.a1 = 0;
        iv.a2 = 0;
        iv.unsign = 0;
        iv.high = 0;
        iv.rv32 = 0;
        iv.zero = 0;
        iv.inv = 0;
        iv.result = 0;
        iv.a1_dbg = 0;
        iv.a2_dbg = 0;
        iv.reference_mul = 0;
        for (int i = 0; i < 16; i++) {
            iv.lvl1[i] = 0;
        }
        for (int i = 0; i < 4; i++) {
            iv.lvl3[i] = 0;
        }
    }

    IntMul_registers v;
    IntMul_registers r;

};

}  // namespace debugger

