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

module Processor #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned hartid = 0,
    parameter bit fpu_ena = 1'b1,
    parameter bit tracer_ena = 1'b1
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // Control path:
    input logic i_req_ctrl_ready,                           // ICache is ready to accept request
    output logic o_req_ctrl_valid,                          // Request to ICache is valid
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_req_ctrl_addr,// Requesting address to ICache
    input logic i_resp_ctrl_valid,                          // ICache response is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_resp_ctrl_addr,// Response address must be equal to the latest request address
    input logic [63:0] i_resp_ctrl_data,                    // Read value
    input logic i_resp_ctrl_load_fault,
    output logic o_resp_ctrl_ready,                         // Core is ready to accept response from ICache
    // Data path:
    input logic i_req_data_ready,                           // DCache is ready to accept request
    output logic o_req_data_valid,                          // Request to DCache is valid
    output logic [river_cfg_pkg::MemopType_Total-1:0] o_req_data_type,// Read/Write transaction plus additional flags
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_req_data_addr,// Requesting address to DCache
    output logic [63:0] o_req_data_wdata,                   // Writing value
    output logic [7:0] o_req_data_wstrb,                    // 8-bytes aligned strobs
    output logic [1:0] o_req_data_size,                     // memory operation 1,2,4 or 8 bytes
    input logic i_resp_data_valid,                          // DCache response is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_resp_data_addr,// DCache response address must be equal to the latest request address
    input logic [63:0] i_resp_data_data,                    // Read value
    input logic i_resp_data_load_fault,                     // Bus response with SLVERR or DECERR on read
    input logic i_resp_data_store_fault,                    // Bus response with SLVERR or DECERR on write
    output logic o_resp_data_ready,                         // Core is ready to accept response from DCache
    // Interrupt line from external interrupts controller (PLIC):
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    // PMP interface
    output logic o_pmp_ena,                                 // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    output logic o_pmp_we,                                  // write enable into PMP
    output logic [river_cfg_pkg::CFG_PMP_TBL_WIDTH-1:0] o_pmp_region,// selected PMP region
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pmp_start_addr,// PMP region start address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pmp_end_addr,// PMP region end address (inclusive)
    output logic [river_cfg_pkg::CFG_PMP_FL_TOTAL-1:0] o_pmp_flags,// {ena, lock, r, w, x}
    // Debug interface:
    input logic i_haltreq,                                  // DMI: halt request from debug unit
    input logic i_resumereq,                                // DMI: resume request from debug unit
    input logic i_dport_req_valid,                          // Debug access from DSU is valid
    input logic [river_cfg_pkg::DPortReq_Total-1:0] i_dport_type,// Debug access type
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_addr,// dport address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_wdata,// Write value
    input logic [2:0] i_dport_size,                         // reg/mem access size:0=1B;...,4=128B;
    output logic o_dport_req_ready,
    input logic i_dport_resp_ready,                         // ready to accepd response
    output logic o_dport_resp_valid,                        // Response is valid
    output logic o_dport_resp_error,                        // Something wrong during command execution
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_rdata,// Response value
    input logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] i_progbuf,// progam buffer
    output logic o_halted,                                  // CPU halted via debug interface
    // Cache debug signals:
    output logic o_flushi_valid,                            // Remove address from ICache is valid
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_flushi_addr,// Address of instruction to remove from ICache
    output logic o_flushd_valid,                            // Remove address from D$ is valid
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_flushd_addr,// Address of instruction to remove from D$
    input logic i_flushd_end
);

import river_cfg_pkg::*;
import proc_pkg::*;

