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

SC_MODULE(CsrRegs) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<RISCV_ARCH>> i_sp;                        // Stack Pointer for border control
    sc_in<bool> i_req_valid;                                // Access to CSR request
    sc_out<bool> o_req_ready;                               // CSR module is ready to accept request
    sc_in<sc_uint<CsrReq_TotalBits>> i_req_type;            // Request type: [0]-read csr; [1]-write csr; [2]-change mode
    sc_in<sc_uint<12>> i_req_addr;                          // Requested CSR address
    sc_in<sc_uint<RISCV_ARCH>> i_req_data;                  // CSR new value
    sc_out<bool> o_resp_valid;                              // CSR module Response is valid
    sc_in<bool> i_resp_ready;                               // Executor is ready to accept response
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data;                // Responded CSR data
    sc_out<bool> o_resp_exception;                          // exception on CSR access
    sc_in<bool> i_e_halted;                                 // core is halted confirmation flag
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_pc;               // current latched instruction pointer in executor
    sc_in<sc_uint<32>> i_e_instr;                           // current latched opcode in executor
    sc_in<sc_uint<1>> i_msip;                               // machine software pening interrupt
    sc_in<sc_uint<1>> i_mtip;                               // machine timer pening interrupt
    sc_in<sc_uint<1>> i_meip;                               // machine external pening interrupt
    sc_in<sc_uint<1>> i_seip;                               // supervisor external pening interrupt
    sc_out<bool> o_irq_software;                            // software interrupt pending bit
    sc_out<bool> o_irq_timer;                               // timer interrupt pending bit
    sc_out<bool> o_irq_external;                            // external interrupt pending bit
    sc_out<bool> o_stack_overflow;                          // stack overflow exception
    sc_out<bool> o_stack_underflow;                         // stack underflow exception
    sc_in<bool> i_e_valid;                                  // instructuin executed flag
    sc_out<sc_uint<64>> o_executed_cnt;                     // Number of executed instructions
    
    sc_out<bool> o_step;                                    // Stepping enabled
    sc_in<bool> i_dbg_progbuf_ena;                          // Executing progbuf is in progress
    sc_out<bool> o_progbuf_end;                             // End of execution from prog buffer
    sc_out<bool> o_progbuf_error;                           // exception during progbuf execution
    sc_out<bool> o_flushi_ena;                              // clear specified addr in ICache without execution of fence.i
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_flushi_addr;       // ICache address to flush
    
    sc_out<bool> o_mpu_region_we;                           // write enable into MPU
    sc_out<sc_uint<CFG_MPU_TBL_WIDTH>> o_mpu_region_idx;    // selected MPU region
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_addr;   // MPU region base address
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_mask;   // MPU region mask
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_mpu_region_flags;   // {ena, cachable, r, w, x}

    void comb();
    void registers();

    SC_HAS_PROCESS(CsrRegs);

    CsrRegs(sc_module_name name,
            bool async_reset,
            uint32_t hartid);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t hartid_;

    static const uint32_t State_Idle = 0;
    static const uint32_t State_RW = 1;
    static const uint32_t State_Exception = 2;
    static const uint32_t State_Breakpoint = 3;
    static const uint32_t State_Interrupt = 4;
    static const uint32_t State_TrapReturn = 5;
    static const uint32_t State_Halt = 6;
    static const uint32_t State_Resume = 7;
    static const uint32_t State_Wfi = 8;
    static const uint32_t State_Response = 9;

    struct CsrRegs_registers {
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<CsrReq_TotalBits>> cmd_type;
        sc_signal<sc_uint<12>> cmd_addr;
        sc_signal<sc_uint<RISCV_ARCH>> cmd_data;
        sc_signal<bool> cmd_exception;                      // exception on CSR access
        sc_signal<bool> progbuf_end;
        sc_signal<bool> progbuf_err;
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
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> uie;                                // mstatus: User level interrupts ena for current priv. mode
        sc_signal<bool> mie;                                // mstatus: Machine level interrupts ena for current priv. mode
        sc_signal<bool> mpie;                               // mstatus: Previous MIE value
        sc_signal<sc_uint<2>> mpp;                          // mstatus: Previous mode
        sc_signal<bool> usie;                               // mie: User software interrupt enable
        sc_signal<bool> ssie;                               // mie: Supervisor software interrupt enable
        sc_signal<bool> msie;                               // mie: machine software interrupt enable
        sc_signal<bool> utie;                               // mie: User timer interrupt enable
        sc_signal<bool> stie;                               // mie: Supervisor timer interrupt enable
        sc_signal<bool> mtie;                               // mie: Machine timer interrupt enable
        sc_signal<bool> ueie;                               // mie: User external interrupt enable
        sc_signal<bool> seie;                               // mie: Supervisor external interrupt enable
        sc_signal<bool> meie;                               // mie: Machine external interrupt enable
        sc_signal<bool> usip;                               // mip: user software interrupt pending
        sc_signal<bool> ssip;                               // mip: supervisor software interrupt pending
        sc_signal<bool> msip;                               // mip: machine software interrupt pending
        sc_signal<bool> utip;                               // mip: user timer interrupt pending
        sc_signal<bool> stip;                               // mip: supervisor timer interrupt pending
        sc_signal<bool> mtip;                               // mip: machine timer interrupt pending
        sc_signal<bool> ueip;                               // mip: user external interrupt pending
        sc_signal<bool> seip;                               // mip: supervisor external interrupt pending
        sc_signal<bool> meip;                               // mip: machine external interrupt pending
        sc_signal<bool> ex_fpu_invalidop;                   // FPU Exception: invalid operation
        sc_signal<bool> ex_fpu_divbyzero;                   // FPU Exception: divide by zero
        sc_signal<bool> ex_fpu_overflow;                    // FPU Exception: overflow
        sc_signal<bool> ex_fpu_underflow;                   // FPU Exception: underflow
        sc_signal<bool> ex_fpu_inexact;                     // FPU Exception: inexact
        sc_signal<bool> trap_irq;
        sc_signal<sc_uint<5>> trap_cause;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> trap_addr;
        sc_signal<sc_uint<64>> timer;                       // Timer in clocks.
        sc_signal<sc_uint<64>> cycle_cnt;                   // Cycle in clocks.
        sc_signal<sc_uint<64>> executed_cnt;                // Number of valid executed instructions
        sc_signal<sc_uint<RISCV_ARCH>> dscratch0;
        sc_signal<sc_uint<RISCV_ARCH>> dscratch1;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> dpc;
        sc_signal<sc_uint<3>> halt_cause;                   // 1=ebreak instruction; 2=breakpoint exception; 3=haltreq; 4=step
        sc_signal<bool> dcsr_ebreakm;                       // Enter or not into Debug Mode on EBREAK instruction
        sc_signal<bool> dcsr_stopcount;
        sc_signal<bool> dcsr_stoptimer;
        sc_signal<bool> dcsr_step;
        sc_signal<bool> dcsr_stepie;                        // interrupt 0=dis;1=ena during stepping
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_cnt;
        sc_signal<sc_uint<RISCV_ARCH>> ins_per_step;        // Number of steps before halt in stepping mode
        sc_signal<bool> flushi_ena;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flushi_addr;
    } v, r;

    void CsrRegs_r_reset(CsrRegs_registers &iv) {
        iv.state = State_Idle;
        iv.cmd_type = 0;
        iv.cmd_addr = 0;
        iv.cmd_data = 0;
        iv.cmd_exception = 0;
        iv.progbuf_end = 0;
        iv.progbuf_err = 0;
        iv.mtvec = 0;
        iv.mtvec_mode = 0;
        iv.mtval = 0;
        iv.mscratch = 0;
        iv.mstackovr = 0;
        iv.mstackund = 0;
        iv.mpu_addr = 0;
        iv.mpu_mask = 0;
        iv.mpu_idx = 0;
        iv.mpu_flags = 0;
        iv.mpu_we = 0;
        iv.mepc = 0;
        iv.uepc = 0;
        iv.mode = PRV_M;
        iv.uie = 0;
        iv.mie = 0;
        iv.mpie = 0;
        iv.mpp = 0;
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
        iv.dscratch0 = 0;
        iv.dscratch1 = 0;
        iv.dpc = CFG_RESET_VECTOR;
        iv.halt_cause = 0;
        iv.dcsr_ebreakm = 0;
        iv.dcsr_stopcount = 0;
        iv.dcsr_stoptimer = 0;
        iv.dcsr_step = 0;
        iv.dcsr_stepie = 0;
        iv.stepping_mode_cnt = 0;
        iv.ins_per_step = 1;
        iv.flushi_ena = 0;
        iv.flushi_addr = 0;
    }

};

}  // namespace debugger

