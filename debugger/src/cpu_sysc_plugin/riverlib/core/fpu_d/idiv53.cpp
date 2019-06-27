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

idiv53::idiv53(sc_module_name name_, bool async_reset) : sc_module(name_),
    divstage0("divstage") {
    async_reset_ = async_reset;

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

    divstage0.i_mux_ena(w_mux_ena_i);
    divstage0.i_muxind(wb_muxind_i);
    divstage0.i_divident(wb_divident_i);
    divstage0.i_divisor(wb_divisor_i);
    divstage0.o_dif(wb_dif_o);
    divstage0.o_bits(wb_bits_o);
    divstage0.o_muxind(wb_muxind_o);
    divstage0.o_muxind_rdy(w_muxind_rdy_o);
};

void idiv53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_clk, "/top/proc0/exec0/fpu0/fdiv/idiv53/i_clk");
        sc_trace(o_vcd, i_nrst, "/top/proc0/exec0/fpu0/fdiv/idiv53/i_nrst");
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fpu0/fdiv/idiv53/i_ena");
        sc_trace(o_vcd, i_divisor, "/top/proc0/exec0/fpu0/fdiv/idiv53/i_divisor");
        sc_trace(o_vcd, i_divident, "/top/proc0/exec0/fpu0/fdiv/idiv53/i_divident");

        sc_trace(o_vcd, o_result, "/top/proc0/exec0/fpu0/fdiv/idiv53/o_result");
        sc_trace(o_vcd, o_rdy, "/top/proc0/exec0/fpu0/fdiv/idiv53/o_rdy");
        sc_trace(o_vcd, r.delay, "/top/proc0/exec0/fpu0/fdiv/idiv53/r_delay");
        sc_trace(o_vcd, r.divident, "/top/proc0/exec0/fpu0/fdiv/idiv53/r_divident");
        sc_trace(o_vcd, r.divisor, "/top/proc0/exec0/fpu0/fdiv/idiv53/r_divisor");
        sc_trace(o_vcd, r.lshift, "/top/proc0/exec0/fpu0/fdiv/idiv53/r_lshift");
        sc_trace(o_vcd, r.zero_resid, "/top/proc0/exec0/fpu0/fdiv/idiv53/r_zero_resid");
        sc_trace(o_vcd, wb_dif_o, "/top/proc0/exec0/fpu0/fdiv/idiv53/wb_dif_o");
        sc_trace(o_vcd, wb_bits_o, "/top/proc0/exec0/fpu0/fdiv/idiv53/wb_bits_o");
        sc_trace(o_vcd, w_mux_ena_i, "/top/proc0/exec0/fpu0/fdiv/idiv53/w_mux_ena_i");
        sc_trace(o_vcd, wb_muxind_i, "/top/proc0/exec0/fpu0/fdiv/idiv53/wb_muxind_i");
        sc_trace(o_vcd, wb_muxind_o, "/top/proc0/exec0/fpu0/fdiv/idiv53/wb_muxind_o");
    }
    divstage0.generateVCD(i_vcd, o_vcd);

}