PipelineType w;                                             // 5-stages CPU pipeline
MmuType immu;
MmuType dmmu;
IntRegsType ireg;
CsrType csr;
DebugType dbg;
BranchPredictorType bp;
// csr bridge to executor unit
logic iccsr_m0_req_ready;
logic iccsr_m0_resp_valid;
logic [RISCV_ARCH-1:0] iccsr_m0_resp_data;
logic iccsr_m0_resp_exception;
// csr bridge to debug unit
logic iccsr_m1_req_ready;
logic iccsr_m1_resp_valid;
logic [RISCV_ARCH-1:0] iccsr_m1_resp_data;
logic iccsr_m1_resp_exception;
// csr bridge to CSR module
logic iccsr_s0_req_valid;
logic [CsrReq_TotalBits-1:0] iccsr_s0_req_type;
logic [11:0] iccsr_s0_req_addr;
logic [RISCV_ARCH-1:0] iccsr_s0_req_data;
logic iccsr_s0_resp_ready;
logic iccsr_s0_resp_exception;
logic w_mem_resp_error;
logic w_writeback_ready;
logic w_reg_wena;
logic [5:0] wb_reg_waddr;
logic [RISCV_ARCH-1:0] wb_reg_wdata;
logic [CFG_REG_TAG_WIDTH-1:0] wb_reg_wtag;
logic w_reg_inorder;
logic w_reg_ignored;
logic w_f_flush_ready;
logic [MemopType_Total-1:0] unused_immu_mem_req_type;
logic [63:0] unused_immu_mem_req_wdata;
logic [7:0] unused_immu_mem_req_wstrb;
logic [1:0] unused_immu_mem_req_size;
logic w_immu_core_req_fetch;                                // assign to 1: fetch instruction
logic w_dmmu_core_req_fetch;                                // assign to 0: data
logic [MemopType_Total-1:0] unused_immu_core_req_type;
logic [63:0] unused_immu_core_req_wdata;
logic [7:0] unused_immu_core_req_wstrb;
logic [1:0] unused_immu_core_req_size;
logic unused_immu_mem_resp_store_fault;

Mmu #(
    .async_reset(async_reset)
) immu0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .o_core_req_ready(immu.req_ready),
    .i_core_req_valid(w.f.imem_req_valid),
    .i_core_req_addr(w.f.imem_req_addr),
    .i_core_req_fetch(w_immu_core_req_fetch),
    .i_core_req_type(unused_immu_core_req_type),
    .i_core_req_wdata(unused_immu_core_req_wdata),
    .i_core_req_wstrb(unused_immu_core_req_wstrb),
    .i_core_req_size(unused_immu_core_req_size),
    .o_core_resp_valid(immu.valid),
    .o_core_resp_addr(immu.addr),
    .o_core_resp_data(immu.data),
    .o_core_resp_load_fault(immu.load_fault),
    .o_core_resp_store_fault(immu.store_fault),
    .o_core_resp_page_x_fault(immu.page_fault_x),
    .o_core_resp_page_r_fault(immu.page_fault_r),
    .o_core_resp_page_w_fault(immu.page_fault_w),
    .i_core_resp_ready(w.f.imem_resp_ready),
    .i_mem_req_ready(i_req_ctrl_ready),
    .o_mem_req_valid(o_req_ctrl_valid),
    .o_mem_req_addr(o_req_ctrl_addr),
    .o_mem_req_type(unused_immu_mem_req_type),
    .o_mem_req_wdata(unused_immu_mem_req_wdata),
    .o_mem_req_wstrb(unused_immu_mem_req_wstrb),
    .o_mem_req_size(unused_immu_mem_req_size),
    .i_mem_resp_valid(i_resp_ctrl_valid),
    .i_mem_resp_addr(i_resp_ctrl_addr),
    .i_mem_resp_data(i_resp_ctrl_data),
    .i_mem_resp_load_fault(i_resp_ctrl_load_fault),
    .i_mem_resp_store_fault(unused_immu_mem_resp_store_fault),
    .o_mem_resp_ready(o_resp_ctrl_ready),
    .i_mmu_ena(csr.mmu_ena),
    .i_mmu_sv39(csr.mmu_sv39),
    .i_mmu_sv48(csr.mmu_sv48),
    .i_mmu_ppn(csr.mmu_ppn),
    .i_mprv(csr.mprv),
    .i_mxr(csr.mxr),
    .i_sum(csr.sum),
    .i_fence(csr.flushmmu_valid),
    .i_fence_addr(csr.flush_addr)
);


