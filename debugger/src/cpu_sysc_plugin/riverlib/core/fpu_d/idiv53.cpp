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

#include "idiv53.h"

namespace debugger {

idiv53::idiv53(sc_module_name name_) : sc_module(name_),
    divstage0("divstage") {
    divstage0.i_mux_ena(w_mux_ena_i);
    divstage0.i_muxind(wb_muxind_i);
    divstage0.i_divident(wb_divident_i);
    divstage0.i_divisor(wb_divisor_i);
    divstage0.o_dif(wb_dif_o);
    divstage0.o_bits(wb_bits_o);
    divstage0.o_muxind(wb_muxind_o);
    divstage0.o_muxind_rdy(w_muxind_rdy_o);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_clk;
    sensitive << i_ena;
    sensitive << i_divident;
    sensitive << i_divisor;
    sensitive << r.delay;
    sensitive << r.lshift;
    sensitive << r.lshift_rdy;
    sensitive << r.divisor;
    sensitive << r.divident;
    sensitive << r.bits;
    sensitive << r.overflow;
    sensitive << r.zero_resid;
    sensitive << wb_dif_o;
    sensitive << wb_bits_o;
    sensitive << wb_muxind_o;
    sensitive << w_muxind_rdy_o;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void idiv53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_result, "/top/proc0/idiv53/o_result");
    }
    sc_trace(i_vcd, i_clk, "/top/proc0/dfpu0/idiv53/i_clk");
    sc_trace(i_vcd, i_nrst, "/top/proc0/dfpu0/idiv53/i_nrst");
    sc_trace(i_vcd, i_ena, "/top/proc0/dfpu0/idiv53/i_ena");
    sc_trace(i_vcd, i_divisor, "/top/proc0/idiv53/i_divisor");
    sc_trace(i_vcd, i_divident, "/top/proc0/idiv53/i_divident");
    divstage0.generateVCD(i_vcd, o_vcd);

}

