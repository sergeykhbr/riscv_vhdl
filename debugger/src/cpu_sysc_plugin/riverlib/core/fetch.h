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

#ifndef __DEBUGGER_RIVERLIB_FETCH_H__
#define __DEBUGGER_RIVERLIB_FETCH_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrFetch) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_bp_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_bp_pc;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_requested_pc;
    sc_in<bool> i_mem_req_ready;
    sc_out<bool> o_mem_addr_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mem_addr;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mem_data_addr;
    sc_in<sc_uint<32>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    sc_in<bool> i_mem_executable;
    sc_out<bool> o_mem_resp_ready;

    sc_in<bool> i_flush_pipeline;                   // reset pipeline and cache
    sc_in<bool> i_progbuf_ena;                      // executing from prog buffer
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_progbuf_pc; // progbuf counter
    sc_in<sc_uint<32>> i_progbuf_instr;             // progbuf instruction

    sc_out<bool> o_instr_load_fault;
    sc_out<bool> o_instr_executable;
    sc_out<bool> o_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_pc;
    sc_out<sc_uint<32>> o_instr;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrFetch);

    InstrFetch(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const uint8_t Idle = 0;
    static const uint8_t WaitReqAccept = 1;
    static const uint8_t WaitResp = 2;

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> req_valid;
        sc_signal<bool> resp_ready;
        sc_signal<bool> buf_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> requested_pc; // need for the branch predictor
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> valid;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
        sc_signal<bool> progbuf_ena;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = Idle;
        iv.req_valid = 0;
        iv.resp_ready = 0;
        iv.buf_valid = 0;
        iv.req_addr = 0;
        iv.requested_pc = 0;
        iv.pc = ~0ull;
        iv.instr = 0;
        iv.valid = 0;
        iv.instr_load_fault = 0;
        iv.instr_executable = 0;
        iv.progbuf_ena = 0;
    }

    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_FETCH_H__