InstrFetch #(
    .async_reset(async_reset)
) fetch0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_bp_valid(bp.f_valid),
    .i_bp_pc(bp.f_pc),
    .o_requested_pc(w.f.requested_pc),
    .o_fetching_pc(w.f.fetching_pc),
    .i_mem_req_ready(immu.req_ready),
    .o_mem_addr_valid(w.f.imem_req_valid),
    .o_mem_addr(w.f.imem_req_addr),
    .i_mem_data_valid(immu.valid),
    .i_mem_data_addr(immu.addr),
    .i_mem_data(immu.data),
    .i_mem_load_fault(immu.load_fault),
    .i_mem_page_fault_x(immu.page_fault_x),
    .o_mem_resp_ready(w.f.imem_resp_ready),
    .i_flush_pipeline(csr.flushpipeline_valid),
    .i_progbuf_ena(dbg.progbuf_ena),
    .i_progbuf_pc(dbg.progbuf_pc),
    .i_progbuf_instr(dbg.progbuf_instr),
    .o_instr_load_fault(w.f.instr_load_fault),
    .o_instr_page_fault_x(w.f.instr_page_fault_x),
    .o_pc(w.f.pc),
    .o_instr(w.f.instr)
);


InstrDecoder #(
    .async_reset(async_reset),
    .fpu_ena(fpu_ena)
) dec0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_f_pc(w.f.pc),
    .i_f_instr(w.f.instr),
    .i_instr_load_fault(w.f.instr_load_fault),
    .i_instr_page_fault_x(w.f.instr_page_fault_x),
    .i_e_npc(w.e.npc),
    .o_radr1(w.d.radr1),
    .o_radr2(w.d.radr2),
    .o_waddr(w.d.waddr),
    .o_csr_addr(w.d.csr_addr),
    .o_imm(w.d.imm),
    .i_flush_pipeline(csr.flushpipeline_valid),
    .i_progbuf_ena(dbg.progbuf_ena),
    .o_pc(w.d.pc),
    .o_instr(w.d.instr),
    .o_memop_store(w.d.memop_store),
    .o_memop_load(w.d.memop_load),
    .o_memop_sign_ext(w.d.memop_sign_ext),
    .o_memop_size(w.d.memop_size),
    .o_rv32(w.d.rv32),
    .o_compressed(w.d.compressed),
    .o_amo(w.d.amo),
    .o_f64(w.d.f64),
    .o_unsigned_op(w.d.unsigned_op),
    .o_isa_type(w.d.isa_type),
    .o_instr_vec(w.d.instr_vec),
    .o_exception(w.d.exception),
    .o_instr_load_fault(w.d.instr_load_fault),
    .o_instr_page_fault_x(w.d.page_fault_x),
    .o_progbuf_ena(w.d.progbuf_ena)
);


