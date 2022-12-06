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
package csr_pkg;

import river_cfg_pkg::*;

localparam int unsigned State_Idle = 0;
localparam int unsigned State_RW = 1;
localparam int unsigned State_Exception = 2;
localparam int unsigned State_Breakpoint = 3;
localparam int unsigned State_Interrupt = 4;
localparam int unsigned State_TrapReturn = 5;
localparam int unsigned State_Halt = 6;
localparam int unsigned State_Resume = 7;
localparam int unsigned State_Wfi = 8;
localparam int unsigned State_Fence = 9;
localparam int unsigned State_WaitPmp = 10;
localparam int unsigned State_Response = 11;

localparam bit [2:0] Fence_None = 3'h0;
localparam bit [2:0] Fence_DataBarrier = 3'h1;
localparam bit [2:0] Fence_DataFlush = 3'h2;
localparam bit [2:0] Fence_WaitDataFlushEnd = 3'h3;
localparam bit [2:0] Fence_FlushInstr = 3'h4;
localparam bit [2:0] Fence_End = 3'h7;

localparam bit [3:0] SATP_MODE_SV39 = 4'h8;                 // 39-bits Page mode
localparam bit [3:0] SATP_MODE_SV48 = 4'h9;                 // 48-bits Page mode

typedef struct {
    logic [RISCV_ARCH-1:0] xepc;
    logic [1:0] xpp;                                        // Previous Privildge mode. If x is not implemented, then xPP mus be 0
    logic xpie;                                             // Previous Privildge mode global interrupt enable
    logic xie;                                              // Global interrupt enbale bit.
    logic xsie;                                             // Enable Software interrupts.
    logic xtie;                                             // Enable Timer interrupts.
    logic xeie;                                             // Enable External interrupts.
    logic [RISCV_ARCH-1:0] xtvec_off;                       // Trap Vector BAR
    logic [1:0] xtvec_mode;                                 // Trap Vector mode: 0=direct; 1=vectored
    logic [RISCV_ARCH-1:0] xtval;                           // Trap value, bad address
    logic xcause_irq;                                       // 0=Exception, 1=Interrupt
    logic [4:0] xcause_code;                                // Exception code
    logic [RISCV_ARCH-1:0] xscratch;                        // software dependable register
    logic [31:0] xcounteren;                                // Counter-enable controls access to timers from the next less priv mode
} RegModeType;

typedef struct {
    logic [7:0] cfg;                                        // pmpcfg bits without changes
    logic [RISCV_ARCH-1:0] addr;                            // Maximal PMP address bits [55:2]
    logic [RISCV_ARCH-1:0] mask;                            // NAPOT mask formed from address
} PmpItemType;


typedef struct {
    RegModeType xmode[0: 4 - 1];
    PmpItemType pmp[0: CFG_PMP_TBL_SIZE - 1];
    logic [3:0] state;
    logic [2:0] fencestate;
    logic [IRQ_TOTAL-1:0] irq_pending;
    logic [CsrReq_TotalBits-1:0] cmd_type;
    logic [11:0] cmd_addr;
    logic [RISCV_ARCH-1:0] cmd_data;
    logic cmd_exception;                                    // exception on CSR access
    logic progbuf_end;
    logic progbuf_err;
    logic mip_ssip;                                         // page 34: SSIP is writable to re-request SW irq from machine to supervisor
    logic mip_stip;                                         // page 34: SSIP is writable in mip
    logic mip_seip;                                         // page 34: SSIP is writable in mip
    logic [63:0] medeleg;
    logic [IRQ_TOTAL-1:0] mideleg;
    logic [31:0] mcountinhibit;                             // When non zero stop specified performance counter
    logic [RISCV_ARCH-1:0] mstackovr;
    logic [RISCV_ARCH-1:0] mstackund;
    logic mmu_ena;                                          // Instruction MMU SV48 enabled in U- and S- modes
    logic [43:0] satp_ppn;                                  // Physcal Page Number
    logic satp_sv39;
    logic satp_sv48;
    logic [1:0] mode;
    logic mprv;                                             // Modify PRiVilege. (Table 8.5) If MPRV=0, load and stores as normal, when MPRV=1, use translation of previous mode
    logic mxr;                                              // Interpret Executable as Readable when =1
    logic sum;                                              // Access to Userspace from Supervisor mode
    logic tvm;                                              // Trap Virtual Memory bit. When 1 SFENCE.VMA or SINVAL.VMA or rw access to SATP raise an illegal instruction
    logic ex_fpu_invalidop;                                 // FPU Exception: invalid operation
    logic ex_fpu_divbyzero;                                 // FPU Exception: divide by zero
    logic ex_fpu_overflow;                                  // FPU Exception: overflow
    logic ex_fpu_underflow;                                 // FPU Exception: underflow
    logic ex_fpu_inexact;                                   // FPU Exception: inexact
    logic [RISCV_ARCH-1:0] trap_addr;
    logic [63:0] mcycle_cnt;                                // Cycle in clocks.
    logic [63:0] minstret_cnt;                              // Number of the instructions the hart has retired
    logic [RISCV_ARCH-1:0] dscratch0;
    logic [RISCV_ARCH-1:0] dscratch1;
    logic [RISCV_ARCH-1:0] dpc;
    logic [2:0] halt_cause;                                 // 1=ebreak instruction; 2=breakpoint exception; 3=haltreq; 4=step
    logic dcsr_ebreakm;                                     // Enter or not into Debug Mode on EBREAK instruction
    logic dcsr_stopcount;
    logic dcsr_stoptimer;
    logic dcsr_step;
    logic dcsr_stepie;                                      // interrupt 0=dis;1=ena during stepping
    logic [RISCV_ARCH-1:0] stepping_mode_cnt;
    logic [RISCV_ARCH-1:0] ins_per_step;                    // Number of steps before halt in stepping mode
    logic [CFG_PMP_TBL_SIZE-1:0] pmp_upd_ena;
    logic [CFG_PMP_TBL_WIDTH-1:0] pmp_upd_cnt;
    logic pmp_ena;
    logic pmp_we;
    logic [CFG_PMP_TBL_WIDTH-1:0] pmp_region;
    logic [RISCV_ARCH-1:0] pmp_start_addr;
    logic [RISCV_ARCH-1:0] pmp_end_addr;
    logic [CFG_PMP_FL_TOTAL-1:0] pmp_flags;
} CsrRegs_registers;

endpackage: csr_pkg
