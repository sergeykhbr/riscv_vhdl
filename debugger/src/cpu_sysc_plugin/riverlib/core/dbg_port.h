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

#ifndef __DEBUGGER_RIVERLIB_DBG_PORT_H__
#define __DEBUGGER_RIVERLIB_DBG_PORT_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "stacktrbuf.h"

namespace debugger {

SC_MODULE(DbgPort) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset. Active LOW.
    // "RIVER" Debug interface
    sc_in<bool> i_dport_req_valid;                      // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                     // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                    // Response is valid
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    // CPU debugging signals:
    sc_out<sc_uint<12>> o_csr_addr;                     // Address of the sub-region register
    sc_out<sc_uint<6>> o_reg_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_core_wdata;           // Write data
    sc_out<bool> o_csr_ena;                             // Region 0: Access to CSR bank is enabled.
    sc_out<bool> o_csr_write;                           // Region 0: CSR write enable
    sc_in<bool> i_csr_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;             // Region 0: CSR read value
    sc_out<bool> o_ireg_ena;                            // Region 1: Access to integer register bank is enabled
    sc_out<bool> o_ireg_write;                          // Region 1: Integer registers bank write pulse
    sc_in<sc_uint<RISCV_ARCH>> i_ireg_rdata;            // Region 1: Integer register read value
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_pc;             // Region 1: Instruction pointer
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_npc;            // Region 1: Next Instruction pointer
    sc_in<bool> i_e_call;                               // pseudo-instruction CALL
    sc_in<bool> i_e_ret;                                // pseudo-instruction RET
    sc_out<bool> o_progbuf_ena;                         // Execution from prog buffer
    sc_out<sc_uint<32>> o_progbuf_pc;                    // prog buffer instruction counter
    sc_out<sc_uint<32>> o_progbuf_data;                  // prog buffer instruction opcode
    sc_out<bool> o_br_fetch_valid;                      // Fetch injection address/instr are valid
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_br_address_fetch; // Fetch injection address to skip ebreak instruciton only once
    sc_out<sc_uint<32>> o_br_instr_fetch;               // Real instruction value that was replaced by ebreak
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_flush_address; // Address of instruction to remove from ICache
    sc_out<bool> o_flush_valid;                         // Remove address from ICache is valid

    void comb();
    void registers();

    SC_HAS_PROCESS(DbgPort);

    DbgPort(sc_module_name name_, bool async_reset);
    virtual ~DbgPort();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    enum dport_state_type {
        idle,
        csr_region,
        reg_bank,
        reg_stktr_cnt,
        reg_stktr_buf_adr,
        reg_stktr_buf_dat,
        control,
        wait_to_accept
    };

    struct RegistersType {
        sc_signal<bool> dport_write;
        sc_signal<sc_uint<2>> dport_region;
        sc_signal<sc_uint<12>> dport_addr;
        sc_signal<sc_uint<RISCV_ARCH>> dport_wdata;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
        sc_signal<sc_uint<4>> dstate;

        sc_signal<bool> trap_on_break;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> br_address_fetch;
        sc_signal<sc_uint<32>> br_instr_fetch;
        sc_signal<bool> br_fetch_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flush_address;
        sc_signal<bool> flush_valid;

        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> stack_trace_cnt;              // Stack trace buffer counter
        sc_signal<bool> progbuf_ena;
        sc_signal<sc_biguint<16*32>> progbuf_data;
        sc_signal<sc_uint<32>> progbuf_data_out;
        sc_signal<sc_uint<5>> progbuf_data_pc;
        sc_signal<sc_uint<5>> progbuf_data_npc;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.dport_write = 0;
        iv.dport_region = 0;
        iv.dport_addr = 0;
        iv.dport_wdata = 0;
        iv.dport_rdata = 0;
        iv.dstate = idle;
        iv.trap_on_break = 0;
        iv.br_address_fetch = 0;
        iv.br_instr_fetch = 0;
        iv.br_fetch_valid = 0;
        iv.flush_address = 0;
        iv.flush_valid = 0;
        iv.rdata = 0;
        iv.stack_trace_cnt = 0;
        iv.progbuf_ena = 0;
        iv.progbuf_data = 0;
        iv.progbuf_data_out = 0;
        iv.progbuf_data_pc = 0;
        iv.progbuf_data_npc = 0;
    }

    sc_signal<sc_uint<5>> wb_stack_raddr;
    sc_signal<sc_biguint<2*CFG_CPU_ADDR_BITS>> wb_stack_rdata;
    sc_signal<bool> w_stack_we;
    sc_signal<sc_uint<5>> wb_stack_waddr;
    sc_signal<sc_biguint<2*CFG_CPU_ADDR_BITS>> wb_stack_wdata;

    StackTraceBuffer *trbuf0;
    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DBG_PORT_H__
