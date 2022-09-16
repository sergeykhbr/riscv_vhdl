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

#include "fdiv_d.h"
#include "api_core.h"

namespace debugger {

DoubleDiv::DoubleDiv(sc_module_name name,
                     bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_a("i_a"),
    i_b("i_b"),
    o_res("o_res"),
    o_illegal_op("o_illegal_op"),
    o_divbyzero("o_divbyzero"),
    o_overflow("o_overflow"),
    o_underflow("o_underflow"),
    o_valid("o_valid"),
    o_busy("o_busy") {

    async_reset_ = async_reset;
    u_idiv53 = 0;

    u_idiv53 = new idiv53("u_idiv53", async_reset);
    u_idiv53->i_clk(i_clk);
    u_idiv53->i_nrst(i_nrst);
    u_idiv53->i_ena(w_idiv_ena);
    u_idiv53->i_divident(wb_divident);
    u_idiv53->i_divisor(wb_divisor);
    u_idiv53->o_result(wb_idiv_result);
    u_idiv53->o_lshift(wb_idiv_lshift);
    u_idiv53->o_rdy(w_idiv_rdy);
    u_idiv53->o_overflow(w_idiv_overflow);
    u_idiv53->o_zero_resid(w_idiv_zeroresid);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << w_idiv_ena;
    sensitive << wb_divident;
    sensitive << wb_divisor;
    sensitive << wb_idiv_result;
    sensitive << wb_idiv_lshift;
    sensitive << w_idiv_rdy;
    sensitive << w_idiv_overflow;
    sensitive << w_idiv_zeroresid;
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
    sensitive << r.illegal_op;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

DoubleDiv::~DoubleDiv() {
    if (u_idiv53) {
        delete u_idiv53;
    }
}

void DoubleDiv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, i_b, i_b.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_illegal_op, o_illegal_op.name());
        sc_trace(o_vcd, o_divbyzero, o_divbyzero.name());
        sc_trace(o_vcd, o_overflow, o_overflow.name());
        sc_trace(o_vcd, o_underflow, o_underflow.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_busy, o_busy.name());
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.a, pn + ".r_a");
        sc_trace(o_vcd, r.b, pn + ".r_b");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.zeroA, pn + ".r_zeroA");
        sc_trace(o_vcd, r.zeroB, pn + ".r_zeroB");
        sc_trace(o_vcd, r.divisor, pn + ".r_divisor");
        sc_trace(o_vcd, r.preShift, pn + ".r_preShift");
        sc_trace(o_vcd, r.expAB, pn + ".r_expAB");
        sc_trace(o_vcd, r.expAlign, pn + ".r_expAlign");
        sc_trace(o_vcd, r.mantAlign, pn + ".r_mantAlign");
        sc_trace(o_vcd, r.postShift, pn + ".r_postShift");
        sc_trace(o_vcd, r.mantPostScale, pn + ".r_mantPostScale");
        sc_trace(o_vcd, r.nanRes, pn + ".r_nanRes");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
        sc_trace(o_vcd, r.underflow, pn + ".r_underflow");
        sc_trace(o_vcd, r.illegal_op, pn + ".r_illegal_op");
    }

    if (u_idiv53) {
        u_idiv53->generateVCD(i_vcd, o_vcd);
    }
}

