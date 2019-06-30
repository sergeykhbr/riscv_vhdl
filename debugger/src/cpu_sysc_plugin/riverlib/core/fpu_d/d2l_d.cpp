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
#include "d2l_d.h"

namespace debugger {

Double2Long::Double2Long(sc_module_name name_, bool async_reset) :
    sc_module(name_) {
    async_reset_ = async_reset;
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_signed;
    sensitive << i_a;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.signA;
    sensitive << r.expA;
    sensitive << r.mantA;
    sensitive << r.result;
    sensitive << r.op_signed;
    sensitive << r.mantPostScale;
    sensitive << r.overflow;
    sensitive << r.underflow;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void Double2Long::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fpu0/d2l/i_ena");
        sc_trace(o_vcd, i_signed, "/top/proc0/exec0/fpu0/d2l/i_signed");
        sc_trace(o_vcd, i_a, "/top/proc0/exec0/fpu0/d2l/i_a");
        sc_trace(o_vcd, o_res, "/top/proc0/exec0/fpu0/d2l/o_res");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/fpu0/d2l/o_valid");
        sc_trace(o_vcd, o_busy, "/top/proc0/exec0/fpu0/d2l/o_busy");
        sc_trace(o_vcd, r.ena, "/top/proc0/exec0/fpu0/d2l/r_ena");
        sc_trace(o_vcd, r.result, "/top/proc0/exec0/fpu0/d2l/r_result");
        sc_trace(o_vcd, r.op_signed, "/top/proc0/exec0/fpu0/d2l/r_op_signed");
        sc_trace(o_vcd, r.mantPostScale, "/top/proc0/exec0/fpu0/d2l/r_mantPostScale");
        sc_trace(o_vcd, expDif, "/top/proc0/exec0/fpu0/d2l/expDif");
        sc_trace(o_vcd, mantPreScale, "/top/proc0/exec0/fpu0/d2l/mantPreScale");
        sc_trace(o_vcd, mantPostScale, "/top/proc0/exec0/fpu0/d2l/mantPostScale");
    }
}

void Double2Long::comb() {
    sc_uint<53> mantA;
    bool expDif_gr;     // greater than 1023 + 63
    bool expDif_ge;     // greater or equal than 1023 + 63
    bool expDif_lt;     // less than 1023
    bool overflow;
    bool underflow;
    sc_uint<64> res;

    v = r;

    v.ena = (r.ena.read()(1, 0), (i_ena.read() & !r.busy));

    mantA(51, 0) = i_a.read()(51, 0);
    mantA[52] = 0;
    if (i_a.read()(62, 52) != 0) {
        mantA[52] = 1;
    }

    if (i_ena.read()) {
        v.busy = 1;
        v.signA = i_a.read()[63];
        v.expA = i_a.read()(62, 52);
        v.mantA = mantA;
        v.op_signed = i_signed.read();
        v.overflow = 0;
        v.underflow = 0;

        // Just for run-rime control (not for VHDL)
        v.a_dbg = i_a;
        v.reference_res = compute_reference(i_signed.read(), i_a.read());
    }

    expDif = 1086 - (0, r.expA.read());
    expDif_gr = expDif[11];
    expDif_ge = 0;
    if (expDif == 0 || expDif[11] == 1) {
        expDif_ge = 1;
    }
    expDif_lt = 0;
    if (r.expA.read() != 0x3FF && r.expA.read()[10] == 0) {
        expDif_lt = 1;
    }

    mantPreScale = r.mantA.read().to_uint64() << 11;

    mantPostScale = 0;
    if (r.op_signed.read() == 1 && expDif_ge) {
        overflow = 1;
        underflow = 0;
    } else if (r.op_signed.read() == 0 &&
                (r.signA.read() && expDif_ge || !r.signA.read() && expDif_gr)) {
        overflow = 1;
        underflow = 0;
    } else if (expDif_lt) {
        overflow = 0;
        underflow = 1;
    } else {
        overflow = 0;
        underflow = 0;
        // Multiplexer, probably switch case in rtl
        for (int i = 0; i < 64; i++) {
            if (expDif == i) {
                mantPostScale = mantPreScale >> i;
            }
        }
    }

    if (r.ena.read()[0] == 1) {
        v.overflow = overflow;
        v.underflow = underflow;
        v.mantPostScale = mantPostScale;
    }

    // Result multiplexers:
    if (r.op_signed.read() == 1) {
        res[63] = (r.signA.read() || r.overflow.read()) && !r.underflow.read();
    } else {
        if (r.overflow.read()) {
            res[63] = 1;
        } else {
            res[63] = r.mantPostScale.read()[63];
        }
    }
    if (r.signA.read() == 1) {
        res(62, 0) = ~r.mantPostScale.read()(62, 0) + 1;
    } else {
        res(62, 0) = r.mantPostScale.read()(62, 0);
    }

    if (r.ena.read()[1] == 1) {
        v.result = res;
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_overflow = r.overflow;
    o_underflow = r.underflow;
    o_valid = r.ena.read()[2];
    o_busy = r.busy;
}

void Double2Long::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

uint64_t Double2Long::compute_reference(bool op_signed, uint64_t a) {
    Reg64Type ra, ret;
    ra.val = a;
    if (op_signed) {
        ret.ival = static_cast<int64_t>(ra.f64);
    } else {
        ret.val = static_cast<uint64_t>(ra.f64);
    }
    return ret.val;
}

}  // namespace debugger

