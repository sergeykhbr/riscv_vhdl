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

#include "imul53.h"

namespace debugger {

imul53::imul53(sc_module_name name_, bool async_reset) : sc_module(name_),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_ena("i_ena"),
    i_a("i_a"),
    i_b("i_b"),
    o_result("o_result"),
    o_shift("o_shift"),
    o_rdy("o_rdy"),
    o_overflow("o_overflow") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_clk;
    sensitive << i_ena;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << r.delay;
    sensitive << r.shift;
    sensitive << r.accum_ena;
    sensitive << r.b;
    sensitive << r.sum;
    sensitive << r.overflow;
 
    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void imul53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, i_b, i_b.name());
        sc_trace(o_vcd, o_result, o_result.name());
        sc_trace(o_vcd, o_rdy, o_rdy.name());

        std::string pn(name());
        sc_trace(o_vcd, r.delay, pn + ".r_delay");
        sc_trace(o_vcd, r.shift, pn + ".r_shift");
        sc_trace(o_vcd, r.accum_ena, pn + ".r_accum_ena");
        sc_trace(o_vcd, r.b, pn + ".r_b");
        sc_trace(o_vcd, r.sum, pn + ".r_sum");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
    }
}

void imul53::comb() {
    sc_uint<57> vb_mux[17];
    sc_uint<57> vb_sel;
    sc_uint<7> vb_shift;
    v = r;

    vb_mux[0] = 0;
    vb_mux[1] = i_a.read();                     // 1*a
    vb_mux[2] = i_a.read().to_uint64() << 1;    // 2*a
    vb_mux[3] = vb_mux[2] + vb_mux[1];          // 2*a + 1*a
    vb_mux[4] = i_a.read().to_uint64() << 2;    // 4*a
    vb_mux[5] = vb_mux[4] + vb_mux[1];          // 4*a + 1*a
    vb_mux[6] = vb_mux[4] + vb_mux[2];          // 4*a + 2*a
    vb_mux[8] = i_a.read().to_uint64() << 3;    // 8*a
    vb_mux[7] = vb_mux[8] - vb_mux[1];          // 8*a - 1*a
    vb_mux[9] = vb_mux[8] + vb_mux[1];          // 8*a + 1*a
    vb_mux[10] = vb_mux[8] + vb_mux[2];         // 8*a + 2*a
    vb_mux[11] = vb_mux[10] + vb_mux[1];        // (8*a + 2*a) + 1*a
    vb_mux[12] = vb_mux[8] + vb_mux[4];         // 8*a + 4*a
    vb_mux[16] = (i_a.read().to_uint64() << 4); // unused
    vb_mux[13] = vb_mux[16] - vb_mux[3];        // 16*a - (2*a + 1*a)
    vb_mux[14] = vb_mux[16] - vb_mux[2];        // 16*a - 2*a
    vb_mux[15] = vb_mux[16] - vb_mux[1];        // 16*a - 1*a

    v.delay = (r.delay.read()(14, 0), i_ena.read());
    if (i_ena == 1) {
        v.b = (0, i_b.read());
        v.overflow = 0;
        v.accum_ena = 1;
        v.sum = 0;
        v.shift = 0;
    } else if (r.delay.read()[13]) {
        v.accum_ena = 0;
    }

    switch (r.b.read()(55, 52)) {
    case 1: vb_sel = vb_mux[1]; break;
    case 2: vb_sel = vb_mux[2]; break;
    case 3: vb_sel = vb_mux[3]; break;
    case 4: vb_sel = vb_mux[4]; break;
    case 5: vb_sel = vb_mux[5]; break;
    case 6: vb_sel = vb_mux[6]; break;
    case 7: vb_sel = vb_mux[7]; break;
    case 8: vb_sel = vb_mux[8]; break;
    case 9: vb_sel = vb_mux[9]; break;
    case 10: vb_sel = vb_mux[10]; break;
    case 11: vb_sel = vb_mux[11]; break;
    case 12: vb_sel = vb_mux[12]; break;
    case 13: vb_sel = vb_mux[13]; break;
    case 14: vb_sel = vb_mux[14]; break;
    case 15: vb_sel = vb_mux[15]; break;
    default:
        vb_sel = 0;
    }
    if (r.accum_ena == 1) {
        v.sum = (r.sum.read() << 4) + vb_sel.to_uint64();
        v.b = r.b.read() << 4;
    }

    // To avoid timing constrains violation try to implement parallel demux
    // for Xilinx Vivado
    sc_biguint<105> vb_sumInv;
    sc_uint<7> vb_lshift_p1;
    sc_uint<7> vb_lshift_p2;
    vb_sumInv[0] = 0;
    for (unsigned i = 0; i < 104; i++) {
        vb_sumInv[i + 1] = r.sum.read()[103 - i];
    }

    vb_lshift_p1 = 0;
    for (unsigned i = 0; i < 64; i++) {
        if (vb_lshift_p1 == 0 && vb_sumInv[i] == 1) {
            vb_lshift_p1 = i;
        }
    }

    vb_lshift_p2 = 0;
    for (unsigned i = 0; i < 41; i++) {
        if (vb_lshift_p2 == 0 && vb_sumInv[64 + i] == 1) {
            vb_lshift_p2 = 0x40 | i;
        }
    }


    if (r.sum.read()[105] == 1) {
        vb_shift = 0x7F;
        v.overflow = 1;
    } else if (r.sum.read()[104] == 1) {
        vb_shift = 0;
    } else if (vb_lshift_p1 != 0) {
        vb_shift = vb_lshift_p1;
    } else {
        vb_shift = vb_lshift_p2;
    }

    if (r.delay.read()[14]) {
        v.shift = vb_shift;
        v.overflow = 0;
        if (vb_shift == 0x7f) {
            v.overflow = 1;
        }
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_result = r.sum.read();
    o_shift = r.shift.read();
    o_overflow = r.overflow;
    o_rdy = r.delay.read()[15];
}

void imul53::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

