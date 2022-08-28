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

#include "idiv53.h"
#include "api_core.h"

namespace debugger {

idiv53::idiv53(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_divident("i_divident"),
    i_divisor("i_divisor"),
    o_result("o_result"),
    o_lshift("o_lshift"),
    o_rdy("o_rdy"),
    o_overflow("o_overflow"),
    o_zero_resid("o_zero_resid") {

    async_reset_ = async_reset;
    divstage0 = 0;

    divstage0 = new divstage53("divstage0");
    divstage0->i_mux_ena(w_mux_ena_i);
    divstage0->i_muxind(wb_muxind_i);
    divstage0->i_divident(wb_divident_i);
    divstage0->i_divisor(wb_divisor_i);
    divstage0->o_dif(wb_dif_o);
    divstage0->o_bits(wb_bits_o);
    divstage0->o_muxind(wb_muxind_o);
    divstage0->o_muxind_rdy(w_muxind_rdy_o);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_divident;
    sensitive << i_divisor;
    sensitive << w_mux_ena_i;
    sensitive << wb_muxind_i;
    sensitive << wb_divident_i;
    sensitive << wb_divisor_i;
    sensitive << wb_dif_o;
    sensitive << wb_bits_o;
    sensitive << wb_muxind_o;
    sensitive << w_muxind_rdy_o;
    sensitive << r.delay;
    sensitive << r.lshift;
    sensitive << r.lshift_rdy;
    sensitive << r.divisor;
    sensitive << r.divident;
    sensitive << r.bits;
    sensitive << r.overflow;
    sensitive << r.zero_resid;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

idiv53::~idiv53() {
    if (divstage0) {
        delete divstage0;
    }
}

void idiv53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_divident, i_divident.name());
        sc_trace(o_vcd, i_divisor, i_divisor.name());
        sc_trace(o_vcd, o_result, o_result.name());
        sc_trace(o_vcd, o_lshift, o_lshift.name());
        sc_trace(o_vcd, o_rdy, o_rdy.name());
        sc_trace(o_vcd, o_overflow, o_overflow.name());
        sc_trace(o_vcd, o_zero_resid, o_zero_resid.name());
        sc_trace(o_vcd, r.delay, pn + ".r_delay");
        sc_trace(o_vcd, r.lshift, pn + ".r_lshift");
        sc_trace(o_vcd, r.lshift_rdy, pn + ".r_lshift_rdy");
        sc_trace(o_vcd, r.divisor, pn + ".r_divisor");
        sc_trace(o_vcd, r.divident, pn + ".r_divident");
        sc_trace(o_vcd, r.bits, pn + ".r_bits");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
        sc_trace(o_vcd, r.zero_resid, pn + ".r_zero_resid");
    }

    if (divstage0) {
        divstage0->generateVCD(i_vcd, o_vcd);
    }
}

