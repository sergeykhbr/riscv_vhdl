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

#include "fmul_d.h"
#include "api_core.h"

namespace debugger {

DoubleMul::DoubleMul(sc_module_name name,
                     bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_a("i_a"),
    i_b("i_b"),
    o_res("o_res"),
    o_illegal_op("o_illegal_op"),
    o_overflow("o_overflow"),
    o_valid("o_valid"),
    o_busy("o_busy") {

    async_reset_ = async_reset;
    u_imul53 = 0;

    u_imul53 = new imul53("u_imul53", async_reset);
    u_imul53->i_clk(i_clk);
    u_imul53->i_nrst(i_nrst);
    u_imul53->i_ena(w_imul_ena);
    u_imul53->i_a(r.mantA);
    u_imul53->i_b(r.mantB);
    u_imul53->o_result(wb_imul_result);
    u_imul53->o_shift(wb_imul_shift);
    u_imul53->o_rdy(w_imul_rdy);
    u_imul53->o_overflow(w_imul_overflow);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << w_imul_ena;
    sensitive << wb_imul_result;
    sensitive << wb_imul_shift;
    sensitive << w_imul_rdy;
    sensitive << w_imul_overflow;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.a;
    sensitive << r.b;
    sensitive << r.result;
    sensitive << r.zeroA;
    sensitive << r.zeroB;
    sensitive << r.mantA;
    sensitive << r.mantB;
    sensitive << r.expAB;
    sensitive << r.expAlign;
    sensitive << r.mantAlign;
    sensitive << r.postShift;
    sensitive << r.mantPostScale;
    sensitive << r.nanA;
    sensitive << r.nanB;
    sensitive << r.overflow;
    sensitive << r.illegal_op;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

DoubleMul::~DoubleMul() {
    if (u_imul53) {
        delete u_imul53;
    }
}

void DoubleMul::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, i_b, i_b.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_illegal_op, o_illegal_op.name());
        sc_trace(o_vcd, o_overflow, o_overflow.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_busy, o_busy.name());
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.a, pn + ".r_a");
        sc_trace(o_vcd, r.b, pn + ".r_b");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.zeroA, pn + ".r_zeroA");
        sc_trace(o_vcd, r.zeroB, pn + ".r_zeroB");
        sc_trace(o_vcd, r.mantA, pn + ".r_mantA");
        sc_trace(o_vcd, r.mantB, pn + ".r_mantB");
        sc_trace(o_vcd, r.expAB, pn + ".r_expAB");
        sc_trace(o_vcd, r.expAlign, pn + ".r_expAlign");
        sc_trace(o_vcd, r.mantAlign, pn + ".r_mantAlign");
        sc_trace(o_vcd, r.postShift, pn + ".r_postShift");
        sc_trace(o_vcd, r.mantPostScale, pn + ".r_mantPostScale");
        sc_trace(o_vcd, r.nanA, pn + ".r_nanA");
        sc_trace(o_vcd, r.nanB, pn + ".r_nanB");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
        sc_trace(o_vcd, r.illegal_op, pn + ".r_illegal_op");
    }

    if (u_imul53) {
        u_imul53->generateVCD(i_vcd, o_vcd);
    }
}