InstrExecute #(
    .async_reset(async_reset),
    .fpu_ena(fpu_ena)
) exec0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_d_radr1(w.d.radr1),
    .i_d_radr2(w.d.radr2),
    .i_d_waddr(w.d.waddr),
    .i_d_csr_addr(w.d.csr_addr),
    .i_d_imm(w.d.imm),
    .i_d_pc(w.d.pc),
    .i_d_instr(w.d.instr),
    .i_d_progbuf_ena(w.d.progbuf_ena),
    .i_wb_waddr(w.w.waddr),
    .i_memop_store(w.d.memop_store),
    .i_memop_load(w.d.memop_load),
    .i_memop_sign_ext(w.d.memop_sign_ext),
    .i_memop_size(w.d.memop_size),
    .i_unsigned_op(w.d.unsigned_op),
    .i_rv32(w.d.rv32),
    .i_compressed(w.d.compressed),
    .i_amo(w.d.amo),
    .i_f64(w.d.f64),
    .i_isa_type(w.d.isa_type),
    .i_ivec(w.d.instr_vec),
    .i_stack_overflow(csr.stack_overflow),
    .i_stack_underflow(csr.stack_underflow),
    .i_unsup_exception(w.d.exception),
    .i_instr_load_fault(w.d.instr_load_fault),
    .i_mem_valid(dmmu.valid),
    .i_mem_rdata(dmmu.data),
    .i_mem_ex_debug(w.m.debug_valid),
    .i_mem_ex_load_fault(dmmu.load_fault),
    .i_mem_ex_store_fault(dmmu.store_fault),
    .i_page_fault_x(w.d.page_fault_x),
    .i_page_fault_r(dmmu.page_fault_r),
    .i_page_fault_w(dmmu.page_fault_w),
    .i_mem_ex_addr(dmmu.addr),
    .i_irq_pending(csr.irq_pending),
    .i_wakeup(csr.o_wakeup),
    .i_haltreq(i_haltreq),
    .i_resumereq(i_resumereq),
    .i_step(csr.step),
    .i_dbg_progbuf_ena(dbg.progbuf_ena),
    .i_rdata1(ireg.rdata1),
    .i_rtag1(ireg.rtag1),
    .i_rdata2(ireg.rdata2),
    .i_rtag2(ireg.rtag2),
    .o_radr1(w.e.radr1),
    .o_radr2(w.e.radr2),
    .o_reg_wena(w.e.reg_wena),
    .o_reg_waddr(w.e.reg_waddr),
    .o_reg_wtag(w.e.reg_wtag),
    .o_reg_wdata(w.e.reg_wdata),
    .o_csr_req_valid(w.e.csr_req_valid),
    .i_csr_req_ready(iccsr_m0_req_ready),
    .o_csr_req_type(w.e.csr_req_type),
    .o_csr_req_addr(w.e.csr_req_addr),
    .o_csr_req_data(w.e.csr_req_data),
    .i_csr_resp_valid(iccsr_m0_resp_valid),
    .o_csr_resp_ready(w.e.csr_resp_ready),
    .i_csr_resp_data(iccsr_m0_resp_data),
    .i_csr_resp_exception(iccsr_m0_resp_exception),
    .o_memop_valid(w.e.memop_valid),
    .o_memop_debug(w.e.memop_debug),
    .o_memop_sign_ext(w.e.memop_sign_ext),
    .o_memop_type(w.e.memop_type),
    .o_memop_size(w.e.memop_size),
    .o_memop_memaddr(w.e.memop_addr),
    .o_memop_wdata(w.e.memop_wdata),
    .i_memop_ready(w.m.memop_ready),
    .i_memop_idle(w.m.idle),
    .i_dbg_mem_req_valid(dbg.mem_req_valid),
    .i_dbg_mem_req_write(dbg.mem_req_write),
    .i_dbg_mem_req_size(dbg.mem_req_size),
    .i_dbg_mem_req_addr(dbg.mem_req_addr),
    .i_dbg_mem_req_wdata(dbg.mem_req_wdata),
    .o_dbg_mem_req_ready(w.e.dbg_mem_req_ready),
    .o_dbg_mem_req_error(w.e.dbg_mem_req_error),
    .o_valid(w.e.valid),
    .o_pc(w.e.pc),
    .o_npc(w.e.npc),
    .o_instr(w.e.instr),
    .o_call(w.e.call),
    .o_ret(w.e.ret),
    .o_jmp(w.e.jmp),
    .o_halted(w.e.halted)
);


