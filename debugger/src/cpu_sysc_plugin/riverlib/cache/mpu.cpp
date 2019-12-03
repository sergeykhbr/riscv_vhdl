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
        sc_trace(o_vcd, o_icachable, o_icachable.name());
        sc_trace(o_vcd, o_iexecutable, o_iexecutable.name());
    }
}

void MPU::comb() {
    bool v_iena;
    bool v_dena;
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

    v_iena = 0;
    v_dena = 0;
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
        }
    }
}

void MPU::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_MPU_TBL_SIZE; i++) {
            tbl[i].ena = 0;
        }
    }
     //else {
    //    r = v;
    //}
}

}  // namespace debugger