void DoubleDiv::comb() {
    sc_uint<5> vb_ena;
    sc_uint<1> signA;
    sc_uint<1> signB;
    sc_uint<53> mantA;
    sc_uint<53> mantB;
    bool zeroA;
    bool zeroB;
    sc_uint<53> divisor;
    sc_uint<6> preShift;
    sc_uint<12> expAB_t;
    sc_uint<13> expAB;
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

    vb_ena = 0;
    signA = 0;
    signB = 0;
    mantA = 0;
    mantB = 0;
    zeroA = 0;
    zeroB = 0;
    divisor = 0;
    preShift = 0;
    expAB_t = 0;
    expAB = 0;
    mantAlign = 0;
    expShift = 0;
    expAlign = 0;
    postShift = 0;
    mantPostScale = 0;
    mantShort = 0;
    tmpMant05 = 0;
    mantOnes = 0;
    mantEven = 0;
    mant05 = 0;
    rndBit = 0;
    nanA = 0;
    nanB = 0;
    mantZeroA = 0;
    mantZeroB = 0;
    res = 0;

    v = r;

    vb_ena[0] = (i_ena && (!r.busy));
    vb_ena[1] = r.ena.read()[0];
    vb_ena(4, 2) = (r.ena.read()(3, 2), w_idiv_rdy.read());
    v.ena = vb_ena;

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.overflow = 0;
        v.underflow = 0;
        v.illegal_op = 0;
        v.a = i_a;
        v.b = i_b;
    }

    signA = r.a.read()[63];
    signB = r.b.read()[63];

    if (r.a.read()(62, 0).or_reduce() == 0) {
        zeroA = 1;
    }

    if (r.b.read()(62, 0).or_reduce() == 0) {
        zeroB = 1;
    }

    mantA(51, 0) = r.a.read()(51, 0);
    mantA[52] = 0;
    if (r.a.read()(62, 52).or_reduce() == 1) {
        mantA[52] = 1;
    }

    mantB(51, 0) = r.b.read()(51, 0);
    mantB[52] = 0;
    if (r.b.read()(62, 52).or_reduce() == 1) {
        mantB[52] = 1;
        divisor = mantB;
    } else {
        // multiplexer for operation with zero expanent
        divisor = mantB;
        for (int i = 1; i < 53; i++) {
            if ((preShift.or_reduce() == 0) && (mantB[(52 - i)] == 1)) {
                divisor = (mantB << i);
                preShift = i;
            }
        }
    }

    // expA - expB + 1023
    expAB_t = ((0, r.a.read()(62, 52)) + 1023);
    expAB = ((0, expAB_t) - (0, r.b.read()(62, 52)));       // signed value

    if (r.ena.read()[0] == 1) {
        v.divisor = divisor;
        v.preShift = preShift;
        v.expAB = expAB;
        v.zeroA = zeroA;
        v.zeroB = zeroB;
    }

    w_idiv_ena = r.ena.read()[1];
    wb_divident = mantA;
    wb_divisor = r.divisor;

    // idiv53 module:
    for (int i = 0; i < 105; i++) {
        if (i == wb_idiv_lshift.read()) {
            mantAlign = (wb_idiv_result.read() << i);
        }
    }

    expShift = ((0, r.preShift.read()) - (0, wb_idiv_lshift.read()));
    if ((r.b.read()(62, 52).or_reduce() == 0) && (r.a.read()(62, 52).or_reduce() == 1)) {
        expShift = (expShift - 1);
    } else if ((r.b.read()(62, 52).or_reduce() == 1) && (r.a.read()(62, 52).or_reduce() == 0)) {
        expShift = (expShift + 1);
    }

    expAlign = (r.expAB.read() + (expShift[11], expShift));
    if (expAlign[12] == 1) {
        postShift = ((~expAlign(11, 0)) + 2);
    } else {
        postShift = 0;
    }

    if (w_idiv_rdy.read() == 1) {
        v.expAlign = expAlign(11, 0);
        v.mantAlign = mantAlign;
        v.postShift = postShift;

        // Exceptions:
        v.nanRes = 0;
        if (expAlign == 0x7FF) {
            v.nanRes = 1;
        }
        v.overflow = ((!expAlign[12]) && expAlign[11]);
        v.underflow = (expAlign[12] && expAlign[11]);
    }

    // Prepare to mantissa post-scale
    if (r.postShift.read().or_reduce() == 0) {
        mantPostScale = r.mantAlign;
    } else if (r.postShift.read() < 105) {
        for (int i = 0; i < 105; i++) {
            if (i == r.postShift.read()) {
                mantPostScale = (r.mantAlign.read() >> i);
            }
        }
    }
    if (r.ena.read()[2] == 1) {
        v.mantPostScale = mantPostScale;
    }

    // Rounding bit
    mantShort = r.mantPostScale.read()(104, 52).to_uint64();
    tmpMant05 = r.mantPostScale.read()(51, 0).to_uint64();
    if (mantShort == 0x001fffffffffffffull) {
        mantOnes = 1;
    }
    mantEven = r.mantPostScale.read()[52];
    if (tmpMant05 == 0x0008000000000000ull) {
        mant05 = 1;
    }
    rndBit = (r.mantPostScale.read()[51] && (!(mant05 && (!mantEven))));

    // Check Borders
    if (r.a.read()(62, 52) == 0x7ff) {
        nanA = 1;
    }
    if (r.b.read()(62, 52) == 0x7ff) {
        nanB = 1;
    }
    if (r.a.read()(51, 0).or_reduce() == 0) {
        mantZeroA = 1;
    }
    if (r.b.read()(51, 0).or_reduce() == 0) {
        mantZeroB = 1;
    }

    // Result multiplexers:
    if (nanA && mantZeroA && nanB && mantZeroB) {
        res[63] = 1;
    } else if (nanA && (!mantZeroA)) {
        res[63] = signA;
    } else if (nanB && (!mantZeroB)) {
        res[63] = signB;
    } else if (r.zeroA && r.zeroB) {
        res[63] = 1;
    } else {
        res[63] = (r.a.read()[63] ^ r.b.read()[63]);
    }

    if ((nanB && (!mantZeroB)) == 1) {
        res(62, 52) = r.b.read()(62, 52);
    } else if (((r.underflow || r.zeroA) && (!r.zeroB)) == 1) {
        res(62, 52) = 0;
    } else if ((r.overflow || r.zeroB) == 1) {
        res(62, 52) = ~0ull;
    } else if (nanA == 1) {
        res(62, 52) = r.a.read()(62, 52);
    } else if (((nanB && mantZeroB) || r.expAlign.read()[11]) == 1) {
        res(62, 52) = 0;
    } else {
        res(62, 52) = (r.expAlign.read()(10, 0) + (mantOnes && rndBit && (!r.overflow)));
    }

    if (((r.zeroA && r.zeroB)
            || (nanA && mantZeroA && nanB && mantZeroB)) == 1) {
        res[51] = 1;
        res(50, 0) = 0;
    } else if ((nanA && (!mantZeroA)) == 1) {
        res[51] = 1;
        res(50, 0) = r.a.read()(50, 0);
    } else if ((nanB && (!mantZeroB)) == 1) {
        res[51] = 1;
        res(50, 0) = r.b.read()(50, 0);
    } else if ((r.overflow
                || r.nanRes
                || (nanA && mantZeroA)
                || (nanB && mantZeroB)) == 1) {
        res(51, 0) = 0;
    } else {
        res(51, 0) = (mantShort(51, 0) + rndBit);
    }

    if (r.ena.read()[3] == 1) {
        v.result = res;
        v.illegal_op = (nanA || nanB);
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        DoubleDiv_r_reset(v);
    }

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_divbyzero = r.zeroB;
    o_overflow = r.overflow;
    o_underflow = r.underflow;
    o_valid = r.ena.read()[4];
    o_busy = r.busy;
}

void DoubleDiv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        DoubleDiv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

