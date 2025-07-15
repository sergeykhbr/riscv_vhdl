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

#include "int_mul.h"
#include "api_core.h"

namespace debugger {

IntMul::IntMul(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_unsigned("i_unsigned"),
    i_hsu("i_hsu"),
    i_high("i_high"),
    i_rv32("i_rv32"),
    i_a1("i_a1"),
    i_a2("i_a2"),
    o_res("o_res"),
    o_valid("o_valid") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_hsu;
    sensitive << i_high;
    sensitive << i_rv32;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.a1;
    sensitive << r.a2;
    sensitive << r.unsign;
    sensitive << r.high;
    sensitive << r.rv32;
    sensitive << r.zero;
    sensitive << r.inv;
    sensitive << r.result;
    sensitive << r.a1_dbg;
    sensitive << r.a2_dbg;
    sensitive << r.reference_mul;
    for (int i = 0; i < 16; i++) {
        sensitive << r.lvl1[i];
    }
    for (int i = 0; i < 4; i++) {
        sensitive << r.lvl3[i];
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void IntMul::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_unsigned, i_unsigned.name());
        sc_trace(o_vcd, i_hsu, i_hsu.name());
        sc_trace(o_vcd, i_high, i_high.name());
        sc_trace(o_vcd, i_rv32, i_rv32.name());
        sc_trace(o_vcd, i_a1, i_a1.name());
        sc_trace(o_vcd, i_a2, i_a2.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, r.busy, pn + ".r.busy");
        sc_trace(o_vcd, r.ena, pn + ".r.ena");
        sc_trace(o_vcd, r.a1, pn + ".r.a1");
        sc_trace(o_vcd, r.a2, pn + ".r.a2");
        sc_trace(o_vcd, r.unsign, pn + ".r.unsign");
        sc_trace(o_vcd, r.high, pn + ".r.high");
        sc_trace(o_vcd, r.rv32, pn + ".r.rv32");
        sc_trace(o_vcd, r.zero, pn + ".r.zero");
        sc_trace(o_vcd, r.inv, pn + ".r.inv");
        sc_trace(o_vcd, r.result, pn + ".r.result");
        sc_trace(o_vcd, r.a1_dbg, pn + ".r.a1_dbg");
        sc_trace(o_vcd, r.a2_dbg, pn + ".r.a2_dbg");
        sc_trace(o_vcd, r.reference_mul, pn + ".r.reference_mul");
        for (int i = 0; i < 16; i++) {
            sc_trace(o_vcd, r.lvl1[i], pn + ".r.lvl1(" + std::to_string(i) + ")");
        }
        for (int i = 0; i < 4; i++) {
            sc_trace(o_vcd, r.lvl3[i], pn + ".r.lvl3(" + std::to_string(i) + ")");
        }
    }

}

