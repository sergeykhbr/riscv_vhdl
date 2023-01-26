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

#include "imul53.h"
#include "api_core.h"

namespace debugger {

imul53::imul53(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_a("i_a"),
    i_b("i_b"),
    o_result("o_result"),
    o_shift("o_shift"),
    o_rdy("o_rdy"),
    o_overflow("o_overflow") {

    async_reset_ = async_reset;
    enc0 = 0;

    enc0 = new zeroenc<105,
                       7>("enc0");
    enc0->i_value(wb_sumInv);
    enc0->o_shift(wb_lshift);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << wb_sumInv;
    sensitive << wb_lshift;
    sensitive << r.delay;
    sensitive << r.shift;
    sensitive << r.accum_ena;
    sensitive << r.b;
    sensitive << r.sum;
    sensitive << r.overflow;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

imul53::~imul53() {
    if (enc0) {
        delete enc0;
    }
}

void imul53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_a, i_a.name());
        sc_trace(o_vcd, i_b, i_b.name());
        sc_trace(o_vcd, o_result, o_result.name());
        sc_trace(o_vcd, o_shift, o_shift.name());
        sc_trace(o_vcd, o_rdy, o_rdy.name());
        sc_trace(o_vcd, o_overflow, o_overflow.name());
        sc_trace(o_vcd, r.delay, pn + ".r_delay");
        sc_trace(o_vcd, r.shift, pn + ".r_shift");
        sc_trace(o_vcd, r.accum_ena, pn + ".r_accum_ena");
        sc_trace(o_vcd, r.b, pn + ".r_b");
        sc_trace(o_vcd, r.sum, pn + ".r_sum");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
    }

    if (enc0) {
        enc0->generateVCD(i_vcd, o_vcd);
    }
}

void imul53::comb() {
    sc_uint<1> v_ena;
    sc_uint<57> vb_mux[17];
    sc_uint<57> vb_sel;
    sc_uint<7> vb_shift;
    sc_biguint<105> vb_sumInv;

    v_ena = 0;
    for (int i = 0; i < 17; i++) {
        vb_mux[i] = 0ull;
    }
    vb_sel = 0;
    vb_shift = 0;
    vb_sumInv = 0;

    v = r;


    vb_mux[0] = 0;
    vb_mux[1] = i_a;
    vb_mux[2] = (i_a.read() << 1);
    vb_mux[3] = (vb_mux[2] + vb_mux[1]);
    vb_mux[4] = (i_a.read() << 2);
    vb_mux[5] = (vb_mux[4] + vb_mux[1]);
    vb_mux[6] = (vb_mux[4] + vb_mux[2]);
    vb_mux[8] = (i_a.read() << 3);
    vb_mux[7] = (vb_mux[8] - vb_mux[1]);
    vb_mux[9] = (vb_mux[8] + vb_mux[1]);
    vb_mux[10] = (vb_mux[8] + vb_mux[2]);
    vb_mux[11] = (vb_mux[10] + vb_mux[1]);
    vb_mux[12] = (vb_mux[8] + vb_mux[4]);
    vb_mux[16] = (i_a.read() << 4);
    vb_mux[13] = (vb_mux[16] - vb_mux[3]);
    vb_mux[14] = (vb_mux[16] - vb_mux[2]);
    vb_mux[15] = (vb_mux[16] - vb_mux[1]);

    v_ena = i_ena;
    v.delay = (r.delay.read()(14, 0), v_ena);

    if (i_ena.read() == 1) {
        v.b = (0, i_b.read());
        v.overflow = 0;
        v.accum_ena = 1;
        v.sum = 0;
        v.shift = 0;
    } else if (r.delay.read()[13] == 1) {
        v.accum_ena = 0;
    }

    switch (r.b.read()(55, 52)) {
    case 1:
        vb_sel = vb_mux[1];
        break;
    case 2:
        vb_sel = vb_mux[2];
        break;
    case 3:
        vb_sel = vb_mux[3];
        break;
    case 4:
        vb_sel = vb_mux[4];
        break;
    case 5:
        vb_sel = vb_mux[5];
        break;
    case 6:
        vb_sel = vb_mux[6];
        break;
    case 7:
        vb_sel = vb_mux[7];
        break;
    case 8:
        vb_sel = vb_mux[8];
        break;
    case 9:
        vb_sel = vb_mux[9];
        break;
    case 10:
        vb_sel = vb_mux[10];
        break;
    case 11:
        vb_sel = vb_mux[11];
        break;
    case 12:
        vb_sel = vb_mux[12];
        break;
    case 13:
        vb_sel = vb_mux[13];
        break;
    case 14:
        vb_sel = vb_mux[14];
        break;
    case 15:
        vb_sel = vb_mux[15];
        break;
    default:
        vb_sel = 0;
        break;
    }
    if (r.accum_ena.read() == 1) {
        v.sum = ((r.sum.read() << 4) + vb_sel);
        v.b = (r.b.read() << 4);
    }

    // To avoid timing constrains violation try to implement parallel demux
    // for Xilinx Vivado
    for (int i = 0; i < 104; i++) {
        vb_sumInv[(i + 1)] = r.sum.read()[(103 - i)];
    }
    wb_sumInv = vb_sumInv;

    if (r.sum.read()[105] == 1) {
        vb_shift = ~0ull;
        v.overflow = 1;
    } else if (r.sum.read()[104] == 1) {
        vb_shift = 0;
    } else {
        vb_shift = wb_lshift;
    }

    if (r.delay.read()[14] == 1) {
        v.shift = vb_shift;
        v.overflow = 0;
        if (vb_shift == 0x7f) {
            v.overflow = 1;
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        imul53_r_reset(v);
    }

    o_result = r.sum;
    o_shift = r.shift;
    o_overflow = r.overflow;
    o_rdy = r.delay.read()[15];
}

void imul53::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        imul53_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