void DoubleMul::comb() {
    sc_uint<5> vb_ena;
    sc_uint<1> signA;
    sc_uint<1> signB;
    sc_uint<53> mantA;
    sc_uint<53> mantB;
    bool zeroA;
    bool zeroB;
    sc_uint<12> expAB_t;
    sc_uint<13> expAB;
    sc_biguint<105> mantAlign;
    sc_uint<13> expAlign_t;
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
    bool v_res_sign;
    sc_uint<11> vb_res_exp;
    sc_uint<52> vb_res_mant;

    vb_ena = 0;
    signA = 0;
    signB = 0;
    mantA = 0;
    mantB = 0;
    zeroA = 0;
    zeroB = 0;
    expAB_t = 0;
    expAB = 0;
    mantAlign = 0;
    expAlign_t = 0;
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
    v_res_sign = 0;
    vb_res_exp = 0;
    vb_res_mant = 0;

    v = r;

    vb_ena[0] = (i_ena && (!r.busy));
    vb_ena[1] = r.ena.read()[0];
    vb_ena(4, 2) = (r.ena.read()(3, 2), w_imul_rdy.read());
    v.ena = vb_ena;

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.overflow = 0;
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
    }

    // expA - expB + 1023
    expAB_t = ((0, r.a.read()(62, 52)) + (0, r.b.read()(62, 52)));
    expAB = ((0, expAB_t) - 1023);

    if (r.ena.read()[0] == 1) {
        v.expAB = expAB;
        v.zeroA = zeroA;
        v.zeroB = zeroB;
        v.mantA = mantA;
        v.mantB = mantB;
    }

    w_imul_ena = r.ena.read()[1];

    // imul53 module:
    if (wb_imul_result.read()[105] == 1) {
        mantAlign = wb_imul_result.read()(105, 1);
    } else if (wb_imul_result.read()[104] == 1) {
        mantAlign = wb_imul_result.read()(104, 0);
    } else {
        for (int i = 1; i < 105; i++) {
            if (i == wb_imul_shift.read().to_int()) {
                mantAlign = (wb_imul_result.read() << i);
            }
        }
    }

    expAlign_t = (r.expAB.read() + 1);
    if (wb_imul_result.read()[105] == 1) {
        expAlign = expAlign_t;
    } else if ((r.a.read()(62, 52).or_reduce() == 0) || (r.b.read()(62, 52).or_reduce() == 0)) {
        expAlign = (expAlign_t - (0, wb_imul_shift.read()));
    } else {
        expAlign = (r.expAB.read() - (0, wb_imul_shift.read()));
    }

    // IMPORTANT exception! new ZERO value
    if ((expAlign[12] == 1) || (expAlign.or_reduce() == 0)) {
        if ((wb_imul_shift.read().or_reduce() == 0)
                || (wb_imul_result.read()[105] == 1)
                || (r.a.read()(62, 52).or_reduce() == 0)
                || (r.b.read()(62, 52).or_reduce() == 0)) {
            postShift = ((~expAlign(11, 0)) + 2);
        } else {
            postShift = ((~expAlign(11, 0)) + 1);
        }
    }

    if (w_imul_rdy.read() == 1) {
        v.expAlign = expAlign(11, 0);
        v.mantAlign = mantAlign;
        v.postShift = postShift;

        // Exceptions:
        v.nanA = 0;
        if (r.a.read()(62, 52) == 0x7FF) {
            v.nanA = 1;
        }
        v.nanB = 0;
        if (r.b.read()(62, 52) == 0x7FF) {
            v.nanB = 1;
        }
        v.overflow = 0;
        if ((expAlign[12] == 0) && (expAlign >= 0x7FF)) {
            v.overflow = 1;
        }
    }

    // Prepare to mantissa post-scale
    if (r.postShift.read().or_reduce() == 0) {
        mantPostScale = r.mantAlign;
    } else if (r.postShift.read() < 105) {
        for (int i = 1; i < 105; i++) {
            if (i == r.postShift.read().to_int()) {
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
    if ((nanA && mantZeroA && r.zeroB) || (nanB && mantZeroB && r.zeroA)) {
        v_res_sign = 1;
    } else if ((nanA && (!mantZeroA)) == 1) {
        // when both values are NaN, value B has higher priority if sign=1
        v_res_sign = (signA || (nanA && signB));
    } else if ((nanB && (!mantZeroB)) == 1) {
        v_res_sign = signB;
    } else {
        v_res_sign = (r.a.read()[63] ^ r.b.read()[63]);
    }

    if (nanA == 1) {
        vb_res_exp = r.a.read()(62, 52);
    } else if (nanB == 1) {
        vb_res_exp = r.b.read()(62, 52);
    } else if ((r.expAlign.read()[11] || r.zeroA || r.zeroB) == 1) {
        vb_res_exp = 0;
    } else if (r.overflow.read() == 1) {
        vb_res_exp = ~0ull;
    } else {
        vb_res_exp = (r.expAlign.read()(10, 0)
                + (mantOnes && rndBit && (!r.overflow)));
    }

    if ((nanA && mantZeroA && (!mantZeroB))
            || (nanB && mantZeroB && (!mantZeroA))
            || ((!nanA) && (!nanB) && r.overflow)) {
        vb_res_mant = 0;
    } else if ((nanA && (!(nanB && signB))) == 1) {
        // when both values are NaN, value B has higher priority if sign=1
        vb_res_mant = (1, r.a.read()(50, 0));
    } else if (nanB == 1) {
        vb_res_mant = (1, r.b.read()(50, 0));
    } else {
        vb_res_mant = (mantShort(51, 0) + rndBit);
    }

    if (r.ena.read()[3] == 1) {
        v.result = (v_res_sign, vb_res_exp, vb_res_mant);
        v.illegal_op = (nanA || nanB);
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        DoubleMul_r_reset(v);
    }

    o_res = r.result;
    o_illegal_op = r.illegal_op;
    o_overflow = r.overflow;
    o_valid = r.ena.read()[4];
    o_busy = r.busy;
}

void DoubleMul::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        DoubleMul_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

