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
#pragma once

#include <systemc.h>
#include <string>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(Tracer) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<64>> i_dbg_executed_cnt;
    sc_in<bool> i_e_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_e_pc;
    sc_in<sc_uint<32>> i_e_instr;
    sc_in<bool> i_e_wena;
    sc_in<sc_uint<6>> i_e_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_e_wdata;
    sc_in<bool> i_e_memop_valid;
    sc_in<sc_uint<MemopType_Total>> i_e_memop_type;
    sc_in<sc_uint<2>> i_e_memop_size;
    sc_in<sc_uint<RISCV_ARCH>> i_e_memop_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_e_memop_wdata;
    sc_in<bool> i_e_flushd;
    sc_in<sc_uint<RISCV_ARCH>> i_m_pc;                      // executed memory/flush request only
    sc_in<bool> i_m_valid;                                  // memory/flush operation completed
    sc_in<bool> i_m_memop_ready;
    sc_in<bool> i_m_wena;
    sc_in<sc_uint<6>> i_m_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_m_wdata;
    sc_in<bool> i_reg_ignored;

    void init();
    void comb();
    void traceout();
    void registers();

    Tracer(sc_module_name name,
           bool async_reset,
           uint32_t hartid,
           std::string trace_file);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t hartid_;
    std::string trace_file_;

    static const int TRACE_TBL_ABITS = 6;
    static const int TRACE_TBL_SZ = 64;

    struct MemopActionType {
        sc_signal<bool> store;                              // 0=load;1=store
        sc_signal<sc_uint<2>> size;
        sc_signal<sc_uint<64>> mask;
        sc_signal<sc_uint<64>> memaddr;
        sc_signal<sc_uint<64>> data;
        sc_signal<sc_uint<6>> regaddr;                      // writeback address
        sc_signal<bool> complete;
        sc_signal<bool> sc_release;
        sc_signal<bool> ignored;
    };

    struct RegActionType {
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<64>> wres;
    };

    struct TraceStepType {
        sc_signal<sc_uint<64>> exec_cnt;
        sc_signal<sc_uint<64>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<32>> regactioncnt;
        sc_signal<sc_uint<32>> memactioncnt;
        RegActionType regaction[TRACE_TBL_SZ];
        MemopActionType memaction[TRACE_TBL_SZ];
        sc_signal<bool> completed;
    };

    struct Tracer_registers {
        TraceStepType trace_tbl[TRACE_TBL_SZ];
        sc_signal<sc_uint<TRACE_TBL_ABITS>> tr_wcnt;
        sc_signal<sc_uint<TRACE_TBL_ABITS>> tr_rcnt;
        sc_signal<sc_uint<TRACE_TBL_ABITS>> tr_total;
        sc_signal<sc_uint<TRACE_TBL_ABITS>> tr_opened;
    };

    void Tracer_r_reset(Tracer_registers& iv) {
        for (int i = 0; i < TRACE_TBL_SZ; i++) {
            iv.trace_tbl[i].exec_cnt = 0;
            iv.trace_tbl[i].pc = 0;
            iv.trace_tbl[i].instr = 0;
            iv.trace_tbl[i].regactioncnt = 0;
            iv.trace_tbl[i].memactioncnt = 0;
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                iv.trace_tbl[i].regaction[j].waddr = 0;
                iv.trace_tbl[i].regaction[j].wres = 0;
            }
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                iv.trace_tbl[i].memaction[j].store = 0;
                iv.trace_tbl[i].memaction[j].size = 0;
                iv.trace_tbl[i].memaction[j].mask = 0;
                iv.trace_tbl[i].memaction[j].memaddr = 0;
                iv.trace_tbl[i].memaction[j].data = 0;
                iv.trace_tbl[i].memaction[j].regaddr = 0;
                iv.trace_tbl[i].memaction[j].complete = 0;
                iv.trace_tbl[i].memaction[j].sc_release = 0;
                iv.trace_tbl[i].memaction[j].ignored = 0;
            }
            iv.trace_tbl[i].completed = 0;
        }
        iv.tr_wcnt = 0;
        iv.tr_rcnt = 0;
        iv.tr_total = 0;
        iv.tr_opened = 0;
    }

    std::string TaskDisassembler(sc_uint<32> instr);
    std::string TraceOutput(sc_uint<TRACE_TBL_ABITS> rcnt);

    std::string trfilename;                                 // formatted string name with hartid
    std::string outstr;
    std::string tracestr;
    FILE* fl;
    Tracer_registers v;
    Tracer_registers r;

};

}  // namespace debugger

