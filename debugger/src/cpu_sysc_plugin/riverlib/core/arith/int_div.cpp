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
#ifdef IDIV_V2
    stage0("stage0"),
    stage1("stage1"),
#endif
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
    o_busy("o_busy")  {
    async_reset_ = async_reset;

#ifdef IDIV_V2
    stage0.i_divident(r.divident_i);
    stage0.i_divisor(wb_divisor0_i);
    stage0.o_bits(wb_bits0_o);
    stage0.o_resid(wb_resid0_o);

    stage1.i_divident(wb_resid0_o);
    stage1.i_divisor(wb_divisor1_i);
    stage1.o_bits(wb_bits1_o);
    stage1.o_resid(wb_resid1_o);
#endif

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
#ifdef IDIV_V2
    sensitive << wb_bits0_o;
    sensitive << wb_resid0_o;
    sensitive << wb_bits1_o;
    sensitive << wb_resid1_o;
    sensitive << r.divisor_i;
    sensitive << r.divident_i;
#else
#endif

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void IntDiv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
#if 0
    sc_trace_file *t_vcd = sc_create_vcd_trace_file("intdiv");
    t_vcd->set_time_unit(1, SC_PS);
    sc_trace(t_vcd, i_clk, "i_clk");
    sc_trace(t_vcd, i_nrst, "i_nrst");
    sc_trace(t_vcd, i_ena, "i_ena");
    sc_trace(t_vcd, i_unsigned, "i_unsigned");
    sc_trace(t_vcd, i_rv32, "i_rv32");
    sc_trace(t_vcd, i_residual, "i_residual");
    sc_trace(t_vcd, i_a1, "i_a1");
    sc_trace(t_vcd, i_a2, "i_a2");
#endif
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
#ifdef IDIV_V2
        sc_trace(o_vcd, r.divident_i, pn + ".r_divident_i");
        sc_trace(o_vcd, r.divisor_i, pn + ".r_divisor_i");
        sc_trace(o_vcd, r.bits, pn + ".r_bits");
#else
        sc_trace(o_vcd, r.qr, pn + ".r_qr");
        sc_trace(o_vcd, wb_qr1, pn + ".wb_qr1");
        sc_trace(o_vcd, wb_qr2, pn + ".wb_qr2");
#endif
    }
#ifdef IDIV_V2
    stage0.generateVCD(i_vcd, o_vcd);
    stage1.generateVCD(i_vcd, o_vcd);
#endif
}

void IntDiv::comb() {
#ifdef IDIV_V2
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
        sc_biguint<128> t_divisor = wb_a2.to_uint64();
        v.divisor_i = t_divisor << 56;

        v.bits = 0;

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
                v.result = ~r.bits.read().to_uint64() + 1;
            } else {
                v.result = r.bits.read();
            }
        }
    } else if (r.busy.read() == 1) {
        v.divident_i = wb_resid1_o;
        v.divisor_i = r.divisor_i.read() >> 8;
        v.bits = (r.bits.read() << 8) | (wb_bits0_o.read() << 4) | wb_bits1_o.read();
    }

    wb_divisor0_i = r.divisor_i.read() << 4;
    wb_divisor1_i = r.divisor_i.read();

    o_res = r.result;
    o_valid = r.ena.read()[9];
    o_busy = r.busy;

#else
    sc_uint<64> wb_a1;
    sc_uint<64> wb_a2;
    sc_biguint<65> wb_divident = 0;
    sc_biguint<65> wb_divider = 0;
    bool w_invert64;
    bool w_invert32;
    v = r;

    w_invert32 = 0;
    w_invert64 = 0;
    wb_divident[64] = 0;
    wb_divider[64] = 0;

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

    wb_divident(63, 0) = wb_a1;
    wb_divider(63, 0) = wb_a2;


    v.ena = (r.ena.read() << 1) | (i_ena & !r.busy);

    // Level 2*i of 64:
    wb_diff1 = r.qr(127, 63) - r.divider;
    if (wb_diff1[64]) {
        wb_qr1 = r.qr << 1;
    } else {
        wb_qr1 = (wb_diff1(63, 0), r.qr(62, 0), 1);
    }

    // Level 2*i + 1 of 64:
    wb_diff2 = wb_qr1(127, 63) - r.divider;
    if (wb_diff2[64]) {
        wb_qr2 = wb_qr1 << 1;
    } else {
        wb_qr2 = (wb_diff2(63, 0), wb_qr1(62, 0), 1);
    }


    if (i_ena.read()) {
        v.qr(127, 65) = 0;
        v.qr(64, 0) = wb_divident;
        v.divider = wb_divider;
        v.busy = 1;
        v.rv32 = i_rv32;
        v.resid = i_residual;

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
    } else if (r.ena.read()[32]) {
        v.busy = 0;
        if (r.resid.read()) {
            if (r.invert.read()) {
                v.result = ~v.qr(127, 64).to_uint64() + 1;
            } else {
                v.result = v.qr(127, 64).to_uint64();
            }
        } else {
            if (r.invert.read()) {
                v.result = ~v.qr(63, 0).to_uint64() + 1;
            } else {
                v.result = v.qr(63, 0).to_uint64();
            }
        }
    } else if (r.busy.read()) {
        v.qr = wb_qr2;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_valid = r.ena.read()[33];
    o_busy = r.busy;
#endif
}

