/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_RIVERLIB_CSR_H__
#define __DEBUGGER_RIVERLIB_CSR_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(CsrRegs) {
    sc_in<bool> i_clk;                      // Clock signal
    sc_in<bool> i_nrst;                     // Reset (active low)
    sc_in<sc_uint<RISCV_ARCH>> i_sp;        // Stack Pointer for border control
    sc_in<bool> i_req_valid;                    // Access to CSR request
    sc_out<bool> o_req_ready;                   // CSR module is ready to accept request
    sc_in<sc_uint<CsrReq_TotalBits>> i_req_type;// Request type: [0]-read csr; [1]-write csr; [2]-change mode
    sc_in<sc_uint<12>> i_req_addr;              // Requested CSR address
    sc_in<sc_uint<RISCV_ARCH>> i_req_data;      // CSR new value
    sc_out<bool> o_resp_valid;                  // CSR module Response is valid
    sc_in<bool> i_resp_ready;                   // Executor is ready to accept response
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data;    // Responded CSR data
    sc_out<bool> o_resp_exception;              // exception on CSR access
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_pc;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_npc;
    sc_in<bool> i_irq_external;
    sc_out<bool> o_irq_software;                // software interrupt pending bit
    sc_out<bool> o_irq_timer;                   // timer interrupt pending bit
    sc_out<bool> o_irq_external;                // external interrupt pending bit
    sc_in<bool> i_e_valid;
    sc_out<sc_uint<64>> o_executed_cnt;     // Number of executed instructions
    sc_out<bool> o_dbg_pc_write;            // Modify pc via debug interface
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_dbg_pc;    // Writing value into pc register

    sc_out<bool> o_progbuf_ena;               // Execution from prog buffer
    sc_out<sc_uint<32>> o_progbuf_pc;         // prog buffer instruction counter
    sc_out<sc_uint<32>> o_progbuf_data;       // prog buffer instruction opcode
    sc_out<bool> o_flushi_ena;                // clear specified addr in ICache without execution of fence.i
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_flushi_addr; // ICache address to flush

    sc_out<bool> o_mpu_region_we;
    sc_out<sc_uint<CFG_MPU_TBL_WIDTH>> o_mpu_region_idx;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_addr;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_mask;
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_mpu_region_flags;  // {ena, cachable, r, w, x}

    sc_out<bool> o_halt;

    void comb();
    void registers();

    SC_HAS_PROCESS(CsrRegs);

    CsrRegs(sc_module_name name_, uint32_t hartid, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    static const int State_Idle = 0;
    static const int State_RW = 1;
    static const int State_Exception = 2;
    static const int State_Interrupt = 3;
    static const int State_TrapReturn = 4;
    static const int State_Response = 5;

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CsrReq_TotalBits>> cmd_type;
        sc_signal<sc_uint<12>> cmd_addr;
        sc_signal<sc_uint<RISCV_ARCH>> cmd_data;
        sc_signal<sc_uint<RISCV_ARCH>> mtvec;
        sc_signal<sc_uint<RISCV_ARCH>> mscratch;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mstackovr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mstackund;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mbadaddr;
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> uie;                    // User level interrupts ena for current priv. mode
        sc_signal<bool> mie;                    // Machine level interrupts ena for current priv. mode
        sc_signal<bool> mpie;                   // Previous MIE value
        sc_signal<bool> mstackovr_ena;          // Stack Overflow control Enabled
        sc_signal<bool> mstackund_ena;          // Stack Underflow control Enabled
        sc_signal<sc_uint<2>> mpp;              // Previous mode
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mepc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> uepc;
        sc_signal<bool> ext_irq;

        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_mask;
        sc_signal<sc_uint<CFG_MPU_TBL_WIDTH>> mpu_idx;
        sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> mpu_flags;
        sc_signal<bool> mpu_we;

        sc_signal<bool> ex_fpu_invalidop;         // FPU Exception: invalid operation
        sc_signal<bool> ex_fpu_divbyzero;         // FPU Exception: divide by zero
        sc_signal<bool> ex_fpu_overflow;          // FPU Exception: overflow
        sc_signal<bool> ex_fpu_underflow;         // FPU Exception: underflow
        sc_signal<bool> ex_fpu_inexact;           // FPU Exception: inexact
        sc_signal<bool> trap_irq;
        sc_signal<sc_uint<5>> trap_code;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> trap_addr;
        sc_signal<bool> break_event;            // 1 clock pulse
        sc_signal<bool> hold_data_store_fault;
        sc_signal<bool> hold_data_load_fault;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> hold_mbadaddr;

        sc_signal<sc_uint<64>> timer;                       // Timer in clocks.
        sc_signal<sc_uint<64>> cycle_cnt;                   // Cycle in clocks.
        sc_signal<sc_uint<64>> executed_cnt;                // Number of valid executed instructions

        sc_signal<bool> break_mode;               // Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
        sc_signal<bool> halt;
        sc_signal<sc_uint<3>> halt_cause;      // 1=ebreak instruction; 2=breakpoint exception; 3=haltreq; 4=step
        sc_signal<bool> progbuf_ena;
        sc_signal<sc_biguint<CFG_PROGBUF_REG_TOTAL*32>> progbuf_data;
        sc_signal<sc_uint<32>> progbuf_data_out;
        sc_signal<sc_uint<5>> progbuf_data_pc;
        sc_signal<sc_uint<5>> progbuf_data_npc;
        sc_signal<sc_uint<3>> progbuf_err;         // 1=busy;2=cmd not supported;3=exception;4=halt/resume;5=bus error
        sc_signal<bool> stepping_mode;
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_cnt;
        sc_signal<sc_uint<RISCV_ARCH>> ins_per_step; // Number of steps before halt in stepping mode
        sc_signal<bool> flushi_ena;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flushi_addr;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.cmd_type = 0;
        iv.cmd_addr = 0;
        iv.cmd_data = 0;
        iv.mtvec = 0;
        iv.mscratch = 0;
        iv.mstackovr = 0;
        iv.mstackund = 0;
        iv.mbadaddr = 0;
        iv.mode = PRV_M;
        iv.uie = 0;
        iv.mie = 0;
        iv.mpie = 0;
        iv.mstackovr_ena = 0;
        iv.mstackund_ena = 0;
        iv.mpp = 0;
        iv.mepc = 0;
        iv.uepc = 0;
        iv.ext_irq = 0;
        iv.mpu_addr = 0;
        iv.mpu_mask = 0;
        iv.mpu_idx = 0;
        iv.mpu_flags = 0;
        iv.mpu_we = 0;
        iv.ex_fpu_invalidop = 0;
        iv.ex_fpu_divbyzero = 0;
        iv.ex_fpu_overflow = 0;
        iv.ex_fpu_underflow = 0;
        iv.ex_fpu_inexact = 0;
        iv.trap_irq = 0;
        iv.trap_code = 0;
        iv.trap_addr = 0;
        iv.break_event = 0;
        iv.hold_data_store_fault = 0;
        iv.hold_data_load_fault = 0;
        iv.hold_mbadaddr = 0;
        iv.timer = 0;
        iv.cycle_cnt = 0;
        iv.executed_cnt = 0;
        iv.break_mode = 0;
        iv.halt = 0;
        iv.halt_cause = 0;
        iv.progbuf_ena = 0;
        iv.progbuf_data = 0;
        iv.progbuf_data_out = 0;
        iv.progbuf_data_pc = 0;
        iv.progbuf_data_npc = 0;
        iv.progbuf_err = PROGBUF_ERR_NONE;
        iv.stepping_mode = 0;
        iv.stepping_mode_cnt = 0;
        iv.ins_per_step = 1;
        iv.flushi_ena = 0;
        iv.flushi_addr = 0;
    }

    uint32_t hartid_;
    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CSR_H__