MemAccess #(
    .async_reset(async_reset)
) mem0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_e_pc(w.e.pc),
    .i_e_instr(w.e.instr),
    .i_flushd_valid(csr.flushd_valid),
    .i_flushd_addr(csr.flush_addr),
    .o_flushd(w.m.flushd),
    .i_mmu_ena(csr.mmu_ena),
    .i_mmu_sv39(csr.mmu_sv39),
    .i_mmu_sv48(csr.mmu_sv48),
    .o_mmu_ena(w.m.dmmu_ena),
    .o_mmu_sv39(w.m.dmmu_sv39),
    .o_mmu_sv48(w.m.dmmu_sv48),
    .i_reg_waddr(w.e.reg_waddr),
    .i_reg_wtag(w.e.reg_wtag),
    .i_memop_valid(w.e.memop_valid),
    .i_memop_debug(w.e.memop_debug),
    .i_memop_wdata(w.e.memop_wdata),
    .i_memop_sign_ext(w.e.memop_sign_ext),
    .i_memop_type(w.e.memop_type),
    .i_memop_size(w.e.memop_size),
    .i_memop_addr(w.e.memop_addr),
    .o_memop_ready(w.m.memop_ready),
    .o_wb_wena(w.w.wena),
    .o_wb_waddr(w.w.waddr),
    .o_wb_wdata(w.w.wdata),
    .o_wb_wtag(w.w.wtag),
    .i_wb_ready(w_writeback_ready),
    .i_mem_req_ready(dmmu.req_ready),
    .o_mem_valid(w.m.req_data_valid),
    .o_mem_type(w.m.req_data_type),
    .o_mem_addr(w.m.req_data_addr),
    .o_mem_wdata(w.m.req_data_wdata),
    .o_mem_wstrb(w.m.req_data_wstrb),
    .o_mem_size(w.m.req_data_size),
    .i_mem_data_valid(dmmu.valid),
    .i_mem_data_addr(dmmu.addr),
    .i_mem_data(dmmu.data),
    .o_mem_resp_ready(w.m.resp_data_ready),
    .o_pc(w.m.pc),
    .o_valid(w.m.valid),
    .o_idle(w.m.idle),
    .o_debug_valid(w.m.debug_valid)
);


Mmu #(
    .async_reset(async_reset)
) dmmu0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .o_core_req_ready(dmmu.req_ready),
    .i_core_req_valid(w.m.req_data_valid),
    .i_core_req_addr(w.m.req_data_addr),
    .i_core_req_fetch(w_dmmu_core_req_fetch),
    .i_core_req_type(w.m.req_data_type),
    .i_core_req_wdata(w.m.req_data_wdata),
    .i_core_req_wstrb(w.m.req_data_wstrb),
    .i_core_req_size(w.m.req_data_size),
    .o_core_resp_valid(dmmu.valid),
    .o_core_resp_addr(dmmu.addr),
    .o_core_resp_data(dmmu.data),
    .o_core_resp_load_fault(dmmu.load_fault),
    .o_core_resp_store_fault(dmmu.store_fault),
    .o_core_resp_page_x_fault(dmmu.page_fault_x),
    .o_core_resp_page_r_fault(dmmu.page_fault_r),
    .o_core_resp_page_w_fault(dmmu.page_fault_w),
    .i_core_resp_ready(w.m.resp_data_ready),
    .i_mem_req_ready(i_req_data_ready),
    .o_mem_req_valid(o_req_data_valid),
    .o_mem_req_addr(o_req_data_addr),
    .o_mem_req_type(o_req_data_type),
    .o_mem_req_wdata(o_req_data_wdata),
    .o_mem_req_wstrb(o_req_data_wstrb),
    .o_mem_req_size(o_req_data_size),
    .i_mem_resp_valid(i_resp_data_valid),
    .i_mem_resp_addr(i_resp_data_addr),
    .i_mem_resp_data(i_resp_data_data),
    .i_mem_resp_load_fault(i_resp_data_load_fault),
    .i_mem_resp_store_fault(i_resp_data_store_fault),
    .o_mem_resp_ready(o_resp_data_ready),
    .i_mmu_ena(w.m.dmmu_ena),
    .i_mmu_sv39(w.m.dmmu_sv39),
    .i_mmu_sv48(w.m.dmmu_sv48),
    .i_mmu_ppn(csr.mmu_ppn),
    .i_mprv(csr.mprv),
    .i_mxr(csr.mxr),
    .i_sum(csr.sum),
    .i_fence(csr.flushmmu_valid),
    .i_fence_addr(csr.flush_addr)
);


BranchPredictor #(
    .async_reset(async_reset)
) predic0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_flush_pipeline(csr.flushpipeline_valid),
    .i_resp_mem_valid(immu.valid),
    .i_resp_mem_addr(immu.addr),
    .i_resp_mem_data(immu.data),
    .i_e_jmp(w.e.jmp),
    .i_e_pc(w.e.pc),
    .i_e_npc(w.e.npc),
    .i_ra(ireg.ra),
    .o_f_valid(bp.f_valid),
    .o_f_pc(bp.f_pc),
    .i_f_requested_pc(w.f.requested_pc),
    .i_f_fetching_pc(w.f.fetching_pc),
    .i_f_fetched_pc(w.f.pc),
    .i_d_pc(w.d.pc)
);


