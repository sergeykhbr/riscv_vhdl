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
#include "l2d_d.h"

namespace debugger {

Long2Double::Long2Double(sc_module_name name_, bool async_reset) :
    sc_module(name_) {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_signed;
    sensitive << i_w32;
    sensitive << i_a;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.signA;
    sensitive << r.absA;
    sensitive << r.result;
    sensitive << r.op_signed;
    sensitive << r.mantAlign;
    sensitive << r.lshift;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void Long2Double::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fpu0/l2d/i_ena");
        sc_trace(o_vcd, i_signed, "/top/proc0/exec0/fpu0/l2d/i_signed");
        sc_trace(o_vcd, i_w32, "/top/proc0/exec0/fpu0/l2d/i_w32");
        sc_trace(o_vcd, i_a, "/top/proc0/exec0/fpu0/l2d/i_a");
        sc_trace(o_vcd, o_res, "/top/proc0/exec0/fpu0/l2d/o_res");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/fpu0/l2d/o_valid");
        sc_trace(o_vcd, o_busy, "/top/proc0/exec0/fpu0/l2d/o_busy");
        sc_trace(o_vcd, r.ena, "/top/proc0/exec0/fpu0/l2d/r_ena");
        sc_trace(o_vcd, r.result, "/top/proc0/exec0/fpu0/l2d/r_result");
        sc_trace(o_vcd, r.op_signed, "/top/proc0/exec0/fpu0/l2d/r_op_signed");
        sc_trace(o_vcd, r.mantAlign, "/top/proc0/exec0/fpu0/l2d/r_mantAlign");
        sc_trace(o_vcd, r.lshift, "/top/proc0/exec0/fpu0/l2d/lshift");
    }
}

void Long2Double::comb() {
    sc_uint<64> mantAlign;
    sc_uint<6> lshift;
    sc_uint<11> expAlign;
    bool mantEven;
    bool mant05;
    bool mantOnes;
    bool rndBit;
    bool v_signA;
    sc_uint<64> vb_A;
    sc_uint<64> res;

    v = r;

    v.ena = (r.ena.read()(1, 0), (i_ena.read() & !r.busy));
    if (i_w32.read() == 0) {
        v_signA = i_a.read()[63];
        vb_A = i_a.read();
    } else if (i_signed.read() && i_a.read()[31]) {
        v_signA = 1;
        vb_A(63, 32) = ~0ul;
        vb_A(31, 0) = i_a.read()(31, 0);
    } else {
        v_signA = 0;
        vb_A(31, 0) = i_a.read()(31, 0);
        vb_A(63, 32) = 0ul;
    }

    if (i_ena.read()) {
        v.busy = 1;
        if (i_signed.read() && v_signA) {
            v.signA = 1;
            v.absA = ~vb_A + 1;
        } else {
            v.signA = 0;
            v.absA = vb_A;
        }
        v.op_signed = i_signed.read();

        // Just for run-rime control (not for VHDL)
        v.a_dbg = i_a;
        v.reference_res = compute_reference(i_signed.read(), i_a.read());
    }

    // multiplexer, probably if/elsif in rtl:
    mantAlign = 0;
    lshift = 63;
    for (int i = 0; i < 64; i++) {
        if (lshift == 63 && r.absA.read()[63 - i] == 1) {
            mantAlign = r.absA.read() << i;
            lshift = i;
        }
    }

    if (r.ena.read()[0] == 1) {
        v.mantAlign = mantAlign;
        v.lshift = lshift;
    }

    if (r.absA.read() == 0) {
        expAlign = 0;
    } else {
        expAlign = 1086 - r.lshift.read();
    }

    mantEven = r.mantAlign.read()[11];
    mant05 = 0;
    if (r.mantAlign.read()(10, 0) == 0x7ff) {
        mant05 = 1;
    }
    rndBit = r.mantAlign.read()[10] && !(mant05 && mantEven);
    mantOnes = 0;
    if (r.mantAlign.read()(63, 11) == 0x001fffffffffffff) {
        mantOnes = 1;
    }

    // Result multiplexers:
    res[63] = r.signA.read() && r.op_signed.read();
    res(62, 52) = expAlign + (mantOnes && rndBit);
    res(51, 0) = r.mantAlign.read()(62, 11) + rndBit;

    if (r.ena.read()[1] == 1) {
        v.result = res;
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_valid = r.ena.read()[2];
    o_busy = r.busy;
}

void Long2Double::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

uint64_t Long2Double::compute_reference(bool op_signed, uint64_t a) {
    Reg64Type ra, ret;
    ra.val = a;
    if (op_signed) {
        ret.f64 = static_cast<double>(ra.ival);
    } else {
        ret.f64 = static_cast<double>(ra.val);
    }
    return ret.val;
}

}  // namespace debugger

