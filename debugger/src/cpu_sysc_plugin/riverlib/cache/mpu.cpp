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
    o_icachable("o_icachable"),
    o_iexecutable("o_iexecutable"),
    o_dcachable("o_dcachable"),
    o_dreadable("o_dreadable"),
    o_dwritable("o_dwritable") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_iaddr;
    sensitive << i_daddr;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void MPU::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_iaddr, i_iaddr.name());
        sc_trace(o_vcd, i_daddr, i_daddr.name());
        sc_trace(o_vcd, i_region_we, i_region_we.name());
        sc_trace(o_vcd, i_region_idx, i_region_idx.name());
        sc_trace(o_vcd, o_icachable, o_icachable.name());
        sc_trace(o_vcd, o_iexecutable, o_iexecutable.name());
    }
}

void MPU::comb() {
    bool v_icachable;
    bool v_dcachable;
    bool v_executable;
    bool v_readable;
    bool v_writable;
    uint64_t t_iaddr;
    uint64_t t_daddr;

    v_icachable = true;
    v_dcachable = true;
    v_executable = true;
    v_readable = true;
    v_writable = true;

    for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
        t_iaddr = i_iaddr.read()(BUS_ADDR_WIDTH-1, CFG_IOFFSET_WIDTH);
        if (tbl[i].addr == (t_iaddr & tbl[i].mask)) {
            if (tbl[i].ena == 1)  {
                v_icachable = tbl[i].cache;
                v_executable = tbl[i].exec;
            }
        }

        t_daddr = i_daddr.read()(BUS_ADDR_WIDTH-1, CFG_IOFFSET_WIDTH);
        if (tbl[i].addr == (t_daddr & tbl[i].mask)) {
            if (tbl[i].ena == 1)  {
                v_dcachable = tbl[i].cache;
                v_writable = tbl[i].wr;
                v_readable = tbl[i].rd;
            }
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            tbl[i].ena = 0;
            tbl[i].addr = ~0ull;
            tbl[i].mask = ~0ull;
        }

        // Make first 128 Byte uncachable to test MPU
        tbl[0].ena = 1;
        tbl[0].addr = 0 >> CFG_IOFFSET_WIDTH;
        tbl[0].mask = 0xFFFFFFFFFFFFFF80ull >> CFG_IOFFSET_WIDTH;
        tbl[0].cache = 0;
        tbl[0].exec = 1;
        tbl[0].rd = 1;
        tbl[0].wr = 1;

        // All address above 0x80000000 are uncached (IO devices)
        tbl[0].ena = 1;
        tbl[0].addr = 0x0000000080000000ull >> CFG_IOFFSET_WIDTH;
        tbl[0].mask = 0xFFFFFFFF80000000ull >> CFG_IOFFSET_WIDTH;
        tbl[0].cache = 0;
        tbl[0].exec = 1;
        tbl[0].rd = 1;
        tbl[0].wr = 1;
    }

    o_icachable = v_icachable;
    o_iexecutable = v_executable;
    o_dcachable = v_dcachable;
    o_dreadable = v_readable;
    o_dwritable= v_writable;
}

void MPU::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            tbl[i].ena = 0;
            tbl[i].addr = ~0ull;
            tbl[i].mask = ~0ull;
        }
    } else {
        if (i_region_we.read() == 1) {
            unsigned tidx = i_region_idx.read();
            if (i_region_flags.read()[CFG_MPU_FL_ENA] == 1) {
                tbl[tidx].addr = i_region_addr.read() >> CFG_IOFFSET_WIDTH;
                tbl[tidx].mask = i_region_mask.read() >> CFG_IOFFSET_WIDTH;
                tbl[tidx].ena = 1;
            } else {
                tbl[tidx].addr = ~0ull;
                tbl[tidx].mask = ~0ull;
                tbl[tidx].ena = 0;
            }
            tbl[tidx].cache = i_region_flags.read()[CFG_MPU_FL_CACHABLE];
            tbl[tidx].exec = i_region_flags.read()[CFG_MPU_FL_EXEC];
            tbl[tidx].rd = i_region_flags.read()[CFG_MPU_FL_RD];
            tbl[tidx].wr = i_region_flags.read()[CFG_MPU_FL_WR];
        }
    }
}

}  // namespace debugger

