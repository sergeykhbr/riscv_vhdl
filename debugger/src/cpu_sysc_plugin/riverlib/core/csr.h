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
    sc_in<bool> i_e_halted;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_pc;   // current latched instruction pointer in executor
    sc_in<sc_uint<32>> i_e_instr;               // current latched opcode in executor
    sc_in<bool> i_irq_timer;
    sc_in<bool> i_irq_external;
    sc_out<bool> o_irq_software;                // software interrupt pending bit
    sc_out<bool> o_irq_timer;                   // timer interrupt pending bit
    sc_out<bool> o_irq_external;                // external interrupt pending bit
    sc_out<bool> o_stack_overflow;              // stack overflow exception
    sc_out<bool> o_stack_underflow;             // stack underflow exception
    sc_in<bool> i_e_valid;
    sc_out<sc_uint<64>> o_executed_cnt;     // Number of executed instructions

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


    void comb();
    void registers();

    SC_HAS_PROCESS(CsrRegs);

    CsrRegs(sc_module_name name_, uint32_t hartid, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    static const int State_Idle = 0;
    static const int State_RW = 1;
    static const int State_Exception = 2;
    static const int State_Breakpoint = 3;
    static const int State_Interrupt = 4;
    static const int State_TrapReturn = 5;
    static const int State_Halt = 6;
    static const int State_Resume = 7;
    static const int State_Response = 8;

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CsrReq_TotalBits>> cmd_type;
        sc_signal<sc_uint<12>> cmd_addr;
        sc_signal<sc_uint<RISCV_ARCH>> cmd_data;
        sc_signal<bool> cmd_exception;          // exception on CSR access
        sc_signal<sc_uint<RISCV_ARCH>> mtvec;
        sc_signal<sc_uint<2>> mtvec_mode;
        sc_signal<sc_uint<RISCV_ARCH>> mtval;
        sc_signal<sc_uint<RISCV_ARCH>> mscratch;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mstackovr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mstackund;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_mask;
        sc_signal<sc_uint<CFG_MPU_TBL_WIDTH>> mpu_idx;
        sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> mpu_flags;
        sc_signal<bool> mpu_we;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mepc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> uepc;
        // mstatus bits:
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> uie;                    // User level interrupts ena for current priv. mode
        sc_signal<bool> mie;                    // Machine level interrupts ena for current priv. mode
        sc_signal<bool> mpie;                   // Previous MIE value
        sc_signal<sc_uint<2>> mpp;              // Previous mode
        // mie bits:
        sc_signal<bool> usie;       // User software interrupt enable
        sc_signal<bool> ssie;       // Supervisor software interrupt enable
        sc_signal<bool> msie;       // machine software interrupt enable
        sc_signal<bool> utie;       // User timer interrupt enable
        sc_signal<bool> stie;       // Supervisor timer interrupt enable
        sc_signal<bool> mtie;       // Machine timer interrupt enable
        sc_signal<bool> ueie;       // User external interrupt enable
        sc_signal<bool> seie;       // Supervisor external interrupt enable
        sc_signal<bool> meie;       // Machine external interrupt enable
        // mip bits:
        sc_signal<bool> usip;       // user software interrupt pending
        sc_signal<bool> ssip;       // supervisor software interrupt pending
        sc_signal<bool> msip;       // machine software interrupt pending
        sc_signal<bool> utip;       // user timer interrupt pending
        sc_signal<bool> stip;       // supervisor timer interrupt pending
        sc_signal<bool> mtip;       // machine timer interrupt pending
        sc_signal<bool> ueip;       // user external interrupt pending
        sc_signal<bool> seip;       // supervisor external interrupt pending
        sc_signal<bool> meip;       // machine external interrupt pending

        sc_signal<bool> ex_fpu_invalidop;         // FPU Exception: invalid operation
        sc_signal<bool> ex_fpu_divbyzero;         // FPU Exception: divide by zero
        sc_signal<bool> ex_fpu_overflow;          // FPU Exception: overflow
        sc_signal<bool> ex_fpu_underflow;         // FPU Exception: underflow
        sc_signal<bool> ex_fpu_inexact;           // FPU Exception: inexact
        sc_signal<bool> trap_irq;
        sc_signal<sc_uint<5>> trap_cause;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> trap_addr;

        sc_signal<sc_uint<64>> timer;                       // Timer in clocks.
        sc_signal<sc_uint<64>> cycle_cnt;                   // Cycle in clocks.
        sc_signal<sc_uint<64>> executed_cnt;                // Number of valid executed instructions

        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> dpc;
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
        iv.cmd_exception = 0;
        iv.mtvec = 0;
        iv.mtvec_mode = 0;
        iv.mtval = 0;
        iv.mscratch = 0;
        iv.mstackovr = 0;
        iv.mstackund = 0;
        iv.mode = PRV_M;
        iv.uie = 0;
        iv.mie = 0;
        iv.mpie = 0;
        iv.mpp = 0;
        iv.mepc = 0;
        iv.uepc = 0;
        iv.mpu_addr = 0;
        iv.mpu_mask = 0;
        iv.mpu_idx = 0;
        iv.mpu_flags = 0;
        iv.mpu_we = 0;
        iv.usie = 0;
        iv.ssie = 0;
        iv.msie = 0;
        iv.utie = 0;
        iv.stie = 0;
        iv.mtie = 0;
        iv.ueie = 0;
        iv.seie = 0;
        iv.meie = 0;

        iv.usip = 0;
        iv.ssip = 0;
        iv.msip = 0;
        iv.utip = 0;
        iv.stip = 0;
        iv.mtip = 0;
        iv.ueip = 0;
        iv.seip = 0;
        iv.meip = 0;
        iv.ex_fpu_invalidop = 0;
        iv.ex_fpu_divbyzero = 0;
        iv.ex_fpu_overflow = 0;
        iv.ex_fpu_underflow = 0;
        iv.ex_fpu_inexact = 0;
        iv.trap_irq = 0;
        iv.trap_cause = 0;
        iv.trap_addr = 0;
        iv.timer = 0;
        iv.cycle_cnt = 0;
        iv.executed_cnt = 0;
        iv.dpc = 0;
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
