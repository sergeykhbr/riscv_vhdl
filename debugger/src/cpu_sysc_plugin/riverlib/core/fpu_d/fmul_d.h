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

#ifndef __DEBUGGER_RIVERLIB_CORE_FPU_D_FMUL_D_H__
#define __DEBUGGER_RIVERLIB_CORE_FPU_D_FMUL_D_H__

#include <systemc.h>
#include "../../river_cfg.h"
#include "imul53.h"

namespace debugger {

SC_MODULE(DoubleMul) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_ena;
    sc_in<sc_uint<64>> i_a;        // Operand 1
    sc_in<sc_uint<64>> i_b;        // Operand 2
    sc_out<sc_uint<64>> o_res;     // Result
    sc_out<bool> o_except;         //
    sc_out<bool> o_valid;          // Result is valid
    sc_out<bool> o_busy;           // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(DoubleMul);

    DoubleMul(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    uint64_t compute_reference(uint64_t a, uint64_t b);

    struct RegistersType {
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
        sc_signal<bool> except;

        sc_uint<RISCV_ARCH> a_dbg;
        sc_uint<RISCV_ARCH> b_dbg;
        sc_uint<RISCV_ARCH> reference_res;          // Used for run-time comparision
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.busy = 0;
        iv.ena = 0;
        iv.a = 0;
        iv.b = 0;
        iv.result = 0;
        iv.zeroA = 0;
        iv.zeroB = 0;
        iv.mantA = 0;
        iv.mantB = 0;
        iv.expAB = 0;
        iv.expAlign = 0;
        iv.mantAlign = 0;
        iv.postShift = 0;
        iv.mantPostScale = 0;
        iv.nanA = 0;
        iv.nanB = 0;
        iv.overflow = 0;
        iv.except = 0;

        iv.a_dbg = 0;
        iv.b_dbg = 0;
        iv.reference_res = 0;
    }

    imul53 u_imul53;
    sc_signal<bool> w_imul_ena;
    sc_signal<sc_biguint<106>> wb_imul_result;
    sc_signal<sc_uint<7>> wb_imul_shift;
    sc_signal<bool>  w_imul_rdy;
    sc_signal<bool>  w_imul_overflow;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CORE_FPU_D_FMUL_D_H__
