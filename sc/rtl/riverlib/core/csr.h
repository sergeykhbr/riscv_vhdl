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
    sc_in<sc_uint<RISCV_ARCH>> i_e_pc;                      // current latched instruction pointer in executor
    sc_in<sc_uint<32>> i_e_instr;                           // current latched opcode in executor
    sc_in<sc_uint<IRQ_TOTAL>> i_irq_pending;                // Per Hart pending interrupts pins
    sc_out<sc_uint<IRQ_TOTAL>> o_irq_pending;               // Enabled and Unmasked interrupt pending bits
    sc_out<bool> o_wakeup;                                  // There's pending bit even if interrupts globally disabled
    sc_out<bool> o_stack_overflow;                          // stack overflow exception
    sc_out<bool> o_stack_underflow;                         // stack underflow exception
    sc_in<bool> i_f_flush_ready;                            // fetcher is ready to accept Flush $I request
    sc_in<bool> i_e_valid;                                  // instructuin executed flag
    sc_in<bool> i_m_memop_ready;                            // memaccess module is ready to accept the request
    sc_in<bool> i_m_idle;                                   // memaccess is in idle state, no memop in progress
    sc_in<bool> i_flushd_end;
    sc_in<sc_uint<64>> i_mtimer;                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    sc_out<sc_uint<64>> o_executed_cnt;                     // Number of executed instructions
    
    sc_out<bool> o_step;                                    // Stepping enabled
    sc_in<bool> i_dbg_progbuf_ena;                          // Executing progbuf is in progress
    sc_out<bool> o_progbuf_end;                             // End of execution from prog buffer
    sc_out<bool> o_progbuf_error;                           // exception during progbuf execution
    sc_out<bool> o_flushd_valid;                            // clear specified addr in DCache
    sc_out<bool> o_flushi_valid;                            // clear specified addr in ICache
    sc_out<bool> o_flushmmu_valid;                          // clear specific leaf entry in MMU
    sc_out<bool> o_flushpipeline_valid;                     // flush pipeline, must be don for fence.VMA and fence.i
    sc_out<sc_uint<RISCV_ARCH>> o_flush_addr;               // Cache address to flush. All ones means flush all.
    
    sc_out<bool> o_pmp_ena;                                 // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    sc_out<bool> o_pmp_we;                                  // write enable into PMP
    sc_out<sc_uint<CFG_PMP_TBL_WIDTH>> o_pmp_region;        // selected PMP region
    sc_out<sc_uint<RISCV_ARCH>> o_pmp_start_addr;           // PMP region start address
    sc_out<sc_uint<RISCV_ARCH>> o_pmp_end_addr;             // PMP region end address (inclusive)
    sc_out<sc_uint<CFG_PMP_FL_TOTAL>> o_pmp_flags;          // {ena, lock, r, w, x}
    
    sc_out<bool> o_mmu_ena;                                 // MMU enabled in U and S modes. Sv48 only.
    sc_out<bool> o_mmu_sv39;                                // Translation mode sv39 is active
    sc_out<bool> o_mmu_sv48;                                // Translation mode sv48 is active
    sc_out<sc_uint<44>> o_mmu_ppn;                          // Physical Page Number
    sc_out<bool> o_mprv;                                    // modify priviledge flag can be active in m-mode
    sc_out<bool> o_mxr;                                     // make executabale readable
    sc_out<bool> o_sum;                                     // permit Supervisor User Mode access

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
    static const uint32_t State_Fence = 9;
    static const uint32_t State_WaitPmp = 10;
    static const uint32_t State_Response = 11;
    
    static const uint8_t Fence_None = 0;
    static const uint8_t Fence_DataBarrier = 1;
    static const uint8_t Fence_DataFlush = 2;
    static const uint8_t Fence_WaitDataFlushEnd = 3;
    static const uint8_t Fence_FlushInstr = 4;
    static const uint8_t Fence_End = 7;
    
    static const uint8_t SATP_MODE_SV39 = 8;
    static const uint8_t SATP_MODE_SV48 = 9;

    struct RegModeType {
        sc_signal<sc_uint<RISCV_ARCH>> xepc;
        sc_signal<sc_uint<2>> xpp;                          // Previous Privildge mode. If x is not implemented, then xPP mus be 0
        sc_signal<bool> xpie;                               // Previous Privildge mode global interrupt enable
        sc_signal<bool> xie;                                // Global interrupt enbale bit.
        sc_signal<bool> xsie;                               // Enable Software interrupts.
        sc_signal<bool> xtie;                               // Enable Timer interrupts.
        sc_signal<bool> xeie;                               // Enable External interrupts.
        sc_signal<sc_uint<RISCV_ARCH>> xtvec_off;           // Trap Vector BAR
        sc_signal<sc_uint<2>> xtvec_mode;                   // Trap Vector mode: 0=direct; 1=vectored
        sc_signal<sc_uint<RISCV_ARCH>> xtval;               // Trap value, bad address
        sc_signal<bool> xcause_irq;                         // 0=Exception, 1=Interrupt
        sc_signal<sc_uint<5>> xcause_code;                  // Exception code
        sc_signal<sc_uint<RISCV_ARCH>> xscratch;            // software dependable register
        sc_signal<sc_uint<32>> xcounteren;                  // Counter-enable controls access to timers from the next less priv mode
    };

    struct PmpItemType {
        sc_signal<sc_uint<8>> cfg;                          // pmpcfg bits without changes
        sc_signal<sc_uint<RISCV_ARCH>> addr;                // Maximal PMP address bits [55:2]
        sc_signal<sc_uint<RISCV_ARCH>> mask;                // NAPOT mask formed from address
    };


    struct CsrRegs_registers {
        RegModeType xmode[4];
        PmpItemType pmp[CFG_PMP_TBL_SIZE];
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<3>> fencestate;
        sc_signal<sc_uint<IRQ_TOTAL>> irq_pending;
        sc_signal<sc_uint<CsrReq_TotalBits>> cmd_type;
        sc_signal<sc_uint<12>> cmd_addr;
        sc_signal<sc_uint<RISCV_ARCH>> cmd_data;
        sc_signal<bool> cmd_exception;                      // exception on CSR access
        sc_signal<bool> progbuf_end;
        sc_signal<bool> progbuf_err;
        sc_signal<bool> mip_ssip;                           // page 34: SSIP is writable to re-request SW irq from machine to supervisor
        sc_signal<bool> mip_stip;                           // page 34: SSIP is writable in mip
        sc_signal<bool> mip_seip;                           // page 34: SSIP is writable in mip
        sc_signal<sc_uint<64>> medeleg;
        sc_signal<sc_uint<IRQ_TOTAL>> mideleg;
        sc_signal<sc_uint<32>> mcountinhibit;               // When non zero stop specified performance counter
        sc_signal<sc_uint<RISCV_ARCH>> mstackovr;
        sc_signal<sc_uint<RISCV_ARCH>> mstackund;
        sc_signal<bool> mmu_ena;                            // Instruction MMU SV48 enabled in U- and S- modes
        sc_signal<sc_uint<44>> satp_ppn;                    // Physcal Page Number
        sc_signal<bool> satp_sv39;
        sc_signal<bool> satp_sv48;
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> mprv;                               // Modify PRiVilege. (Table 8.5) If MPRV=0, load and stores as normal, when MPRV=1, use translation of previous mode
        sc_signal<bool> mxr;                                // Interpret Executable as Readable when =1
        sc_signal<bool> sum;                                // Access to Userspace from Supervisor mode
        sc_signal<bool> tvm;                                // Trap Virtual Memory bit. When 1 SFENCE.VMA or SINVAL.VMA or rw access to SATP raise an illegal instruction
        sc_signal<bool> ex_fpu_invalidop;                   // FPU Exception: invalid operation
        sc_signal<bool> ex_fpu_divbyzero;                   // FPU Exception: divide by zero
        sc_signal<bool> ex_fpu_overflow;                    // FPU Exception: overflow
        sc_signal<bool> ex_fpu_underflow;                   // FPU Exception: underflow
        sc_signal<bool> ex_fpu_inexact;                     // FPU Exception: inexact
        sc_signal<sc_uint<RISCV_ARCH>> trap_addr;
        sc_signal<sc_uint<64>> mcycle_cnt;                  // Cycle in clocks.
        sc_signal<sc_uint<64>> minstret_cnt;                // Number of the instructions the hart has retired
        sc_signal<sc_uint<RISCV_ARCH>> dscratch0;
        sc_signal<sc_uint<RISCV_ARCH>> dscratch1;
        sc_signal<sc_uint<RISCV_ARCH>> dpc;
        sc_signal<sc_uint<3>> halt_cause;                   // 1=ebreak instruction; 2=breakpoint exception; 3=haltreq; 4=step
        sc_signal<bool> dcsr_ebreakm;                       // Enter or not into Debug Mode on EBREAK instruction
        sc_signal<bool> dcsr_stopcount;
        sc_signal<bool> dcsr_stoptimer;
        sc_signal<bool> dcsr_step;
        sc_signal<bool> dcsr_stepie;                        // interrupt 0=dis;1=ena during stepping
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_cnt;
        sc_signal<sc_uint<RISCV_ARCH>> ins_per_step;        // Number of steps before halt in stepping mode
        sc_signal<sc_uint<CFG_PMP_TBL_SIZE>> pmp_upd_ena;
        sc_signal<sc_uint<CFG_PMP_TBL_WIDTH>> pmp_upd_cnt;
        sc_signal<bool> pmp_ena;
        sc_signal<bool> pmp_we;
        sc_signal<sc_uint<CFG_PMP_TBL_WIDTH>> pmp_region;
        sc_signal<sc_uint<RISCV_ARCH>> pmp_start_addr;
        sc_signal<sc_uint<RISCV_ARCH>> pmp_end_addr;
        sc_signal<sc_uint<CFG_PMP_FL_TOTAL>> pmp_flags;
    } v, r;


};

}  // namespace debugger

