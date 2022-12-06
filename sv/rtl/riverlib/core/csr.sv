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

`timescale 1ns/10ps

module CsrRegs #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned hartid = 0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_sp,       // Stack Pointer for border control
    input logic i_req_valid,                                // Access to CSR request
    output logic o_req_ready,                               // CSR module is ready to accept request
    input logic [river_cfg_pkg::CsrReq_TotalBits-1:0] i_req_type,// Request type: [0]-read csr; [1]-write csr; [2]-change mode
    input logic [11:0] i_req_addr,                          // Requested CSR address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_req_data, // CSR new value
    output logic o_resp_valid,                              // CSR module Response is valid
    input logic i_resp_ready,                               // Executor is ready to accept response
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_resp_data,// Responded CSR data
    output logic o_resp_exception,                          // exception on CSR access
    input logic i_e_halted,                                 // core is halted confirmation flag
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_pc,     // current latched instruction pointer in executor
    input logic [31:0] i_e_instr,                           // current latched opcode in executor
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    output logic [river_cfg_pkg::IRQ_TOTAL-1:0] o_irq_pending,// Enabled and Unmasked interrupt pending bits
    output logic o_wakeup,                                  // There's pending bit even if interrupts globally disabled
    output logic o_stack_overflow,                          // stack overflow exception
    output logic o_stack_underflow,                         // stack underflow exception
    input logic i_f_flush_ready,                            // fetcher is ready to accept Flush $I request
    input logic i_e_valid,                                  // instructuin executed flag
    input logic i_m_memop_ready,                            // memaccess module is ready to accept the request
    input logic i_m_idle,                                   // memaccess is in idle state, no memop in progress
    input logic i_flushd_end,
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    output logic [63:0] o_executed_cnt,                     // Number of executed instructions
    
    output logic o_step,                                    // Stepping enabled
    input logic i_dbg_progbuf_ena,                          // Executing progbuf is in progress
    output logic o_progbuf_end,                             // End of execution from prog buffer
    output logic o_progbuf_error,                           // exception during progbuf execution
    output logic o_flushd_valid,                            // clear specified addr in DCache
    output logic o_flushi_valid,                            // clear specified addr in ICache
    output logic o_flushmmu_valid,                          // clear specific leaf entry in MMU
    output logic o_flushpipeline_valid,                     // flush pipeline, must be don for fence.VMA and fence.i
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_flush_addr,// Cache address to flush. All ones means flush all.
    
    output logic o_pmp_ena,                                 // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    output logic o_pmp_we,                                  // write enable into PMP
    output logic [river_cfg_pkg::CFG_PMP_TBL_WIDTH-1:0] o_pmp_region,// selected PMP region
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pmp_start_addr,// PMP region start address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pmp_end_addr,// PMP region end address (inclusive)
    output logic [river_cfg_pkg::CFG_PMP_FL_TOTAL-1:0] o_pmp_flags,// {ena, lock, r, w, x}
    
    output logic o_mmu_ena,                                 // MMU enabled in U and S modes. Sv48 only.
    output logic o_mmu_sv39,                                // Translation mode sv39 is active
    output logic o_mmu_sv48,                                // Translation mode sv48 is active
    output logic [43:0] o_mmu_ppn,                          // Physical Page Number
    output logic o_mprv,                                    // modify priviledge flag can be active in m-mode
    output logic o_mxr,                                     // make executabale readable
    output logic o_sum                                      // permit Supervisor User Mode access
);

import river_cfg_pkg::*;
import csr_pkg::*;

CsrRegs_registers r, rin;

always_comb
begin: comb_proc
    CsrRegs_registers v;
    int iM;
    int iH;
    int iS;
    int iU;
    logic [1:0] vb_xpp;
    logic [IRQ_TOTAL-1:0] vb_pending;
    logic [IRQ_TOTAL-1:0] vb_irq_ena;
    logic [63:0] vb_e_emux;                                 // Exception request from executor to process
    logic [15:0] vb_e_imux;                                 // Interrupt request from executor to process
    logic [4:0] wb_trap_cause;
    logic [RISCV_ARCH-1:0] vb_xtval;                        // trap value
    logic w_mstackovr;
    logic w_mstackund;
    logic v_csr_rena;
    logic v_csr_wena;
    logic v_csr_trapreturn;
    logic [RISCV_ARCH-1:0] vb_rdata;
    logic v_req_halt;
    logic v_req_resume;
    logic v_req_progbuf;
    logic v_req_ready;
    logic v_resp_valid;
    logic v_medeleg_ena;
    logic v_mideleg_ena;
    logic [RISCV_ARCH-1:0] vb_xtvec_off_ideleg;             // 4-bytes aligned
    logic [RISCV_ARCH-1:0] vb_xtvec_off_edeleg;             // 4-bytes aligned
    logic v_flushd;
    logic v_flushi;
    logic v_flushmmu;
    logic v_flushpipeline;
    logic [CFG_PMP_TBL_SIZE-1:0] vb_pmp_upd_ena;
    logic [RISCV_ARCH-1:0] vb_pmp_napot_mask;
    logic v_napot_shift;
    int t_pmpdataidx;
    int t_pmpcfgidx;

    iM = int'(PRV_M);
    iH = int'(PRV_H);
    iS = int'(PRV_S);
    iU = int'(PRV_U);
    vb_xpp = 0;
    vb_pending = 0;
    vb_irq_ena = 0;
    vb_e_emux = 64'h0000000000000000;
    vb_e_imux = 16'h0000;
    wb_trap_cause = 0;
    vb_xtval = 64'h0000000000000000;
    w_mstackovr = 0;
    w_mstackund = 0;
    v_csr_rena = 0;
    v_csr_wena = 0;
    v_csr_trapreturn = 0;
    vb_rdata = 0;
    v_req_halt = 0;
    v_req_resume = 0;
    v_req_progbuf = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    v_medeleg_ena = 0;
    v_mideleg_ena = 0;
    vb_xtvec_off_ideleg = 64'h0000000000000000;
    vb_xtvec_off_edeleg = 64'h0000000000000000;
    v_flushd = 0;
    v_flushi = 0;
    v_flushmmu = 0;
    v_flushpipeline = 0;
    vb_pmp_upd_ena = 0;
    vb_pmp_napot_mask = 0;
    v_napot_shift = 0;
    t_pmpdataidx = 0;
    t_pmpcfgidx = 0;

    for (int i = 0; i < 4; i++) begin
        v.xmode[i].xepc = r.xmode[i].xepc;
        v.xmode[i].xpp = r.xmode[i].xpp;
        v.xmode[i].xpie = r.xmode[i].xpie;
        v.xmode[i].xie = r.xmode[i].xie;
        v.xmode[i].xsie = r.xmode[i].xsie;
        v.xmode[i].xtie = r.xmode[i].xtie;
        v.xmode[i].xeie = r.xmode[i].xeie;
        v.xmode[i].xtvec_off = r.xmode[i].xtvec_off;
        v.xmode[i].xtvec_mode = r.xmode[i].xtvec_mode;
        v.xmode[i].xtval = r.xmode[i].xtval;
        v.xmode[i].xcause_irq = r.xmode[i].xcause_irq;
        v.xmode[i].xcause_code = r.xmode[i].xcause_code;
        v.xmode[i].xscratch = r.xmode[i].xscratch;
        v.xmode[i].xcounteren = r.xmode[i].xcounteren;
    end
    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
        v.pmp[i].cfg = r.pmp[i].cfg;
        v.pmp[i].addr = r.pmp[i].addr;
        v.pmp[i].mask = r.pmp[i].mask;
    end
    v.state = r.state;
    v.fencestate = r.fencestate;
    v.irq_pending = r.irq_pending;
    v.cmd_type = r.cmd_type;
    v.cmd_addr = r.cmd_addr;
    v.cmd_data = r.cmd_data;
    v.cmd_exception = r.cmd_exception;
    v.progbuf_end = r.progbuf_end;
    v.progbuf_err = r.progbuf_err;
    v.mip_ssip = r.mip_ssip;
    v.mip_stip = r.mip_stip;
    v.mip_seip = r.mip_seip;
    v.medeleg = r.medeleg;
    v.mideleg = r.mideleg;
    v.mcountinhibit = r.mcountinhibit;
    v.mstackovr = r.mstackovr;
    v.mstackund = r.mstackund;
    v.mmu_ena = r.mmu_ena;
    v.satp_ppn = r.satp_ppn;
    v.satp_sv39 = r.satp_sv39;
    v.satp_sv48 = r.satp_sv48;
    v.mode = r.mode;
    v.mprv = r.mprv;
    v.mxr = r.mxr;
    v.sum = r.sum;
    v.tvm = r.tvm;
    v.ex_fpu_invalidop = r.ex_fpu_invalidop;
    v.ex_fpu_divbyzero = r.ex_fpu_divbyzero;
    v.ex_fpu_overflow = r.ex_fpu_overflow;
    v.ex_fpu_underflow = r.ex_fpu_underflow;
    v.ex_fpu_inexact = r.ex_fpu_inexact;
    v.trap_addr = r.trap_addr;
    v.mcycle_cnt = r.mcycle_cnt;
    v.minstret_cnt = r.minstret_cnt;
    v.dscratch0 = r.dscratch0;
    v.dscratch1 = r.dscratch1;
    v.dpc = r.dpc;
    v.halt_cause = r.halt_cause;
    v.dcsr_ebreakm = r.dcsr_ebreakm;
    v.dcsr_stopcount = r.dcsr_stopcount;
    v.dcsr_stoptimer = r.dcsr_stoptimer;
    v.dcsr_step = r.dcsr_step;
    v.dcsr_stepie = r.dcsr_stepie;
    v.stepping_mode_cnt = r.stepping_mode_cnt;
    v.ins_per_step = r.ins_per_step;
    v.pmp_upd_ena = r.pmp_upd_ena;
    v.pmp_upd_cnt = r.pmp_upd_cnt;
    v.pmp_ena = r.pmp_ena;
    v.pmp_we = r.pmp_we;
    v.pmp_region = r.pmp_region;
    v.pmp_start_addr = r.pmp_start_addr;
    v.pmp_end_addr = r.pmp_end_addr;
    v.pmp_flags = r.pmp_flags;

    vb_xpp = r.xmode[int'(r.mode)].xpp;
    vb_pmp_upd_ena = r.pmp_upd_ena;
    t_pmpdataidx = (int'(r.cmd_addr) - 32'h000003b0);
    t_pmpcfgidx = (8 * int'(r.cmd_addr[3: 1]));
    vb_pmp_napot_mask = 32'h00000007;

    vb_xtvec_off_edeleg = r.xmode[iM].xtvec_off;
    if ((r.mode <= PRV_S) && (r.medeleg[int'(r.cmd_addr[4: 0])] == 1'b1)) begin
        // Exception delegation to S-mode
        v_medeleg_ena = 1'b1;
        vb_xtvec_off_edeleg = r.xmode[iS].xtvec_off;
    end

    vb_xtvec_off_ideleg = r.xmode[iM].xtvec_off;
    if ((r.mode <= PRV_S) && (r.mideleg[int'(r.cmd_addr[3: 0])] == 1'b1)) begin
        // Interrupt delegation to S-mode
        v_mideleg_ena = 1'b1;
        vb_xtvec_off_ideleg = r.xmode[iS].xtvec_off;
    end

    case (r.state)
    State_Idle: begin
        v.progbuf_end = 1'b0;
        v.progbuf_err = 1'b0;
        v_req_ready = 1'b1;
        if (i_req_valid == 1'b1) begin
            v.cmd_type = i_req_type;
            v.cmd_addr = i_req_addr;
            v.cmd_data = i_req_data;
            v.cmd_exception = 1'b0;
            if (i_req_type[CsrReq_ExceptionBit] == 1'b1) begin
                v.state = State_Exception;
                if (i_req_addr == EXCEPTION_CallFromXMode) begin
                    v.cmd_addr = (i_req_addr + r.mode);
                end
            end else if (i_req_type[CsrReq_BreakpointBit] == 1'b1) begin
                v.state = State_Breakpoint;
            end else if (i_req_type[CsrReq_HaltBit] == 1'b1) begin
                v.state = State_Halt;
            end else if (i_req_type[CsrReq_ResumeBit] == 1'b1) begin
                v.state = State_Resume;
            end else if (i_req_type[CsrReq_InterruptBit] == 1'b1) begin
                v.state = State_Interrupt;
            end else if (i_req_type[CsrReq_TrapReturnBit] == 1'b1) begin
                v.state = State_TrapReturn;
            end else if (i_req_type[CsrReq_WfiBit] == 1'b1) begin
                v.state = State_Wfi;
            end else if (i_req_type[CsrReq_FenceBit] == 1'b1) begin
                v.state = State_Fence;
                if (i_req_addr[0] == 1'b1) begin
                    // FENCE
                    v.fencestate = Fence_DataBarrier;
                end else if (i_req_addr[1] == 1'b1) begin
                    // FENCE.I
                    v_flushmmu = 1'b1;
                    v.fencestate = Fence_DataFlush;
                end else if ((i_req_addr[2] == 1'b1)
                            && (~((r.tvm == 1'b1) && (r.mode[1] == 1'b0)))) begin
                    // FENCE.VMA: is illegal in S-mode when TVM bit=1
                    v_flushmmu = 1'b1;
                    v.fencestate = Fence_End;
                end else begin
                    // Illegal fence
                    v.state = State_Response;
                    v.cmd_exception = 1'b1;
                end
            end else begin
                v.state = State_RW;
            end
        end
    end
    State_Exception: begin
        v.state = State_Response;
        vb_e_emux[int'(r.cmd_addr[4: 0])] = 1'b1;
        vb_xtval = r.cmd_data;
        wb_trap_cause = r.cmd_addr[4: 0];
        v.cmd_data = vb_xtvec_off_edeleg;
        if (i_dbg_progbuf_ena == 1'b1) begin
            v.progbuf_err = 1'b1;
            v.progbuf_end = 1'b1;
            v.cmd_exception = 1'b1;
        end
    end
    State_Breakpoint: begin                                 // software breakpoint
        v.state = State_Response;
        if (i_dbg_progbuf_ena == 1'b1) begin
            // do not modify halt cause in debug mode
            v.progbuf_end = 1'b1;
            v.cmd_data = '1;                                // signal to executor to switch into Debug Mode and halt
        end else if (r.dcsr_ebreakm == 1'b1) begin
            v.halt_cause = HALT_CAUSE_EBREAK;
            v.dpc = r.cmd_data;
            v.cmd_data = '1;                                // signal to executor to switch into Debug Mode and halt
        end else begin
            vb_e_emux[EXCEPTION_Breakpoint] = 1'b1;
            wb_trap_cause = r.cmd_addr[4: 0];
            vb_xtval = i_e_pc;
            v.cmd_data = vb_xtvec_off_edeleg;               // Jump to exception handler
        end
    end
    State_Halt: begin
        v.state = State_Response;
        v.halt_cause = r.cmd_addr[2: 0];                    // Halt Request or Step done
        v.dpc = i_e_pc;
    end
    State_Resume: begin
        v.state = State_Response;
        if (i_dbg_progbuf_ena == 1'b1) begin
            v.cmd_data = '0;
        end else begin
            v.cmd_data = {'0, r.dpc};
        end
    end
    State_Interrupt: begin
        v.state = State_Response;
        vb_e_imux[int'(r.cmd_addr[3: 0])] = 1'b1;
        wb_trap_cause = r.cmd_addr[4: 0];
        v.cmd_data = vb_xtvec_off_ideleg;
        if (r.xmode[int'(r.mode)].xtvec_mode == 2'h1) begin
            // vectorized
            v.cmd_data = (vb_xtvec_off_ideleg + {wb_trap_cause, 2'h0});
        end
    end
    State_TrapReturn: begin
        v.state = State_Response;
        v_csr_trapreturn = 1'b1;
        v.cmd_data = {'0, r.xmode[int'(r.mode)].xepc};
    end
    State_RW: begin
        v.state = State_Response;
        // csr[9:8] encode the loweset priviledge level that can access to CSR
        // csr[11:10] register is read/write (00, 01 or 10) or read-only (11)
        if (r.mode < r.cmd_addr[9: 8]) begin
            // Not enough priv to access this register
            v.cmd_exception = 1'b1;
        end else begin
            v_csr_rena = r.cmd_type[CsrReq_ReadBit];
            v_csr_wena = r.cmd_type[CsrReq_WriteBit];
            if (r.cmd_addr[11: 4] == 8'h3a) begin           // pmpcfgx
                v.state = State_WaitPmp;
            end
        end
        // All operation into CSR implemented through the Read-Modify-Write
        // and we cannot generate exception on write access into read-only regs
        // So do not check bits csr[11:10], otherwise always will be the exception.
    end
    State_Wfi: begin
        v.state = State_Response;
        v.cmd_data = '0;                                    // no error, valid for all mdoes
    end
    State_Fence: begin
        if (r.fencestate == Fence_End) begin
            v.cmd_data = '0;
            v.state = State_Response;
            v.fencestate = Fence_None;
        end
    end
    State_WaitPmp: begin
        if ((|r.pmp_upd_ena) == 1'b0) begin
            v.state = State_Response;
        end
    end
    State_Response: begin
        v_resp_valid = 1'b1;
        if (i_resp_ready == 1'b1) begin
            v.state = State_Idle;
        end
    end
    default: begin
    end
    endcase

    // Caches flushing state machine
    case (r.fencestate)
    Fence_None: begin
    end
    Fence_DataBarrier: begin
        if (i_m_idle == 1'b1) begin
            v.fencestate = Fence_End;
        end
    end
    Fence_DataFlush: begin
        v_flushd = 1'b1;
        if (i_m_memop_ready == 1'b1) begin
            v.fencestate = Fence_WaitDataFlushEnd;
        end
    end
    Fence_WaitDataFlushEnd: begin
        if (i_flushd_end == 1'b1) begin
            v.fencestate = Fence_FlushInstr;
        end
    end
    Fence_FlushInstr: begin
        v_flushi = 1'b1;
        if (i_f_flush_ready == 1'b1) begin
            v.fencestate = Fence_End;
        end
    end
    Fence_End: begin
        v_flushpipeline = 1'b1;
    end
    default: begin
    end
    endcase

    // CSR registers. Priviledge Arch. V20211203, page 9(19):
    //     CSR[11:10] indicate whether the register is read/write (00, 01, or 10) or read-only (11)
    //     CSR[9:8] encode the lowest privilege level that can access the CSR
    if (r.cmd_addr == 12'h000) begin                        // ustatus: [URW] User status register
    end else if (r.cmd_addr == 12'h004) begin               // uie: [URW] User interrupt-enable register
    end else if (r.cmd_addr == 12'h005) begin               // ustatus: [URW] User trap handler base address
    end else if (r.cmd_addr == 12'h040) begin               // uscratch: [URW] Scratch register for user trap handlers
    end else if (r.cmd_addr == 12'h041) begin               // uepc: [URW] User exception program counter
        vb_rdata = r.xmode[iU].xepc;
        if (v_csr_wena) begin
            v.xmode[iU].xepc = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h042) begin               // ucause: [URW] User trap cause
    end else if (r.cmd_addr == 12'h043) begin               // utval: [URW] User bad address or instruction
    end else if (r.cmd_addr == 12'h044) begin               // uip: [URW] User interrupt pending
    end else if (r.cmd_addr == 12'h001) begin               // fflags: [URW] Floating-Point Accrued Exceptions
        vb_rdata[0] = r.ex_fpu_inexact;
        vb_rdata[1] = r.ex_fpu_underflow;
        vb_rdata[2] = r.ex_fpu_overflow;
        vb_rdata[3] = r.ex_fpu_divbyzero;
        vb_rdata[4] = r.ex_fpu_invalidop;
    end else if (r.cmd_addr == 12'h002) begin               // fflags: [URW] Floating-Point Dynamic Rounding Mode
        if (CFG_HW_FPU_ENABLE) begin
            vb_rdata[2: 0] = 3'h4;                          // Round mode: round to Nearest (RMM)
        end
    end else if (r.cmd_addr == 12'h003) begin               // fcsr: [URW] Floating-Point Control and Status Register (frm + fflags)
        vb_rdata[0] = r.ex_fpu_inexact;
        vb_rdata[1] = r.ex_fpu_underflow;
        vb_rdata[2] = r.ex_fpu_overflow;
        vb_rdata[3] = r.ex_fpu_divbyzero;
        vb_rdata[4] = r.ex_fpu_invalidop;
        if (CFG_HW_FPU_ENABLE) begin
            vb_rdata[7: 5] = 3'h4;                          // Round mode: round to Nearest (RMM)
        end
    end else if (r.cmd_addr == 12'hc00) begin               // cycle: [URO] User Cycle counter for RDCYCLE pseudo-instruction
        if ((r.mode == PRV_U)
                && ((r.xmode[iM].xcounteren[0] == 1'b0)
                        || (r.xmode[iS].xcounteren[0] == 1'b0))) begin
            // Available only if all more prv. bit CY are set
            v.cmd_exception = 1'b1;
        end else if ((r.mode == PRV_S)
                    && (r.xmode[iM].xcounteren[0] == 1'b0)) begin
            // Available only if bit CY is set
            v.cmd_exception = 1'b1;
        end else begin
            vb_rdata = r.mcycle_cnt;                        // Read-only shadows of mcycle
        end
    end else if (r.cmd_addr == 12'hc01) begin               // time: [URO] User Timer for RDTIME pseudo-instruction
        if ((r.mode == PRV_U)
                && ((r.xmode[iM].xcounteren[1] == 1'b0)
                        || (r.xmode[iS].xcounteren[1] == 1'b0))) begin
            // Available only if all more prv. bit TM are set
            v.cmd_exception = 1'b1;
        end else if ((r.mode == PRV_S)
                    && (r.xmode[iM].xcounteren[1] == 1'b0)) begin
            // Available only if bit TM is set
            v.cmd_exception = 1'b1;
        end else begin
            vb_rdata = i_mtimer;
        end
    end else if (r.cmd_addr == 12'hc03) begin               // insret: [URO] User Instructions-retired counter for RDINSTRET pseudo-instruction
        if ((r.mode == PRV_U)
                && ((r.xmode[iM].xcounteren[2] == 1'b0)
                        || (r.xmode[iS].xcounteren[2] == 1'b0))) begin
            // Available only if all more prv. bit IR are set
            v.cmd_exception = 1'b1;
        end else if ((r.mode == PRV_S)
                    && (r.xmode[iM].xcounteren[2] == 1'b0)) begin
            // Available only if bit IR is set
            v.cmd_exception = 1'b1;
        end else begin
            vb_rdata = r.minstret_cnt;                      // Read-only shadow of minstret
        end
    end else if (r.cmd_addr == 12'h100) begin               // sstatus: [SRW] Supervisor status register
        // [0] WPRI
        vb_rdata[1] = r.xmode[iS].xie;
        // [4:2] WPRI
        vb_rdata[5] = r.xmode[iS].xpie;
        // [6] UBE: Endianess: 0=little-endian; 1=big-endian. Instruction fetch is always little endian
        // [7] WPRI
        vb_rdata[8] = r.xmode[iS].xpp[0];                   // SPP can have onle 0 or 1 values, so 1 bit only
        // [10:9] VS
        // [12:11] WPRI
        if (CFG_HW_FPU_ENABLE) begin
            vb_rdata[14: 13] = 2'h1;                        // FS field: Initial state
        end
        // [16:15] XS
        // [17] WPRI
        // [18] SUM
        // [19] MXR
        // [31:20] WPRI
        vb_rdata[33: 32] = 2'h2;                            // UXL: User is 64-bits
        // [62:34] WPRI
        // [63] SD. Read-only bit that summize FS, VS or XS fields
        if (v_csr_wena == 1'b1) begin
            v.xmode[iS].xie = r.cmd_data[1];
            v.xmode[iS].xpie = r.cmd_data[5];
            v.xmode[iS].xpp = {1'h0, r.cmd_data[8]};
        end
    end else if (r.cmd_addr == 12'h104) begin               // sie: [SRW] Supervisor interrupt-enable register
        vb_rdata[(IRQ_SSIP - 1)] = r.xmode[iS].xsie;
        vb_rdata[(IRQ_STIP - 1)] = r.xmode[iS].xtie;
        vb_rdata[(IRQ_SEIP - 1)] = r.xmode[iS].xeie;
        if (v_csr_wena) begin
            // Write only supported interrupts
            v.xmode[iS].xsie = r.cmd_data[IRQ_SSIP];
            v.xmode[iS].xtie = r.cmd_data[IRQ_STIP];
            v.xmode[iS].xeie = r.cmd_data[IRQ_SEIP];
        end
    end else if (r.cmd_addr == 12'h105) begin               // stvec: [SRW] Supervisor trap handler base address
        vb_rdata = r.xmode[iS].xtvec_off;
        vb_rdata[1: 0] = r.xmode[iS].xtvec_mode;
        if (v_csr_wena == 1'b1) begin
            v.xmode[iS].xtvec_off = {r.cmd_data[(RISCV_ARCH - 1): 2], 2'h0};
            v.xmode[iS].xtvec_mode = r.cmd_data[1: 0];
        end
    end else if (r.cmd_addr == 12'h106) begin               // scounteren: [SRW] Supervisor counter enable
        vb_rdata = r.xmode[iS].xcounteren;
        if (v_csr_wena == 1'b1) begin
            v.xmode[iS].xcounteren = r.cmd_data[15: 0];
        end
    end else if (r.cmd_addr == 12'h10a) begin               // senvcfg: [SRW] Supervisor environment configuration register
    end else if (r.cmd_addr == 12'h140) begin               // sscratch: [SRW] Supervisor register for supervisor trap handlers
        vb_rdata = r.xmode[iS].xscratch;
        if (v_csr_wena) begin
            v.xmode[iS].xscratch = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h141) begin               // sepc: [SRW] Supervisor exception program counter
        vb_rdata = r.xmode[iS].xepc;
        if (v_csr_wena) begin
            v.xmode[iS].xepc = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h142) begin               // scause: [SRW] Supervisor trap cause
        vb_rdata[63] = r.xmode[iS].xcause_irq;
        vb_rdata[4: 0] = r.xmode[iS].xcause_code;
    end else if (r.cmd_addr == 12'h143) begin               // stval: [SRW] Supervisor bad address or instruction
        vb_rdata = r.xmode[iS].xtval;
        if (v_csr_wena) begin
            v.xmode[iS].xtval = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h144) begin               // sip: [SRW] Supervisor interrupt pending
        vb_rdata[15: 0] = (r.irq_pending & 16'h0222);       // see fig 4.7. Only s-bits are visible
        if (v_csr_wena) begin
            v.mip_ssip = r.cmd_data[IRQ_SSIP];
            v.mip_stip = r.cmd_data[IRQ_STIP];
            v.mip_seip = r.cmd_data[IRQ_SEIP];
        end
    end else if (r.cmd_addr == 12'h180) begin               // satp: [SRW] Supervisor address translation and protection
        // Writing unssoprted MODE[63:60], entire write has no effect
        //     MODE = 0 Bare. No translation or protection
        //     MODE = 9 Sv48. Page based 48-bit virtual addressing
        if ((r.tvm == 1'b1) && (r.mode == PRV_S)) begin
            // SATP is illegal in S-mode when TVM=1
            v.cmd_exception = 1'b1;
        end else begin
            vb_rdata[43: 0] = r.satp_ppn;
            if (r.satp_sv39 == 1'b1) begin
                vb_rdata[63: 60] = SATP_MODE_SV39;
            end else if (r.satp_sv48 == 1'b1) begin
                vb_rdata[63: 60] = SATP_MODE_SV48;
            end
            if (v_csr_wena == 1'b1) begin
                v.satp_ppn = r.cmd_data[43: 0];
                v.satp_sv39 = 1'b0;
                v.satp_sv48 = 1'b0;
                if (r.cmd_data[63: 60] == SATP_MODE_SV39) begin
                    v.satp_sv39 = 1'b1;
                end else if (r.cmd_data[63: 60] == SATP_MODE_SV48) begin
                    v.satp_sv48 = 1'b1;
                end
            end
        end
    end else if (r.cmd_addr == 12'h5a8) begin               // scontext: [SRW] Supervisor-mode context register
    end else if (r.cmd_addr == 12'hf11) begin               // mvendorid: [MRO] Vendor ID
        vb_rdata = CFG_VENDOR_ID;
    end else if (r.cmd_addr == 12'hf12) begin               // marchid: [MRO] Architecture ID
    end else if (r.cmd_addr == 12'hf13) begin               // mimplementationid: [MRO] Implementation ID
        vb_rdata = CFG_IMPLEMENTATION_ID;
    end else if (r.cmd_addr == 12'hf14) begin               // mhartid: [MRO] Hardware thread ID
        vb_rdata[63: 0] = hartid;
    end else if (r.cmd_addr == 12'hf15) begin               // mconfigptr: [MRO] Pointer to configuration data structure
    end else if (r.cmd_addr == 12'h300) begin               // mstatus: [MRW] Machine mode status register
        // [0] WPRI
        vb_rdata[1] = r.xmode[iS].xie;
        // [2] WPRI
        vb_rdata[3] = r.xmode[iM].xie;
        // [4] WPRI
        vb_rdata[5] = r.xmode[iS].xpie;
        // [6] UBE: Endianess: 0=little-endian; 1=big-endian. Instruction fetch is always little endian
        vb_rdata[7] = r.xmode[iM].xpie;
        vb_rdata[8] = r.xmode[iS].xpp[0];                   // SPP can have onle 0 or 1 values, so 1 bit only
        // [10:9] VS
        vb_rdata[12: 11] = r.xmode[iM].xpp;
        if (CFG_HW_FPU_ENABLE) begin
            vb_rdata[14: 13] = 2'h1;                        // FS field: Initial state
        end
        // [16:15] XS
        vb_rdata[17] = r.mprv;
        vb_rdata[18] = r.sum;
        vb_rdata[19] = r.mxr;
        vb_rdata[20] = r.tvm;                               // Trap Virtual Memory
        // [21] TW
        // [22] TSR
        // [31:23] WPRI
        vb_rdata[33: 32] = 2'h2;                            // UXL: User is 64-bits
        vb_rdata[35: 34] = 2'h2;                            // SXL: Supervisor is 64-bits
        // [36] SBE
        // [37] MBE
        // [62:38] WPRI
        // [63] SD. Read-only bit that summize FS, VS or XS fields
        if (v_csr_wena == 1'b1) begin
            v.xmode[iS].xie = r.cmd_data[1];
            v.xmode[iM].xie = r.cmd_data[3];
            v.xmode[iS].xpie = r.cmd_data[5];
            v.xmode[iM].xpie = r.cmd_data[7];
            v.xmode[iS].xpp = {1'h0, r.cmd_data[8]};
            v.xmode[iM].xpp = r.cmd_data[12: 11];
            v.mprv = r.cmd_data[17];
            v.sum = r.cmd_data[18];
            v.mxr = r.cmd_data[19];
            v.tvm = r.cmd_data[20];
        end
    end else if (r.cmd_addr == 12'h301) begin               // misa: [MRW] ISA and extensions
        // Base[XLEN-1:XLEN-2]
        //      1 = 32
        //      2 = 64
        //      3 = 128

        vb_rdata[(RISCV_ARCH - 1): (RISCV_ARCH - 2)] = 2'h2;
        // BitCharacterDescription
        //      0  A Atomic extension
        //      1  B Tentatively reserved for Bit operations extension
        //      2  C Compressed extension
        //      3  D Double-precision Foating-point extension
        //      4  E RV32E base ISA (embedded)
        //      5  F Single-precision Foating-point extension
        //      6  G Additional standard extensions present
        //      7  H Hypervisor mode implemented
        //      8  I RV32I/64I/128I base ISA
        //      9  J Reserved
        //      10 K Reserved
        //      11 L Tentatively reserved for Decimal Floating-Point extension
        //      12 M Integer Multiply/Divide extension
        //      13 N User-level interrupts supported
        //      14 O Reserved
        //      15 P Tentatively reserved for Packed-SIMD extension
        //      16 Q Quad-precision Foating-point extension
        //      17 R Reserved
        //      18 S Supervisor mode implemented
        //      19 T Tentatively reserved for Transactional Memory extension
        //      20 U User mode implemented
        //      21 V Tentatively reserved for Vector extension
        //      22 W Reserved
        //      23 X Non-standard extensions present
        //      24 Y Reserved
        //      25 Z Reserve
        vb_rdata[0] = 1'b1;                                 // A-extension
        vb_rdata[8] = 1'b1;                                 // I-extension
        vb_rdata[12] = 1'b1;                                // M-extension
        vb_rdata[18] = 1'b1;                                // S-extension
        vb_rdata[20] = 1'b1;                                // U-extension
        vb_rdata[2] = 1'b1;                                 // C-extension
        if (CFG_HW_FPU_ENABLE) begin
            vb_rdata[3] = 1'b1;                             // D-extension
        end
    end else if (r.cmd_addr == 12'h302) begin               // medeleg: [MRW] Machine exception delegation
        vb_rdata = r.medeleg;
        if (v_csr_wena) begin
            // page 31. Read-only zero for exceptions that could not be delegated, especially Call from M-mode
            v.medeleg = (r.cmd_data & 64'h000000000000b3ff);
        end
    end else if (r.cmd_addr == 12'h303) begin               // mideleg: [MRW] Machine interrupt delegation
        vb_rdata = r.mideleg;
        if (v_csr_wena) begin
            // No need to delegate machine interrupts to supervisor (but possible)
            v.mideleg = (r.cmd_data[(IRQ_TOTAL - 1): 0] & 12'h222);
        end
    end else if (r.cmd_addr == 12'h304) begin               // mie: [MRW] Machine interrupt enable bit
        vb_rdata[(IRQ_SSIP - 1)] = r.xmode[iS].xsie;
        vb_rdata[(IRQ_MSIP - 1)] = r.xmode[iM].xsie;
        vb_rdata[(IRQ_STIP - 1)] = r.xmode[iS].xtie;
        vb_rdata[(IRQ_MTIP - 1)] = r.xmode[iM].xtie;
        vb_rdata[(IRQ_SEIP - 1)] = r.xmode[iS].xeie;
        vb_rdata[(IRQ_MEIP - 1)] = r.xmode[iM].xeie;
        if (v_csr_wena) begin
            // Write only supported interrupts
            v.xmode[iS].xsie = r.cmd_data[IRQ_SSIP];
            v.xmode[iM].xsie = r.cmd_data[IRQ_MSIP];
            v.xmode[iS].xtie = r.cmd_data[IRQ_STIP];
            v.xmode[iM].xtie = r.cmd_data[IRQ_MTIP];
            v.xmode[iS].xeie = r.cmd_data[IRQ_SEIP];
            v.xmode[iM].xeie = r.cmd_data[IRQ_MEIP];
        end
    end else if (r.cmd_addr == 12'h305) begin               // mtvec: [MRW] Machine trap-handler base address
        vb_rdata = r.xmode[iM].xtvec_off;
        vb_rdata[1: 0] = r.xmode[iM].xtvec_mode;
        if (v_csr_wena == 1'b1) begin
            v.xmode[iM].xtvec_off = {r.cmd_data[(RISCV_ARCH - 1): 2], 2'h0};
            v.xmode[iM].xtvec_mode = r.cmd_data[1: 0];
        end
    end else if (r.cmd_addr == 12'h306) begin               // mcounteren: [MRW] Machine counter enable
        vb_rdata = r.xmode[iM].xcounteren;
        if (v_csr_wena == 1'b1) begin
            v.xmode[iM].xcounteren = r.cmd_data[15: 0];
        end
    end else if (r.cmd_addr == 12'h340) begin               // mscratch: [MRW] Machine scratch register
        vb_rdata = r.xmode[iM].xscratch;
        if (v_csr_wena) begin
            v.xmode[iM].xscratch = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h341) begin               // mepc: [MRW] Machine program counter
        vb_rdata = r.xmode[iM].xepc;
        if (v_csr_wena) begin
            v.xmode[iM].xepc = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h342) begin               // mcause: [MRW] Machine trap cause
        vb_rdata[63] = r.xmode[iM].xcause_irq;
        vb_rdata[4: 0] = r.xmode[iM].xcause_code;
    end else if (r.cmd_addr == 12'h343) begin               // mtval: [MRW] Machine bad address or instruction
        vb_rdata = r.xmode[iM].xtval;
        if (v_csr_wena) begin
            v.xmode[iM].xtval = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h344) begin               // mip: [MRW] Machine interrupt pending
        vb_rdata[(IRQ_TOTAL - 1): 0] = r.irq_pending;
        if (v_csr_wena) begin
            v.mip_ssip = r.cmd_data[IRQ_SSIP];
            v.mip_stip = r.cmd_data[IRQ_STIP];
            v.mip_seip = r.cmd_data[IRQ_SEIP];
        end
    end else if (r.cmd_addr == 12'h34a) begin               // mtinst: [MRW] Machine trap instruction (transformed)
    end else if (r.cmd_addr == 12'h34b) begin               // mtval2: [MRW] Machine bad guest physical register
    end else if (r.cmd_addr == 12'h30a) begin               // menvcfg: [MRW] Machine environment configuration register
    end else if (r.cmd_addr == 12'h747) begin               // mseccfg: [MRW] Machine security configuration register
    end else if (r.cmd_addr[11: 4] == 8'h3a) begin          // pmpcfg0..63: [MRW] Physical memory protection configuration
        if (r.cmd_addr[0] == 1'b1) begin
            v.cmd_exception = 1'b1;                         // RV32 only
        end else if (t_pmpcfgidx < CFG_PMP_TBL_SIZE) begin
            for (int i = 0; i < 8; i++) begin
                vb_rdata[(8 * i) +: 8] = r.pmp[(t_pmpcfgidx + i)].cfg;
                if ((v_csr_wena == 1'b1)
                        && (r.pmp[(t_pmpcfgidx + i)].cfg[7] == 1'b0)) begin
                    // [7] Bit L = locked cannot be modified upto reset
                    v.pmp[(t_pmpcfgidx + i)].cfg = r.cmd_data[(8 * i) +: 8];
                    vb_pmp_upd_ena[(t_pmpcfgidx + i)] = 1'b1;
                end
            end
        end
    end else if ((r.cmd_addr >= 12'h3b0)
                && (r.cmd_addr <= 12'h3ef)) begin
        // pmpaddr0..63: [MRW] Physical memory protection address register
        for (int i = 0; i < (RISCV_ARCH - 2); i++) begin
            if ((r.cmd_data[i] == 1'b1) && (v_napot_shift == 1'b0)) begin
                vb_pmp_napot_mask = {vb_pmp_napot_mask, 1'h1};
            end else begin
                v_napot_shift = 1'b1;
            end
        end
        if (t_pmpdataidx < CFG_PMP_TBL_SIZE) begin
            vb_rdata[(RISCV_ARCH - 3): 0] = r.pmp[t_pmpdataidx].addr;
            if ((v_csr_wena == 1'b1)
                    && (r.pmp[t_pmpdataidx].cfg[7] == 1'b0)) begin
                v.pmp[t_pmpdataidx].addr = {r.cmd_data[(RISCV_ARCH - 3): 0], 2'h0};
                v.pmp[t_pmpdataidx].mask = vb_pmp_napot_mask;
            end
        end
    end else if (r.cmd_addr <= 12'h3ef) begin               // pmpaddr63: [MRW] Physical memory protection address register
    end else if (r.cmd_addr == 12'hb00) begin               // mcycle: [MRW] Machine cycle counter
        vb_rdata = r.mcycle_cnt;
        if (v_csr_wena) begin
            v.mcycle_cnt = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'hb02) begin               // minstret: [MRW] Machine instructions-retired counter
        vb_rdata = r.minstret_cnt;
        if (v_csr_wena) begin
            v.minstret_cnt = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h320) begin               // mcountinhibit: [MRW] Machine counter-inhibit register
        vb_rdata = r.mcountinhibit;
        if (v_csr_wena == 1'b1) begin
            v.mcountinhibit = r.cmd_data[15: 0];
        end
    end else if (r.cmd_addr == 12'h323) begin               // mpevent3: [MRW] Machine performance-monitoring event selector
    end else if (r.cmd_addr == 12'h324) begin               // mpevent4: [MRW] Machine performance-monitoring event selector
    end else if (r.cmd_addr == 12'h33f) begin               // mpevent31: [MRW] Machine performance-monitoring event selector
    end else if (r.cmd_addr == 12'h7a0) begin               // tselect: [MRW] Debug/Trace trigger register select
    end else if (r.cmd_addr == 12'h7a1) begin               // tdata1: [MRW] First Debug/Trace trigger data register
    end else if (r.cmd_addr == 12'h7a2) begin               // tdata2: [MRW] Second Debug/Trace trigger data register
    end else if (r.cmd_addr == 12'h7a3) begin               // tdata3: [MRW] Third Debug/Trace trigger data register
    end else if (r.cmd_addr == 12'h7a8) begin               // mcontext: [MRW] Machine-mode context register
    end else if (r.cmd_addr == 12'h7b0) begin               // dcsr: [DRW] Debug control and status register
        vb_rdata[31: 28] = 4'h4;                            // xdebugver: 4=External debug supported
        vb_rdata[15] = r.dcsr_ebreakm;
        vb_rdata[11] = r.dcsr_stepie;                       // interrupt dis/ena during step
        vb_rdata[10] = r.dcsr_stopcount;                    // don't increment any counter
        vb_rdata[9] = r.dcsr_stoptimer;                     // don't increment timer
        vb_rdata[8: 6] = r.halt_cause;
        vb_rdata[2] = r.dcsr_step;
        vb_rdata[1: 0] = 2'h3;                              // prv: privilege in debug mode: 3=machine
        if (v_csr_wena == 1'b1) begin
            v.dcsr_ebreakm = r.cmd_data[15];
            v.dcsr_stepie = r.cmd_data[11];
            v.dcsr_stopcount = r.cmd_data[10];
            v.dcsr_stoptimer = r.cmd_data[9];
            v.dcsr_step = r.cmd_data[2];
        end
    end else if (r.cmd_addr == 12'h7b1) begin               // dpc: [DRW] Debug PC
        // Upon entry into debug mode DPC must contains:
        //        cause        |   Address
        // --------------------|----------------
        //  ebreak             |  Address of ebreak instruction
        //  single step        |  Address of next instruction to be executed
        //  trigger (HW BREAK) |  if timing=0, cause isntruction, if timing=1 enxt instruction
        //  halt request       |  next instruction

        if (i_e_halted == 1'b1) begin
            vb_rdata = r.dpc;
        end else begin
            // make visible current pc for the debugger even in running state
            vb_rdata = i_e_pc;
        end
        if (v_csr_wena == 1'b1) begin
            v.dpc = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h7b2) begin               // dscratch0: [DRW] Debug scratch register 0
        vb_rdata = r.dscratch0;
        if (v_csr_wena == 1'b1) begin
            v.dscratch0 = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'h7b3) begin               // dscratch1: [DRW] Debug scratch register 1
        vb_rdata = r.dscratch1;
        if (v_csr_wena) begin
            v.dscratch1 = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'hbc0) begin               // mstackovr: [MRW] Machine Stack Overflow
        vb_rdata = r.mstackovr;
        if (v_csr_wena == 1'b1) begin
            v.mstackovr = r.cmd_data;
        end
    end else if (r.cmd_addr == 12'hbc1) begin               // mstackund: [MRW] Machine Stack Underflow
        vb_rdata = r.mstackund;
        if (v_csr_wena == 1'b1) begin
            v.mstackund = r.cmd_data;
        end
    end else begin
        // Not implemented CSR:
        if (r.state == State_RW) begin
            v.cmd_exception = 1'b1;
        end
    end

    if (v_csr_rena == 1'b1) begin
        v.cmd_data = vb_rdata;
    end


    if (v_csr_trapreturn == 1'b1) begin
        if (r.mode == r.cmd_addr) begin
            v.mode = vb_xpp;
            v.xmode[int'(r.mode)].xpie = 1'h1;              // xPIE is set to 1 always, page 21
            v.xmode[int'(r.mode)].xpp = PRV_U;              // Set to least-privildged supported mode (U), page 21
        end else begin
            v.cmd_exception = 1'b1;                         // xret not matched to current mode
        end
        if (r.xmode[int'(r.mode)].xpp != PRV_M) begin       // see page 21
            v.mprv = 1'b0;
        end
    end

    // Check MMU:
    v.mmu_ena = 1'b0;
    if ((r.satp_sv39 == 1'b1) || (r.satp_sv48 == 1'b1)) begin// Sv39 and Sv48 are implemented
        if (r.mode[1] == 1'b0) begin
            // S and U modes
            v.mmu_ena = 1'b1;
            v_flushpipeline = (~r.mmu_ena);                 // Flush pipeline on MMU turning on
        end else if ((r.mprv == 1'b1) && (vb_xpp[1] == 1'b0)) begin
            // Previous state is S or U mode
            // Instruction address-translation and protection are unaffected
            v.mmu_ena = 1'b1;
            v_flushpipeline = (~r.mmu_ena);                 // Flush pipeline on MMU turning on
        end
    end

    if ((r.mode <= PRV_S)
            && ((vb_e_emux & r.medeleg) || (vb_e_imux & r.mideleg))) begin
        // Trap delegated to Supervisor mode
        v.xmode[iS].xpp = r.mode;
        v.xmode[iS].xpie = r.xmode[int'(r.mode)].xie;
        v.xmode[iS].xie = 1'h0;
        v.xmode[iS].xepc = i_e_pc;                          // current latched instruction not executed overwritten by exception/interrupt
        v.xmode[iS].xtval = vb_xtval;                       // trap value/bad address
        v.xmode[iS].xcause_code = wb_trap_cause;
        v.xmode[iS].xcause_irq = (~(|vb_e_emux));
        v.mode = PRV_S;
    end else if (((|vb_e_emux) == 1'b1) || ((|vb_e_imux) == 1'b1)) begin
        // By default all exceptions and interrupts handled in M-mode (not delegations)
        v.xmode[iM].xpp = r.mode;
        v.xmode[iM].xpie = r.xmode[int'(r.mode)].xie;
        v.xmode[iM].xie = 1'h0;
        v.xmode[iM].xepc = i_e_pc;                          // current latched instruction not executed overwritten by exception/interrupt
        v.xmode[iM].xtval = vb_xtval;                       // trap value/bad address
        v.xmode[iM].xcause_code = wb_trap_cause;
        v.xmode[iM].xcause_irq = (~(|vb_e_emux));
        v.mode = PRV_M;
        v.mmu_ena = 1'b0;
    end

    // Step is not enabled or interrupt enabled during stepping
    if (((~r.dcsr_step) || r.dcsr_stepie) == 1'b1) begin
        if (r.xmode[iM].xie) begin
            // ALL not-delegated interrupts
            vb_irq_ena = (~r.mideleg);
        end
        if (r.xmode[iS].xie) begin
            // Delegated to S-mode:
            vb_irq_ena = (vb_irq_ena | r.mideleg);
        end
    end

    // The following pending interrupt could be set in mip:
    vb_pending[IRQ_MSIP] = (i_irq_pending[IRQ_MSIP] && r.xmode[iM].xsie);
    vb_pending[IRQ_MTIP] = (i_irq_pending[IRQ_MTIP] && r.xmode[iM].xtie);
    vb_pending[IRQ_MEIP] = (i_irq_pending[IRQ_MEIP] && r.xmode[iM].xeie);
    vb_pending[IRQ_SSIP] = ((i_irq_pending[IRQ_SSIP] || r.mip_ssip) && r.xmode[iS].xsie);
    vb_pending[IRQ_STIP] = ((i_irq_pending[IRQ_STIP] || r.mip_stip) && r.xmode[iS].xtie);
    vb_pending[IRQ_SEIP] = ((i_irq_pending[IRQ_SEIP] || r.mip_seip) && r.xmode[iS].xeie);
    v.irq_pending = vb_pending;

    // Transmit data to MPU
    if ((|r.pmp_upd_ena) == 1'b1) begin
        vb_pmp_upd_ena[int'(r.pmp_upd_cnt)] = 1'b0;
        v.pmp_upd_cnt = (r.pmp_upd_cnt + 1);
        v.pmp_we = r.pmp_upd_ena[int'(r.pmp_upd_cnt)];
        v.pmp_region = r.pmp_upd_cnt;
        if (r.pmp[int'(r.pmp_upd_cnt)].cfg[4: 3] == 2'h0) begin
            // OFF: Null region (disabled)
            v.pmp_start_addr = '0;
            v.pmp_end_addr = r.pmp[int'(r.pmp_upd_cnt)].addr;
            v.pmp_flags = '0;
        end else if (r.pmp[int'(r.pmp_upd_cnt)].cfg[4: 3] == 2'h1) begin
            // TOR: Top of range
            if (r.pmp_end_addr[0] == 1'b1) begin
                v.pmp_start_addr = (r.pmp_end_addr + 1);
            end else begin
                v.pmp_start_addr = r.pmp_end_addr;
            end
            v.pmp_end_addr = (r.pmp[int'(r.pmp_upd_cnt)].addr - 1);
            v.pmp_flags = {1'h1, r.pmp[int'(r.pmp_upd_cnt)].cfg[7], r.pmp[int'(r.pmp_upd_cnt)].cfg[2: 0]};
        end else if (r.pmp[int'(r.pmp_upd_cnt)].cfg[4: 3] == 2'h2) begin
            // NA4: Naturally aligned four-bytes region
            v.pmp_start_addr = r.pmp[int'(r.pmp_upd_cnt)].addr;
            v.pmp_end_addr = (r.pmp[int'(r.pmp_upd_cnt)].addr | 2'h3);
            v.pmp_flags = {1'h1, r.pmp[int'(r.pmp_upd_cnt)].cfg[7], r.pmp[int'(r.pmp_upd_cnt)].cfg[2: 0]};
        end else begin
            // NAPOT: Naturally aligned power-of-two region, >=8 bytes
            v.pmp_start_addr = (r.pmp[int'(r.pmp_upd_cnt)].addr & (~r.pmp[int'(r.pmp_upd_cnt)].mask));
            v.pmp_end_addr = (r.pmp[int'(r.pmp_upd_cnt)].addr | r.pmp[int'(r.pmp_upd_cnt)].mask);
            v.pmp_flags = {1'h1, r.pmp[int'(r.pmp_upd_cnt)].cfg[7], r.pmp[int'(r.pmp_upd_cnt)].cfg[2: 0]};
        end
    end else begin
        v.pmp_upd_cnt = '0;
        v.pmp_start_addr = '0;
        v.pmp_end_addr = '0;
        v.pmp_flags = '0;
        v.pmp_we = 1'b0;
    end
    v.pmp_upd_ena = vb_pmp_upd_ena;
    v.pmp_ena = ((~r.mode[1]) | r.mprv);                    // S,U mode or MPRV is set

    w_mstackovr = 1'b0;
    if (((|r.mstackovr) == 1'b1) && (i_sp < r.mstackovr)) begin
        w_mstackovr = 1'b1;
        v.mstackovr = '0;
    end
    w_mstackund = 1'b0;
    if (((|r.mstackund) == 1'b1) && (i_sp > r.mstackund)) begin
        w_mstackund = 1'b1;
        v.mstackund = '0;
    end

    // if (i_fpu_valid.read()) {
    //     v.ex_fpu_invalidop = i_ex_fpu_invalidop.read();
    //     v.ex_fpu_divbyzero = i_ex_fpu_divbyzero.read();
    //     v.ex_fpu_overflow = i_ex_fpu_overflow.read();
    //     v.ex_fpu_underflow = i_ex_fpu_underflow.read();
    //     v.ex_fpu_inexact = i_ex_fpu_inexact.read();
    // }

    // stopcount: do not increment any counters including cycle and instret
    if ((i_e_halted == 1'b0) && (r.dcsr_stopcount == 1'b0) && (r.mcountinhibit[0] == 1'b0)) begin
        v.mcycle_cnt = (r.mcycle_cnt + 1);
    end
    if ((i_e_valid == 1'b1)
            && (r.dcsr_stopcount == 1'b0)
            && (i_dbg_progbuf_ena == 1'b0)
            && (r.mcountinhibit[2] == 1'b0)) begin
        v.minstret_cnt = (r.minstret_cnt + 1);
    end

    if (~async_reset && i_nrst == 1'b0) begin
        for (int i = 0; i < 4; i++) begin
            v.xmode[i].xepc = 64'h0000000000000000;
            v.xmode[i].xpp = 2'h0;
            v.xmode[i].xpie = 1'h0;
            v.xmode[i].xie = 1'h0;
            v.xmode[i].xsie = 1'h0;
            v.xmode[i].xtie = 1'h0;
            v.xmode[i].xeie = 1'h0;
            v.xmode[i].xtvec_off = 64'h0000000000000000;
            v.xmode[i].xtvec_mode = 2'h0;
            v.xmode[i].xtval = 64'h0000000000000000;
            v.xmode[i].xcause_irq = 1'h0;
            v.xmode[i].xcause_code = 5'h00;
            v.xmode[i].xscratch = 64'h0000000000000000;
            v.xmode[i].xcounteren = 32'h00000000;
        end
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
            v.pmp[i].cfg = 8'h00;
            v.pmp[i].addr = 64'h0000000000000000;
            v.pmp[i].mask = 64'h0000000000000000;
        end
        v.state = State_Idle;
        v.fencestate = Fence_None;
        v.irq_pending = 16'h0000;
        v.cmd_type = 10'h000;
        v.cmd_addr = 12'h000;
        v.cmd_data = 64'h0000000000000000;
        v.cmd_exception = 1'h0;
        v.progbuf_end = 1'h0;
        v.progbuf_err = 1'h0;
        v.mip_ssip = 1'h0;
        v.mip_stip = 1'h0;
        v.mip_seip = 1'h0;
        v.medeleg = 64'h0000000000000000;
        v.mideleg = 16'h0000;
        v.mcountinhibit = 32'h00000000;
        v.mstackovr = 64'h0000000000000000;
        v.mstackund = 64'h0000000000000000;
        v.mmu_ena = 1'h0;
        v.satp_ppn = 44'h00000000000;
        v.satp_sv39 = 1'h0;
        v.satp_sv48 = 1'h0;
        v.mode = PRV_M;
        v.mprv = 1'h0;
        v.mxr = 1'h0;
        v.sum = 1'h0;
        v.tvm = 1'h0;
        v.ex_fpu_invalidop = 1'h0;
        v.ex_fpu_divbyzero = 1'h0;
        v.ex_fpu_overflow = 1'h0;
        v.ex_fpu_underflow = 1'h0;
        v.ex_fpu_inexact = 1'h0;
        v.trap_addr = 64'h0000000000000000;
        v.mcycle_cnt = 64'h0000000000000000;
        v.minstret_cnt = 64'h0000000000000000;
        v.dscratch0 = 64'h0000000000000000;
        v.dscratch1 = 64'h0000000000000000;
        v.dpc = CFG_RESET_VECTOR;
        v.halt_cause = 3'h0;
        v.dcsr_ebreakm = 1'h0;
        v.dcsr_stopcount = 1'h0;
        v.dcsr_stoptimer = 1'h0;
        v.dcsr_step = 1'h0;
        v.dcsr_stepie = 1'h0;
        v.stepping_mode_cnt = 64'h0000000000000000;
        v.ins_per_step = 64'h0000000000000001;
        v.pmp_upd_ena = 8'h00;
        v.pmp_upd_cnt = 3'h0;
        v.pmp_ena = 1'h0;
        v.pmp_we = 1'h0;
        v.pmp_region = 3'h0;
        v.pmp_start_addr = 64'h0000000000000000;
        v.pmp_end_addr = 64'h0000000000000000;
        v.pmp_flags = 5'h00;
    end

    o_req_ready = v_req_ready;
    o_resp_valid = v_resp_valid;
    o_resp_data = r.cmd_data;
    o_resp_exception = r.cmd_exception;
    o_progbuf_end = (r.progbuf_end && i_resp_ready);
    o_progbuf_error = (r.progbuf_err && i_resp_ready);
    o_irq_pending = (r.irq_pending & vb_irq_ena);
    o_wakeup = (|r.irq_pending);
    o_stack_overflow = w_mstackovr;
    o_stack_underflow = w_mstackund;
    o_executed_cnt = r.minstret_cnt;
    o_pmp_ena = r.pmp_ena;                                  // S,U mode or MPRV is set
    o_pmp_we = r.pmp_we;
    o_pmp_region = r.pmp_region;
    o_pmp_start_addr = r.pmp_start_addr;
    o_pmp_end_addr = r.pmp_end_addr;
    o_pmp_flags = r.pmp_flags;
    o_mmu_ena = r.mmu_ena;
    o_mmu_sv39 = r.satp_sv39;
    o_mmu_sv48 = r.satp_sv48;
    o_mmu_ppn = r.satp_ppn;
    o_mprv = r.mprv;
    o_mxr = r.mxr;
    o_sum = r.sum;
    o_step = r.dcsr_step;
    o_flushd_valid = v_flushd;
    o_flushi_valid = v_flushi;
    o_flushmmu_valid = v_flushmmu;
    o_flushpipeline_valid = v_flushpipeline;
    o_flush_addr = r.cmd_data;

    for (int i = 0; i < 4; i++) begin
        rin.xmode[i].xepc = v.xmode[i].xepc;
        rin.xmode[i].xpp = v.xmode[i].xpp;
        rin.xmode[i].xpie = v.xmode[i].xpie;
        rin.xmode[i].xie = v.xmode[i].xie;
        rin.xmode[i].xsie = v.xmode[i].xsie;
        rin.xmode[i].xtie = v.xmode[i].xtie;
        rin.xmode[i].xeie = v.xmode[i].xeie;
        rin.xmode[i].xtvec_off = v.xmode[i].xtvec_off;
        rin.xmode[i].xtvec_mode = v.xmode[i].xtvec_mode;
        rin.xmode[i].xtval = v.xmode[i].xtval;
        rin.xmode[i].xcause_irq = v.xmode[i].xcause_irq;
        rin.xmode[i].xcause_code = v.xmode[i].xcause_code;
        rin.xmode[i].xscratch = v.xmode[i].xscratch;
        rin.xmode[i].xcounteren = v.xmode[i].xcounteren;
    end
    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
        rin.pmp[i].cfg = v.pmp[i].cfg;
        rin.pmp[i].addr = v.pmp[i].addr;
        rin.pmp[i].mask = v.pmp[i].mask;
    end
    rin.state = v.state;
    rin.fencestate = v.fencestate;
    rin.irq_pending = v.irq_pending;
    rin.cmd_type = v.cmd_type;
    rin.cmd_addr = v.cmd_addr;
    rin.cmd_data = v.cmd_data;
    rin.cmd_exception = v.cmd_exception;
    rin.progbuf_end = v.progbuf_end;
    rin.progbuf_err = v.progbuf_err;
    rin.mip_ssip = v.mip_ssip;
    rin.mip_stip = v.mip_stip;
    rin.mip_seip = v.mip_seip;
    rin.medeleg = v.medeleg;
    rin.mideleg = v.mideleg;
    rin.mcountinhibit = v.mcountinhibit;
    rin.mstackovr = v.mstackovr;
    rin.mstackund = v.mstackund;
    rin.mmu_ena = v.mmu_ena;
    rin.satp_ppn = v.satp_ppn;
    rin.satp_sv39 = v.satp_sv39;
    rin.satp_sv48 = v.satp_sv48;
    rin.mode = v.mode;
    rin.mprv = v.mprv;
    rin.mxr = v.mxr;
    rin.sum = v.sum;
    rin.tvm = v.tvm;
    rin.ex_fpu_invalidop = v.ex_fpu_invalidop;
    rin.ex_fpu_divbyzero = v.ex_fpu_divbyzero;
    rin.ex_fpu_overflow = v.ex_fpu_overflow;
    rin.ex_fpu_underflow = v.ex_fpu_underflow;
    rin.ex_fpu_inexact = v.ex_fpu_inexact;
    rin.trap_addr = v.trap_addr;
    rin.mcycle_cnt = v.mcycle_cnt;
    rin.minstret_cnt = v.minstret_cnt;
    rin.dscratch0 = v.dscratch0;
    rin.dscratch1 = v.dscratch1;
    rin.dpc = v.dpc;
    rin.halt_cause = v.halt_cause;
    rin.dcsr_ebreakm = v.dcsr_ebreakm;
    rin.dcsr_stopcount = v.dcsr_stopcount;
    rin.dcsr_stoptimer = v.dcsr_stoptimer;
    rin.dcsr_step = v.dcsr_step;
    rin.dcsr_stepie = v.dcsr_stepie;
    rin.stepping_mode_cnt = v.stepping_mode_cnt;
    rin.ins_per_step = v.ins_per_step;
    rin.pmp_upd_ena = v.pmp_upd_ena;
    rin.pmp_upd_cnt = v.pmp_upd_cnt;
    rin.pmp_ena = v.pmp_ena;
    rin.pmp_we = v.pmp_we;
    rin.pmp_region = v.pmp_region;
    rin.pmp_start_addr = v.pmp_start_addr;
    rin.pmp_end_addr = v.pmp_end_addr;
    rin.pmp_flags = v.pmp_flags;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < 4; i++) begin
                    r.xmode[i].xepc <= 64'h0000000000000000;
                    r.xmode[i].xpp <= 2'h0;
                    r.xmode[i].xpie <= 1'h0;
                    r.xmode[i].xie <= 1'h0;
                    r.xmode[i].xsie <= 1'h0;
                    r.xmode[i].xtie <= 1'h0;
                    r.xmode[i].xeie <= 1'h0;
                    r.xmode[i].xtvec_off <= 64'h0000000000000000;
                    r.xmode[i].xtvec_mode <= 2'h0;
                    r.xmode[i].xtval <= 64'h0000000000000000;
                    r.xmode[i].xcause_irq <= 1'h0;
                    r.xmode[i].xcause_code <= 5'h00;
                    r.xmode[i].xscratch <= 64'h0000000000000000;
                    r.xmode[i].xcounteren <= 32'h00000000;
                end
                for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                    r.pmp[i].cfg <= 8'h00;
                    r.pmp[i].addr <= 64'h0000000000000000;
                    r.pmp[i].mask <= 64'h0000000000000000;
                end
                r.state <= State_Idle;
                r.fencestate <= Fence_None;
                r.irq_pending <= 16'h0000;
                r.cmd_type <= 10'h000;
                r.cmd_addr <= 12'h000;
                r.cmd_data <= 64'h0000000000000000;
                r.cmd_exception <= 1'h0;
                r.progbuf_end <= 1'h0;
                r.progbuf_err <= 1'h0;
                r.mip_ssip <= 1'h0;
                r.mip_stip <= 1'h0;
                r.mip_seip <= 1'h0;
                r.medeleg <= 64'h0000000000000000;
                r.mideleg <= 16'h0000;
                r.mcountinhibit <= 32'h00000000;
                r.mstackovr <= 64'h0000000000000000;
                r.mstackund <= 64'h0000000000000000;
                r.mmu_ena <= 1'h0;
                r.satp_ppn <= 44'h00000000000;
                r.satp_sv39 <= 1'h0;
                r.satp_sv48 <= 1'h0;
                r.mode <= PRV_M;
                r.mprv <= 1'h0;
                r.mxr <= 1'h0;
                r.sum <= 1'h0;
                r.tvm <= 1'h0;
                r.ex_fpu_invalidop <= 1'h0;
                r.ex_fpu_divbyzero <= 1'h0;
                r.ex_fpu_overflow <= 1'h0;
                r.ex_fpu_underflow <= 1'h0;
                r.ex_fpu_inexact <= 1'h0;
                r.trap_addr <= 64'h0000000000000000;
                r.mcycle_cnt <= 64'h0000000000000000;
                r.minstret_cnt <= 64'h0000000000000000;
                r.dscratch0 <= 64'h0000000000000000;
                r.dscratch1 <= 64'h0000000000000000;
                r.dpc <= CFG_RESET_VECTOR;
                r.halt_cause <= 3'h0;
                r.dcsr_ebreakm <= 1'h0;
                r.dcsr_stopcount <= 1'h0;
                r.dcsr_stoptimer <= 1'h0;
                r.dcsr_step <= 1'h0;
                r.dcsr_stepie <= 1'h0;
                r.stepping_mode_cnt <= 64'h0000000000000000;
                r.ins_per_step <= 64'h0000000000000001;
                r.pmp_upd_ena <= 8'h00;
                r.pmp_upd_cnt <= 3'h0;
                r.pmp_ena <= 1'h0;
                r.pmp_we <= 1'h0;
                r.pmp_region <= 3'h0;
                r.pmp_start_addr <= 64'h0000000000000000;
                r.pmp_end_addr <= 64'h0000000000000000;
                r.pmp_flags <= 5'h00;
            end else begin
                for (int i = 0; i < 4; i++) begin
                    r.xmode[i].xepc <= rin.xmode[i].xepc;
                    r.xmode[i].xpp <= rin.xmode[i].xpp;
                    r.xmode[i].xpie <= rin.xmode[i].xpie;
                    r.xmode[i].xie <= rin.xmode[i].xie;
                    r.xmode[i].xsie <= rin.xmode[i].xsie;
                    r.xmode[i].xtie <= rin.xmode[i].xtie;
                    r.xmode[i].xeie <= rin.xmode[i].xeie;
                    r.xmode[i].xtvec_off <= rin.xmode[i].xtvec_off;
                    r.xmode[i].xtvec_mode <= rin.xmode[i].xtvec_mode;
                    r.xmode[i].xtval <= rin.xmode[i].xtval;
                    r.xmode[i].xcause_irq <= rin.xmode[i].xcause_irq;
                    r.xmode[i].xcause_code <= rin.xmode[i].xcause_code;
                    r.xmode[i].xscratch <= rin.xmode[i].xscratch;
                    r.xmode[i].xcounteren <= rin.xmode[i].xcounteren;
                end
                for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                    r.pmp[i].cfg <= rin.pmp[i].cfg;
                    r.pmp[i].addr <= rin.pmp[i].addr;
                    r.pmp[i].mask <= rin.pmp[i].mask;
                end
                r.state <= rin.state;
                r.fencestate <= rin.fencestate;
                r.irq_pending <= rin.irq_pending;
                r.cmd_type <= rin.cmd_type;
                r.cmd_addr <= rin.cmd_addr;
                r.cmd_data <= rin.cmd_data;
                r.cmd_exception <= rin.cmd_exception;
                r.progbuf_end <= rin.progbuf_end;
                r.progbuf_err <= rin.progbuf_err;
                r.mip_ssip <= rin.mip_ssip;
                r.mip_stip <= rin.mip_stip;
                r.mip_seip <= rin.mip_seip;
                r.medeleg <= rin.medeleg;
                r.mideleg <= rin.mideleg;
                r.mcountinhibit <= rin.mcountinhibit;
                r.mstackovr <= rin.mstackovr;
                r.mstackund <= rin.mstackund;
                r.mmu_ena <= rin.mmu_ena;
                r.satp_ppn <= rin.satp_ppn;
                r.satp_sv39 <= rin.satp_sv39;
                r.satp_sv48 <= rin.satp_sv48;
                r.mode <= rin.mode;
                r.mprv <= rin.mprv;
                r.mxr <= rin.mxr;
                r.sum <= rin.sum;
                r.tvm <= rin.tvm;
                r.ex_fpu_invalidop <= rin.ex_fpu_invalidop;
                r.ex_fpu_divbyzero <= rin.ex_fpu_divbyzero;
                r.ex_fpu_overflow <= rin.ex_fpu_overflow;
                r.ex_fpu_underflow <= rin.ex_fpu_underflow;
                r.ex_fpu_inexact <= rin.ex_fpu_inexact;
                r.trap_addr <= rin.trap_addr;
                r.mcycle_cnt <= rin.mcycle_cnt;
                r.minstret_cnt <= rin.minstret_cnt;
                r.dscratch0 <= rin.dscratch0;
                r.dscratch1 <= rin.dscratch1;
                r.dpc <= rin.dpc;
                r.halt_cause <= rin.halt_cause;
                r.dcsr_ebreakm <= rin.dcsr_ebreakm;
                r.dcsr_stopcount <= rin.dcsr_stopcount;
                r.dcsr_stoptimer <= rin.dcsr_stoptimer;
                r.dcsr_step <= rin.dcsr_step;
                r.dcsr_stepie <= rin.dcsr_stepie;
                r.stepping_mode_cnt <= rin.stepping_mode_cnt;
                r.ins_per_step <= rin.ins_per_step;
                r.pmp_upd_ena <= rin.pmp_upd_ena;
                r.pmp_upd_cnt <= rin.pmp_upd_cnt;
                r.pmp_ena <= rin.pmp_ena;
                r.pmp_we <= rin.pmp_we;
                r.pmp_region <= rin.pmp_region;
                r.pmp_start_addr <= rin.pmp_start_addr;
                r.pmp_end_addr <= rin.pmp_end_addr;
                r.pmp_flags <= rin.pmp_flags;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < 4; i++) begin
                r.xmode[i].xepc <= rin.xmode[i].xepc;
                r.xmode[i].xpp <= rin.xmode[i].xpp;
                r.xmode[i].xpie <= rin.xmode[i].xpie;
                r.xmode[i].xie <= rin.xmode[i].xie;
                r.xmode[i].xsie <= rin.xmode[i].xsie;
                r.xmode[i].xtie <= rin.xmode[i].xtie;
                r.xmode[i].xeie <= rin.xmode[i].xeie;
                r.xmode[i].xtvec_off <= rin.xmode[i].xtvec_off;
                r.xmode[i].xtvec_mode <= rin.xmode[i].xtvec_mode;
                r.xmode[i].xtval <= rin.xmode[i].xtval;
                r.xmode[i].xcause_irq <= rin.xmode[i].xcause_irq;
                r.xmode[i].xcause_code <= rin.xmode[i].xcause_code;
                r.xmode[i].xscratch <= rin.xmode[i].xscratch;
                r.xmode[i].xcounteren <= rin.xmode[i].xcounteren;
            end
            for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) begin
                r.pmp[i].cfg <= rin.pmp[i].cfg;
                r.pmp[i].addr <= rin.pmp[i].addr;
                r.pmp[i].mask <= rin.pmp[i].mask;
            end
            r.state <= rin.state;
            r.fencestate <= rin.fencestate;
            r.irq_pending <= rin.irq_pending;
            r.cmd_type <= rin.cmd_type;
            r.cmd_addr <= rin.cmd_addr;
            r.cmd_data <= rin.cmd_data;
            r.cmd_exception <= rin.cmd_exception;
            r.progbuf_end <= rin.progbuf_end;
            r.progbuf_err <= rin.progbuf_err;
            r.mip_ssip <= rin.mip_ssip;
            r.mip_stip <= rin.mip_stip;
            r.mip_seip <= rin.mip_seip;
            r.medeleg <= rin.medeleg;
            r.mideleg <= rin.mideleg;
            r.mcountinhibit <= rin.mcountinhibit;
            r.mstackovr <= rin.mstackovr;
            r.mstackund <= rin.mstackund;
            r.mmu_ena <= rin.mmu_ena;
            r.satp_ppn <= rin.satp_ppn;
            r.satp_sv39 <= rin.satp_sv39;
            r.satp_sv48 <= rin.satp_sv48;
            r.mode <= rin.mode;
            r.mprv <= rin.mprv;
            r.mxr <= rin.mxr;
            r.sum <= rin.sum;
            r.tvm <= rin.tvm;
            r.ex_fpu_invalidop <= rin.ex_fpu_invalidop;
            r.ex_fpu_divbyzero <= rin.ex_fpu_divbyzero;
            r.ex_fpu_overflow <= rin.ex_fpu_overflow;
            r.ex_fpu_underflow <= rin.ex_fpu_underflow;
            r.ex_fpu_inexact <= rin.ex_fpu_inexact;
            r.trap_addr <= rin.trap_addr;
            r.mcycle_cnt <= rin.mcycle_cnt;
            r.minstret_cnt <= rin.minstret_cnt;
            r.dscratch0 <= rin.dscratch0;
            r.dscratch1 <= rin.dscratch1;
            r.dpc <= rin.dpc;
            r.halt_cause <= rin.halt_cause;
            r.dcsr_ebreakm <= rin.dcsr_ebreakm;
            r.dcsr_stopcount <= rin.dcsr_stopcount;
            r.dcsr_stoptimer <= rin.dcsr_stoptimer;
            r.dcsr_step <= rin.dcsr_step;
            r.dcsr_stepie <= rin.dcsr_stepie;
            r.stepping_mode_cnt <= rin.stepping_mode_cnt;
            r.ins_per_step <= rin.ins_per_step;
            r.pmp_upd_ena <= rin.pmp_upd_ena;
            r.pmp_upd_cnt <= rin.pmp_upd_cnt;
            r.pmp_ena <= rin.pmp_ena;
            r.pmp_we <= rin.pmp_we;
            r.pmp_region <= rin.pmp_region;
            r.pmp_start_addr <= rin.pmp_start_addr;
            r.pmp_end_addr <= rin.pmp_end_addr;
            r.pmp_flags <= rin.pmp_flags;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: CsrRegs