void idiv53::comb() {
    sc_uint<56> vb_muxind;
    sc_bv<105> vb_bits;
    v = r;
    vb_bits = r.bits;

    w_mux_ena_i = 0;
    v.delay = (r.delay.read()(15, 1), i_ena.read());
    if (i_ena.read()) {
        v.divident = i_divident.read();
        v.divisor = i_divisor.read();
        v.lshift_rdy = 0;
        v.overflow = 0;
        v.zero_resid = 0;
    } else if (r.delay.read()[0]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 0;
        vb_muxind(48, 42) = 0;
        vb_muxind(41, 35) = 0;
        vb_muxind(34, 28) = 0;
        vb_muxind(27, 21) = 0;
        vb_muxind(20, 14) = 0;
        vb_muxind(13, 7) = 0;
        vb_muxind(6, 0) = 0;
    } else if (r.delay.read()[1]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 1;
        vb_muxind(48, 42) = 2;
        vb_muxind(41, 35) = 3;
        vb_muxind(34, 28) = 4;
        vb_muxind(27, 21) = 5;
        vb_muxind(20, 14) = 6;
        vb_muxind(13, 7) = 7;
        vb_muxind(6, 0) = 8;
        vb_bits[104] = wb_dif_o.read()[0].to_bool();
    } else if (r.delay.read()[2]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 9;
        vb_muxind(48, 42) = 10;
        vb_muxind(41, 35) = 11;
        vb_muxind(34, 28) = 12;
        vb_muxind(27, 21) = 13;
        vb_muxind(20, 14) = 14;
        vb_muxind(13, 7) = 15;
        vb_muxind(6, 0) = 16;
        vb_bits(103, 96) = wb_dif_o.read();
    } else if (r.delay.read()[3]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 17;
        vb_muxind(48, 42) = 18;
        vb_muxind(41, 35) = 19;
        vb_muxind(34, 28) = 20;
        vb_muxind(27, 21) = 21;
        vb_muxind(20, 14) = 22;
        vb_muxind(13, 7) = 23;
        vb_muxind(6, 0) = 24;
        vb_bits(95, 88) = wb_dif_o.read();
    } else if (r.delay.read()[4]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 25;
        vb_muxind(48, 42) = 26;
        vb_muxind(41, 35) = 27;
        vb_muxind(34, 28) = 28;
        vb_muxind(27, 21) = 29;
        vb_muxind(20, 14) = 30;
        vb_muxind(13, 7) = 31;
        vb_muxind(6, 0) = 32;
        vb_bits(87, 80) = wb_dif_o.read();
    } else if (r.delay.read()[5]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 33;
        vb_muxind(48, 42) = 34;
        vb_muxind(41, 35) = 35;
        vb_muxind(34, 28) = 36;
        vb_muxind(27, 21) = 37;
        vb_muxind(20, 14) = 38;
        vb_muxind(13, 7) = 39;
        vb_muxind(6, 0) = 40;
        vb_bits(79, 72) = wb_dif_o.read();
    } else if (r.delay.read()[6]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 41;
        vb_muxind(48, 42) = 42;
        vb_muxind(41, 35) = 43;
        vb_muxind(34, 28) = 44;
        vb_muxind(27, 21) = 45;
        vb_muxind(20, 14) = 46;
        vb_muxind(13, 7) = 47;
        vb_muxind(6, 0) = 48;
        vb_bits(71, 64) = wb_dif_o.read();
    } else if (r.delay.read()[7]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 49;
        vb_muxind(48, 42) = 50;
        vb_muxind(41, 35) = 51;
        vb_muxind(34, 28) = 52;
        vb_muxind(27, 21) = 53;
        vb_muxind(20, 14) = 54;
        vb_muxind(13, 7) = 55;
        vb_muxind(6, 0) = 56;
        vb_bits(63, 56) = wb_dif_o.read();
    } else if (r.delay.read()[8]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 57;
        vb_muxind(48, 42) = 58;
        vb_muxind(41, 35) = 59;
        vb_muxind(34, 28) = 60;
        vb_muxind(27, 21) = 61;
        vb_muxind(20, 14) = 62;
        vb_muxind(13, 7) = 63;
        vb_muxind(6, 0) = 64;
        vb_bits(55, 48) = wb_dif_o.read();
    } else if (r.delay.read()[9]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 65;
        vb_muxind(48, 42) = 66;
        vb_muxind(41, 35) = 67;
        vb_muxind(34, 28) = 68;
        vb_muxind(27, 21) = 69;
        vb_muxind(20, 14) = 70;
        vb_muxind(13, 7) = 71;
        vb_muxind(6, 0) = 72;
        vb_bits(47, 40) = wb_dif_o.read();
    } else if (r.delay.read()[10]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 73;
        vb_muxind(48, 42) = 74;
        vb_muxind(41, 35) = 75;
        vb_muxind(34, 28) = 76;
        vb_muxind(27, 21) = 77;
        vb_muxind(20, 14) = 78;
        vb_muxind(13, 7) = 79;
        vb_muxind(6, 0) = 80;
        vb_bits(39, 32) = wb_dif_o.read();
    } else if (r.delay.read()[11]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 81;
        vb_muxind(48, 42) = 82;
        vb_muxind(41, 35) = 83;
        vb_muxind(34, 28) = 84;
        vb_muxind(27, 21) = 85;
        vb_muxind(20, 14) = 86;
        vb_muxind(13, 7) = 87;
        vb_muxind(6, 0) = 88;
        vb_bits(31, 24) = wb_dif_o.read();
    } else if (r.delay.read()[12]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 89;
        vb_muxind(48, 42) = 90;
        vb_muxind(41, 35) = 91;
        vb_muxind(34, 28) = 92;
        vb_muxind(27, 21) = 93;
        vb_muxind(20, 14) = 94;
        vb_muxind(13, 7) = 95;
        vb_muxind(6, 0) = 96;
        vb_bits(23, 16) = wb_dif_o.read();
    } else if (r.delay.read()[13]) {
        w_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(56, 49) = 97;
        vb_muxind(48, 42) = 98;
        vb_muxind(41, 35) = 99;
        vb_muxind(34, 28) = 100;
        vb_muxind(27, 21) = 101;
        vb_muxind(20, 14) = 102;
        vb_muxind(13, 7) = 103;
        vb_muxind(6, 0) = 104;
        vb_bits(15, 8) = wb_dif_o.read();
    } else if (r.delay.read()[14]) {
        vb_bits(7, 0) = wb_dif_o.read();
        if (wb_dif_o.read() == 0) {
            v.zero_resid = 1;
        }
        if (r.lshift.read() == 0x7F) {
            v.overflow = 1;
        }
    }

    if (r.lshift_rdy.read() == 0 && w_muxind_rdy_o == 1) {
        v.lshift_rdy = 1;
        v.lshift = wb_muxind_o.read();
    } else if (r.delay.read()[14] == 1) {
        v.lshift_rdy = 1;
        v.lshift = 104;
    }

    wb_divident_i = r.divident.read();
    wb_divisor_i = r.divisor.read();
    wb_muxind_i = vb_muxind;
    v.bits = vb_bits;

    if (!i_nrst.read()) {
        v.delay = 0;
        v.lshift = 0;
        v.lshift_rdy = 0;
        v.divisor = 0;
        v.divident = 0;
        v.bits = 0;
        v.overflow = 0;
        v.zero_resid = 0;
    }

    o_result = r.bits.read();
    o_lshift = r.lshift.read();
    o_overflow = r.overflow;
    o_zero_resid = r.zero_resid;
    o_rdy = r.delay.read()[15];
}

void idiv53::registers() {
    r = v;
}

}  // namespace debugger

