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

#ifndef __DEBUGGER_RIVERLIB_CORE_FPU_D_FPU_TOP_H__
#define __DEBUGGER_RIVERLIB_CORE_FPU_D_FPU_TOP_H__

#include <systemc.h>
#include "../../river_cfg.h"
#include "fadd_d.h"
#include "fdiv_d.h"
#include "fmul_d.h"

namespace debugger {

SC_MODULE(FpuTop) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_ena;
    sc_in<sc_bv<Instr_FPU_Total>> i_ivec;
    sc_in<sc_uint<64>> i_a;        // Operand 1
    sc_in<sc_uint<64>> i_b;        // Operand 2
    sc_out<sc_uint<64>> o_res;     // Result
    sc_out<bool> o_except;         //
    sc_out<bool> o_valid;          // Result is valid
    sc_out<bool> o_busy;           // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(FpuTop);

    FpuTop(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct RegistersType {
        sc_signal<bool> ena;
        sc_signal<sc_bv<Instr_FPU_Total>> ivec;
        sc_signal<bool> busy;
        sc_signal<bool> ready;
        sc_signal<sc_uint<64>> a;
        sc_signal<sc_uint<64>> b;
        sc_signal<sc_uint<64>> result;
        sc_signal<bool> except;
        sc_signal<bool> ena_fadd;
        sc_signal<bool> ena_fdiv;
        sc_signal<bool> ena_fmul;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.ena = 0;
        iv.ivec = 0;
        iv.busy = 0;
        iv.ready = 0;
        iv.a = 0;
        iv.b = 0;
        iv.result = 0;
        iv.except = 0;
        iv.ena_fadd = 0;
        iv.ena_fdiv = 0;
        iv.ena_fmul = 0;
    }

    sc_signal<bool> w_fadd_d;
    sc_signal<bool> w_fsub_d;
    sc_signal<bool> w_feq_d;
    sc_signal<bool> w_flt_d;
    sc_signal<bool> w_fle_d;
    sc_signal<bool> w_fmax_d;
    sc_signal<bool> w_fmin_d;
    sc_signal<sc_uint<64>> wb_res_fadd;
    sc_signal<bool> w_valid_fadd;
    sc_signal<bool> w_exception_fadd;
    sc_signal<bool> w_busy_fadd;
    DoubleAdd fadd_d0;

    sc_signal<sc_uint<64>> wb_res_fdiv;
    sc_signal<bool> w_valid_fdiv;
    sc_signal<bool> w_exception_fdiv;
    sc_signal<bool> w_busy_fdiv;
    DoubleDiv fdiv_d0;

    sc_signal<sc_uint<64>> wb_res_fmul;
    sc_signal<bool> w_valid_fmul;
    sc_signal<bool> w_exception_fmul;
    sc_signal<bool> w_busy_fmul;
    DoubleMul fmul_d0;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CORE_FPU_D_FPU_TOP_H__
