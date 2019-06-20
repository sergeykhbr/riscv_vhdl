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
#include "fdiv_d.h"

namespace debugger {

DoubleDiv::DoubleDiv(sc_module_name name_) : sc_module(name_),
    u_idiv53("idiv53") {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.a;
    sensitive << r.b;
    sensitive << r.result;
    sensitive << r.zeroA;
    sensitive << r.zeroB;
    sensitive << r.divisor;
    sensitive << r.preShift;
    sensitive << r.expAB;
    sensitive << r.expAlign;
    sensitive << r.mantAlign;
    sensitive << r.postShift;
    sensitive << r.mantPostScale;
    sensitive << r.nanRes;
    sensitive << r.overflow;
    sensitive << r.underflow;
    sensitive << r.except;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    u_idiv53.i_nrst(i_nrst);
    u_idiv53.i_clk(i_clk);
    u_idiv53.i_ena(w_idiv_ena);
    u_idiv53.i_divident(wb_divident);
    u_idiv53.i_divisor(wb_divisor);
    u_idiv53.o_result(wb_idiv_result);
    u_idiv53.o_lshift(wb_idiv_lshift);
    u_idiv53.o_rdy(w_idiv_rdy);
    u_idiv53.o_overflow(w_idiv_overflow);
    u_idiv53.o_zero_resid(w_idiv_zeroresid);
};

void DoubleDiv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fadd_d0/i_ena");
        sc_trace(o_vcd, i_a, "/top/proc0/exec0/fadd_d0/i_a");
        sc_trace(o_vcd, i_b, "/top/proc0/exec0/fadd_d0/i_b");
        sc_trace(o_vcd, o_res, "/top/proc0/exec0/fadd_d0/o_res");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/fadd_d0/o_valid");
        sc_trace(o_vcd, o_busy, "/top/proc0/exec0/fadd_d0/o_busy");
        sc_trace(o_vcd, r.ena, "/top/proc0/exec0/fadd_d0/r_ena");
        sc_trace(o_vcd, r.result, "/top/proc0/exec0/fadd_d0/r_result");
    }
}

