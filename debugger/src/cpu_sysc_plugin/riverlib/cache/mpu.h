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

#ifndef __DEBUGGER_RIVERLIB_CACHE_MPU_H__
#define __DEBUGGER_RIVERLIB_CACHE_MPU_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(MPU) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_iaddr;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_daddr;
    sc_in<bool> i_region_we;
    sc_in<sc_uint<CFG_MPU_TBL_WIDTH>> i_region_idx;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_region_addr;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_region_mask;
    sc_in<sc_uint<CFG_MPU_FL_TOTAL>> i_region_flags;  // {ena, cachable, r, w, x}
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_iflags;
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_dflags;

    void comb();
    void registers();

    SC_HAS_PROCESS(MPU);

    MPU(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct MpuTableItemType {
        sc_uint<CFG_CPU_ADDR_BITS> addr;
        sc_uint<CFG_CPU_ADDR_BITS> mask;
        sc_uint<CFG_MPU_FL_TOTAL> flags;
    };

    MpuTableItemType tbl[CFG_MPU_TBL_SIZE];
    MpuTableItemType v_tbl[CFG_MPU_TBL_SIZE];

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_MPU_H__
