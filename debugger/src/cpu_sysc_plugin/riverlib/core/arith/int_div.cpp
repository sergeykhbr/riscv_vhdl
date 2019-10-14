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

#include "int_div.h"
#include "api_core.h"

namespace debugger {

IntDiv::IntDiv(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_unsigned("i_unsigned"),
    i_rv32("i_rv32"),
    i_residual("i"),
    i_a1("i_a1"),
    i_a2("i_a2"),
    o_res("o_res"),
    o_valid("o_valid"),
    o_busy("o_busy"),
    stage0("stage0"),
    stage1("stage1") {
    async_reset_ = async_reset;

    stage0.i_divident(r.divident_i);
    stage0.i_divisor(wb_divisor0_i);
    stage0.o_bits(wb_bits0_o);
    stage0.o_resid(wb_resid0_o);

    stage1.i_divident(wb_resid0_o);
    stage1.i_divisor(wb_divisor1_i);
    stage1.o_bits(wb_bits1_o);
    stage1.o_resid(wb_resid1_o);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_rv32;
    sensitive << i_residual;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.result;
    sensitive << r.ena;
    sensitive << r.busy;
    sensitive << wb_bits0_o;
    sensitive << wb_resid0_o;
    sensitive << wb_bits1_o;
    sensitive << wb_resid1_o;
    sensitive << r.divisor_i;
    sensitive << r.divident_i;
    sensitive << r.bits_i;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void IntDiv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_valid, o_valid.name());

        std::string pn(name());
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.invert, pn + ".r_invert");
        sc_trace(o_vcd, r.rv32, pn + ".r_rv32");
        sc_trace(o_vcd, r.resid, pn + ".r_resid");
        sc_trace(o_vcd, r.reference_div, pn + ".r_reference_div");
        sc_trace(o_vcd, r.divident_i, pn + ".r_divident_i");
        sc_trace(o_vcd, r.divisor_i, pn + ".r_divisor_i");
        sc_trace(o_vcd, r.bits_i, pn + ".r_bits_i");
    }
    stage0.generateVCD(i_vcd, o_vcd);
    stage1.generateVCD(i_vcd, o_vcd);
}

void IntDiv::comb() {
    bool w_invert64;
    bool w_invert32;
    sc_uint<64> wb_a1;
    sc_uint<64> wb_a2;

    v = r;

    w_invert64 = 0;
    w_invert32 = 0;

    if (i_rv32.read()) {
        wb_a1(63, 32) = 0;
        wb_a2(63, 32) = 0;
        if (i_unsigned.read() || i_a1.read()[31] == 0) {
            wb_a1(31, 0) = i_a1.read()(31, 0);
        } else {
            wb_a1(31, 0) = (~i_a1.read()(31, 0)) + 1;
        }
        if (i_unsigned.read() || i_a2.read()[31] == 0) {
            wb_a2(31, 0) = i_a2.read()(31, 0);
        } else {
            wb_a2(31, 0) = (~i_a2.read()(31, 0)) + 1;
        }
    } else {
        if (i_unsigned.read() || i_a1.read()[63] == 0) {
            wb_a1(63, 0) = i_a1.read();
        } else {
            wb_a1(63, 0) = (~i_a1.read()) + 1;
        }
        if (i_unsigned.read() || i_a2.read()[63] == 0) {
            wb_a2(63, 0) = i_a2.read();
        } else {
            wb_a2(63, 0) = (~i_a2.read()) + 1;
        }
    }

    v.ena = (r.ena.read() << 1) | (i_ena & !r.busy);

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.rv32 = i_rv32;
        v.resid = i_residual;

        v.divident_i = wb_a1;
        sc_biguint<120> t_divisor = wb_a2.to_uint64();
        v.divisor_i = t_divisor << 56;

        w_invert32 = !i_unsigned.read() && 
                ((!i_residual.read() && (i_a1.read()[31] ^ i_a2.read()[31]))
                || (i_residual.read() && i_a1.read()[31]));
        w_invert64 = !i_unsigned.read() &&
                ((!i_residual.read() && (i_a1.read()[63] ^ i_a2.read()[63]))
                || (i_residual.read() && i_a1.read()[63]));
        v.invert = (!i_rv32.read() && w_invert64) 
                || (i_rv32.read() && w_invert32);

        v.a1_dbg = i_a1;
        v.a2_dbg = i_a2;
        v.reference_div = compute_reference(i_unsigned.read(), i_rv32.read(),
                                     i_residual.read(),
                                     i_a1.read(), i_a2.read());
    } else if (r.ena.read()[8]) {
        v.busy = 0;
        if (r.resid.read()) {
            if (r.invert.read()) {
                v.result = ~r.divident_i.read().to_uint64() + 1;
            } else {
                v.result = r.divident_i.read();
            }
        } else {
            if (r.invert.read()) {
                v.result = ~r.bits_i.read().to_uint64() + 1;
            } else {
                v.result = r.bits_i.read();
            }
        }
    } else if (r.busy.read() == 1) {
        v.divident_i = wb_resid1_o;
        v.divisor_i = r.divisor_i.read() >> 8;
        v.bits_i = (r.bits_i.read() << 8) | (wb_bits0_o.read() << 4) | wb_bits1_o.read();
    }

    sc_uint<4> t_zero4;
    t_zero4 = 0;
    wb_divisor0_i = (r.divisor_i.read(), t_zero4);
    wb_divisor1_i = (t_zero4, r.divisor_i.read());

    o_res = r.result;
    o_valid = r.ena.read()[9];
    o_busy = r.busy;
}

