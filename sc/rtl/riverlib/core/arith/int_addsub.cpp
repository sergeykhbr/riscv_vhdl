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

#include "int_addsub.h"
#include "api_core.h"

namespace debugger {

IntAddSub::IntAddSub(sc_module_name name,
                     bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mode("i_mode"),
    i_a1("i_a1"),
    i_a2("i_a2"),
    o_res("o_res") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mode;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.res;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void IntAddSub::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mode, i_mode.name());
        sc_trace(o_vcd, i_a1, i_a1.name());
        sc_trace(o_vcd, i_a2, i_a2.name());
        sc_trace(o_vcd, o_res, o_res.name());
        sc_trace(o_vcd, r.res, pn + ".r_res");
    }

}

void IntAddSub::comb() {
    sc_uint<RISCV_ARCH> vb_rdata1;
    sc_uint<RISCV_ARCH> vb_rdata2;
    sc_uint<RISCV_ARCH> vb_add;
    sc_uint<RISCV_ARCH> vb_sub;
    sc_uint<RISCV_ARCH> vb_res;

    vb_rdata1 = 0;
    vb_rdata2 = 0;
    vb_add = 0;
    vb_sub = 0;
    vb_res = 0;

    v = r;

    // To support 32-bits instruction transform 32-bits operands to 64 bits
    if (i_mode.read()[0] == 1) {
        vb_rdata1(31, 0) = i_a1.read()(31, 0);
        vb_rdata2(31, 0) = i_a2.read()(31, 0);
        if (vb_rdata1[31] == 1) {
            vb_rdata1(63, 32) = ~0ull;
        }
        if (vb_rdata2[31] == 1) {
            vb_rdata2(63, 32) = ~0ull;
        }
    } else {
        vb_rdata1 = i_a1;
        vb_rdata2 = i_a2;
    }

    vb_add = (vb_rdata1 + vb_rdata2);
    vb_sub = (vb_rdata1 - vb_rdata2);
    if (i_mode.read()[2] == 1) {
        vb_res = vb_add;
    } else if (i_mode.read()[3] == 1) {
        vb_res = vb_sub;
    } else if (i_mode.read()[4] == 1) {
        if (i_mode.read()[1] == 1) {
            // unsigned less
            if (vb_rdata1 < vb_rdata2) {
                vb_res[0] = 1;
            }
        } else {
            // signed less
            vb_res[0] = vb_sub[63];
        }
    } else if (i_mode.read()[5] == 1) {
        if (i_mode.read()[1] == 1) {
            // unsigned min
            if (vb_rdata1 < vb_rdata2) {
                vb_res = vb_rdata1;
            } else {
                vb_res = vb_rdata2;
            }
        } else {
            // signed min
            if (vb_sub[63] == 1) {
                vb_res = vb_rdata1;
            } else {
                vb_res = vb_rdata2;
            }
        }
    } else if (i_mode.read()[6] == 1) {
        if (i_mode.read()[1] == 1) {
            // unsigned max
            if (vb_rdata1 < vb_rdata2) {
                vb_res = vb_rdata2;
            } else {
                vb_res = vb_rdata1;
            }
        } else {
            // signed max
            if (vb_sub[63] == 1) {
                vb_res = vb_rdata2;
            } else {
                vb_res = vb_rdata1;
            }
        }
    }
    if (i_mode.read()[0] == 1) {
        if (vb_res[31] == 1) {
            vb_res(63, 32) = ~0ull;
        } else {
            vb_res(63, 32) = 0;
        }
    }

    v.res = vb_res;

    if (!async_reset_ && i_nrst.read() == 0) {
        IntAddSub_r_reset(v);
    }

    o_res = r.res;
}

void IntAddSub::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        IntAddSub_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

