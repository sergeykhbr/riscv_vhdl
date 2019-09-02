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
#include "fadd_d.h"

namespace debugger {

DoubleAdd::DoubleAdd(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_add("i_add"),
    i_sub("i_sub"),
    i_eq("i_eq"),
    i_lt("i_lt"),
    i_le("i_le"),
    i_max("i_max"),
    i_min("i_min"),
    i_a("i_a"),
    i_b("i_b"),
    o_res("o_res"),
    o_illegal_op("o_illegal_op"),
    o_overflow("o_overflow"),
    o_valid("o_valid"),
    o_busy("o_busy") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_add;
    sensitive << i_sub;
    sensitive << i_eq;
    sensitive << i_lt;
    sensitive << i_le;
    sensitive << i_max;
    sensitive << i_min;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.a;
    sensitive << r.b;
    sensitive << r.result;
    sensitive << r.illegal_op;
    sensitive << r.overflow;
    sensitive << r.add;
    sensitive << r.sub;
    sensitive << r.eq;
    sensitive << r.lt;
    sensitive << r.le;
    sensitive << r.max;
    sensitive << r.min;
    sensitive << r.flMore;
    sensitive << r.flEqual;
    sensitive << r.flLess;
    sensitive << r.preShift;
    sensitive << r.signOpMore;
    sensitive << r.expMore;
    sensitive << r.mantMore;
    sensitive << r.mantLess;
    sensitive << r.mantLessScale;
    sensitive << r.mantSum;
    sensitive << r.lshift;
    sensitive << r.mantAlign;
    sensitive << r.expPostScale;
    sensitive << r.expPostScaleInv;
    sensitive << r.mantPostScale;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void DoubleAdd::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_add, i_add.name());
        sc_trace(o_vcd, i_sub, i_sub.name());
        sc_trace(o_vcd, i_eq, i_eq.name());
        sc_trace(o_vcd, i_lt, i_lt.name());
        sc_trace(o_vcd, i_le, i_le.name());
        sc_trace(o_vcd, i_max, i_max.name());
        sc_trace(o_vcd, i_min, i_min.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, i_b, i_b.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_busy, o_busy.name());

        std::string pn(name());
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.preShift, pn + ".r_preShift");
        sc_trace(o_vcd, r.signOpMore, pn + ".r_signOpMore");
        sc_trace(o_vcd, r.expMore, pn + ".r_expMore");
        sc_trace(o_vcd, r.mantMore, pn + ".r_mantMore");
        sc_trace(o_vcd, r.mantLess, pn + ".r_mantLess");
        sc_trace(o_vcd, r.lshift, pn + ".r_lshift");
        sc_trace(o_vcd, r.mantSum, pn + ".r_mantSum");
        sc_trace(o_vcd, r.mantAlign, pn + ".r_mantAlign");
        sc_trace(o_vcd, r.mantPostScale, pn + ".r_mantPostScale");
        sc_trace(o_vcd, r.expPostScale, pn + ".r_expPostScale");
        sc_trace(o_vcd, r.expPostScaleInv, pn + ".r_expPostScaleInv");
    }
}

