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
#include "divstage64.h"

namespace debugger {

#define IDIV_V2
#define DBG_IDIV_TB

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

#ifdef IDIV_V2
    divstage64 stage0;
    divstage64 stage1;
    sc_signal<sc_biguint<128>> wb_divisor0_i;
    sc_signal<sc_biguint<128>> wb_divisor1_i;
    sc_signal<sc_uint<64>> wb_resid0_o;
    sc_signal<sc_uint<64>> wb_resid1_o;
    sc_signal<sc_uint<4>> wb_bits0_o;
    sc_signal<sc_uint<4>> wb_bits1_o;
#endif

    struct RegistersType {
        sc_signal<bool> rv32;
        sc_signal<bool> resid;
        sc_signal<bool> invert;
        sc_signal<bool> busy;
        sc_signal<sc_uint<34>> ena;
        sc_signal<sc_uint<RISCV_ARCH>> result;

        sc_uint<RISCV_ARCH> reference_div;
        sc_uint<64> a1_dbg;     // Store this value for output in a case of error
        sc_uint<64> a2_dbg;
#ifdef IDIV_V2
        sc_signal<sc_uint<64>> divident_i;
        sc_signal<sc_biguint<128>> divisor_i;
        sc_signal<sc_uint<64>> bits;
#else
        sc_biguint<128> qr;
        sc_biguint<65> divider;
#endif
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.rv32 = 0;
        iv.resid = 0;
        iv.invert = 0;
        iv.busy = 0;
        iv.ena = 0;
        iv.result = 0;
#ifdef IDIV_V2
        iv.divident_i = 0;
        iv.divisor_i = 0;
        iv.bits = 0;
#else
        iv.qr = 0;
        iv.divider = 0;
#endif
    }

    // 2 stages per one clock to improve divider performance
    sc_biguint<65> wb_diff1;
    sc_biguint<65> wb_diff2;
    sc_biguint<128> wb_qr1;
    sc_biguint<128> wb_qr2;
    bool async_reset_;
};

#ifdef DBG_IDIV_TB
SC_MODULE(IntDiv_tb) {
    void comb();
    void registers() {
        r = v;
    }

    SC_HAS_PROCESS(IntDiv_tb);

    IntDiv_tb(sc_module_name name_);

private:
    IntDiv *tt;

    sc_clock w_clk_i;
    sc_signal<bool> w_nrst_i;
    sc_signal<bool> w_ena_i;
    sc_signal<bool> w_unsigned_i;
    sc_signal<bool> w_rv32_i;
    sc_signal<bool> w_residual_i;
    sc_signal<sc_uint<RISCV_ARCH>> wb_a1_i;
    sc_signal<sc_uint<RISCV_ARCH>> wb_a2_i;
    sc_signal<sc_uint<RISCV_ARCH>> wb_res_o;
    sc_signal<bool> w_valid_o;
    sc_signal<bool> w_busy_o;

    struct RegistersType {
        sc_signal<sc_uint<32>> clk_cnt;
    } v, r;

    sc_trace_file *tb_vcd;
};
#endif  // DBG_IDIV_TB

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_INT_DIV_H__