void DoubleDiv::comb() {
    sc_uint<7> vb_ena;
    sc_uint<1> signA;
    sc_uint<1> signB;
    sc_uint<53> mantA;
    sc_uint<53> mantB;
    bool zeroA;
    bool zeroB;
    sc_uint<53> divisor;
    sc_uint<6> preShift;
    sc_uint<12> expAB;
    sc_biguint<105> mantAlign;
    sc_uint<12> expShift;
    sc_uint<13> expAlign;
    sc_uint<12> postShift;
    sc_biguint<105> mantPostScale;
    sc_uint<53> mantShort;
    sc_uint<52> tmpMant05;
    bool mantOnes;
    bool mantEven;
    bool mant05;
    sc_uint<1> rndBit;
    bool nanA;
    bool nanB;
    bool mantZeroA;
    bool mantZeroB;
    sc_uint<64> res;

    v = r;

    vb_ena[0] = (i_ena.read() & !r.busy);
    vb_ena[1] = r.ena.read()[0];
    w_idiv_ena = r.ena.read()[1];
    vb_ena(7, 2) = (r.ena.read()(6, 3), w_idiv_rdy);

    v.ena = vb_ena;

    if (i_ena.read()) {
        v.busy = 1;
        v.a = i_a;
        v.b = i_b;

        // Just for run-rime control (not for VHDL)
        v.a_dbg = i_a;
        v.b_dbg = i_b;
        v.reference_res = compute_reference(i_a.read(),
                                            i_b.read());
    }

    signA = r.a.read()[63];
    signB = r.b.read()[63];

    zeroA = 0;
    if (r.a.read()(62, 0) == 0) {
        zeroA = 1;
    }

    zeroB = 0;
    if (r.b.read()(62, 0) == 0) {
        zeroB = 1;
    }

    mantA(51, 0) = r.a.read()(51, 0);
    mantA[52] = 0;
    if (r.a.read()(62, 52) != 0) {
        mantA[52] = 1;
    }

    mantB(51, 0) = r.b.read()(51, 0);
    mantB[52] = 0;
    if (r.b.read()(62, 52) != 0) {
        mantB[52] = 1;
        divisor = mantB;
        preShift = 0;
    } else {
        // multiplexer for operation with zero expanent
        divisor = 0;
        for (unsigned i = 0; i < 52; i++) {
            if (divisor == 0 && mantB[51 - i] == 1) {
                divisor = mantB << i;
                preShift = i;
            }
        }
    }

    // expA - expB + 1023
    expAB = (0, r.a.read()(62, 52)) + (1023 - r.b.read()(62, 52));

    if (r.ena.read()[0] == 1) {
        v.divisor = divisor;
        v.preShift = preShift;
        v.expAB = expAB;
        v.zeroA = zeroA;
        v.zeroB = zeroB;
    }

    w_idiv_ena = r.ena.read()[1];
    wb_divident = mantA;
    wb_divisor = r.divisor.read();

    // IDiv53 module:
    mantAlign = 0;
    for (unsigned i = 0; i < 105; i++) {
        if (i == wb_idiv_lshift.read()) {
            mantAlign = wb_idiv_result << i;
        }
    }

    expShift = (0, r.preShift.read()) - (0, wb_idiv_lshift.read());
    if (r.b.read()(62, 52) == 0 && r.a.read()(62, 52) != 0) {
        expShift = expShift - 1;
    } else if (r.b.read()(62, 52) != 0 && r.a.read()(62, 52) == 0) {
        expShift = expShift + 1;
    }

    expAlign = (r.expAB.read()[11], r.expAB.read()) + (expShift[11], expShift);
    if (expAlign == 0) {
        postShift = 1;
    } else if (expAlign[12] == 1) {
        postShift = ~expAlign(11, 0) + 1;
    } else {
        postShift = expAlign(11, 0);
    }

    if (r.ena.read()[2] == 1) {
        v.expAlign = expAlign(11, 0);
        v.mantAlign = mantAlign;
        v.postShift = postShift;

        // Exceptions:
        v.nanRes = 0;
        if (expAlign == 0x7FF) {
            v.nanRes = 1;
        }
        v.overflow = !expAlign[12] && expAlign[11];
        v.underflow = expAlign[12] && expAlign[11];
    }

    // Prepare to mantissa post-scale
    if (r.postShift.read() >= 105) {
        mantPostScale = 0;
    } else {
        for (unsigned i = 0; i < 105; i++) {
            if (i == r.postShift.read()) {
                mantPostScale = r.mantAlign.read() >> i;
            }
        }
    }
    if (r.ena.read()[3] == 1) {
        v.mantPostScale = mantPostScale;
    }

    // Rounding bit
    mantShort = r.mantPostScale.read().range(104, 52).to_uint64();
    tmpMant05 = r.mantPostScale.read().range(51, 0).to_uint64();
    mantOnes = 0;
    if (mantShort == 0x001fffffffffffff) {
        mantOnes = 1;
    }
    mantEven = r.mantPostScale.read()[52];
    mant05 = 0;
    if (tmpMant05 == 0x0008000000000000) {
        mant05 = 1;
    }
    rndBit = r.mantPostScale.read()[51] & !(mant05 & !mantEven);

    // Check Borders
    nanA = 0;
    if (r.a.read()(62, 52) == 0x7ff) {
        nanA = 1;
    }
    nanB = 0;
    if (r.b.read()(62, 52) == 0x7ff) {
        nanB = 1;
    }
    mantZeroA = 0;
    if (r.a.read()(62, 52) == 0) {
        mantZeroA = 1;
    }
    mantZeroB = 0;
    if (r.b.read()(62, 52) == 0) {
        mantZeroB = 1;
    }

    // Result multiplexers:
    if (nanA && mantZeroA && nanB && mantZeroB) {
        res[63] = 1;
    } else if (nanA && !mantZeroA) {
        res[63] = signA;
    } else if (nanB && !mantZeroB) {
        res[63] = signB;
    } else if (r.zeroA.read() && r.zeroB.read()) {
        res[63] = 1;
    } else {
        res[63] = r.a.read()[63] ^ r.b.read()[63];
    }

    if (nanB && !mantZeroB) {
        res(62, 52) = r.b.read()(62, 52);
    } else if ((r.underflow.read() || r.zeroA.read()) && !r.zeroB.read()) {
        res(62, 52) = 0;
    } else if (r.overflow.read() || r.zeroB.read()) {
        res(62, 52) = 0x7FF;
    } else if (nanA) {
        res(62, 52) = r.a.read()(62, 52);
    } else if ((nanB && mantZeroB) || r.expAlign.read()[11]) {
        res(62, 52) = 0;
    } else {
        res(62, 52) = r.expAlign.read()
                       + (mantOnes && rndBit && !r.overflow.read());
    }

    if ((r.zeroA.read() && r.zeroB.read())
        || (nanA & mantZeroA & nanB & mantZeroB)) {
        res[51] = 1;
        res(50, 0) = 0;
    } else if (nanA && !mantZeroA) {
        res[51] = 1;
        res(50, 0) = r.a.read()(50, 0);
    } else if (nanB&& !mantZeroB) {
        res[51] = 1;
        res(50, 0) = r.b.read()(50, 0);
    } else if (r.overflow.read() | r.nanRes.read() | (nanA && mantZeroA)
        || (nanB && mantZeroB)) {
        res(51, 0) = 0;
    } else {
        res(51, 0) = mantShort + rndBit;
    }

    if (r.ena.read()[4] == 1) {
        v.result = res;
        v.except = nanA | nanB | r.overflow.read() | r.underflow.read();
        v.busy = 0;
    }

    if (i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_except = r.except;
    o_valid = r.ena.read()[5];
    o_busy = r.busy;
}

void DoubleDiv::registers() {
    r = v;
}

uint64_t DoubleDiv::compute_reference(uint64_t a, uint64_t b) {
    Reg64Type ra, rb, ret;
    ra.val = a;
    rb.val = b;
    ret.f64 = ra.f64 / rb.f64;
    return ret.val;
}

}  // namespace debugger