void DoubleAdd::comb() {
    sc_uint<1> signOp;
    sc_uint<1> signA;
    sc_uint<1> signB;
    sc_uint<1> signOpB;
    sc_uint<53> mantA;
    sc_uint<53> mantB;
    sc_uint<54> mantDif;
    sc_uint<12> expDif;
    bool v_flMore;
    bool v_flEqual;
    bool v_flLess;
    sc_uint<12> vb_preShift;
    bool v_signOpMore;
    sc_uint<11> vb_expMore;
    sc_uint<53> vb_mantMore;
    sc_uint<53> vb_mantLess;
    sc_biguint<105> mantMoreScale;
    sc_biguint<105> mantLessScale;
    sc_biguint<106> vb_mantSum;
    sc_uint<7> vb_lshift;
    sc_biguint<105> vb_mantAlign;
    sc_uint<12> vb_expPostScale;
    sc_biguint<105> vb_mantPostScale;
    sc_uint<53> mantShort;
    sc_uint<52> tmpMant05;
    bool mantOnes;
    bool mantEven;
    bool mant05;
    sc_uint<1> rndBit;
    bool mantZeroA;
    bool mantZeroB;
    bool allZero;
    bool sumZero;
    bool nanA;
    bool nanB;
    bool nanAB;
    bool overflow;
    sc_uint<64> resAdd;
    sc_uint<64> resEQ;
    sc_uint<64> resLT;
    sc_uint<64> resLE;
    sc_uint<64> resMax;
    sc_uint<64> resMin;

    v = r;

    v.ena = (r.ena.read() << 1) | (i_ena.read() & !r.busy);

    if (i_ena.read()) {
        v.busy = 1;
        v.add = i_add;
        v.sub = i_sub;
        v.eq = i_eq;
        v.lt = i_lt;
        v.le = i_le;
        v.max = i_max;
        v.min = i_min;
        v.a = i_a;
        v.b = i_b;
        v.illegal_op = 0;
        v.overflow = 0;

        // Just for run-rime control (not for VHDL)
        v.a_dbg = i_a;
        v.b_dbg = i_b;
        v.reference_res = compute_reference(i_add.read(),
                                            i_sub.read(),
                                            i_eq.read(),
                                            i_lt.read(),
                                            i_le.read(),
                                            i_max.read(),
                                            i_min.read(),
                                            i_a.read(),
                                            i_b.read());
    }

    signOp = r.sub.read() | r.le.read() | r.lt.read();
    signA = r.a.read()[63];
    signB = r.b.read()[63];
    signOpB = signB ^ signOp;

    mantA(51, 0) = r.a.read()(51, 0);
    mantA[52] = 0;
    if (r.a.read()(62, 52) != 0) {
        mantA[52] = 1;
    }

    mantB(51, 0) = r.b.read()(51, 0);
    mantB[52] = 0;
    if (r.b.read()(62, 52) != 0) {
        mantB[52] = 1;
    }

    if (r.a.read()(62, 52) != 0 && r.b.read()(62, 52) == 0) {
        expDif = r.a.read()(62, 52) - 1;
    } else if (r.a.read()(62, 52) == 0 && r.b.read()(62, 52) != 0) {
        expDif = 1 - r.b.read()(62, 52);
    } else {
        expDif = r.a.read()(62, 52) - r.b.read()(62, 52);
    }

    mantDif = (0, mantA) - (0, mantB);
    if (expDif == 0) {
        vb_preShift = expDif;
        if (mantDif == 0) {
            v_flMore = !signA & (signA ^ signB);
            v_flEqual = !(signA ^ signB);
            v_flLess = signA & (signA ^ signB);

            v_signOpMore = signA;
            vb_expMore = r.a.read()(62, 52);
            vb_mantMore = mantA;
            vb_mantLess = mantB;
        } else if (mantDif[53] == 0) {  // A > B
            v_flMore = !signA;
            v_flEqual = 0;
            v_flLess = signA;

            v_signOpMore = signA;
            vb_expMore = r.a.read()(62, 52);
            vb_mantMore = mantA;
            vb_mantLess = mantB;
        } else {
            v_flMore = signB;
            v_flEqual = 0;
            v_flLess = !signB;

            v_signOpMore = signOpB;
            vb_expMore = r.b.read()(62, 52);
            vb_mantMore = mantB;
            vb_mantLess = mantA;
        }
    } else if (expDif[11] == 0) {
        v_flMore = !signA;
        v_flEqual = 0;
        v_flLess = signA;

        vb_preShift = expDif;
        v_signOpMore = signA;
        vb_expMore = r.a.read()(62, 52);
        vb_mantMore = mantA;
        vb_mantLess = mantB;
    } else {
        v_flMore = signB;
        v_flEqual = 0;
        v_flLess = !signB;

        vb_preShift = ~expDif + 1;
        v_signOpMore = signOpB;
        vb_expMore = r.b.read()(62, 52);
        vb_mantMore = mantB;
        vb_mantLess = mantA;
    }
    if (r.ena.read()[0] == 1) {
        v.flMore = v_flMore;
        v.flEqual = v_flEqual;
        v.flLess = v_flLess;
        v.preShift = vb_preShift;
        v.signOpMore = v_signOpMore;
        v.expMore = vb_expMore;
        v.mantMore = vb_mantMore;
        v.mantLess = vb_mantLess;
    }

    // Pre-scale 105-bits mantissa if preShift < 105:
    // M = {mantM, 52'd0}
    mantLessScale = r.mantLess.read().to_uint64();
    mantLessScale <<= 52;
    if (r.ena.read()[1] == 1) {
        v.mantLessScale = 0;
        for (unsigned i = 0; i < 105; i++) {
            if (i == r.preShift.read()) {
                v.mantLessScale = mantLessScale >> i;
            }
        }
    }

    mantMoreScale = r.mantMore.read().to_uint64();
    mantMoreScale <<= 52;

    // 106-bits adder/subtractor
    if (signA ^ signOpB) {
        vb_mantSum = mantMoreScale - r.mantLessScale;
    } else {
        vb_mantSum = mantMoreScale + r.mantLessScale;
    }

    if (r.ena.read()[2] == 1) {
        v.mantSum = vb_mantSum;
    }

    // To avoid timing constrains violation try to implement parallel demux
    // for Xilinx Vivado
    sc_biguint<105> vb_mantSumInv;
    sc_uint<7> vb_lshift_p1;
    sc_uint<7> vb_lshift_p2;
    vb_mantSumInv[0] = 0;
    for (unsigned i = 0; i < 104; i++) {
        vb_mantSumInv[i + 1] = r.mantSum.read()[103 - i];
    }

    vb_lshift_p1 = 0;
    for (unsigned i = 0; i < 64; i++) {
        if (vb_lshift_p1 == 0 && vb_mantSumInv[i] == 1) {
            vb_lshift_p1 = i;
        }
    }

    vb_lshift_p2 = 0;
    for (unsigned i = 0; i < 41; i++) {
        if (vb_lshift_p2 == 0 && vb_mantSumInv[64 + i] == 1) {
            vb_lshift_p2 = 0x40 | i;
        }
    }

    // multiplexer
    if (r.mantSum.read()[105] == 1) {
        // shift right
        vb_lshift = 0x7F;
    } else if (r.mantSum.read()[104] == 1) {
        vb_lshift = 0;
    } else if (vb_lshift_p1 != 0) {
        vb_lshift = vb_lshift_p1;
    } else {
        vb_lshift = vb_lshift_p2;
#if 0
        // shift left
        vb_lshift = 0;
        for (unsigned i = 1; i < 105; i++) {
            if (vb_lshift == 0 && r.mantSum.read()[104 - i] == 1) {
                vb_lshift = i;
            }
        }
#endif
    }
    if (r.ena.read()[3] == 1) {
        v.lshift = vb_lshift;
    }

    // Prepare to mantissa post-scale
    vb_mantAlign = 0;
    if (r.lshift.read() == 0x7F) {
        vb_mantAlign = r.mantSum.read() >> 1;
    } else if (r.lshift.read() == 0) {
        vb_mantAlign = r.mantSum.read();
    } else {
        for (unsigned i = 1; i < 105; i++) {
            if (i == r.lshift.read()) {
                vb_mantAlign = r.mantSum.read() << i;
            }
        }
    }
    if (r.lshift.read() == 0x7F) {
        if (r.expMore.read() == 0x7FF) {
            vb_expPostScale = (0, r.expMore);
        } else {
            vb_expPostScale = (0, r.expMore.read()) + 1;
        }
    } else {
        if (r.expMore.read() == 0 && r.lshift.read() == 0) {
            vb_expPostScale = 1;
        } else {
            vb_expPostScale = (0, r.expMore.read()) - (0, r.lshift.read());
        }
    }
    if (signA ^ signOpB) {
        // subtractor only: result value becomes with exp=0
        if (r.expMore.read() != 0 && 
            (vb_expPostScale[11] == 1 || vb_expPostScale == 0)) {
            vb_expPostScale -= 1;
        }
    }
    if (r.ena.read()[4] == 1) {
        v.mantAlign = vb_mantAlign;
        v.expPostScale = vb_expPostScale;
        v.expPostScaleInv = ~vb_expPostScale + 1;
    }

    // Mantissa post-scale:
    //    Scaled = SumScale>>(-ExpSum) only if ExpSum < 0;
    vb_mantPostScale = r.mantAlign;
    if (r.expPostScale.read()[11] == 1) {
        for (unsigned i = 1; i < 105; i++) {
            if (i == r.expPostScaleInv.read()) {
                vb_mantPostScale = r.mantAlign.read() >> i;
            }
        }
    }
    if (r.ena.read()[5] == 1) {
        v.mantPostScale = vb_mantPostScale;
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

    // Check borders
    mantZeroA = 0;
    if (r.a.read()(51, 0) == 0) {
        mantZeroA = 1;
    }
    mantZeroB = 0;
    if (r.b.read()(51, 0) == 0) {
        mantZeroB = 1;
    }

    // Exceptions
    allZero = 0;
    if (r.a.read()(62, 0) == 0 && r.b.read()(62, 0) == 0) {
        allZero = 1;
    }
    sumZero = 0;
    if (r.mantPostScale.read() == 0) {
        sumZero = 1;
    }
    nanA = 0;
    if (r.a.read()(62, 52) == 0x7ff) {
        nanA = 1;
    }
    nanB = 0;
    if (r.b.read()(62, 52) == 0x7ff) {
        nanB = 1;
    }
    nanAB = nanA && mantZeroA && nanB && mantZeroB;
    overflow = 0;
    if (r.expPostScale.read() == 0x7FF) {   // positive
        overflow = 1;
    }

    // Result multiplexers:
    if (nanAB && signOp) {
        resAdd[63] = signA ^ signOpB;
    } else if (nanA) {
        /** when both values are NaN, value B has higher priority if sign=1 */
        resAdd[63] = signA || (nanB && signOpB);
    } else if (nanB) {
        resAdd[63] = signOpB ^ (signOp && !mantZeroB);
    } else if (allZero) {
        resAdd[63] = signA && signOpB;
    } else if (sumZero) {
        resAdd[63] = 0;
    } else {
        resAdd[63] = r.signOpMore; 
    }

    if (nanA | nanB) {
        resAdd(62, 52) = 0x7FF;
    } else if (r.expPostScale.read()[11] == 1 || sumZero) {
        resAdd(62, 52) = 0;
    } else {
        resAdd(62, 52) = r.expPostScale.read()
                       + (mantOnes && rndBit && !overflow);
    }

    if (nanA & mantZeroA & nanB & mantZeroB) {
        resAdd[51] = signOp;
        resAdd(50, 0) = 0;
    } else if (nanA && !(nanB && signOpB)) {
        /** when both values are NaN, value B has higher priority if sign=1 */
        resAdd[51] = 1;
        resAdd(50, 0) = r.a.read()(50, 0);
    } else if (nanB) {
        resAdd[51] = 1;
        resAdd(50, 0) = r.b.read()(50, 0);
    } else if (overflow) {
        resAdd(51, 0) = 0;
    } else {
        resAdd(51, 0) = mantShort(51, 0) + rndBit;
    }

    resEQ(63, 1) = 0;
    resEQ[0] = r.flEqual;

    resLT(63, 1) = 0;
    resLT[0] = r.flLess;

    resLE(63, 1) = 0;
    resLE[0] = r.flLess | r.flEqual;

    if (nanA | nanB) {
        resMax = r.b;
    } else if (r.flMore.read() == 1) {
        resMax = r.a;
    } else {
        resMax = r.b;
    }

    if (nanA | nanB) {
        resMin = r.b;
    } else if (r.flLess.read() == 1) {
        resMin = r.a;
    } else {
        resMin = r.b;
    }

    if (r.ena.read()[6] == 1) {
        if (r.eq.read() == 1) {
            v.result = resEQ;
        } else if (r.lt.read() == 1) {
            v.result = resLT;
        } else if (r.le.read() == 1) {
            v.result = resLE;
        } else if (r.max.read() == 1) {
            v.result = resMax;
        } else if (r.min.read() == 1) {
            v.result = resMin;
        } else {
            v.result = resAdd;
        }

        v.illegal_op = nanA | nanB;
        v.overflow = overflow;

        v.busy = 0;
        v.add = 0;
        v.sub = 0;
        v.eq = 0;
        v.lt = 0;
        v.le = 0;
        v.max = 0;
        v.min = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_overflow = r.overflow;
    o_valid = r.ena.read()[7];
    o_busy = r.busy;
}

void DoubleAdd::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

uint64_t DoubleAdd::compute_reference(bool addEna, bool subEna,
                                      bool eqEna, bool ltEna, bool leEna,
                                      bool maxEna, bool minEna,
                                      uint64_t a, uint64_t b) {
    Reg64Type ra, rb, ret;
    ra.val = a;
    rb.val = b;
    ret.val = 0;
    if (addEna) {
        ret.f64 = ra.f64 + rb.f64;
    } else if (subEna) {
        ret.f64 = ra.f64 - rb.f64;
    } else if (eqEna) {
        ret.val = ra.f64 == rb.f64 ? 1 : 0;
    } else if (ltEna) {
        ret.val = ra.f64 < rb.f64 ? 1 : 0;
    } else if (leEna) {
        ret.val = ra.f64 <= rb.f64 ? 1 : 0;
    } else if (maxEna) {
        ret.f64 = ra.f64 > rb.f64 ? ra.f64 : rb.f64;
    } else if (minEna) {
        ret.f64 = ra.f64 < rb.f64 ? ra.f64 : rb.f64;
    }
    return ret.val;
}

}  // namespace debugger

