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
#include "stacktrbuf.h"

namespace debugger {

SC_MODULE(DbgPort) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // "RIVER" Debug interface
    sc_in<bool> i_dport_req_valid;                          // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_type;            // Debug access type
    sc_in<sc_uint<RISCV_ARCH>> i_dport_addr;                // Debug address (register or memory)
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;               // Write value
    sc_in<sc_uint<3>> i_dport_size;                         // reg/mem access size:0=1B;...,4=128B;
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                         // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                        // Response is valid
    sc_out<bool> o_dport_resp_error;                        // Something wrong during command execution
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;              // Response value
    // CSR bus master interface:
    sc_out<bool> o_csr_req_valid;                           // Region 0: Access to CSR bank is enabled.
    sc_in<bool> i_csr_req_ready;
    sc_out<sc_uint<CsrReq_TotalBits>> o_csr_req_type;       // Region 0: CSR operation read/modify/write
    sc_out<sc_uint<12>> o_csr_req_addr;                     // Address of the sub-region register
    sc_out<sc_uint<RISCV_ARCH>> o_csr_req_data;             // Write data
    sc_in<bool> i_csr_resp_valid;
    sc_out<bool> o_csr_resp_ready;
    sc_in<sc_uint<RISCV_ARCH>> i_csr_resp_data;             // Region 0: CSR read value
    sc_in<bool> i_csr_resp_exception;                       // Exception on CSR access
    sc_in<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> i_progbuf;// progam buffer
    sc_out<bool> o_progbuf_ena;                             // Execution from the progbuffer is in progress
    sc_out<sc_uint<RISCV_ARCH>> o_progbuf_pc;               // prog buffer instruction counter
    sc_out<sc_uint<64>> o_progbuf_instr;                    // prog buffer instruction opcode
    sc_in<bool> i_csr_progbuf_end;                          // End of execution from progbuf
    sc_in<bool> i_csr_progbuf_error;                        // Exception is occured during progbuf execution
    sc_out<sc_uint<6>> o_ireg_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_ireg_wdata;               // Write data
    sc_out<bool> o_ireg_ena;                                // Region 1: Access to integer register bank is enabled
    sc_out<bool> o_ireg_write;                              // Region 1: Integer registers bank write pulse
    sc_in<sc_uint<RISCV_ARCH>> i_ireg_rdata;                // Region 1: Integer register read value
    sc_out<bool> o_mem_req_valid;                           // Type 2: request is valid
    sc_in<bool> i_mem_req_ready;                            // Type 2: memory request was accepted
    sc_in<bool> i_mem_req_error;                            // Type 2: memory request is invalid and cannot be processed
    sc_out<bool> o_mem_req_write;                           // Type 2: is write
    sc_out<sc_uint<RISCV_ARCH>> o_mem_req_addr;             // Type 2: Debug memory request
    sc_out<sc_uint<2>> o_mem_req_size;                      // Type 2: memory operation size: 0=1B; 1=2B; 2=4B; 3=8B
    sc_out<sc_uint<RISCV_ARCH>> o_mem_req_wdata;            // Type 2: memory write data
    sc_in<bool> i_mem_resp_valid;                           // Type 2: response is valid
    sc_in<bool> i_mem_resp_error;                           // Type 2: response error (MPU or unmapped access)
    sc_in<sc_uint<RISCV_ARCH>> i_mem_resp_rdata;            // Type 2: Memory response from memaccess module
    sc_in<sc_uint<RISCV_ARCH>> i_e_pc;                      // Instruction pointer
    sc_in<sc_uint<RISCV_ARCH>> i_e_npc;                     // Next Instruction pointer
    sc_in<bool> i_e_call;                                   // pseudo-instruction CALL
    sc_in<bool> i_e_ret;                                    // pseudo-instruction RET
    sc_in<bool> i_e_memop_valid;                            // Memory request from executor
    sc_in<bool> i_m_valid;                                  // Memory request processed

    void comb();
    void registers();

    SC_HAS_PROCESS(DbgPort);

    DbgPort(sc_module_name name,
            bool async_reset);
    virtual ~DbgPort();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t idle = 0;
    static const uint8_t csr_region = 1;
    static const uint8_t reg_bank = 2;
    static const uint8_t reg_stktr_cnt = 3;
    static const uint8_t reg_stktr_buf_adr = 4;
    static const uint8_t reg_stktr_buf_dat = 5;
    static const uint8_t exec_progbuf_start = 6;
    static const uint8_t exec_progbuf_next = 7;
    static const uint8_t exec_progbuf_waitmemop = 8;
    static const uint8_t abstract_mem_request = 9;
    static const uint8_t abstract_mem_response = 10;
    static const uint8_t wait_to_accept = 11;

    struct DbgPort_registers {
        sc_signal<bool> dport_write;
        sc_signal<sc_uint<RISCV_ARCH>> dport_addr;
        sc_signal<sc_uint<RISCV_ARCH>> dport_wdata;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
        sc_signal<sc_uint<2>> dport_size;
        sc_signal<sc_uint<4>> dstate;
        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> stack_trace_cnt;// Stack trace buffer counter
        sc_signal<bool> req_accepted;
        sc_signal<bool> resp_error;
        sc_signal<bool> progbuf_ena;
        sc_signal<sc_uint<RISCV_ARCH>> progbuf_pc;
        sc_signal<sc_uint<64>> progbuf_instr;
    } v, r;

    void DbgPort_r_reset(DbgPort_registers &iv) {
        iv.dport_write = 0;
        iv.dport_addr = 0ull;
        iv.dport_wdata = 0ull;
        iv.dport_rdata = 0ull;
        iv.dport_size = 0;
        iv.dstate = idle;
        iv.rdata = 0ull;
        iv.stack_trace_cnt = 0;
        iv.req_accepted = 0;
        iv.resp_error = 0;
        iv.progbuf_ena = 0;
        iv.progbuf_pc = 0ull;
        iv.progbuf_instr = 0ull;
    }

    sc_signal<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> wb_stack_raddr;
    sc_signal<sc_biguint<(2 * RISCV_ARCH)>> wb_stack_rdata;
    sc_signal<bool> w_stack_we;
    sc_signal<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> wb_stack_waddr;
    sc_signal<sc_biguint<(2 * RISCV_ARCH)>> wb_stack_wdata;

    StackTraceBuffer *trbuf0;

};

}  // namespace debugger

