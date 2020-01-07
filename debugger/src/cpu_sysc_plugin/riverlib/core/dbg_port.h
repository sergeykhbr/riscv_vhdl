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
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    // CPU debugging signals:
    sc_out<sc_uint<12>> o_csr_addr;                     // Address of the sub-region register
    sc_out<sc_uint<6>> o_reg_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_core_wdata;           // Write data
    sc_out<bool> o_csr_ena;                             // Region 0: Access to CSR bank is enabled.
    sc_out<bool> o_csr_write;                           // Region 0: CSR write enable
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;             // Region 0: CSR read value
    sc_out<bool> o_ireg_ena;                            // Region 1: Access to integer register bank is enabled
    sc_out<bool> o_ireg_write;                          // Region 1: Integer registers bank write pulse
    sc_out<bool> o_npc_write;                           // Region 1: npc write enable
    sc_in<sc_uint<RISCV_ARCH>> i_ireg_rdata;            // Region 1: Integer register read value
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_pc;                // Region 1: Instruction pointer
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_npc;               // Region 1: Next Instruction pointer
    sc_in<bool> i_e_next_ready;
    sc_in<bool> i_e_valid;                              // Stepping control signal
    sc_in<bool> i_e_call;                               // pseudo-instruction CALL
    sc_in<bool> i_e_ret;                                // pseudo-instruction RET
    sc_out<sc_uint<64>> o_clock_cnt;                    // Number of clocks excluding halt state
    sc_out<sc_uint<64>> o_executed_cnt;                 // Number of executed instructions
    sc_out<bool> o_halt;                                // Halt signal is equal to hold pipeline
    sc_in<bool> i_ebreak;                               // ebreak instruction decoded
    sc_out<bool> o_break_mode;                          // Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
    sc_out<bool> o_br_fetch_valid;                      // Fetch injection address/instr are valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_br_address_fetch; // Fetch injection address to skip ebreak instruciton only once
    sc_out<sc_uint<32>> o_br_instr_fetch;               // Real instruction value that was replaced by ebreak
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_flush_address;    // Address of instruction to remove from ICache
    sc_out<bool> o_flush_valid;                         // Remove address from ICache is valid
    // Cache debug signals:
    sc_in<sc_uint<4>> i_istate;                         // ICache transaction state
    sc_in<sc_uint<4>> i_dstate;                         // DCache transaction state
    sc_in<sc_uint<2>> i_cstate;                         // CacheTop state machine value

    void comb();
    void registers();

    SC_HAS_PROCESS(DbgPort);

    DbgPort(sc_module_name name_, bool async_reset);
    virtual ~DbgPort();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<bool> ready;
        sc_signal<bool> halt;
        sc_signal<bool> breakpoint;
        sc_signal<bool> stepping_mode;
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_cnt;
        sc_signal<bool> trap_on_break;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> br_address_fetch;
        sc_signal<sc_uint<32>> br_instr_fetch;
        sc_signal<bool> br_fetch_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> flush_address;
        sc_signal<bool> flush_valid;

        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_steps; // Number of steps before halt in stepping mode
        sc_signal<sc_uint<64>> clock_cnt;                   // Timer in clocks.
        sc_signal<sc_uint<64>> executed_cnt;                // Number of valid executed instructions
        sc_signal<sc_uint<5>> stack_trace_cnt;              // Stack trace buffer counter (Log2[CFG_STACK_TRACE_BUF_SIZE])
        sc_signal<bool> rd_trbuf_ena;
        sc_signal<bool> rd_trbuf_addr0;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.ready = 0;
        iv.halt = 0;
        iv.breakpoint = 0;
        iv.stepping_mode = 0;
        iv.stepping_mode_cnt = 0;
        iv.trap_on_break = 0;
        iv.br_address_fetch = 0;
        iv.br_instr_fetch = 0;
        iv.br_fetch_valid = 0;
        iv.flush_address = 0;
        iv.flush_valid = 0;
        iv.rdata = 0;
        iv.stepping_mode_steps = 0;
        iv.clock_cnt = 0;
        iv.executed_cnt = 0;
        iv.stack_trace_cnt = 0;
        iv.rd_trbuf_ena = 0;
        iv.rd_trbuf_addr0 = 0;
    }

    sc_signal<sc_uint<5>> wb_stack_raddr;
    sc_signal<sc_biguint<2*BUS_ADDR_WIDTH>> wb_stack_rdata;
    sc_signal<bool> w_stack_we;
    sc_signal<sc_uint<5>> wb_stack_waddr;
    sc_signal<sc_biguint<2*BUS_ADDR_WIDTH>> wb_stack_wdata;

    StackTraceBuffer *trbuf0;
    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DBG_PORT_H__