RegIntBank #(
    .async_reset(async_reset)
) iregs0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_radr1(w.e.radr1),
    .o_rdata1(ireg.rdata1),
    .o_rtag1(ireg.rtag1),
    .i_radr2(w.e.radr2),
    .o_rdata2(ireg.rdata2),
    .o_rtag2(ireg.rtag2),
    .i_waddr(wb_reg_waddr),
    .i_wena(w_reg_wena),
    .i_wtag(wb_reg_wtag),
    .i_wdata(wb_reg_wdata),
    .i_inorder(w_reg_inorder),
    .o_ignored(w_reg_ignored),
    .i_dport_addr(dbg.ireg_addr),
    .i_dport_ena(dbg.ireg_ena),
    .i_dport_write(dbg.ireg_write),
    .i_dport_wdata(dbg.ireg_wdata),
    .o_dport_rdata(ireg.dport_rdata),
    .o_ra(ireg.ra),
    .o_sp(ireg.sp),
    .o_gp(ireg.gp),
    .o_tp(ireg.tp),
    .o_t0(ireg.t0),
    .o_t1(ireg.t1),
    .o_t2(ireg.t2),
    .o_fp(ireg.fp),
    .o_s1(ireg.s1),
    .o_a0(ireg.a0),
    .o_a1(ireg.a1),
    .o_a2(ireg.a2),
    .o_a3(ireg.a3),
    .o_a4(ireg.a4),
    .o_a5(ireg.a5),
    .o_a6(ireg.a6),
    .o_a7(ireg.a7),
    .o_s2(ireg.s2),
    .o_s3(ireg.s3),
    .o_s4(ireg.s4),
    .o_s5(ireg.s5),
    .o_s6(ireg.s6),
    .o_s7(ireg.s7),
    .o_s8(ireg.s8),
    .o_s9(ireg.s9),
    .o_s10(ireg.s10),
    .o_s11(ireg.s11),
    .o_t3(ireg.t3),
    .o_t4(ireg.t4),
    .o_t5(ireg.t5),
    .o_t6(ireg.t6)
);


ic_csr_m2_s1 #(
    .async_reset(async_reset)
) iccsr0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_m0_req_valid(w.e.csr_req_valid),
    .o_m0_req_ready(iccsr_m0_req_ready),
    .i_m0_req_type(w.e.csr_req_type),
    .i_m0_req_addr(w.e.csr_req_addr),
    .i_m0_req_data(w.e.csr_req_data),
    .o_m0_resp_valid(iccsr_m0_resp_valid),
    .i_m0_resp_ready(w.e.csr_resp_ready),
    .o_m0_resp_data(iccsr_m0_resp_data),
    .o_m0_resp_exception(iccsr_m0_resp_exception),
    .i_m1_req_valid(dbg.csr_req_valid),
    .o_m1_req_ready(iccsr_m1_req_ready),
    .i_m1_req_type(dbg.csr_req_type),
    .i_m1_req_addr(dbg.csr_req_addr),
    .i_m1_req_data(dbg.csr_req_data),
    .o_m1_resp_valid(iccsr_m1_resp_valid),
    .i_m1_resp_ready(dbg.csr_resp_ready),
    .o_m1_resp_data(iccsr_m1_resp_data),
    .o_m1_resp_exception(iccsr_m1_resp_exception),
    .o_s0_req_valid(iccsr_s0_req_valid),
    .i_s0_req_ready(csr.req_ready),
    .o_s0_req_type(iccsr_s0_req_type),
    .o_s0_req_addr(iccsr_s0_req_addr),
    .o_s0_req_data(iccsr_s0_req_data),
    .i_s0_resp_valid(csr.resp_valid),
    .o_s0_resp_ready(iccsr_s0_resp_ready),
    .i_s0_resp_data(csr.resp_data),
    .i_s0_resp_exception(csr.resp_exception)
);