void IntMul::comb() {
    sc_uint<RISCV_ARCH> vb_a1;
    sc_uint<RISCV_ARCH> vb_a2;
    sc_uint<2> wb_mux_lvl0;
    sc_biguint<66> wb_lvl0[32];
    sc_biguint<74> wb_lvl2[8];
    sc_biguint<100> wb_lvl4[2];
    sc_biguint<128> wb_lvl5;
    sc_biguint<128> wb_res32;
    sc_uint<64> wb_res;
    sc_uint<64> vb_a1s;
    sc_uint<64> vb_a2s;
    bool v_a1s_nzero;
    bool v_a2s_nzero;
    sc_uint<1> v_ena;

    v.busy = r.busy.read();
    v.ena = r.ena.read();
    v.a1 = r.a1.read();
    v.a2 = r.a2.read();
    v.unsign = r.unsign.read();
    v.high = r.high.read();
    v.rv32 = r.rv32.read();
    v.zero = r.zero.read();
    v.inv = r.inv.read();
    v.result = r.result.read();
    v.a1_dbg = r.a1_dbg.read();
    v.a2_dbg = r.a2_dbg.read();
    v.reference_mul = r.reference_mul.read();
    for (int i = 0; i < 16; i++) {
        v.lvl1[i] = r.lvl1[i].read();
    }
    for (int i = 0; i < 4; i++) {
        v.lvl3[i] = r.lvl3[i].read();
    }
    vb_a1 = 0;
    vb_a2 = 0;
    wb_mux_lvl0 = 0;
    for (int i = 0; i < 32; i++) {
        wb_lvl0[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        wb_lvl2[i] = 0;
    }
    for (int i = 0; i < 2; i++) {
        wb_lvl4[i] = 0;
    }
    wb_lvl5 = 0;
    wb_res32 = 0;
    wb_res = 0;
    vb_a1s = 0;
    vb_a2s = 0;
    v_a1s_nzero = 0;
    v_a2s_nzero = 0;
    v_ena = 0;


    if (i_a1.read()(62, 0).or_reduce() == 1) {
        v_a1s_nzero = 1;
    }
    if ((v_a1s_nzero && i_a1.read()[63]) == 1) {
        vb_a1s = ((~i_a1.read()) + 1);
    } else {
        vb_a1s = i_a1.read();
    }

    if (i_a2.read()(62, 0).or_reduce() == 1) {
        v_a2s_nzero = 1;
    }
    if ((v_a2s_nzero && i_a2.read()[63]) == 1) {
        vb_a2s = ((~i_a2.read()) + 1);
    } else {
        vb_a2s = i_a2.read();
    }

    v_ena = (i_ena.read() && (!r.busy.read()));
    v.ena = (r.ena.read()(2, 0), v_ena);

    if (i_ena.read() == 1) {
        v.busy = 1;
        v.inv = 0;
        v.zero = 0;
        if (i_rv32.read() == 1) {
            vb_a1 = i_a1.read()(31, 0);
            if ((i_unsigned.read() == 0) && (i_a1.read()[31] == 1)) {
                vb_a1(63, 32) = ~0ull;
            }
            vb_a2 = i_a2.read()(31, 0);
            if ((i_unsigned.read() == 0) && (i_a2.read()[31] == 1)) {
                vb_a2(63, 32) = ~0ull;
            }
        } else if (i_high.read() == 1) {
            if (i_hsu.read() == 1) {
                if ((v_a1s_nzero == 0) || (i_a2.read().or_reduce() == 0)) {
                    v.zero = 1;
                }
                v.inv = i_a1.read()[63];
                vb_a1 = vb_a1s;
                vb_a2 = i_a2.read();
            } else if (i_unsigned.read() == 1) {
                vb_a1 = i_a1.read();
                vb_a2 = i_a2.read();
            } else {
                v.zero = ((!v_a1s_nzero) || (!v_a2s_nzero));
                v.inv = (i_a1.read()[63] ^ i_a2.read()[63]);
                vb_a1 = vb_a1s;
                vb_a2 = vb_a2s;
            }
        } else {
            vb_a1 = i_a1.read();
            vb_a2 = i_a2.read();
        }
        v.a1 = vb_a1;
        v.a2 = vb_a2;
        v.rv32 = i_rv32.read();
        v.unsign = i_unsigned.read();
        v.high = i_high.read();

        // Just for run-rime control (not for VHDL)
        v.a1_dbg = i_a1.read();
        v.a2_dbg = i_a2.read();
    }

    if (r.ena.read()[0] == 1) {
        for (int i = 0; i < 32; i++) {
            wb_mux_lvl0 = r.a2.read()((2 * i) + 2 - 1, (2 * i));
            if (wb_mux_lvl0 == 0) {
                wb_lvl0[i] = 0;
            } else if (wb_mux_lvl0 == 1) {
                wb_lvl0[i] = (0, sc_biguint<66>(r.a1.read()));
            } else if (wb_mux_lvl0 == 2) {
                wb_lvl0[i] = (sc_biguint<66>(r.a1.read()) << 1);
            } else {
                wb_lvl0[i] = ((0, sc_biguint<66>(r.a1.read()))
                        + (sc_biguint<66>(r.a1.read()) << 1));
            }
        }
        for (int i = 0; i < 16; i++) {
            v.lvl1[i] = ((sc_biguint<69>(wb_lvl0[((2 * i) + 1)]) << 2)
                    + (0, sc_biguint<69>(wb_lvl0[(2 * i)])));
        }
    }

    if (r.ena.read()[1] == 1) {
        for (int i = 0; i < 8; i++) {
            wb_lvl2[i] = ((sc_biguint<74>(r.lvl1[((2 * i) + 1)].read()) << 4)
                    + sc_biguint<74>(r.lvl1[(2 * i)].read()));
        }
        for (int i = 0; i < 4; i++) {
            v.lvl3[i] = ((sc_biguint<83>(wb_lvl2[((2 * i) + 1)]) << 8)
                    + sc_biguint<83>(wb_lvl2[(2 * i)]));
        }
    }

    if (r.ena.read()[2] == 1) {
        v.busy = 0;
        for (int i = 0; i < 2; i++) {
            wb_lvl4[i] = ((sc_biguint<100>(r.lvl3[((2 * i) + 1)].read()) << 16)
                    + sc_biguint<100>(r.lvl3[(2 * i)].read()));
        }
        wb_lvl5 = ((sc_biguint<128>(wb_lvl4[1]) << 32)
                + sc_biguint<128>(wb_lvl4[0]));
        if (r.rv32.read() == 1) {
            wb_res32(31, 0) = wb_lvl5(31, 0);
            if ((r.unsign.read() == 1) || (wb_lvl5[31] == 0)) {
                wb_res32(127, 32) = 0;
            } else {
                wb_res32(127, 32) = ~0ull;
            }
            v.result = wb_res32;
        } else if (r.high.read() == 1) {
            if (r.zero.read() == 1) {
                v.result = 0;
            } else if (r.inv.read() == 1) {
                v.result = (~wb_lvl5);
            } else {
                v.result = wb_lvl5;
            }
        } else {
            v.result = wb_lvl5;
        }
    }

    wb_res = r.result.read()(63, 0);
    if (r.high.read() == 1) {
        wb_res = r.result.read()(127, 64);
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        IntMul_r_reset(v);
    }

    o_res = wb_res;
    o_valid = r.ena.read()[3];
}

void IntMul::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        IntMul_r_reset(r);
    } else {
        r.busy = v.busy.read();
        r.ena = v.ena.read();
        r.a1 = v.a1.read();
        r.a2 = v.a2.read();
        r.unsign = v.unsign.read();
        r.high = v.high.read();
        r.rv32 = v.rv32.read();
        r.zero = v.zero.read();
        r.inv = v.inv.read();
        r.result = v.result.read();
        r.a1_dbg = v.a1_dbg.read();
        r.a2_dbg = v.a2_dbg.read();
        r.reference_mul = v.reference_mul.read();
        for (int i = 0; i < 16; i++) {
            r.lvl1[i] = v.lvl1[i].read();
        }
        for (int i = 0; i < 4; i++) {
            r.lvl3[i] = v.lvl3[i].read();
        }
    }
}

}  // namespace debugger

