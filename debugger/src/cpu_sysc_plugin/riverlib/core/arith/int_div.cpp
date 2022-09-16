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

#include "int_div.h"
#include "api_core.h"

namespace debugger {

IntDiv::IntDiv(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_unsigned("i_unsigned"),
    i_rv32("i_rv32"),
    i_residual("i_residual"),
    i_a1("i_a1"),
    i_a2("i_a2"),
    o_res("o_res"),
    o_valid("o_valid") {

    async_reset_ = async_reset;
    stage0 = 0;
    stage1 = 0;

    stage0 = new divstage64("stage0");
    stage0->i_divident(r.divident_i);
    stage0->i_divisor(wb_divisor0_i);
    stage0->o_resid(wb_resid0_o);
    stage0->o_bits(wb_bits0_o);


    stage1 = new divstage64("stage1");
    stage1->i_divident(wb_resid0_o);
    stage1->i_divisor(wb_divisor1_i);
    stage1->o_resid(wb_resid1_o);
    stage1->o_bits(wb_bits1_o);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_rv32;
    sensitive << i_residual;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << wb_divisor0_i;
    sensitive << wb_divisor1_i;
    sensitive << wb_resid0_o;
    sensitive << wb_resid1_o;
    sensitive << wb_bits0_o;
    sensitive << wb_bits1_o;
    sensitive << r.rv32;
    sensitive << r.resid;
    sensitive << r.invert;
    sensitive << r.div_on_zero;
    sensitive << r.overflow;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.divident_i;
    sensitive << r.divisor_i;
    sensitive << r.bits_i;
    sensitive << r.result;
    sensitive << r.reference_div;
    sensitive << r.a1_dbg;
    sensitive << r.a2_dbg;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

IntDiv::~IntDiv() {
    if (stage0) {
        delete stage0;
    }
    if (stage1) {
        delete stage1;
    }
}

void IntDiv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_unsigned, i_unsigned.name());
        sc_trace(o_vcd, i_rv32, i_rv32.name());
        sc_trace(o_vcd, i_residual, i_residual.name());
        sc_trace(o_vcd, i_a1, i_a1.name());
        sc_trace(o_vcd, i_a2, i_a2.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, r.rv32, pn + ".r_rv32");
        sc_trace(o_vcd, r.resid, pn + ".r_resid");
        sc_trace(o_vcd, r.invert, pn + ".r_invert");
        sc_trace(o_vcd, r.div_on_zero, pn + ".r_div_on_zero");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
        sc_trace(o_vcd, r.busy, pn + ".r_busy");
        sc_trace(o_vcd, r.ena, pn + ".r_ena");
        sc_trace(o_vcd, r.divident_i, pn + ".r_divident_i");
        sc_trace(o_vcd, r.divisor_i, pn + ".r_divisor_i");
        sc_trace(o_vcd, r.bits_i, pn + ".r_bits_i");
        sc_trace(o_vcd, r.result, pn + ".r_result");
        sc_trace(o_vcd, r.reference_div, pn + ".r_reference_div");
        sc_trace(o_vcd, r.a1_dbg, pn + ".r_a1_dbg");
        sc_trace(o_vcd, r.a2_dbg, pn + ".r_a2_dbg");
    }

    if (stage0) {
        stage0->generateVCD(i_vcd, o_vcd);
    }
    if (stage1) {
        stage1->generateVCD(i_vcd, o_vcd);
    }
}

