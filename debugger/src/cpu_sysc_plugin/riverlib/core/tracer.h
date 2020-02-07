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
 *
 * @brief      Stack trace buffer on hardware level.
 */

#ifndef __DEBUGGER_RIVERLIB_TRACER_H__
#define __DEBUGGER_RIVERLIB_TRACER_H__

#include <systemc.h>
#include <string>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(Tracer) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<64>> i_dbg_executed_cnt;
    sc_in<bool> i_e_valid;
    sc_in<sc_uint<CFG_RIVER_ADDR_BITS>> i_e_pc;
    sc_in<sc_uint<32>> i_e_instr;
    sc_in<bool> i_e_multi_ready;
    sc_in<bool> i_e_wena;
    sc_in<bool> i_e_whazard;
    sc_in<sc_uint<6>> i_e_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_e_wdata;
    sc_in<bool> i_e_memop_store;
    sc_in<bool> i_e_memop_load;
    sc_in<sc_uint<CFG_RIVER_ADDR_BITS>> i_e_memop_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_e_memop_wdata;
    sc_in<bool> i_m_wena;
    sc_in<sc_uint<6>> i_m_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_m_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(Tracer);

    Tracer(sc_module_name name_, bool async_reset, const char *trace_file);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    void task_disassembler(uint32_t instr);

    struct TraceStepType {
        bool entry_valid;
        bool whazard;
        uint64_t exec_cnt;
        uint64_t pc;
        uint32_t instr;
        uint32_t waddr;
        uint64_t wres;
        bool memop_load;
        bool memop_store;
        uint64_t memop_addr;
    };

    static const int TRACE_TBL_SZ = 64;
    TraceStepType trace_tbl_[TRACE_TBL_SZ];
    int tr_wcnt_;
    int tr_rcnt_;
    int tr_total_;

    FILE *fl_;
    char disasm[1024];

    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_TRACER_H__