void IntDiv::registers() {
    // Debug purpose only"
    if (v.ena.read()[9]) {
        uint64_t t1 = v.result.read()(63,0).to_uint64();
        uint64_t t2 = r.reference_div.to_uint64();
        if (t1 != t2) {
            char tstr[512];
            RISCV_sprintf(tstr, sizeof(tstr), 
                "IntDiv error: rv32=%d, resid=%d, invert=%d, "
                "(%016" RV_PRI64 "x/%016" RV_PRI64 "x) => "
                "%016" RV_PRI64 "x != %016" RV_PRI64 "x\n",
                r.rv32.read(), r.resid.read(), r.invert.read(),
                r.a1_dbg.to_uint64(), r.a2_dbg.to_uint64(), t1, t2);
            cout << tstr;
            cout.flush();
        }
    }

    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

uint64_t IntDiv::compute_reference(bool unsign, bool rv32, bool resid,
                                   uint64_t a1, uint64_t a2) {
    uint64_t ret;
    if (a2 == 0) {
        ret = 0;
    } else if (rv32) {
        if (unsign) {
            if (resid) {
                ret = (uint32_t)a1 % (uint32_t)a2;
            } else {
                ret = (uint32_t)a1 / (uint32_t)a2;
            }
        } else {
            if (resid) {
                ret = (uint64_t)((int64_t)((int32_t)a1 % (int32_t)a2));
            } else {
                ret = (uint64_t)((int64_t)((int32_t)a1 / (int32_t)a2));
            }
        }
    } else {
        if (unsign) {
            if (resid) {
                ret = a1 % a2;
            } else {
                ret = a1 / a2;
            }
        } else {
            if (resid) {
                ret = (int64_t)a1 % (int64_t)a2;
            } else {
                ret = (int64_t)a1 / (int64_t)a2;
            }
        }
    }
    return ret;
}

#ifdef DBG_IDIV_TB
IntDiv_tb::IntDiv_tb(sc_module_name name_) : sc_module(name_),
    i_clk("clk0", 10, SC_NS) {

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << r.clk_cnt;

    SC_METHOD(registers);
    sensitive << i_clk.posedge_event();

    tt = new IntDiv("tt", 0);
    tt->i_clk(i_clk);
    tt->i_nrst(i_nrst);
    tt->i_ena(i_ena);
    tt->i_unsigned(i_unsigned);
    tt->i_rv32(i_rv32);
    tt->i_residual(i_residual);
    tt->i_a1(i_a1);
    tt->i_a2(i_a2);
    tt->o_res(o_res);
    tt->o_valid(o_valid);
    tt->o_busy(o_busy);

    tb_vcd_i = sc_create_vcd_trace_file("i_intdiv");
    tb_vcd_i->set_time_unit(1, SC_PS);
    sc_trace(tb_vcd_i, i_nrst, "i_nrst");
    sc_trace(tb_vcd_i, i_clk, "i_clk");
    //sc_trace(tb_vcd_i, r.clk_cnt, "clk_cnt");
    sc_trace(tb_vcd_i, i_ena, "i_ena");
    sc_trace(tb_vcd_i, i_unsigned, "i_unsigned");
    sc_trace(tb_vcd_i, i_rv32, "i_rv32");
    sc_trace(tb_vcd_i, i_residual, "i_residual");
    sc_trace(tb_vcd_i, i_a1, "i_a1");
    sc_trace(tb_vcd_i, i_a2, "i_a2");

    tb_vcd_o = sc_create_vcd_trace_file("o_intdiv");
    tb_vcd_o->set_time_unit(1, SC_PS);
    sc_trace(tb_vcd_o, o_res, "o_res");
    sc_trace(tb_vcd_o, o_valid, "o_valid");
    sc_trace(tb_vcd_o, o_busy, "o_busy");

    tt->generateVCD(tb_vcd_i, tb_vcd_o);
}


void IntDiv_tb::comb() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    if (r.clk_cnt.read() < 10) {
        i_nrst = 0;
    } else {
        i_nrst = 1;

        i_unsigned = 0;
        i_rv32 = 0;
        i_residual = 0;
        i_ena = 0;

        uint64_t t_mod = r.clk_cnt.read().to_uint64() % 400;
        if (t_mod == 0) {
            i_a1 = 0;
            i_a2 = 0;
        } else if (t_mod == 39) {
            //i_a1 = 0x0000000000000666ull;
            //i_a2 = 0x000000000000000aull;
            i_a1 = rand() | (static_cast<uint64_t>(rand()) << 60);
            i_a2 = rand() | (static_cast<uint64_t>(rand()) << 60);
        } else if (t_mod == 100) {
            i_unsigned = 0;
            i_rv32 = 0;
            i_residual = 0;
            i_ena = 1;
        } else if (t_mod == 120) {
            i_unsigned = 1;
            i_rv32 = 0;
            i_residual = 0;
            i_ena = 1;
        } else if (t_mod == 140) {
            i_unsigned = 0;
            i_rv32 = 1;
            i_residual = 0;
            i_ena = 1;
        } else if (t_mod == 160) {
            i_unsigned = 1;
            i_rv32 = 1;
            i_residual = 0;
            i_ena = 1;
        } else if (t_mod == 180) {
            i_unsigned = 0;
            i_rv32 = 0;
            i_residual = 1;
            i_ena = 1;
        } else if (t_mod == 200) {
            i_unsigned = 1;
            i_rv32 = 0;
            i_residual = 1;
            i_ena = 1;
        } else if (t_mod == 220) {
            i_unsigned = 0;
            i_rv32 = 1;
            i_residual = 1;
            i_ena = 1;
        } else if (t_mod == 240) {
            i_unsigned = 1;
            i_rv32 = 1;
            i_residual = 1;
            i_ena = 1;
        }
    }
}
#endif

}  // namespace debugger

