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

#include "l2d_d.h"
#include "api_core.h"

namespace debugger {

Long2Double::Long2Double(sc_module_name name,
                         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_signed("i_signed"),
    i_w32("i_w32"),
    i_a("i_a"),
    o_res("o_res"),
    o_valid("o_valid"),
    o_busy("o_busy") {

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
}

void Long2Double::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_signed, i_signed.name());
        sc_trace(o_vcd, i_w32, i_w32.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_busy, o_busy.name());
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.signA, pn + ".r_signA");
        sc_trace(o_vcd, r.absA, pn + ".r_absA");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.op_signed, pn + ".r_op_signed");
        sc_trace(o_vcd, r.mantAlign, pn + ".r_mantAlign");
        sc_trace(o_vcd, r.lshift, pn + ".r_lshift");
    }

}

void Long2Double::comb() {
    sc_uint<1> v_ena;
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

    v_ena = 0;
    mantAlign = 0;
    lshift = 0;
    expAlign = 0;
    mantEven = 0;
    mant05 = 0;
    mantOnes = 0;
    rndBit = 0;
    v_signA = 0;
    vb_A = 0;
    res = 0;

    v = r;

    v_ena = (i_ena && (!r.busy));
    v.ena = (r.ena.read()(1, 0), v_ena);
    if (i_w32.read() == 0) {
        v_signA = i_a.read()[63];
        vb_A = i_a;
    } else if ((i_signed && i_a.read()[31]) == 1) {
        v_signA = 1;
        vb_A(63, 32) = ~0ull;
        vb_A(31, 0) = i_a.read()(31, 0);
    } else {
        v_signA = 0;
        vb_A(31, 0) = i_a.read()(31, 0);
        vb_A(63, 32) = 0;
    }

    if (i_ena.read() == 1) {
        v.busy = 1;
        if ((i_signed && v_signA) == 1) {
            v.signA = 1;
            v.absA = ((~vb_A) + 1);
        } else {
            v.signA = 0;
            v.absA = vb_A;
        }
        v.op_signed = i_signed;
    }

    // multiplexer, probably if/elsif in rtl:
    lshift = 63;
    if (r.absA.read()[63] == 1) {
        mantAlign = r.absA;
    } else {
        for (int i = 1; i < 64; i++) {
            if ((lshift == 63) && (r.absA.read()[(63 - i)] == 1)) {
                mantAlign = (r.absA.read() << i);
                lshift = i;
            }
        }
    }

    if (r.ena.read()[0] == 1) {
        v.mantAlign = mantAlign;
        v.lshift = lshift;
    }

    if (r.absA.read().or_reduce() == 0) {
        expAlign = 0;
    } else {
        expAlign = (1086 - r.lshift.read());
    }

    mantEven = r.mantAlign.read()[11];
    if (r.mantAlign.read()(10, 0) == 0x7ff) {
        mant05 = 1;
    }
    rndBit = (r.mantAlign.read()[10] && (!(mant05 && mantEven)));
    if (r.mantAlign.read()(63, 11) == 0x000fffffffffffffull) {
        mantOnes = 1;
    }

    // Result multiplexers:
    res[63] = (r.signA && r.op_signed);
    res(62, 52) = (expAlign + (mantOnes && rndBit));
    res(51, 0) = (r.mantAlign.read()(62, 11) + rndBit);

    if (r.ena.read()[1] == 1) {
        v.result = res;
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        Long2Double_r_reset(v);
    }

    o_res = r.result;
    o_valid = r.ena.read()[2];
    o_busy = r.busy;
}

void Long2Double::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        Long2Double_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

