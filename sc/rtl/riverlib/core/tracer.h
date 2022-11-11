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

    void comb();
    void registers();

    SC_HAS_PROCESS(Tracer);

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

    std::string TaskDisassembler(sc_uint<32> instr);
    std::string TraceOutput(sc_uint<TRACE_TBL_ABITS> rcnt);

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
    } v, r;

    std::string trfilename;                                 // formatted string name with hartid
    std::string outstr;
    std::string tracestr;
    FILE *fl;


};

}  // namespace debugger

