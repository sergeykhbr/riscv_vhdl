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

#include "mpu.h"
#include "api_core.h"

namespace debugger {

MPU::MPU(sc_module_name name,
         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_iaddr("i_iaddr"),
    i_daddr("i_daddr"),
    i_region_we("i_region_we"),
    i_region_idx("i_region_idx"),
    i_region_addr("i_region_addr"),
    i_region_mask("i_region_mask"),
    i_region_flags("i_region_flags"),
    o_iflags("o_iflags"),
    o_dflags("o_dflags") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_iaddr;
    sensitive << i_daddr;
    sensitive << i_region_we;
    sensitive << i_region_idx;
    sensitive << i_region_addr;
    sensitive << i_region_mask;
    sensitive << i_region_flags;
    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        sensitive << r.tbl[i].addr;
        sensitive << r.tbl[i].mask;
        sensitive << r.tbl[i].flags;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void MPU::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_iaddr, i_iaddr.name());
        sc_trace(o_vcd, i_daddr, i_daddr.name());
        sc_trace(o_vcd, i_region_we, i_region_we.name());
        sc_trace(o_vcd, i_region_idx, i_region_idx.name());
        sc_trace(o_vcd, i_region_addr, i_region_addr.name());
        sc_trace(o_vcd, i_region_mask, i_region_mask.name());
        sc_trace(o_vcd, i_region_flags, i_region_flags.name());
        sc_trace(o_vcd, o_iflags, o_iflags.name());
        sc_trace(o_vcd, o_dflags, o_dflags.name());
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_tbl%d_flags", pn.c_str(), i);
            sc_trace(o_vcd, r.tbl[i].flags, tstr);
        }
    }

}

void MPU::comb() {
    sc_uint<CFG_MPU_FL_TOTAL> v_iflags;
    sc_uint<CFG_MPU_FL_TOTAL> v_dflags;
    sc_uint<CFG_CPU_ADDR_BITS> vb_addr;
    sc_uint<CFG_CPU_ADDR_BITS> vb_mask;
    sc_uint<CFG_MPU_FL_TOTAL> vb_flags;

    v_iflags = 0;
    v_dflags = 0;
    vb_addr = 0;
    vb_mask = 0;
    vb_flags = 0;

    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        v.tbl[i].addr = r.tbl[i].addr;
        v.tbl[i].mask = r.tbl[i].mask;
        v.tbl[i].flags = r.tbl[i].flags;
    }


    vb_flags = i_region_flags;
    if (i_region_flags.read()[CFG_MPU_FL_ENA] == 1) {
        vb_addr = i_region_addr;
        vb_mask = i_region_mask;
    } else {
        vb_addr = ~0ull;
        vb_mask = ~0ull;
    }

    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        if (r.tbl[i].addr.read() == (i_iaddr.read() & r.tbl[i].mask.read())) {
            if (r.tbl[i].flags.read()[CFG_MPU_FL_ENA] == 1) {
                v_iflags = r.tbl[i].flags;
            }
        }

        if (r.tbl[i].addr.read() == (i_daddr.read() & r.tbl[i].mask.read())) {
            if (r.tbl[i].flags.read()[CFG_MPU_FL_ENA] == 1) {
                v_dflags = r.tbl[i].flags;
            }
        }
    }

    if (i_region_we.read() == 1) {
        v.tbl[i_region_idx.read().to_int()].addr = vb_addr;
        v.tbl[i_region_idx.read().to_int()].mask = vb_mask;
        v.tbl[i_region_idx.read().to_int()].flags = vb_flags;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            v.tbl[i].addr = 0ull;
            v.tbl[i].mask = 0ull;
            v.tbl[i].flags = ~0ul;
        }
    }

    o_iflags = v_iflags;
    o_dflags = v_dflags;
}

void MPU::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            r.tbl[i].addr = 0ull;
            r.tbl[i].mask = 0ull;
            r.tbl[i].flags = ~0ul;
        }
    } else {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            r.tbl[i].addr = v.tbl[i].addr;
            r.tbl[i].mask = v.tbl[i].mask;
            r.tbl[i].flags = v.tbl[i].flags;
        }
    }
}

}  // namespace debugger

