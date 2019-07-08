/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RIVERLIB_INT_DIV_H__
#define __DEBUGGER_RIVERLIB_INT_DIV_H__

#include <systemc.h>
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(IntDiv) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                 // Reset: active LOW
    sc_in<bool> i_ena;                  // Enable pulse
    sc_in<bool> i_unsigned;             // Unsigned operands
    sc_in<bool> i_rv32;                 // 32-bits instruction flag
    sc_in<bool> i_residual;             // Compute: 0 =division; 1=residual
    sc_in<sc_uint<RISCV_ARCH>> i_a1;    // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;    // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;  // Result
    sc_out<bool> o_valid;               // Result is valid
    sc_out<bool> o_busy;                // Multiclock instruction processing

    void comb();
    void registers();

    SC_HAS_PROCESS(IntDiv);

    IntDiv(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    uint64_t compute_reference(bool unsign, bool rv32, bool resid,
                               uint64_t a1, uint64_t a2);

    struct RegistersType {
        sc_signal<bool> rv32;
        sc_signal<bool> resid;
        sc_signal<bool> invert;
        sc_signal<bool> busy;
        sc_signal<sc_uint<34>> ena;
        sc_signal<sc_uint<RISCV_ARCH>> result;
        sc_biguint<128> qr;
        sc_biguint<65> divider;

        sc_uint<RISCV_ARCH> reference_div;
        sc_uint<64> a1_dbg;     // Store this value for output in a case of error
        sc_uint<64> a2_dbg;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.rv32 = 0;
        iv.resid = 0;
        iv.invert = 0;
        iv.busy = 0;
        iv.ena = 0;
        iv.result = 0;
        iv.qr = 0;
        iv.divider = 0;
    }

    // 2 stages per one clock to improve divider performance
    sc_biguint<65> wb_diff1;
    sc_biguint<65> wb_diff2;
    sc_biguint<128> wb_qr1;
    sc_biguint<128> wb_qr2;
    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_INT_DIV_H__
