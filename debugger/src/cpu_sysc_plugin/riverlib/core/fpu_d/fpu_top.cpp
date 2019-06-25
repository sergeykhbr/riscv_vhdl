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

#include "api_core.h"
#include "fpu_top.h"

namespace debugger {

FpuTop::FpuTop(sc_module_name name_) : sc_module(name_),
    fadd_d0("fadd_d0"),
    fdiv_d0("fdiv_d0"),
    fmul_d0("fmul_d0"),
    d2l_d0("d2l_d0"),
    l2d_d0("l2d_d0") {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_ivec;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << r.ena;
    sensitive << r.ivec;
    sensitive << r.busy;
    sensitive << r.ready;
    sensitive << r.a;
    sensitive << r.b;
    sensitive << r.result;
    sensitive << r.ex_invalidop;
    sensitive << r.ex_divbyzero;
    sensitive << r.ex_overflow;
    sensitive << r.ex_underflow;
    sensitive << r.ex_inexact;
    sensitive << r.ena_fadd;
    sensitive << r.ena_fdiv;
    sensitive << r.ena_fmul;
    sensitive << r.ena_d2l;
    sensitive << r.ena_l2d;
    sensitive << wb_res_fadd;
    sensitive << w_valid_fadd;
    sensitive << w_exception_fadd;
    sensitive << w_busy_fadd;
    sensitive << wb_res_fdiv;
    sensitive << w_valid_fdiv;
    sensitive << w_exception_fdiv;
    sensitive << w_busy_fdiv;
    sensitive << wb_res_fmul;
    sensitive << w_valid_fmul;
    sensitive << w_exception_fmul;
    sensitive << w_busy_fmul;
    sensitive << wb_res_d2l;
    sensitive << w_valid_d2l;
    sensitive << w_exception_d2l;
    sensitive << w_busy_d2l;
    sensitive << wb_res_l2d;
    sensitive << w_valid_l2d;
    sensitive << w_busy_l2d;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    fadd_d0.i_clk(i_clk);
    fadd_d0.i_nrst(i_nrst);
    fadd_d0.i_ena(r.ena_fadd);
    fadd_d0.i_add(w_fadd_d);
    fadd_d0.i_sub(w_fsub_d);
    fadd_d0.i_eq(w_feq_d);
    fadd_d0.i_lt(w_flt_d);
    fadd_d0.i_le(w_fle_d);
    fadd_d0.i_max(w_fmax_d);
    fadd_d0.i_min(w_fmin_d);
    fadd_d0.i_a(r.a);
    fadd_d0.i_b(r.b);
    fadd_d0.o_res(wb_res_fadd);
    fadd_d0.o_except(w_exception_fadd);
    fadd_d0.o_valid(w_valid_fadd);
    fadd_d0.o_busy(w_busy_fadd);

    fdiv_d0.i_clk(i_clk);
    fdiv_d0.i_nrst(i_nrst);
    fdiv_d0.i_ena(r.ena_fdiv);
    fdiv_d0.i_a(r.a);
    fdiv_d0.i_b(r.b);
    fdiv_d0.o_res(wb_res_fdiv);
    fdiv_d0.o_except(w_exception_fdiv);
    fdiv_d0.o_valid(w_valid_fdiv);
    fdiv_d0.o_busy(w_busy_fdiv);

    fmul_d0.i_clk(i_clk);
    fmul_d0.i_nrst(i_nrst);
    fmul_d0.i_ena(r.ena_fmul);
    fmul_d0.i_a(r.a);
    fmul_d0.i_b(r.b);
    fmul_d0.o_res(wb_res_fmul);
    fmul_d0.o_except(w_exception_fmul);
    fmul_d0.o_valid(w_valid_fmul);
    fmul_d0.o_busy(w_busy_fmul);

    d2l_d0.i_clk(i_clk);
    d2l_d0.i_nrst(i_nrst);
    d2l_d0.i_ena(r.ena_d2l);
    d2l_d0.i_signed(w_fcvt_signed);
    d2l_d0.i_a(r.a);
    d2l_d0.o_res(wb_res_d2l);
    d2l_d0.o_except(w_exception_d2l);
    d2l_d0.o_valid(w_valid_d2l);
    d2l_d0.o_busy(w_busy_d2l);

    l2d_d0.i_clk(i_clk);
    l2d_d0.i_nrst(i_nrst);
    l2d_d0.i_ena(r.ena_l2d);
    l2d_d0.i_signed(w_fcvt_signed);
    l2d_d0.i_a(r.a);
    l2d_d0.o_res(wb_res_l2d);
    l2d_d0.o_valid(w_valid_l2d);
    l2d_d0.o_busy(w_busy_l2d);
};

void FpuTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fpu0/i_ena");
        sc_trace(o_vcd, i_a, "/top/proc0/exec0/fpu0/i_a");
        sc_trace(o_vcd, i_b, "/top/proc0/exec0/fpu0/i_b");
        sc_trace(o_vcd, i_ivec, "/top/proc0/exec0/fpu0/i_ivec");
        sc_trace(o_vcd, o_res, "/top/proc0/exec0/fpu0/o_res");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/fpu0/o_valid");
        sc_trace(o_vcd, o_busy, "/top/proc0/exec0/fpu0/o_busy");
        sc_trace(o_vcd, r.ena, "/top/proc0/exec0/fpu0/r_ena");
        sc_trace(o_vcd, r.result, "/top/proc0/exec0/fpu0/r_result");
        sc_trace(o_vcd, r.ena_fadd, "/top/proc0/exec0/fpu0/r_ena_fadd");
        sc_trace(o_vcd, r.ena_fdiv, "/top/proc0/exec0/fpu0/r_ena_fdiv");
        sc_trace(o_vcd, r.ena_fmul, "/top/proc0/exec0/fpu0/r_ena_fmul");
        sc_trace(o_vcd, r.ena_d2l, "/top/proc0/exec0/fpu0/r_ena_d2l");
        sc_trace(o_vcd, r.ena_l2d, "/top/proc0/exec0/fpu0/r_ena_l2d");
        sc_trace(o_vcd, r.ivec, "/top/proc0/exec0/fpu0/r_ivec");
    }
    fadd_d0.generateVCD(i_vcd, o_vcd);
    fdiv_d0.generateVCD(i_vcd, o_vcd);
    fmul_d0.generateVCD(i_vcd, o_vcd);
    d2l_d0.generateVCD(i_vcd, o_vcd);
    l2d_d0.generateVCD(i_vcd, o_vcd);
}