void idiv53::comb() {
    sc_uint<56> vb_muxind;
    sc_bv<105> vb_bits;
    bool v_mux_ena_i;
    v = r;
    vb_bits = r.bits;

    v_mux_ena_i = 0;
    v.delay = (r.delay.read()(13, 0), i_ena.read());
    if (i_ena.read()) {
        v.divident = (0, i_divident.read());
        v.divisor = i_divisor.read();
        v.lshift_rdy = 0;
        v.overflow = 0;
        v.zero_resid = 0;
    } else if (r.delay.read()[0]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 0;
        vb_muxind(48, 42) = 0;
        vb_muxind(41, 35) = 0;
        vb_muxind(34, 28) = 0;
        vb_muxind(27, 21) = 0;
        vb_muxind(20, 14) = 0;
        vb_muxind(13, 7) = 0;
        vb_muxind(6, 0) = 0;
        vb_bits[104] = !wb_dif_o.read()[52];
    } else if (r.delay.read()[1]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 1;
        vb_muxind(48, 42) = 2;
        vb_muxind(41, 35) = 3;
        vb_muxind(34, 28) = 4;
        vb_muxind(27, 21) = 5;
        vb_muxind(20, 14) = 6;
        vb_muxind(13, 7) = 7;
        vb_muxind(6, 0) = 8;
        vb_bits(103, 96) = wb_bits_o.read();
    } else if (r.delay.read()[2]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 9;
        vb_muxind(48, 42) = 10;
        vb_muxind(41, 35) = 11;
        vb_muxind(34, 28) = 12;
        vb_muxind(27, 21) = 13;
        vb_muxind(20, 14) = 14;
        vb_muxind(13, 7) = 15;
        vb_muxind(6, 0) = 16;
        vb_bits(95, 88) = wb_bits_o.read();
    } else if (r.delay.read()[3]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 17;
        vb_muxind(48, 42) = 18;
        vb_muxind(41, 35) = 19;
        vb_muxind(34, 28) = 20;
        vb_muxind(27, 21) = 21;
        vb_muxind(20, 14) = 22;
        vb_muxind(13, 7) = 23;
        vb_muxind(6, 0) = 24;
        vb_bits(87, 80) = wb_bits_o.read();
    } else if (r.delay.read()[4]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 25;
        vb_muxind(48, 42) = 26;
        vb_muxind(41, 35) = 27;
        vb_muxind(34, 28) = 28;
        vb_muxind(27, 21) = 29;
        vb_muxind(20, 14) = 30;
        vb_muxind(13, 7) = 31;
        vb_muxind(6, 0) = 32;
        vb_bits(79, 72) = wb_bits_o.read();
    } else if (r.delay.read()[5]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 33;
        vb_muxind(48, 42) = 34;
        vb_muxind(41, 35) = 35;
        vb_muxind(34, 28) = 36;
        vb_muxind(27, 21) = 37;
        vb_muxind(20, 14) = 38;
        vb_muxind(13, 7) = 39;
        vb_muxind(6, 0) = 40;
        vb_bits(71, 64) = wb_bits_o.read();
    } else if (r.delay.read()[6]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 41;
        vb_muxind(48, 42) = 42;
        vb_muxind(41, 35) = 43;
        vb_muxind(34, 28) = 44;
        vb_muxind(27, 21) = 45;
        vb_muxind(20, 14) = 46;
        vb_muxind(13, 7) = 47;
        vb_muxind(6, 0) = 48;
        vb_bits(63, 56) = wb_bits_o.read();
    } else if (r.delay.read()[7]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 49;
        vb_muxind(48, 42) = 50;
        vb_muxind(41, 35) = 51;
        vb_muxind(34, 28) = 52;
        vb_muxind(27, 21) = 53;
        vb_muxind(20, 14) = 54;
        vb_muxind(13, 7) = 55;
        vb_muxind(6, 0) = 56;
        vb_bits(55, 48) = wb_bits_o.read();
    } else if (r.delay.read()[8]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 57;
        vb_muxind(48, 42) = 58;
        vb_muxind(41, 35) = 59;
        vb_muxind(34, 28) = 60;
        vb_muxind(27, 21) = 61;
        vb_muxind(20, 14) = 62;
        vb_muxind(13, 7) = 63;
        vb_muxind(6, 0) = 64;
        vb_bits(47, 40) = wb_bits_o.read();
    } else if (r.delay.read()[9]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 65;
        vb_muxind(48, 42) = 66;
        vb_muxind(41, 35) = 67;
        vb_muxind(34, 28) = 68;
        vb_muxind(27, 21) = 69;
        vb_muxind(20, 14) = 70;
        vb_muxind(13, 7) = 71;
        vb_muxind(6, 0) = 72;
        vb_bits(39, 32) = wb_bits_o.read();
    } else if (r.delay.read()[10]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 73;
        vb_muxind(48, 42) = 74;
        vb_muxind(41, 35) = 75;
        vb_muxind(34, 28) = 76;
        vb_muxind(27, 21) = 77;
        vb_muxind(20, 14) = 78;
        vb_muxind(13, 7) = 79;
        vb_muxind(6, 0) = 80;
        vb_bits(31, 24) = wb_bits_o.read();
    } else if (r.delay.read()[11]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 81;
        vb_muxind(48, 42) = 82;
        vb_muxind(41, 35) = 83;
        vb_muxind(34, 28) = 84;
        vb_muxind(27, 21) = 85;
        vb_muxind(20, 14) = 86;
        vb_muxind(13, 7) = 87;
        vb_muxind(6, 0) = 88;
        vb_bits(23, 16) = wb_bits_o.read();
    } else if (r.delay.read()[12]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 89;
        vb_muxind(48, 42) = 90;
        vb_muxind(41, 35) = 91;
        vb_muxind(34, 28) = 92;
        vb_muxind(27, 21) = 93;
        vb_muxind(20, 14) = 94;
        vb_muxind(13, 7) = 95;
        vb_muxind(6, 0) = 96;
        vb_bits(15, 8) = wb_bits_o.read();
    } else if (r.delay.read()[13]) {
        v_mux_ena_i = !r.lshift_rdy.read();
        v.divident = wb_dif_o.read() << 8;
        vb_muxind(55, 49) = 97;
        vb_muxind(48, 42) = 98;
        vb_muxind(41, 35) = 99;
        vb_muxind(34, 28) = 100;
        vb_muxind(27, 21) = 101;
        vb_muxind(20, 14) = 102;
        vb_muxind(13, 7) = 103;
        vb_muxind(6, 0) = 104;
        vb_bits(7, 0) = wb_bits_o.read();

        if (wb_dif_o.read() == 0) {
            v.zero_resid = 1;
        }
        if (r.lshift.read() == 0x7F) {
            v.overflow = 1;
        }
    }

    if (r.lshift_rdy.read() == 0) {
        if(w_muxind_rdy_o == 1) {
            v.lshift_rdy = 1;
            v.lshift = wb_muxind_o.read();
        } else if (r.delay.read()[13] == 1) {
            v.lshift_rdy = 1;
            v.lshift = 104;
        }
    }

    w_mux_ena_i = v_mux_ena_i;
    wb_divident_i = r.divident.read();
    wb_divisor_i = r.divisor.read();
    wb_muxind_i = vb_muxind;
    v.bits = vb_bits;

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_result = r.bits.read();
    o_lshift = r.lshift.read();
    o_overflow = r.overflow;
    o_zero_resid = r.zero_resid;
    o_rdy = r.delay.read()[14];
}

void idiv53::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

