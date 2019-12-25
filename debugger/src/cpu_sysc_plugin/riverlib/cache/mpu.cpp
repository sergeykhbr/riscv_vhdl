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

#include "mpu.h"
#include <api_core.h>

namespace debugger {

MPU::MPU(sc_module_name name_, bool async_reset) : sc_module(name_) ,
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

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void MPU::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_iaddr, i_iaddr.name());
        sc_trace(o_vcd, i_daddr, i_daddr.name());
        sc_trace(o_vcd, i_region_we, i_region_we.name());
        sc_trace(o_vcd, i_region_idx, i_region_idx.name());
        sc_trace(o_vcd, o_iflags, o_iflags.name());
        sc_trace(o_vcd, o_dflags, o_dflags.name());
    }
}

void MPU::comb() {
    MpuTableItemType v_item;
    sc_uint<CFG_MPU_FL_TOTAL> v_iflags;
    sc_uint<CFG_MPU_FL_TOTAL> v_dflags;

    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        v_tbl[i] = tbl[i];
    }
    v_iflags = ~0ul;
    v_dflags = ~0ul;

    v_item.flags = i_region_flags.read();
    if (i_region_flags.read()[CFG_MPU_FL_ENA] == 1) {
        v_item.addr = i_region_addr.read();
        v_item.mask = i_region_mask.read();
    } else {
        v_item.addr = ~0ull;
        v_item.mask = ~0ull;
    }

    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        if (tbl[i].addr == (i_iaddr.read() & tbl[i].mask)) {
            if (tbl[i].flags[CFG_MPU_FL_ENA] == 1)  {
                v_iflags = tbl[i].flags;
            }
        }

        if (tbl[i].addr == (i_daddr.read() & tbl[i].mask)) {
            if (tbl[i].flags[CFG_MPU_FL_ENA] == 1)  {
                v_dflags = tbl[i].flags;
            }
        }
    }

    if (i_region_we.read() == 1) {
        v_tbl[i_region_idx.read().to_int()] = v_item;
    }


    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            v_tbl[i].flags = 0;
            v_tbl[i].addr = 0;
            v_tbl[i].mask = ~0ull;
        }

        // All address above 0x80000000 are uncached (IO devices)
        v_tbl[0].addr(31, 0) = 0x80000000ull;
        v_tbl[0].mask(31, 0) = 0x80000000ull;
        v_tbl[0].flags[CFG_MPU_FL_ENA] = 1;
        v_tbl[0].flags[CFG_MPU_FL_CACHABLE] = 0;
        v_tbl[0].flags[CFG_MPU_FL_EXEC] = 1;
        v_tbl[0].flags[CFG_MPU_FL_RD] = 1;
        v_tbl[0].flags[CFG_MPU_FL_WR] = 1;

#if 1
        // Debug: Make first 128 Byte uncachable to test MPU
        v_tbl[1].addr(31, 0) = 0x00000000ull;
        v_tbl[1].mask(31, 0) = 0xFFFFFF80ull;
        v_tbl[1].flags[CFG_MPU_FL_ENA] = 1;
        v_tbl[1].flags[CFG_MPU_FL_CACHABLE] = 0;
        v_tbl[1].flags[CFG_MPU_FL_EXEC] = 1;
        v_tbl[1].flags[CFG_MPU_FL_RD] = 1;
        v_tbl[1].flags[CFG_MPU_FL_WR] = 1;
#endif
    }

    o_iflags = v_iflags;
    o_dflags = v_dflags;
}

void MPU::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 1; i < CFG_MPU_TBL_SIZE; i++) {
            tbl[i].flags = 0;
            tbl[i].addr = 0;
            tbl[i].mask = ~0ull;
        }

        // All address above 0x80000000 are uncached (IO devices)
        tbl[0].addr(31, 0) = 0x80000000ull;
        tbl[0].mask(31, 0) = 0x80000000ull;
        tbl[0].flags[CFG_MPU_FL_ENA] = 1;
        tbl[0].flags[CFG_MPU_FL_CACHABLE] = 0;
        tbl[0].flags[CFG_MPU_FL_EXEC] = 1;
        tbl[0].flags[CFG_MPU_FL_RD] = 1;
        tbl[0].flags[CFG_MPU_FL_WR] = 1;
    } else {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            tbl[i] = v_tbl[i];
        }
    }
}

}  // namespace debugger