void FpuTop::comb() {
    sc_bv<Instr_FPU_Total> iv;
    v = r;

    iv = i_ivec.read();
    v.ena_fadd = 0;
    v.ena_fdiv = 0;
    v.ena_fmul = 0;
    v.ena_d2l = 0;
    v.ena_l2d = 0;
    v.ready = 0;
    if (i_ena.read() == 1 && r.busy.read() == 0) {
        v.busy = 1;
        v.a = i_a.read();
        v.b = i_b.read();
        v.ivec = i_ivec.read();
        v.ex_invalidop = 0;
        v.ex_divbyzero = 0;
        v.ex_overflow = 0;
        v.ex_underflow = 0;
        v.ex_inexact = 0;

        v.ena_fadd = (iv[Instr_FADD_D - Instr_FADD_D]
                    | iv[Instr_FSUB_D - Instr_FADD_D]
                    | iv[Instr_FLE_D - Instr_FADD_D]
                    | iv[Instr_FLT_D - Instr_FADD_D]
                    | iv[Instr_FEQ_D - Instr_FADD_D]
                    | iv[Instr_FMAX_D - Instr_FADD_D]
                    | iv[Instr_FMIN_D - Instr_FADD_D]).to_bool();
        v.ena_fdiv = iv[Instr_FDIV_D - Instr_FADD_D].to_bool();
        v.ena_fmul = iv[Instr_FMUL_D - Instr_FADD_D].to_bool();
        v.ena_d2l = (iv[Instr_FCVT_LU_D - Instr_FADD_D]
                    | iv[Instr_FCVT_L_D - Instr_FADD_D]).to_bool();
        v.ena_l2d = (iv[Instr_FCVT_D_LU - Instr_FADD_D]
                    | iv[Instr_FCVT_D_L - Instr_FADD_D]).to_bool();
    }

    if (r.busy.read() == 1 && (r.ivec.read()[Instr_FMOV_X_D - Instr_FADD_D]
                        | r.ivec.read()[Instr_FMOV_D_X - Instr_FADD_D]) == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = r.a;
    } else if (w_valid_fadd == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = wb_res_fadd;
        v.ex_invalidop = w_exception_fadd;
    } else if (w_valid_fdiv == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = wb_res_fdiv;
        v.ex_invalidop = w_exception_fdiv;
    } else if (w_valid_fmul == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = wb_res_fmul;
        v.ex_invalidop = w_exception_fmul;
    } else if (w_valid_d2l == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = wb_res_d2l;
        v.ex_invalidop = w_exception_d2l;
    } else if (w_valid_l2d == 1) {
        v.busy = 0;
        v.ready = 1;
        v.result = wb_res_l2d;
    }

    w_fadd_d = r.ivec.read()[Instr_FADD_D - Instr_FADD_D].to_bool();
    w_fsub_d = r.ivec.read()[Instr_FSUB_D - Instr_FADD_D].to_bool();
    w_feq_d = r.ivec.read()[Instr_FEQ_D - Instr_FADD_D].to_bool();
    w_flt_d = r.ivec.read()[Instr_FLT_D - Instr_FADD_D].to_bool();
    w_fle_d = r.ivec.read()[Instr_FLE_D - Instr_FADD_D].to_bool();
    w_fmax_d = r.ivec.read()[Instr_FMAX_D - Instr_FADD_D].to_bool();
    w_fmin_d = r.ivec.read()[Instr_FMIN_D - Instr_FADD_D].to_bool();
    w_fcvt_signed = (r.ivec.read()[Instr_FCVT_L_D - Instr_FADD_D] |
                     r.ivec.read()[Instr_FCVT_D_L - Instr_FADD_D]).to_bool();

    o_res = r.result;
    o_ex_invalidop = r.ex_invalidop;
    o_ex_divbyzero = r.ex_divbyzero;
    o_ex_overflow = r.ex_overflow;
    o_ex_underflow = r.ex_underflow;
    o_ex_inexact = r.ex_inexact;
    o_valid = r.ready;
    o_busy = r.busy;
}

void FpuTop::registers() {
    r = v;
}

}  // namespace debugger

