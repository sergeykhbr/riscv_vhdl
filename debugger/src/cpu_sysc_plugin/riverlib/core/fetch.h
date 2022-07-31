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
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrFetch) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_bp_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_bp_pc;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_requested_pc;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_fetching_pc;
    sc_in<bool> i_mem_req_ready;
    sc_out<bool> o_mem_addr_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mem_addr;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mem_data_addr;
    sc_in<sc_uint<64>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    sc_in<bool> i_mem_executable;
    sc_out<bool> o_mem_resp_ready;
    sc_in<bool> i_flush_pipeline;                           // reset pipeline and cache
    sc_in<bool> i_progbuf_ena;                              // executing from prog buffer
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_progbuf_pc;         // progbuf counter
    sc_in<sc_uint<64>> i_progbuf_instr;                     // progbuf instruction
    sc_out<bool> o_instr_load_fault;
    sc_out<bool> o_instr_executable;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_pc;
    sc_out<sc_uint<64>> o_instr;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrFetch);

    InstrFetch(sc_module_name name,
               bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint32_t Idle = 0;
    static const uint32_t WaitReqAccept = 1;
    static const uint32_t WaitResp = 2;

    struct InstrFetch_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> req_valid;
        sc_signal<bool> resp_ready;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mem_resp_shadow;// the same as memory response but internal
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<64>> instr;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
        sc_signal<bool> progbuf_ena;
    } v, r;

    void InstrFetch_r_reset(InstrFetch_registers &iv) {
        iv.state = Idle;
        iv.req_valid = 0;
        iv.resp_ready = 0;
        iv.req_addr = ~0ull;
        iv.mem_resp_shadow = ~0ull;
        iv.pc = ~0ull;
        iv.instr = 0ull;
        iv.instr_load_fault = 0;
        iv.instr_executable = 0;
        iv.progbuf_ena = 0;
    }

};

}  // namespace debugger