void IntDiv::comb() {
    bool v_invert64;
    bool v_invert32;
    sc_uint<64> vb_a1;
    sc_uint<64> vb_a2;
    sc_uint<64> vb_rem;
    sc_uint<64> vb_div;
    bool v_a1_m0;                                           // a1 == -0ll
    bool v_a2_m1;                                           // a2 == -1ll
    sc_uint<1> v_ena;                                       // 1
    sc_biguint<120> t_divisor;

    v_invert64 = 0;
    v_invert32 = 0;
    vb_a1 = 0;
    vb_a2 = 0;
    vb_rem = 0;
    vb_div = 0;
    v_a1_m0 = 0;
    v_a2_m1 = 0;
    v_ena = 0;
    t_divisor = 0;

    v = r;

    if (i_rv32.read() == 1) {
        if ((i_unsigned.read() == 1) || (i_a1.read()[31] == 0)) {
            vb_a1(31, 0) = i_a1.read()(31, 0);
        } else {
            vb_a1(31, 0) = ((~i_a1.read()(31, 0)) + 1);
        }
        if ((i_unsigned.read() == 1) || (i_a2.read()[31] == 0)) {
            vb_a2(31, 0) = i_a2.read()(31, 0);
        } else {
            vb_a2(31, 0) = ((~i_a2.read()(31, 0)) + 1);
        }
    } else {
        if ((i_unsigned.read() == 1) || (i_a1.read()[63] == 0)) {
            vb_a1(63, 0) = i_a1;
        } else {
            vb_a1(63, 0) = ((~i_a1.read()) + 1);
        }
        if ((i_unsigned.read() == 1) || (i_a2.read()[63] == 0)) {
            vb_a2(63, 0) = i_a2;
        } else {
            vb_a2(63, 0) = ((~i_a2.read()) + 1);
        }
    }

    if ((vb_a1[63] == 1) && (vb_a1(62, 0).or_reduce() == 0)) {
        v_a1_m0 = 1;                                        // = (1ull << 63)
    }
    if (vb_a2.and_reduce() == 1) {
        v_a2_m1 = 1;                                        // = -1ll
    }

    v_ena = (i_ena && (!r.busy));
    v.ena = (r.ena.read()(8, 0), v_ena);

    if (r.invert.read() == 1) {
        vb_rem = ((~r.divident_i.read().to_uint64()) + 1);
    } else {
        vb_rem = r.divident_i;
    }

    if (r.invert.read() == 1) {
        vb_div = ((~r.bits_i.read().to_uint64()) + 1);
    } else {
        vb_div = r.bits_i;
    }

    // DIVW, DIVUW, REMW and REMUW sign-extended accordingly with
    // User Level ISA v2.2
    if (r.rv32.read() == 1) {
        vb_div(63, 32) = 0;
        vb_rem(63, 32) = 0;
        if (vb_div[31] == 1) {
            vb_div(63, 32) = ~0ull;
        }
        if (vb_rem[31] == 1) {
            vb_rem(63, 32) = ~0ull;
        }
    }

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.rv32 = i_rv32;
        v.resid = i_residual;

        v.divident_i = vb_a1;
        t_divisor(119, 56) = vb_a2;
        v.divisor_i = t_divisor;
        v_invert32 = ((!i_unsigned)
                && (((!i_residual) && (i_a1.read()[31] ^ i_a2.read()[31]))
                        || (i_residual && i_a1.read()[31])));
        v_invert64 = ((!i_unsigned)
                && (((!i_residual) && (i_a1.read()[63] ^ i_a2.read()[63]))
                        || (i_residual && i_a1.read()[63])));
        v.invert = (((!i_rv32) && v_invert64)
                || (i_rv32 && v_invert32));

        if (i_rv32.read() == 1) {
            if (i_unsigned.read() == 1) {
                v.div_on_zero = 0;
                if (i_a2.read()(31, 0).or_reduce() == 0) {
                    v.div_on_zero = 1;
                }
                v.overflow = 0;
            } else {
                v.div_on_zero = 0;
                if (i_a2.read()(30, 0).or_reduce() == 0) {
                    v.div_on_zero = 1;
                }
                v.overflow = (v_a1_m0 && v_a2_m1);
            }
        } else {
            if (i_unsigned.read() == 1) {
                v.div_on_zero = 0;
                if (i_a2.read()(63, 0).or_reduce() == 0) {
                    v.div_on_zero = 1;
                }
                v.overflow = 0;
            } else {
                v.div_on_zero = 0;
                if (i_a2.read()(62, 0).or_reduce() == 0) {
                    v.div_on_zero = 1;
                }
                v.overflow = (v_a1_m0 && v_a2_m1);
            }
        }
        v.a1_dbg = i_a1;
        v.a2_dbg = i_a2;
        // v.reference_div = compute_reference(i_unsigned.read(), i_rv32.read(),
        //                              i_residual.read(),
        //                              i_a1.read(), i_a2.read());
    } else if (r.ena.read()[8] == 1) {
        v.busy = 0;
        if (r.resid.read() == 1) {
            if (r.overflow.read() == 1) {
                v.result = 0;
            } else {
                v.result = vb_rem;
            }
        } else if (r.div_on_zero.read() == 1) {
            v.result = ~0ull;
        } else if (r.overflow.read() == 1) {
            v.result = 0x8000000000000000ull;
        } else {
            v.result = vb_div;
        }
    } else if (r.busy.read() == 1) {
        v.divident_i = wb_resid1_o;
        v.divisor_i = (0, r.divisor_i.read()(119, 8));
        v.bits_i = (r.bits_i, wb_bits0_o, wb_bits1_o);
    }
    wb_divisor0_i = (r.divisor_i.read() << 4);
    wb_divisor1_i = (0, r.divisor_i.read());

    if (!async_reset_ && i_nrst.read() == 0) {
        IntDiv_r_reset(v);
    }

    o_res = r.result;
    o_valid = r.ena.read()[9];
}

void IntDiv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        IntDiv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

