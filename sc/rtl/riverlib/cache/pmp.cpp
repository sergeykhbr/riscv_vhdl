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

#include "pmp.h"
#include "api_core.h"

namespace debugger {

PMP::PMP(sc_module_name name,
         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_ena("i_ena"),
    i_iaddr("i_iaddr"),
    i_daddr("i_daddr"),
    i_we("i_we"),
    i_region("i_region"),
    i_start_addr("i_start_addr"),
    i_end_addr("i_end_addr"),
    i_flags("i_flags"),
    o_r("o_r"),
    o_w("o_w"),
    o_x("o_x") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_iaddr;
    sensitive << i_daddr;
    sensitive << i_we;
    sensitive << i_region;
    sensitive << i_start_addr;
    sensitive << i_end_addr;
    sensitive << i_flags;
    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
        sensitive << r.tbl[i].start_addr;
        sensitive << r.tbl[i].end_addr;
        sensitive << r.tbl[i].flags;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void PMP::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_iaddr, i_iaddr.name());
        sc_trace(o_vcd, i_daddr, i_daddr.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_region, i_region.name());
        sc_trace(o_vcd, i_start_addr, i_start_addr.name());
        sc_trace(o_vcd, i_end_addr, i_end_addr.name());
        sc_trace(o_vcd, i_flags, i_flags.name());
        sc_trace(o_vcd, o_r, o_r.name());
        sc_trace(o_vcd, o_w, o_w.name());
        sc_trace(o_vcd, o_x, o_x.name());
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            char tstr[1024];
        }
    }

}

void PMP::comb() {
    bool v_r;
    bool v_w;
    bool v_x;
    sc_uint<RISCV_ARCH> vb_start_addr;
    sc_uint<RISCV_ARCH> vb_end_addr;
    sc_uint<CFG_PMP_FL_TOTAL> vb_flags;

    v_r = 0;
    v_w = 0;
    v_x = 0;
    vb_start_addr = 0;
    vb_end_addr = 0;
    vb_flags = 0;

    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
        v.tbl[i].start_addr = r.tbl[i].start_addr;
        v.tbl[i].end_addr = r.tbl[i].end_addr;
        v.tbl[i].flags = r.tbl[i].flags;
    }

    // PMP is active for S,U modes or in M-mode when L-bit is set:
    v_r = (!i_ena);
    v_w = (!i_ena);
    v_x = (!i_ena);

    vb_flags = i_flags;
    if (i_flags.read()[CFG_PMP_FL_V] == 1) {
        vb_start_addr = i_start_addr;
        vb_end_addr = i_end_addr;
    } else {
        vb_start_addr = 0;
        vb_end_addr = 0;
    }

    for (int i = (CFG_PMP_TBL_SIZE - 1); i >= 0; i--) {
        if ((i_iaddr.read() >= r.tbl[i].start_addr.read()((CFG_CPU_ADDR_BITS - 1), 0))
                && (i_iaddr.read() <= r.tbl[i].end_addr.read()((CFG_CPU_ADDR_BITS - 1), 0))) {
            if ((r.tbl[i].flags.read()[CFG_PMP_FL_V] == 1)
                    && (i_ena || r.tbl[i].flags.read()[CFG_PMP_FL_L])) {
                v_x = r.tbl[i].flags.read()[CFG_PMP_FL_X];
            }
        }

        if ((i_daddr.read() >= r.tbl[i].start_addr.read()((CFG_CPU_ADDR_BITS - 1), 0))
                && (i_daddr.read() <= r.tbl[i].end_addr.read()((CFG_CPU_ADDR_BITS - 1), 0))) {
            if ((r.tbl[i].flags.read()[CFG_PMP_FL_V] == 1)
                    && (i_ena || r.tbl[i].flags.read()[CFG_PMP_FL_L])) {
                v_r = r.tbl[i].flags.read()[CFG_PMP_FL_R];
                v_w = r.tbl[i].flags.read()[CFG_PMP_FL_W];
            }
        }
    }

    if (i_we.read() == 1) {
        v.tbl[i_region.read().to_int()].start_addr = vb_start_addr;
        v.tbl[i_region.read().to_int()].end_addr = vb_end_addr;
        v.tbl[i_region.read().to_int()].flags = vb_flags;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            v.tbl[i].start_addr = 0ull;
            v.tbl[i].end_addr = 0ull;
            v.tbl[i].flags = 0;
        }
    }

    o_r = v_r;
    o_w = v_w;
    o_x = v_x;
}

void PMP::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            r.tbl[i].start_addr = 0ull;
            r.tbl[i].end_addr = 0ull;
            r.tbl[i].flags = 0;
        }
    } else {
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            r.tbl[i].start_addr = v.tbl[i].start_addr;
            r.tbl[i].end_addr = v.tbl[i].end_addr;
            r.tbl[i].flags = v.tbl[i].flags;
        }
    }
}

}  // namespace debugger

