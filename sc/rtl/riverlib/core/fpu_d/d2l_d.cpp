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

#include "d2l_d.h"
#include "api_core.h"

namespace debugger {

Double2Long::Double2Long(sc_module_name name,
                         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_signed("i_signed"),
    i_w32("i_w32"),
    i_a("i_a"),
    o_res("o_res"),
    o_overflow("o_overflow"),
    o_underflow("o_underflow"),
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
    sensitive << r.expA;
    sensitive << r.mantA;
    sensitive << r.result;
    sensitive << r.op_signed;
    sensitive << r.w32;
    sensitive << r.mantPostScale;
    sensitive << r.overflow;
    sensitive << r.underflow;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void Double2Long::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_signed, i_signed.name());
        sc_trace(o_vcd, i_w32, i_w32.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_overflow, o_overflow.name());
        sc_trace(o_vcd, o_underflow, o_underflow.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_busy, o_busy.name());
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.signA, pn + ".r_signA");
        sc_trace(o_vcd, r.expA, pn + ".r_expA");
        sc_trace(o_vcd, r.mantA, pn + ".r_mantA");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.op_signed, pn + ".r_op_signed");
        sc_trace(o_vcd, r.w32, pn + ".r_w32");
        sc_trace(o_vcd, r.mantPostScale, pn + ".r_mantPostScale");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
        sc_trace(o_vcd, r.underflow, pn + ".r_underflow");
    }

}

void Double2Long::comb() {
    sc_uint<1> v_ena;
    sc_uint<53> mantA;
    bool expDif_gr;                                         // greater than 1023 + 63
    bool expDif_lt;                                         // less than 1023
    bool overflow;
    bool underflow;
    sc_uint<12> expDif;
    sc_uint<64> mantPreScale;
    sc_uint<64> mantPostScale;
    sc_uint<11> expMax;
    sc_uint<6> expShift;
    bool resSign;
    sc_uint<64> resMant;
    sc_uint<64> res;

    v_ena = 0;
    mantA = 0;
    expDif_gr = 0;
    expDif_lt = 0;
    overflow = 0;
    underflow = 0;
    expDif = 0;
    mantPreScale = 0;
    mantPostScale = 0;
    expMax = 0;
    expShift = 0;
    resSign = 0;
    resMant = 0;
    res = 0;

    v = r;

    v_ena = (i_ena && (!r.busy));
    v.ena = (r.ena.read()(1, 0), v_ena);

    mantA(51, 0) = i_a.read()(51, 0);
    mantA[52] = 0;
    if (i_a.read()(62, 52).or_reduce() == 1) {
        mantA[52] = 1;
    }

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.signA = i_a.read()[63];
        v.expA = i_a.read()(62, 52);
        v.mantA = mantA;
        v.op_signed = i_signed;
        v.w32 = i_w32;
        v.overflow = 0;
        v.underflow = 0;
    }

    // (1086 - expA)[5:0]
    expShift = (0x3e - r.expA.read()(5, 0));
    if (r.w32.read() == 1) {
        if (r.op_signed.read() == 1) {
            expMax = 1053;
        } else {
            expMax = 1085;
        }
    } else {
        if ((r.op_signed || r.signA) == 1) {
            expMax = 1085;
        } else {
            expMax = 1086;
        }
    }
    expDif = ((0, expMax) - (0, r.expA.read()));

    expDif_gr = expDif[11];
    expDif_lt = 0;
    if ((r.expA.read() != 0x3FF) && (r.expA.read()[10] == 0)) {
        expDif_lt = 1;
    }

    mantPreScale = (r.mantA.read().to_uint64() << 11);

    mantPostScale = 0;
    if (expDif_gr == 1) {
        overflow = 1;
        underflow = 0;
    } else if (expDif_lt == 1) {
        overflow = 0;
        underflow = 1;
    } else {
        overflow = 0;
        underflow = 0;
        // Multiplexer, probably switch case in rtl
        for (int i = 0; i < 64; i++) {
            if (expShift == i) {
                mantPostScale = (mantPreScale >> i);
            }
        }
    }

    if (r.ena.read()[0] == 1) {
        v.overflow = overflow;
        v.underflow = underflow;
        v.mantPostScale = mantPostScale;
    }

    // Result multiplexers:
    resSign = ((r.signA || r.overflow) && (!r.underflow));
    if (r.signA.read() == 1) {
        resMant = ((~r.mantPostScale.read()) + 1);
    } else {
        resMant = r.mantPostScale;
    }

    res = resMant;
    if (r.op_signed.read() == 1) {
        if (resSign == 1) {
            if (r.w32.read() == 1) {
                res(63, 31) = ~0ull;
            } else {
                res[63] = 1;
            }
        }
    } else {
        if (r.w32.read() == 1) {
            res(63, 32) = 0;
        } else if (r.overflow.read() == 1) {
            res[63] = 1;
        }
    }

    if (r.ena.read()[1] == 1) {
        v.result = res;
        v.busy = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        Double2Long_r_reset(v);
    }

    o_res = r.result;
    o_overflow = r.overflow;
    o_underflow = r.underflow;
    o_valid = r.ena.read()[2];
    o_busy = r.busy;
}

void Double2Long::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        Double2Long_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