void idiv53::comb() {
    sc_uint<1> v_ena;
    sc_uint<56> vb_muxind;
    sc_biguint<105> vb_bits;
    bool v_mux_ena_i;

    v_ena = 0;
    vb_muxind = 0;
    vb_bits = 0;
    v_mux_ena_i = 0;

    v = r;

    vb_bits = r.bits;

    v_ena = i_ena;
    v.delay = (r.delay.read()(13, 0), v_ena);
    if (i_ena.read() == 1) {
        v.divident = (0, i_divident.read());
        v.divisor = i_divisor;
        v.lshift_rdy = 0;
        v.overflow = 0;
        v.zero_resid = 0;
    } else if (r.delay.read()[0] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_bits[104] = (!wb_dif_o.read()[52]);
    } else if (r.delay.read()[1] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 1;
        vb_muxind(48, 42) = 2;
        vb_muxind(41, 35) = 3;
        vb_muxind(34, 28) = 4;
        vb_muxind(27, 21) = 5;
        vb_muxind(20, 14) = 6;
        vb_muxind(13, 7) = 7;
        vb_muxind(6, 0) = 8;
        vb_bits(103, 96) = wb_bits_o;
    } else if (r.delay.read()[2] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 9;
        vb_muxind(48, 42) = 10;
        vb_muxind(41, 35) = 11;
        vb_muxind(34, 28) = 12;
        vb_muxind(27, 21) = 13;
        vb_muxind(20, 14) = 14;
        vb_muxind(13, 7) = 15;
        vb_muxind(6, 0) = 16;
        vb_bits(95, 88) = wb_bits_o;
    } else if (r.delay.read()[3] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 17;
        vb_muxind(48, 42) = 18;
        vb_muxind(41, 35) = 19;
        vb_muxind(34, 28) = 20;
        vb_muxind(27, 21) = 21;
        vb_muxind(20, 14) = 22;
        vb_muxind(13, 7) = 23;
        vb_muxind(6, 0) = 24;
        vb_bits(87, 80) = wb_bits_o;
    } else if (r.delay.read()[4] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 25;
        vb_muxind(48, 42) = 26;
        vb_muxind(41, 35) = 27;
        vb_muxind(34, 28) = 28;
        vb_muxind(27, 21) = 29;
        vb_muxind(20, 14) = 30;
        vb_muxind(13, 7) = 31;
        vb_muxind(6, 0) = 32;
        vb_bits(79, 72) = wb_bits_o;
    } else if (r.delay.read()[5] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 33;
        vb_muxind(48, 42) = 34;
        vb_muxind(41, 35) = 35;
        vb_muxind(34, 28) = 36;
        vb_muxind(27, 21) = 37;
        vb_muxind(20, 14) = 38;
        vb_muxind(13, 7) = 39;
        vb_muxind(6, 0) = 40;
        vb_bits(71, 64) = wb_bits_o;
    } else if (r.delay.read()[6] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 41;
        vb_muxind(48, 42) = 42;
        vb_muxind(41, 35) = 43;
        vb_muxind(34, 28) = 44;
        vb_muxind(27, 21) = 45;
        vb_muxind(20, 14) = 46;
        vb_muxind(13, 7) = 47;
        vb_muxind(6, 0) = 48;
        vb_bits(63, 56) = wb_bits_o;
    } else if (r.delay.read()[7] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 49;
        vb_muxind(48, 42) = 50;
        vb_muxind(41, 35) = 51;
        vb_muxind(34, 28) = 52;
        vb_muxind(27, 21) = 53;
        vb_muxind(20, 14) = 54;
        vb_muxind(13, 7) = 55;
        vb_muxind(6, 0) = 56;
        vb_bits(55, 48) = wb_bits_o;
    } else if (r.delay.read()[8] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 57;
        vb_muxind(48, 42) = 58;
        vb_muxind(41, 35) = 59;
        vb_muxind(34, 28) = 60;
        vb_muxind(27, 21) = 61;
        vb_muxind(20, 14) = 62;
        vb_muxind(13, 7) = 63;
        vb_muxind(6, 0) = 64;
        vb_bits(47, 40) = wb_bits_o;
    } else if (r.delay.read()[9] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 65;
        vb_muxind(48, 42) = 66;
        vb_muxind(41, 35) = 67;
        vb_muxind(34, 28) = 68;
        vb_muxind(27, 21) = 69;
        vb_muxind(20, 14) = 70;
        vb_muxind(13, 7) = 71;
        vb_muxind(6, 0) = 72;
        vb_bits(39, 32) = wb_bits_o;
    } else if (r.delay.read()[10] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 73;
        vb_muxind(48, 42) = 74;
        vb_muxind(41, 35) = 75;
        vb_muxind(34, 28) = 76;
        vb_muxind(27, 21) = 77;
        vb_muxind(20, 14) = 78;
        vb_muxind(13, 7) = 79;
        vb_muxind(6, 0) = 80;
        vb_bits(31, 24) = wb_bits_o;
    } else if (r.delay.read()[11] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 81;
        vb_muxind(48, 42) = 82;
        vb_muxind(41, 35) = 83;
        vb_muxind(34, 28) = 84;
        vb_muxind(27, 21) = 85;
        vb_muxind(20, 14) = 86;
        vb_muxind(13, 7) = 87;
        vb_muxind(6, 0) = 88;
        vb_bits(23, 16) = wb_bits_o;
    } else if (r.delay.read()[12] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 89;
        vb_muxind(48, 42) = 90;
        vb_muxind(41, 35) = 91;
        vb_muxind(34, 28) = 92;
        vb_muxind(27, 21) = 93;
        vb_muxind(20, 14) = 94;
        vb_muxind(13, 7) = 95;
        vb_muxind(6, 0) = 96;
        vb_bits(15, 8) = wb_bits_o;
    } else if (r.delay.read()[13] == 1) {
        v_mux_ena_i = (!r.lshift_rdy);
        v.divident = (wb_dif_o.read() << 8);
        vb_muxind(55, 49) = 97;
        vb_muxind(48, 42) = 98;
        vb_muxind(41, 35) = 99;
        vb_muxind(34, 28) = 100;
        vb_muxind(27, 21) = 101;
        vb_muxind(20, 14) = 102;
        vb_muxind(13, 7) = 103;
        vb_muxind(6, 0) = 104;
        vb_bits(7, 0) = wb_bits_o;

        if (wb_dif_o.read().or_reduce() == 0) {
            v.zero_resid = 1;
        }
        if (r.lshift.read() == 0x7F) {
            v.overflow = 1;
        }
    }

    if (r.lshift_rdy.read() == 0) {
        if (w_muxind_rdy_o.read() == 1) {
            v.lshift_rdy = 1;
            v.lshift = wb_muxind_o;
        } else if (r.delay.read()[13] == 1) {
            v.lshift_rdy = 1;
            v.lshift = 104;
        }
    }

    w_mux_ena_i = v_mux_ena_i;
    wb_divident_i = r.divident;
    wb_divisor_i = r.divisor;
    wb_muxind_i = vb_muxind;
    v.bits = vb_bits;

    if (!async_reset_ && i_nrst.read() == 0) {
        idiv53_r_reset(v);
    }

    o_result = r.bits;
    o_lshift = r.lshift;
    o_overflow = r.overflow;
    o_zero_resid = r.zero_resid;
    o_rdy = r.delay.read()[14];
}

void idiv53::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        idiv53_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

