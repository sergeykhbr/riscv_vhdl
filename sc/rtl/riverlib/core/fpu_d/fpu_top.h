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
#include "fadd_d.h"
#include "fdiv_d.h"
#include "fmul_d.h"
#include "d2l_d.h"
#include "l2d_d.h"

namespace debugger {

SC_MODULE(FpuTop) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;
    sc_in<sc_uint<Instr_FPU_Total>> i_ivec;
    sc_in<sc_uint<64>> i_a;                                 // Operand 1
    sc_in<sc_uint<64>> i_b;                                 // Operand 2
    sc_out<sc_uint<64>> o_res;                              // Result
    sc_out<bool> o_ex_invalidop;                            // Exception: invalid operation
    sc_out<bool> o_ex_divbyzero;                            // Exception: divide by zero
    sc_out<bool> o_ex_overflow;                             // Exception: overflow
    sc_out<bool> o_ex_underflow;                            // Exception: underflow
    sc_out<bool> o_ex_inexact;                              // Exception: inexact
    sc_out<bool> o_valid;                                   // Result is valid

    void comb();
    void registers();

    SC_HAS_PROCESS(FpuTop);

    FpuTop(sc_module_name name,
           bool async_reset);
    virtual ~FpuTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct FpuTop_registers {
        sc_signal<sc_uint<Instr_FPU_Total>> ivec;
        sc_signal<bool> busy;
        sc_signal<bool> ready;
        sc_signal<sc_uint<64>> a;
        sc_signal<sc_uint<64>> b;
        sc_signal<sc_uint<64>> result;
        sc_signal<bool> ex_invalidop;                       // Exception: invalid operation
        sc_signal<bool> ex_divbyzero;                       // Exception: divide by zero
        sc_signal<bool> ex_overflow;                        // Exception: overflow
        sc_signal<bool> ex_underflow;                       // Exception: underflow
        sc_signal<bool> ex_inexact;                         // Exception: inexact
        sc_signal<bool> ena_fadd;
        sc_signal<bool> ena_fdiv;
        sc_signal<bool> ena_fmul;
        sc_signal<bool> ena_d2l;
        sc_signal<bool> ena_l2d;
        sc_signal<bool> ena_w32;
    } v, r;

    void FpuTop_r_reset(FpuTop_registers &iv) {
        iv.ivec = 0;
        iv.busy = 0;
        iv.ready = 0;
        iv.a = 0ull;
        iv.b = 0ull;
        iv.result = 0ull;
        iv.ex_invalidop = 0;
        iv.ex_divbyzero = 0;
        iv.ex_overflow = 0;
        iv.ex_underflow = 0;
        iv.ex_inexact = 0;
        iv.ena_fadd = 0;
        iv.ena_fdiv = 0;
        iv.ena_fmul = 0;
        iv.ena_d2l = 0;
        iv.ena_l2d = 0;
        iv.ena_w32 = 0;
    }

    sc_signal<bool> w_fadd_d;
    sc_signal<bool> w_fsub_d;
    sc_signal<bool> w_feq_d;
    sc_signal<bool> w_flt_d;
    sc_signal<bool> w_fle_d;
    sc_signal<bool> w_fmax_d;
    sc_signal<bool> w_fmin_d;
    sc_signal<bool> w_fcvt_signed;
    sc_signal<sc_uint<64>> wb_res_fadd;
    sc_signal<bool> w_valid_fadd;
    sc_signal<bool> w_illegalop_fadd;
    sc_signal<bool> w_overflow_fadd;
    sc_signal<bool> w_busy_fadd;
    sc_signal<sc_uint<64>> wb_res_fdiv;
    sc_signal<bool> w_valid_fdiv;
    sc_signal<bool> w_illegalop_fdiv;
    sc_signal<bool> w_divbyzero_fdiv;
    sc_signal<bool> w_overflow_fdiv;
    sc_signal<bool> w_underflow_fdiv;
    sc_signal<bool> w_busy_fdiv;
    sc_signal<sc_uint<64>> wb_res_fmul;
    sc_signal<bool> w_valid_fmul;
    sc_signal<bool> w_illegalop_fmul;
    sc_signal<bool> w_overflow_fmul;
    sc_signal<bool> w_busy_fmul;
    sc_signal<sc_uint<64>> wb_res_d2l;
    sc_signal<bool> w_valid_d2l;
    sc_signal<bool> w_overflow_d2l;
    sc_signal<bool> w_underflow_d2l;
    sc_signal<bool> w_busy_d2l;
    sc_signal<sc_uint<64>> wb_res_l2d;
    sc_signal<bool> w_valid_l2d;
    sc_signal<bool> w_busy_l2d;

    DoubleAdd *fadd_d0;
    DoubleDiv *fdiv_d0;
    DoubleMul *fmul_d0;
    Double2Long *d2l_d0;
    Long2Double *l2d_d0;

};

}  // namespace debugger