CsrRegs #(
    .async_reset(async_reset),
    .hartid(hartid)
) csr0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_sp(ireg.sp),
    .i_req_valid(iccsr_s0_req_valid),
    .o_req_ready(csr.req_ready),
    .i_req_type(iccsr_s0_req_type),
    .i_req_addr(iccsr_s0_req_addr),
    .i_req_data(iccsr_s0_req_data),
    .o_resp_valid(csr.resp_valid),
    .i_resp_ready(iccsr_s0_resp_ready),
    .o_resp_data(csr.resp_data),
    .o_resp_exception(csr.resp_exception),
    .i_e_halted(w.e.halted),
    .i_e_pc(w.e.pc),
    .i_e_instr(w.e.instr),
    .i_irq_pending(i_irq_pending),
    .o_irq_pending(csr.irq_pending),
    .o_wakeup(csr.o_wakeup),
    .o_stack_overflow(csr.stack_overflow),
    .o_stack_underflow(csr.stack_underflow),
    .i_f_flush_ready(w_f_flush_ready),
    .i_e_valid(w.e.valid),
    .i_m_memop_ready(w.m.memop_ready),
    .i_m_idle(w.m.idle),
    .i_flushd_end(i_flushd_end),
    .i_mtimer(i_mtimer),
    .o_executed_cnt(csr.executed_cnt),
    .o_step(csr.step),
    .i_dbg_progbuf_ena(dbg.progbuf_ena),
    .o_progbuf_end(csr.progbuf_end),
    .o_progbuf_error(csr.progbuf_error),
    .o_flushd_valid(csr.flushd_valid),
    .o_flushi_valid(csr.flushi_valid),
    .o_flushmmu_valid(csr.flushmmu_valid),
    .o_flushpipeline_valid(csr.flushpipeline_valid),
    .o_flush_addr(csr.flush_addr),
    .o_pmp_ena(o_pmp_ena),
    .o_pmp_we(o_pmp_we),
    .o_pmp_region(o_pmp_region),
    .o_pmp_start_addr(o_pmp_start_addr),
    .o_pmp_end_addr(o_pmp_end_addr),
    .o_pmp_flags(o_pmp_flags),
    .o_mmu_ena(csr.mmu_ena),
    .o_mmu_sv39(csr.mmu_sv39),
    .o_mmu_sv48(csr.mmu_sv48),
    .o_mmu_ppn(csr.mmu_ppn),
    .o_mprv(csr.mprv),
    .o_mxr(csr.mxr),
    .o_sum(csr.sum)
);


DbgPort #(
    .async_reset(async_reset)
) dbg0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_dport_req_valid(i_dport_req_valid),
    .i_dport_type(i_dport_type),
    .i_dport_addr(i_dport_addr),
    .i_dport_wdata(i_dport_wdata),
    .i_dport_size(i_dport_size),
    .o_dport_req_ready(o_dport_req_ready),
    .i_dport_resp_ready(i_dport_resp_ready),
    .o_dport_resp_valid(o_dport_resp_valid),
    .o_dport_resp_error(o_dport_resp_error),
    .o_dport_rdata(o_dport_rdata),
    .o_csr_req_valid(dbg.csr_req_valid),
    .i_csr_req_ready(iccsr_m1_req_ready),
    .o_csr_req_type(dbg.csr_req_type),
    .o_csr_req_addr(dbg.csr_req_addr),
    .o_csr_req_data(dbg.csr_req_data),
    .i_csr_resp_valid(iccsr_m1_resp_valid),
    .o_csr_resp_ready(dbg.csr_resp_ready),
    .i_csr_resp_data(iccsr_m1_resp_data),
    .i_csr_resp_exception(iccsr_m1_resp_exception),
    .i_progbuf(i_progbuf),
    .o_progbuf_ena(dbg.progbuf_ena),
    .o_progbuf_pc(dbg.progbuf_pc),
    .o_progbuf_instr(dbg.progbuf_instr),
    .i_csr_progbuf_end(csr.progbuf_end),
    .i_csr_progbuf_error(csr.progbuf_error),
    .o_ireg_addr(dbg.ireg_addr),
    .o_ireg_wdata(dbg.ireg_wdata),
    .o_ireg_ena(dbg.ireg_ena),
    .o_ireg_write(dbg.ireg_write),
    .i_ireg_rdata(ireg.dport_rdata),
    .o_mem_req_valid(dbg.mem_req_valid),
    .i_mem_req_ready(w.e.dbg_mem_req_ready),
    .i_mem_req_error(w.e.dbg_mem_req_error),
    .o_mem_req_write(dbg.mem_req_write),
    .o_mem_req_addr(dbg.mem_req_addr),
    .o_mem_req_size(dbg.mem_req_size),
    .o_mem_req_wdata(dbg.mem_req_wdata),
    .i_mem_resp_valid(w.m.debug_valid),
    .i_mem_resp_error(w_mem_resp_error),
    .i_mem_resp_rdata(w.w.wdata),
    .i_e_pc(w.e.pc),
    .i_e_npc(w.e.npc),
    .i_e_call(w.e.call),
    .i_e_ret(w.e.ret),
    .i_e_memop_valid(w.e.memop_valid),
    .i_m_valid(w.m.valid)
);