void IntDiv::registers() {
    // Debug purpose only"
#ifdef IDIV_V2
    if (v.ena.read()[9]) {
#else
    if (v.ena.read()[33]) {
#endif
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
    w_clk_i("clk0", 10, SC_NS) {

    SC_METHOD(comb);
    sensitive << w_nrst_i;
    sensitive << r.clk_cnt;

    SC_METHOD(registers);
    sensitive << w_clk_i.posedge_event();

    tt = new IntDiv("tt", 0);
    tt->i_clk(w_clk_i);
    tt->i_nrst(w_nrst_i);
    tt->i_ena(w_ena_i);
    tt->i_unsigned(w_unsigned_i);
    tt->i_rv32(w_rv32_i);
    tt->i_residual(w_residual_i);
    tt->i_a1(wb_a1_i);
    tt->i_a2(wb_a2_i);
    tt->o_res(wb_res_o);
    tt->o_valid(w_valid_o);
    tt->o_busy(w_busy_o);

    tb_vcd = sc_create_vcd_trace_file("IntDiv_tb");
    tb_vcd->set_time_unit(1, SC_PS);
    sc_trace(tb_vcd, w_nrst_i, "w_nrst_i");
    sc_trace(tb_vcd, w_clk_i, "w_clk_i");
    sc_trace(tb_vcd, r.clk_cnt, "clk_cnt");
    sc_trace(tb_vcd, w_ena_i, "w_ena_i");
    sc_trace(tb_vcd, w_unsigned_i, "w_unsigned_i");
    sc_trace(tb_vcd, w_rv32_i, "w_rv32_i");
    sc_trace(tb_vcd, w_residual_i, "w_residual_i");
    sc_trace(tb_vcd, wb_a1_i, "wb_a1_i");
    sc_trace(tb_vcd, wb_a2_i, "wb_a2_i");
    sc_trace(tb_vcd, wb_res_o, "wb_res_o");
    sc_trace(tb_vcd, w_valid_o, "w_valid_o");
    sc_trace(tb_vcd, w_busy_o, "w_busy_o");

    tt->generateVCD(tb_vcd, tb_vcd);
}


void IntDiv_tb::comb() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    if (r.clk_cnt.read() < 10) {
        w_nrst_i = 0;
    } else {
        w_nrst_i = 1;

        w_unsigned_i = 0;
        w_rv32_i = 0;
        w_residual_i = 0;
        w_ena_i = 0;
        //if ((r.clk_cnt.read().to_uint64() % 40) == 39) {
        if (r.clk_cnt.read().to_uint64() == 39) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 0;
            w_rv32_i = 0;
            w_residual_i = 0;
        } else if (r.clk_cnt.read().to_uint64() == 100) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 1;
            w_rv32_i = 0;
            w_residual_i = 0;
        } else if (r.clk_cnt.read().to_uint64() == 200) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 0;
            w_rv32_i = 1;
            w_residual_i = 0;
        } else if (r.clk_cnt.read().to_uint64() == 300) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 1;
            w_rv32_i = 1;
            w_residual_i = 0;
        } else if (r.clk_cnt.read().to_uint64() == 400) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 0;
            w_rv32_i = 0;
            w_residual_i = 1;
        } else if (r.clk_cnt.read().to_uint64() == 500) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 1;
            w_rv32_i = 0;
            w_residual_i = 1;
        } else if (r.clk_cnt.read().to_uint64() == 600) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 0;
            w_rv32_i = 1;
            w_residual_i = 1;
        } else if (r.clk_cnt.read().to_uint64() == 700) {
            w_ena_i = 1;
            //w_unsigned_i = rand() & 1;
            //w_rv32_i = rand() & 1;
            //w_residual_i = rand() & 1;
            wb_a1_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a2_i = rand() | (static_cast<uint64_t>(rand()) << 60);
            wb_a1_i = 0xe000000000003456ull;
            wb_a2_i = 0x0400000000003456ull;
            w_unsigned_i = 1;
            w_rv32_i = 1;
            w_residual_i = 1;
        }
    }
}
#endif

}  // namespace debugger

