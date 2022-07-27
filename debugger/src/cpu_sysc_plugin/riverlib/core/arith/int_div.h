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
#include "divstage64.h"

namespace debugger {

SC_MODULE(IntDiv) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;                                      // Enable bit
    sc_in<bool> i_unsigned;                                 // Unsigned operands
    sc_in<bool> i_rv32;                                     // 32-bits operands enabled
    sc_in<bool> i_residual;                                 // Compute: 0 =division; 1=residual
    sc_in<sc_uint<RISCV_ARCH>> i_a1;                        // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;                        // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;                      // Result
    sc_out<bool> o_valid;                                   // Result is valid

    void comb();
    void registers();

    SC_HAS_PROCESS(IntDiv);

    IntDiv(sc_module_name name,
           bool async_reset);
    virtual ~IntDiv();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct IntDiv_registers {
        sc_signal<bool> rv32;
        sc_signal<bool> resid;
        sc_signal<bool> invert;
        sc_signal<bool> div_on_zero;
        sc_signal<bool> overflow;
        sc_signal<bool> busy;
        sc_signal<sc_uint<10>> ena;
        sc_signal<sc_uint<64>> divident_i;
        sc_signal<sc_biguint<120>> divisor_i;
        sc_signal<sc_uint<64>> bits_i;
        sc_signal<sc_uint<RISCV_ARCH>> result;
        sc_signal<sc_uint<RISCV_ARCH>> reference_div;
        sc_signal<sc_uint<64>> a1_dbg;                      // Store this value for output in a case of error
        sc_signal<sc_uint<64>> a2_dbg;
    } v, r;

    void IntDiv_r_reset(IntDiv_registers &iv) {
        iv.rv32 = 0;
        iv.resid = 0;
        iv.invert = 0;
        iv.div_on_zero = 0;
        iv.overflow = 0;
        iv.busy = 0;
        iv.ena = 0;
        iv.divident_i = 0ull;
        iv.divisor_i = 0ull;
        iv.bits_i = 0ull;
        iv.result = 0ull;
        iv.reference_div = 0ull;
        iv.a1_dbg = 0ull;
        iv.a2_dbg = 0ull;
    }

    sc_signal<sc_biguint<124>> wb_divisor0_i;
    sc_signal<sc_biguint<124>> wb_divisor1_i;
    sc_signal<sc_uint<64>> wb_resid0_o;
    sc_signal<sc_uint<64>> wb_resid1_o;
    sc_signal<sc_uint<4>> wb_bits0_o;
    sc_signal<sc_uint<4>> wb_bits1_o;

    divstage64 *stage0;
    divstage64 *stage1;

};

}  // namespace debugger