generate
    if (tracer_ena) begin: tr_en
        Tracer #(
            .async_reset(async_reset),
            .hartid(hartid),
            .trace_file(trace_file)
        ) trace0 (
            .i_clk(i_clk),
            .i_nrst(i_nrst),
            .i_dbg_executed_cnt(csr.executed_cnt),
            .i_e_valid(w.e.valid),
            .i_e_pc(w.e.pc),
            .i_e_instr(w.e.instr),
            .i_e_wena(w.e.reg_wena),
            .i_e_waddr(w.e.reg_waddr),
            .i_e_wdata(w.e.reg_wdata),
            .i_e_memop_valid(w.e.memop_valid),
            .i_e_memop_type(w.e.memop_type),
            .i_e_memop_size(w.e.memop_size),
            .i_e_memop_addr(w.e.memop_addr),
            .i_e_memop_wdata(w.e.memop_wdata),
            .i_e_flushd(csr.flushd_valid),
            .i_m_pc(w.m.pc),
            .i_m_valid(w.m.valid),
            .i_m_memop_ready(w.m.memop_ready),
            .i_m_wena(w.w.wena),
            .i_m_waddr(w.w.waddr),
            .i_m_wdata(w.w.wdata),
            .i_reg_ignored(w_reg_ignored)
        );
    end: tr_en

endgenerate

always_comb
begin: comb_proc
    w_mem_resp_error = (i_resp_data_load_fault || i_resp_data_store_fault);
    w_writeback_ready = (~w.e.reg_wena);
    if (w.e.reg_wena == 1'b1) begin
        w_reg_wena = w.e.reg_wena;
        wb_reg_waddr = w.e.reg_waddr;
        wb_reg_wdata = w.e.reg_wdata;
        wb_reg_wtag = w.e.reg_wtag;
        w_reg_inorder = 1'b0;                               // Executor can overwrite memory loading before it was loaded
    end else begin
        w_reg_wena = w.w.wena;
        wb_reg_waddr = w.w.waddr;
        wb_reg_wdata = w.w.wdata;
        wb_reg_wtag = w.w.wtag;
        w_reg_inorder = 1'b1;                               // Cannot write loaded from memory value if it was overwritten
    end
    w_f_flush_ready = 1'b1;
    w_immu_core_req_fetch = 1'b1;
    w_dmmu_core_req_fetch = 1'b0;
    unused_immu_core_req_type = '0;
    unused_immu_core_req_wdata = '0;
    unused_immu_core_req_wstrb = '0;
    unused_immu_core_req_size = '0;
    unused_immu_mem_resp_store_fault = 1'b0;
    o_flushi_valid = csr.flushi_valid;
    o_flushi_addr = csr.flush_addr;
    o_flushd_addr = '1;
    o_flushd_valid = w.m.flushd;
    o_halted = w.e.halted;
end: comb_proc

endmodule: Processor
