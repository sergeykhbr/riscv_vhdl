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

#include "proc.h"
#include "api_core.h"

namespace debugger {

static std::string trace_file = "trace_river_sysc";

Processor::Processor(sc_module_name name,
                     bool async_reset,
                     uint32_t hartid,
                     bool fpu_ena,
                     bool tracer_ena)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_ctrl_ready("i_req_ctrl_ready"),
    o_req_ctrl_valid("o_req_ctrl_valid"),
    o_req_ctrl_addr("o_req_ctrl_addr"),
    i_resp_ctrl_valid("i_resp_ctrl_valid"),
    i_resp_ctrl_addr("i_resp_ctrl_addr"),
    i_resp_ctrl_data("i_resp_ctrl_data"),
    i_resp_ctrl_load_fault("i_resp_ctrl_load_fault"),
    i_resp_ctrl_executable("i_resp_ctrl_executable"),
    o_resp_ctrl_ready("o_resp_ctrl_ready"),
    i_req_data_ready("i_req_data_ready"),
    o_req_data_valid("o_req_data_valid"),
    o_req_data_type("o_req_data_type"),
    o_req_data_addr("o_req_data_addr"),
    o_req_data_wdata("o_req_data_wdata"),
    o_req_data_wstrb("o_req_data_wstrb"),
    o_req_data_size("o_req_data_size"),
    i_resp_data_valid("i_resp_data_valid"),
    i_resp_data_addr("i_resp_data_addr"),
    i_resp_data_data("i_resp_data_data"),
    i_resp_data_fault_addr("i_resp_data_fault_addr"),
    i_resp_data_load_fault("i_resp_data_load_fault"),
    i_resp_data_store_fault("i_resp_data_store_fault"),
    i_resp_data_er_mpu_load("i_resp_data_er_mpu_load"),
    i_resp_data_er_mpu_store("i_resp_data_er_mpu_store"),
    o_resp_data_ready("o_resp_data_ready"),
    i_irq_pending("i_irq_pending"),
    o_mpu_region_we("o_mpu_region_we"),
    o_mpu_region_idx("o_mpu_region_idx"),
    o_mpu_region_addr("o_mpu_region_addr"),
    o_mpu_region_mask("o_mpu_region_mask"),
    o_mpu_region_flags("o_mpu_region_flags"),
    i_haltreq("i_haltreq"),
    i_resumereq("i_resumereq"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_type("i_dport_type"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    i_dport_size("i_dport_size"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_resp_error("o_dport_resp_error"),
    o_dport_rdata("o_dport_rdata"),
    i_progbuf("i_progbuf"),
    o_halted("o_halted"),
    o_flush_address("o_flush_address"),
    o_flush_valid("o_flush_valid"),
    o_data_flush_address("o_data_flush_address"),
    o_data_flush_valid("o_data_flush_valid"),
    i_data_flush_end("i_data_flush_end") {

    async_reset_ = async_reset;
    hartid_ = hartid;
    fpu_ena_ = fpu_ena;
    tracer_ena_ = tracer_ena;
    fetch0 = 0;
    dec0 = 0;
    exec0 = 0;
    mem0 = 0;
    immu0 = 0;
    dmmu0 = 0;
    predic0 = 0;
    iregs0 = 0;
    iccsr0 = 0;
    csr0 = 0;
    trace0 = 0;
    dbg0 = 0;

    fetch0 = new InstrFetch("fetch0", async_reset);
    fetch0->i_clk(i_clk);
    fetch0->i_nrst(i_nrst);
    fetch0->i_bp_valid(bp.f_valid);
    fetch0->i_bp_pc(bp.f_pc);
    fetch0->o_requested_pc(w.f.requested_pc);
    fetch0->o_fetching_pc(w.f.fetching_pc);
    fetch0->i_mem_req_ready(immu.fetch_req_ready);
    fetch0->o_mem_addr_valid(w.f.imem_req_valid);
    fetch0->o_mem_addr(w.f.imem_req_addr);
    fetch0->i_mem_data_valid(immu.fetch_data_valid);
    fetch0->i_mem_data_addr(immu.fetch_data_addr);
    fetch0->i_mem_data(immu.fetch_data);
    fetch0->i_mem_load_fault(immu.load_fault);
    fetch0->i_mem_executable(immu.fetch_executable);
    fetch0->o_mem_resp_ready(w.f.imem_resp_ready);
    fetch0->i_flush_pipeline(w_flush_pipeline);
    fetch0->i_progbuf_ena(dbg.progbuf_ena);
    fetch0->i_progbuf_pc(dbg.progbuf_pc);
    fetch0->i_progbuf_instr(dbg.progbuf_instr);
    fetch0->o_instr_load_fault(w.f.instr_load_fault);
    fetch0->o_instr_executable(w.f.instr_executable);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);


    immu0 = new Mmu("immu0", async_reset);
    immu0->i_clk(i_clk);
    immu0->i_nrst(i_nrst);
    immu0->o_core_req_ready(immu.fetch_req_ready);
    immu0->i_core_req_valid(w.f.imem_req_valid);
    immu0->i_core_req_addr(w.f.imem_req_addr);
    immu0->i_core_req_fetch(w_immu_core_req_fetch);
    immu0->i_core_req_type(unused_immu_core_req_type);
    immu0->i_core_req_wdata(unused_immu_core_req_wdata);
    immu0->i_core_req_wstrb(unused_immu_core_req_wstrb);
    immu0->i_core_req_size(unused_immu_core_req_size);
    immu0->o_core_resp_valid(immu.fetch_data_valid);
    immu0->o_core_resp_addr(immu.fetch_data_addr);
    immu0->o_core_resp_data(immu.fetch_data);
    immu0->o_core_resp_executable(immu.fetch_executable);
    immu0->o_core_resp_load_fault(immu.load_fault);
    immu0->o_core_resp_store_fault(immu.store_fault);
    immu0->o_core_resp_page_x_fault(immu.page_fault_x);
    immu0->o_core_resp_page_r_fault(immu.page_fault_r);
    immu0->o_core_resp_page_w_fault(immu.page_fault_w);
    immu0->i_core_resp_ready(w.f.imem_resp_ready);
    immu0->i_mem_req_ready(i_req_ctrl_ready);
    immu0->o_mem_req_valid(o_req_ctrl_valid);
    immu0->o_mem_req_addr(o_req_ctrl_addr);
    immu0->o_mem_req_type(unused_immu_mem_req_type);
    immu0->o_mem_req_wdata(unused_immu_mem_req_wdata);
    immu0->o_mem_req_wstrb(unused_immu_mem_req_wstrb);
    immu0->o_mem_req_size(unused_immu_mem_req_size);
    immu0->i_mem_resp_valid(i_resp_ctrl_valid);
    immu0->i_mem_resp_addr(i_resp_ctrl_addr);
    immu0->i_mem_resp_data(i_resp_ctrl_data);
    immu0->i_mem_resp_executable(i_resp_ctrl_executable);
    immu0->i_mem_resp_load_fault(i_resp_ctrl_load_fault);
    immu0->i_mem_resp_store_fault(unused_immu_mem_resp_store_fault);
    immu0->o_mem_resp_ready(o_resp_ctrl_ready);
    immu0->i_mmu_ena(w_mmu_ena);
    immu0->i_mmu_ppn(wb_mmu_ppn);
    immu0->i_fence(w_flush_pipeline);
    immu0->i_fence_addr(unused_immu_fence_addr);


    dec0 = new InstrDecoder("dec0", async_reset, fpu_ena);
    dec0->i_clk(i_clk);
    dec0->i_nrst(i_nrst);
    dec0->i_f_pc(w.f.pc);
    dec0->i_f_instr(w.f.instr);
    dec0->i_instr_load_fault(w.f.instr_load_fault);
    dec0->i_instr_executable(w.f.instr_executable);
    dec0->i_e_npc(w.e.npc);
    dec0->o_radr1(w.d.radr1);
    dec0->o_radr2(w.d.radr2);
    dec0->o_waddr(w.d.waddr);
    dec0->o_csr_addr(w.d.csr_addr);
    dec0->o_imm(w.d.imm);
    dec0->i_flush_pipeline(w_flush_pipeline);
    dec0->i_progbuf_ena(dbg.progbuf_ena);
    dec0->o_pc(w.d.pc);
    dec0->o_instr(w.d.instr);
    dec0->o_memop_store(w.d.memop_store);
    dec0->o_memop_load(w.d.memop_load);
    dec0->o_memop_sign_ext(w.d.memop_sign_ext);
    dec0->o_memop_size(w.d.memop_size);
    dec0->o_rv32(w.d.rv32);
    dec0->o_compressed(w.d.compressed);
    dec0->o_amo(w.d.amo);
    dec0->o_f64(w.d.f64);
    dec0->o_unsigned_op(w.d.unsigned_op);
    dec0->o_isa_type(w.d.isa_type);
    dec0->o_instr_vec(w.d.instr_vec);
    dec0->o_exception(w.d.exception);
    dec0->o_instr_load_fault(w.d.instr_load_fault);
    dec0->o_instr_executable(w.d.instr_executable);
    dec0->o_progbuf_ena(w.d.progbuf_ena);


    exec0 = new InstrExecute("exec0", async_reset, fpu_ena);
    exec0->i_clk(i_clk);
    exec0->i_nrst(i_nrst);
    exec0->i_d_radr1(w.d.radr1);
    exec0->i_d_radr2(w.d.radr2);
    exec0->i_d_waddr(w.d.waddr);
    exec0->i_d_csr_addr(w.d.csr_addr);
    exec0->i_d_imm(w.d.imm);
    exec0->i_d_pc(w.d.pc);
    exec0->i_d_instr(w.d.instr);
    exec0->i_d_progbuf_ena(w.d.progbuf_ena);
    exec0->i_wb_waddr(w.w.waddr);
    exec0->i_memop_store(w.d.memop_store);
    exec0->i_memop_load(w.d.memop_load);
    exec0->i_memop_sign_ext(w.d.memop_sign_ext);
    exec0->i_memop_size(w.d.memop_size);
    exec0->i_unsigned_op(w.d.unsigned_op);
    exec0->i_rv32(w.d.rv32);
    exec0->i_compressed(w.d.compressed);
    exec0->i_amo(w.d.amo);
    exec0->i_f64(w.d.f64);
    exec0->i_isa_type(w.d.isa_type);
    exec0->i_ivec(w.d.instr_vec);
    exec0->i_stack_overflow(csr.stack_overflow);
    exec0->i_stack_underflow(csr.stack_underflow);
    exec0->i_unsup_exception(w.d.exception);
    exec0->i_instr_load_fault(w.d.instr_load_fault);
    exec0->i_instr_executable(w.d.instr_executable);
    exec0->i_mem_ex_debug(w.m.debug_valid);
    exec0->i_mem_ex_load_fault(dmmu.load_fault);
    exec0->i_mem_ex_store_fault(dmmu.store_fault);
    exec0->i_mem_ex_mpu_store(i_resp_data_er_mpu_store);
    exec0->i_mem_ex_mpu_load(i_resp_data_er_mpu_load);
    exec0->i_mem_ex_addr(i_resp_data_fault_addr);
    exec0->i_irq_software(csr.irq_software);
    exec0->i_irq_timer(csr.irq_timer);
    exec0->i_irq_external(csr.irq_external);
    exec0->i_haltreq(i_haltreq);
    exec0->i_resumereq(i_resumereq);
    exec0->i_step(csr.step);
    exec0->i_dbg_progbuf_ena(dbg.progbuf_ena);
    exec0->i_rdata1(ireg.rdata1);
    exec0->i_rtag1(ireg.rtag1);
    exec0->i_rdata2(ireg.rdata2);
    exec0->i_rtag2(ireg.rtag2);
    exec0->o_radr1(w.e.radr1);
    exec0->o_radr2(w.e.radr2);
    exec0->o_reg_wena(w.e.reg_wena);
    exec0->o_reg_waddr(w.e.reg_waddr);
    exec0->o_reg_wtag(w.e.reg_wtag);
    exec0->o_reg_wdata(w.e.reg_wdata);
    exec0->o_csr_req_valid(w.e.csr_req_valid);
    exec0->i_csr_req_ready(iccsr_m0_req_ready);
    exec0->o_csr_req_type(w.e.csr_req_type);
    exec0->o_csr_req_addr(w.e.csr_req_addr);
    exec0->o_csr_req_data(w.e.csr_req_data);
    exec0->i_csr_resp_valid(iccsr_m0_resp_valid);
    exec0->o_csr_resp_ready(w.e.csr_resp_ready);
    exec0->i_csr_resp_data(iccsr_m0_resp_data);
    exec0->i_csr_resp_exception(iccsr_m0_resp_exception);
    exec0->o_memop_valid(w.e.memop_valid);
    exec0->o_memop_debug(w.e.memop_debug);
    exec0->o_memop_sign_ext(w.e.memop_sign_ext);
    exec0->o_memop_type(w.e.memop_type);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_memop_memaddr(w.e.memop_addr);
    exec0->o_memop_wdata(w.e.memop_wdata);
    exec0->i_memop_ready(w.m.memop_ready);
    exec0->i_dbg_mem_req_valid(dbg.mem_req_valid);
    exec0->i_dbg_mem_req_write(dbg.mem_req_write);
    exec0->i_dbg_mem_req_size(dbg.mem_req_size);
    exec0->i_dbg_mem_req_addr(dbg.mem_req_addr);
    exec0->i_dbg_mem_req_wdata(dbg.mem_req_wdata);
    exec0->o_dbg_mem_req_ready(w.e.dbg_mem_req_ready);
    exec0->o_dbg_mem_req_error(w.e.dbg_mem_req_error);
    exec0->o_valid(w.e.valid);
    exec0->o_pc(w.e.pc);
    exec0->o_npc(w.e.npc);
    exec0->o_instr(w.e.instr);
    exec0->i_flushd_end(i_data_flush_end);
    exec0->o_flushd(w.e.flushd);
    exec0->o_flushi(w.e.flushi);
    exec0->o_flushi_addr(w.e.flushi_addr);
    exec0->o_call(w.e.call);
    exec0->o_ret(w.e.ret);
    exec0->o_jmp(w.e.jmp);
    exec0->o_halted(w.e.halted);


    mem0 = new MemAccess("mem0", async_reset);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_e_pc(w.e.pc);
    mem0->i_e_instr(w.e.instr);
    mem0->i_e_flushd(w.e.flushd);
    mem0->o_flushd(w.m.flushd);
    mem0->i_mmu_ena(w_mmu_ena);
    mem0->o_mmu_ena(w.m.mmu_ena);
    mem0->i_reg_waddr(w.e.reg_waddr);
    mem0->i_reg_wtag(w.e.reg_wtag);
    mem0->i_memop_valid(w.e.memop_valid);
    mem0->i_memop_debug(w.e.memop_debug);
    mem0->i_memop_wdata(w.e.memop_wdata);
    mem0->i_memop_sign_ext(w.e.memop_sign_ext);
    mem0->i_memop_type(w.e.memop_type);
    mem0->i_memop_size(w.e.memop_size);
    mem0->i_memop_addr(w.e.memop_addr);
    mem0->o_memop_ready(w.m.memop_ready);
    mem0->o_wb_wena(w.w.wena);
    mem0->o_wb_waddr(w.w.waddr);
    mem0->o_wb_wdata(w.w.wdata);
    mem0->o_wb_wtag(w.w.wtag);
    mem0->i_wb_ready(w_writeback_ready);
    mem0->i_mem_req_ready(dmmu.fetch_req_ready);
    mem0->o_mem_valid(w.m.req_data_valid);
    mem0->o_mem_type(w.m.req_data_type);
    mem0->o_mem_addr(w.m.req_data_addr);
    mem0->o_mem_wdata(w.m.req_data_wdata);
    mem0->o_mem_wstrb(w.m.req_data_wstrb);
    mem0->o_mem_size(w.m.req_data_size);
    mem0->i_mem_data_valid(dmmu.fetch_data_valid);
    mem0->i_mem_data_addr(dmmu.fetch_data_addr);
    mem0->i_mem_data(dmmu.fetch_data);
    mem0->o_mem_resp_ready(w.m.resp_data_ready);
    mem0->o_pc(w.m.pc);
    mem0->o_valid(w.m.valid);
    mem0->o_debug_valid(w.m.debug_valid);


    dmmu0 = new Mmu("dmmu0", async_reset);
    dmmu0->i_clk(i_clk);
    dmmu0->i_nrst(i_nrst);
    dmmu0->o_core_req_ready(dmmu.fetch_req_ready);
    dmmu0->i_core_req_valid(w.m.req_data_valid);
    dmmu0->i_core_req_addr(w.m.req_data_addr);
    dmmu0->i_core_req_fetch(w_dmmu_core_req_fetch);
    dmmu0->i_core_req_type(w.m.req_data_type);
    dmmu0->i_core_req_wdata(w.m.req_data_wdata);
    dmmu0->i_core_req_wstrb(w.m.req_data_wstrb);
    dmmu0->i_core_req_size(w.m.req_data_size);
    dmmu0->o_core_resp_valid(dmmu.fetch_data_valid);
    dmmu0->o_core_resp_addr(dmmu.fetch_data_addr);
    dmmu0->o_core_resp_data(dmmu.fetch_data);
    dmmu0->o_core_resp_executable(dmmu.fetch_executable);
    dmmu0->o_core_resp_load_fault(dmmu.load_fault);
    dmmu0->o_core_resp_store_fault(dmmu.store_fault);
    dmmu0->o_core_resp_page_x_fault(dmmu.page_fault_x);
    dmmu0->o_core_resp_page_r_fault(dmmu.page_fault_r);
    dmmu0->o_core_resp_page_w_fault(dmmu.page_fault_w);
    dmmu0->i_core_resp_ready(w.m.resp_data_ready);
    dmmu0->i_mem_req_ready(i_req_data_ready);
    dmmu0->o_mem_req_valid(o_req_data_valid);
    dmmu0->o_mem_req_addr(o_req_data_addr);
    dmmu0->o_mem_req_type(o_req_data_type);
    dmmu0->o_mem_req_wdata(o_req_data_wdata);
    dmmu0->o_mem_req_wstrb(o_req_data_wstrb);
    dmmu0->o_mem_req_size(o_req_data_size);
    dmmu0->i_mem_resp_valid(i_resp_data_valid);
    dmmu0->i_mem_resp_addr(i_resp_data_addr);
    dmmu0->i_mem_resp_data(i_resp_data_data);
    dmmu0->i_mem_resp_executable(unused_dmmu_mem_resp_executable);
    dmmu0->i_mem_resp_load_fault(i_resp_data_load_fault);
    dmmu0->i_mem_resp_store_fault(i_resp_data_store_fault);
    dmmu0->o_mem_resp_ready(o_resp_data_ready);
    dmmu0->i_mmu_ena(w.m.mmu_ena);
    dmmu0->i_mmu_ppn(wb_mmu_ppn);
    dmmu0->i_fence(w_flush_pipeline);
    dmmu0->i_fence_addr(unused_immu_fence_addr);


    predic0 = new BranchPredictor("predic0", async_reset);
    predic0->i_clk(i_clk);
    predic0->i_nrst(i_nrst);
    predic0->i_flush_pipeline(w_flush_pipeline);
    predic0->i_resp_mem_valid(i_resp_ctrl_valid);
    predic0->i_resp_mem_addr(i_resp_ctrl_addr);
    predic0->i_resp_mem_data(i_resp_ctrl_data);
    predic0->i_e_jmp(w.e.jmp);
    predic0->i_e_pc(w.e.pc);
    predic0->i_e_npc(w.e.npc);
    predic0->i_ra(ireg.ra);
    predic0->o_f_valid(bp.f_valid);
    predic0->o_f_pc(bp.f_pc);
    predic0->i_f_requested_pc(w.f.requested_pc);
    predic0->i_f_fetching_pc(w.f.fetching_pc);
    predic0->i_f_fetched_pc(w.f.pc);
    predic0->i_d_pc(w.d.pc);


    iregs0 = new RegIntBank("iregs0", async_reset);
    iregs0->i_clk(i_clk);
    iregs0->i_nrst(i_nrst);
    iregs0->i_radr1(w.e.radr1);
    iregs0->o_rdata1(ireg.rdata1);
    iregs0->o_rtag1(ireg.rtag1);
    iregs0->i_radr2(w.e.radr2);
    iregs0->o_rdata2(ireg.rdata2);
    iregs0->o_rtag2(ireg.rtag2);
    iregs0->i_waddr(wb_reg_waddr);
    iregs0->i_wena(w_reg_wena);
    iregs0->i_wtag(wb_reg_wtag);
    iregs0->i_wdata(wb_reg_wdata);
    iregs0->i_inorder(w_reg_inorder);
    iregs0->o_ignored(w_reg_ignored);
    iregs0->i_dport_addr(dbg.ireg_addr);
    iregs0->i_dport_ena(dbg.ireg_ena);
    iregs0->i_dport_write(dbg.ireg_write);
    iregs0->i_dport_wdata(dbg.ireg_wdata);
    iregs0->o_dport_rdata(ireg.dport_rdata);
    iregs0->o_ra(ireg.ra);
    iregs0->o_sp(ireg.sp);


    iccsr0 = new ic_csr_m2_s1("iccsr0", async_reset);
    iccsr0->i_clk(i_clk);
    iccsr0->i_nrst(i_nrst);
    iccsr0->i_m0_req_valid(w.e.csr_req_valid);
    iccsr0->o_m0_req_ready(iccsr_m0_req_ready);
    iccsr0->i_m0_req_type(w.e.csr_req_type);
    iccsr0->i_m0_req_addr(w.e.csr_req_addr);
    iccsr0->i_m0_req_data(w.e.csr_req_data);
    iccsr0->o_m0_resp_valid(iccsr_m0_resp_valid);
    iccsr0->i_m0_resp_ready(w.e.csr_resp_ready);
    iccsr0->o_m0_resp_data(iccsr_m0_resp_data);
    iccsr0->o_m0_resp_exception(iccsr_m0_resp_exception);
    iccsr0->i_m1_req_valid(dbg.csr_req_valid);
    iccsr0->o_m1_req_ready(iccsr_m1_req_ready);
    iccsr0->i_m1_req_type(dbg.csr_req_type);
    iccsr0->i_m1_req_addr(dbg.csr_req_addr);
    iccsr0->i_m1_req_data(dbg.csr_req_data);
    iccsr0->o_m1_resp_valid(iccsr_m1_resp_valid);
    iccsr0->i_m1_resp_ready(dbg.csr_resp_ready);
    iccsr0->o_m1_resp_data(iccsr_m1_resp_data);
    iccsr0->o_m1_resp_exception(iccsr_m1_resp_exception);
    iccsr0->o_s0_req_valid(iccsr_s0_req_valid);
    iccsr0->i_s0_req_ready(csr.req_ready);
    iccsr0->o_s0_req_type(iccsr_s0_req_type);
    iccsr0->o_s0_req_addr(iccsr_s0_req_addr);
    iccsr0->o_s0_req_data(iccsr_s0_req_data);
    iccsr0->i_s0_resp_valid(csr.resp_valid);
    iccsr0->o_s0_resp_ready(iccsr_s0_resp_ready);
    iccsr0->i_s0_resp_data(csr.resp_data);
    iccsr0->i_s0_resp_exception(csr.resp_exception);


    csr0 = new CsrRegs("csr0", async_reset, hartid);
    csr0->i_clk(i_clk);
    csr0->i_nrst(i_nrst);
    csr0->i_sp(ireg.sp);
    csr0->i_req_valid(iccsr_s0_req_valid);
    csr0->o_req_ready(csr.req_ready);
    csr0->i_req_type(iccsr_s0_req_type);
    csr0->i_req_addr(iccsr_s0_req_addr);
    csr0->i_req_data(iccsr_s0_req_data);
    csr0->o_resp_valid(csr.resp_valid);
    csr0->i_resp_ready(iccsr_s0_resp_ready);
    csr0->o_resp_data(csr.resp_data);
    csr0->o_resp_exception(csr.resp_exception);
    csr0->i_e_halted(w.e.halted);
    csr0->i_e_pc(w.e.pc);
    csr0->i_e_instr(w.e.instr);
    csr0->i_irq_pending(i_irq_pending);
    csr0->o_irq_software(csr.irq_software);
    csr0->o_irq_timer(csr.irq_timer);
    csr0->o_irq_external(csr.irq_external);
    csr0->o_stack_overflow(csr.stack_overflow);
    csr0->o_stack_underflow(csr.stack_underflow);
    csr0->i_e_valid(w.e.valid);
    csr0->o_executed_cnt(csr.executed_cnt);
    csr0->o_step(csr.step);
    csr0->i_dbg_progbuf_ena(dbg.progbuf_ena);
    csr0->o_progbuf_end(csr.progbuf_end);
    csr0->o_progbuf_error(csr.progbuf_error);
    csr0->o_flushi_ena(csr.flushi_ena);
    csr0->o_flushi_addr(csr.flushi_addr);
    csr0->o_mpu_region_we(o_mpu_region_we);
    csr0->o_mpu_region_idx(o_mpu_region_idx);
    csr0->o_mpu_region_addr(o_mpu_region_addr);
    csr0->o_mpu_region_mask(o_mpu_region_mask);
    csr0->o_mpu_region_flags(o_mpu_region_flags);
    csr0->o_mmu_ena(w_mmu_ena);
    csr0->o_mmu_ppn(wb_mmu_ppn);


    dbg0 = new DbgPort("dbg0", async_reset);
    dbg0->i_clk(i_clk);
    dbg0->i_nrst(i_nrst);
    dbg0->i_dport_req_valid(i_dport_req_valid);
    dbg0->i_dport_type(i_dport_type);
    dbg0->i_dport_addr(i_dport_addr);
    dbg0->i_dport_wdata(i_dport_wdata);
    dbg0->i_dport_size(i_dport_size);
    dbg0->o_dport_req_ready(o_dport_req_ready);
    dbg0->i_dport_resp_ready(i_dport_resp_ready);
    dbg0->o_dport_resp_valid(o_dport_resp_valid);
    dbg0->o_dport_resp_error(o_dport_resp_error);
    dbg0->o_dport_rdata(o_dport_rdata);
    dbg0->o_csr_req_valid(dbg.csr_req_valid);
    dbg0->i_csr_req_ready(iccsr_m1_req_ready);
    dbg0->o_csr_req_type(dbg.csr_req_type);
    dbg0->o_csr_req_addr(dbg.csr_req_addr);
    dbg0->o_csr_req_data(dbg.csr_req_data);
    dbg0->i_csr_resp_valid(iccsr_m1_resp_valid);
    dbg0->o_csr_resp_ready(dbg.csr_resp_ready);
    dbg0->i_csr_resp_data(iccsr_m1_resp_data);
    dbg0->i_csr_resp_exception(iccsr_m1_resp_exception);
    dbg0->i_progbuf(i_progbuf);
    dbg0->o_progbuf_ena(dbg.progbuf_ena);
    dbg0->o_progbuf_pc(dbg.progbuf_pc);
    dbg0->o_progbuf_instr(dbg.progbuf_instr);
    dbg0->i_csr_progbuf_end(csr.progbuf_end);
    dbg0->i_csr_progbuf_error(csr.progbuf_error);
    dbg0->o_ireg_addr(dbg.ireg_addr);
    dbg0->o_ireg_wdata(dbg.ireg_wdata);
    dbg0->o_ireg_ena(dbg.ireg_ena);
    dbg0->o_ireg_write(dbg.ireg_write);
    dbg0->i_ireg_rdata(ireg.dport_rdata);
    dbg0->o_mem_req_valid(dbg.mem_req_valid);
    dbg0->i_mem_req_ready(w.e.dbg_mem_req_ready);
    dbg0->i_mem_req_error(w.e.dbg_mem_req_error);
    dbg0->o_mem_req_write(dbg.mem_req_write);
    dbg0->o_mem_req_addr(dbg.mem_req_addr);
    dbg0->o_mem_req_size(dbg.mem_req_size);
    dbg0->o_mem_req_wdata(dbg.mem_req_wdata);
    dbg0->i_mem_resp_valid(w.m.debug_valid);
    dbg0->i_mem_resp_error(w_mem_resp_error);
    dbg0->i_mem_resp_rdata(w.w.wdata);
    dbg0->i_e_pc(w.e.pc);
    dbg0->i_e_npc(w.e.npc);
    dbg0->i_e_call(w.e.call);
    dbg0->i_e_ret(w.e.ret);
    dbg0->i_e_memop_valid(w.e.memop_valid);
    dbg0->i_m_valid(w.m.valid);


    // generate
    if (tracer_ena_) {
        trace0 = new Tracer("trace0", async_reset, hartid, trace_file);
        trace0->i_clk(i_clk);
        trace0->i_nrst(i_nrst);
        trace0->i_dbg_executed_cnt(csr.executed_cnt);
        trace0->i_e_valid(w.e.valid);
        trace0->i_e_pc(w.e.pc);
        trace0->i_e_instr(w.e.instr);
        trace0->i_e_wena(w.e.reg_wena);
        trace0->i_e_waddr(w.e.reg_waddr);
        trace0->i_e_wdata(w.e.reg_wdata);
        trace0->i_e_memop_valid(w.e.memop_valid);
        trace0->i_e_memop_type(w.e.memop_type);
        trace0->i_e_memop_size(w.e.memop_size);
        trace0->i_e_memop_addr(w.e.memop_addr);
        trace0->i_e_memop_wdata(w.e.memop_wdata);
        trace0->i_e_flushd(w.e.flushd);
        trace0->i_m_pc(w.m.pc);
        trace0->i_m_valid(w.m.valid);
        trace0->i_m_memop_ready(w.m.memop_ready);
        trace0->i_m_wena(w.w.wena);
        trace0->i_m_waddr(w.w.waddr);
        trace0->i_m_wdata(w.w.wdata);
        trace0->i_reg_ignored(w_reg_ignored);
    }

    // endgenerate


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_ready;
    sensitive << i_resp_ctrl_valid;
    sensitive << i_resp_ctrl_addr;
    sensitive << i_resp_ctrl_data;
    sensitive << i_resp_ctrl_load_fault;
    sensitive << i_resp_ctrl_executable;
    sensitive << i_req_data_ready;
    sensitive << i_resp_data_valid;
    sensitive << i_resp_data_addr;
    sensitive << i_resp_data_data;
    sensitive << i_resp_data_fault_addr;
    sensitive << i_resp_data_load_fault;
    sensitive << i_resp_data_store_fault;
    sensitive << i_resp_data_er_mpu_load;
    sensitive << i_resp_data_er_mpu_store;
    sensitive << i_irq_pending;
    sensitive << i_haltreq;
    sensitive << i_resumereq;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_type;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_dport_size;
    sensitive << i_dport_resp_ready;
    sensitive << i_progbuf;
    sensitive << i_data_flush_end;
    sensitive << w.f.instr_load_fault;
    sensitive << w.f.instr_executable;
    sensitive << w.f.requested_pc;
    sensitive << w.f.fetching_pc;
    sensitive << w.f.pc;
    sensitive << w.f.instr;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.imem_resp_ready;
    sensitive << w.d.pc;
    sensitive << w.d.instr;
    sensitive << w.d.memop_store;
    sensitive << w.d.memop_load;
    sensitive << w.d.memop_sign_ext;
    sensitive << w.d.memop_size;
    sensitive << w.d.rv32;
    sensitive << w.d.compressed;
    sensitive << w.d.amo;
    sensitive << w.d.f64;
    sensitive << w.d.unsigned_op;
    sensitive << w.d.isa_type;
    sensitive << w.d.instr_vec;
    sensitive << w.d.exception;
    sensitive << w.d.instr_load_fault;
    sensitive << w.d.instr_executable;
    sensitive << w.d.radr1;
    sensitive << w.d.radr2;
    sensitive << w.d.waddr;
    sensitive << w.d.csr_addr;
    sensitive << w.d.imm;
    sensitive << w.d.progbuf_ena;
    sensitive << w.e.valid;
    sensitive << w.e.instr;
    sensitive << w.e.pc;
    sensitive << w.e.npc;
    sensitive << w.e.radr1;
    sensitive << w.e.radr2;
    sensitive << w.e.reg_wena;
    sensitive << w.e.reg_waddr;
    sensitive << w.e.reg_wtag;
    sensitive << w.e.reg_wdata;
    sensitive << w.e.csr_req_valid;
    sensitive << w.e.csr_req_type;
    sensitive << w.e.csr_req_addr;
    sensitive << w.e.csr_req_data;
    sensitive << w.e.csr_resp_ready;
    sensitive << w.e.memop_valid;
    sensitive << w.e.memop_debug;
    sensitive << w.e.memop_sign_ext;
    sensitive << w.e.memop_type;
    sensitive << w.e.memop_size;
    sensitive << w.e.memop_addr;
    sensitive << w.e.memop_wdata;
    sensitive << w.e.flushd;
    sensitive << w.e.flushi;
    sensitive << w.e.flushi_addr;
    sensitive << w.e.call;
    sensitive << w.e.ret;
    sensitive << w.e.jmp;
    sensitive << w.e.halted;
    sensitive << w.e.dbg_mem_req_ready;
    sensitive << w.e.dbg_mem_req_error;
    sensitive << w.m.memop_ready;
    sensitive << w.m.flushd;
    sensitive << w.m.pc;
    sensitive << w.m.valid;
    sensitive << w.m.debug_valid;
    sensitive << w.m.mmu_ena;
    sensitive << w.m.req_data_valid;
    sensitive << w.m.req_data_type;
    sensitive << w.m.req_data_addr;
    sensitive << w.m.req_data_wdata;
    sensitive << w.m.req_data_wstrb;
    sensitive << w.m.req_data_size;
    sensitive << w.m.resp_data_ready;
    sensitive << w.w.wena;
    sensitive << w.w.waddr;
    sensitive << w.w.wdata;
    sensitive << w.w.wtag;
    sensitive << immu.fetch_req_ready;
    sensitive << immu.fetch_data_valid;
    sensitive << immu.fetch_data_addr;
    sensitive << immu.fetch_data;
    sensitive << immu.fetch_executable;
    sensitive << immu.load_fault;
    sensitive << immu.store_fault;
    sensitive << immu.page_fault_x;
    sensitive << immu.page_fault_r;
    sensitive << immu.page_fault_w;
    sensitive << dmmu.fetch_req_ready;
    sensitive << dmmu.fetch_data_valid;
    sensitive << dmmu.fetch_data_addr;
    sensitive << dmmu.fetch_data;
    sensitive << dmmu.fetch_executable;
    sensitive << dmmu.load_fault;
    sensitive << dmmu.store_fault;
    sensitive << dmmu.page_fault_x;
    sensitive << dmmu.page_fault_r;
    sensitive << dmmu.page_fault_w;
    sensitive << ireg.rdata1;
    sensitive << ireg.rtag1;
    sensitive << ireg.rdata2;
    sensitive << ireg.rtag2;
    sensitive << ireg.dport_rdata;
    sensitive << ireg.ra;
    sensitive << ireg.sp;
    sensitive << csr.req_ready;
    sensitive << csr.resp_valid;
    sensitive << csr.resp_data;
    sensitive << csr.resp_exception;
    sensitive << csr.flushi_ena;
    sensitive << csr.flushi_addr;
    sensitive << csr.executed_cnt;
    sensitive << csr.irq_software;
    sensitive << csr.irq_timer;
    sensitive << csr.irq_external;
    sensitive << csr.stack_overflow;
    sensitive << csr.stack_underflow;
    sensitive << csr.step;
    sensitive << csr.progbuf_end;
    sensitive << csr.progbuf_error;
    sensitive << dbg.csr_req_valid;
    sensitive << dbg.csr_req_type;
    sensitive << dbg.csr_req_addr;
    sensitive << dbg.csr_req_data;
    sensitive << dbg.csr_resp_ready;
    sensitive << dbg.ireg_addr;
    sensitive << dbg.ireg_wdata;
    sensitive << dbg.ireg_ena;
    sensitive << dbg.ireg_write;
    sensitive << dbg.mem_req_valid;
    sensitive << dbg.mem_req_write;
    sensitive << dbg.mem_req_addr;
    sensitive << dbg.mem_req_size;
    sensitive << dbg.mem_req_wdata;
    sensitive << dbg.progbuf_ena;
    sensitive << dbg.progbuf_pc;
    sensitive << dbg.progbuf_instr;
    sensitive << bp.f_valid;
    sensitive << bp.f_pc;
    sensitive << iccsr_m0_req_ready;
    sensitive << iccsr_m0_resp_valid;
    sensitive << iccsr_m0_resp_data;
    sensitive << iccsr_m0_resp_exception;
    sensitive << iccsr_m1_req_ready;
    sensitive << iccsr_m1_resp_valid;
    sensitive << iccsr_m1_resp_data;
    sensitive << iccsr_m1_resp_exception;
    sensitive << iccsr_s0_req_valid;
    sensitive << iccsr_s0_req_type;
    sensitive << iccsr_s0_req_addr;
    sensitive << iccsr_s0_req_data;
    sensitive << iccsr_s0_resp_ready;
    sensitive << iccsr_s0_resp_exception;
    sensitive << w_flush_pipeline;
    sensitive << w_mem_resp_error;
    sensitive << w_writeback_ready;
    sensitive << w_reg_wena;
    sensitive << wb_reg_waddr;
    sensitive << wb_reg_wdata;
    sensitive << wb_reg_wtag;
    sensitive << w_reg_inorder;
    sensitive << w_reg_ignored;
    sensitive << w_mmu_ena;
    sensitive << wb_mmu_ppn;
    sensitive << unused_immu_mem_req_type;
    sensitive << unused_immu_mem_req_wdata;
    sensitive << unused_immu_mem_req_wstrb;
    sensitive << unused_immu_mem_req_size;
    sensitive << w_immu_core_req_fetch;
    sensitive << w_dmmu_core_req_fetch;
    sensitive << unused_immu_core_req_type;
    sensitive << unused_immu_core_req_wdata;
    sensitive << unused_immu_core_req_wstrb;
    sensitive << unused_immu_core_req_size;
    sensitive << unused_immu_mem_resp_store_fault;
    sensitive << unused_immu_fence_addr;
    sensitive << unused_dmmu_mem_resp_executable;
}

Processor::~Processor() {
    if (fetch0) {
        delete fetch0;
    }
    if (dec0) {
        delete dec0;
    }
    if (exec0) {
        delete exec0;
    }
    if (mem0) {
        delete mem0;
    }
    if (immu0) {
        delete immu0;
    }
    if (dmmu0) {
        delete dmmu0;
    }
    if (predic0) {
        delete predic0;
    }
    if (iregs0) {
        delete iregs0;
    }
    if (iccsr0) {
        delete iccsr0;
    }
    if (csr0) {
        delete csr0;
    }
    if (trace0) {
        delete trace0;
    }
    if (dbg0) {
        delete dbg0;
    }
}

void Processor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_ctrl_ready, i_req_ctrl_ready.name());
        sc_trace(o_vcd, o_req_ctrl_valid, o_req_ctrl_valid.name());
        sc_trace(o_vcd, o_req_ctrl_addr, o_req_ctrl_addr.name());
        sc_trace(o_vcd, i_resp_ctrl_valid, i_resp_ctrl_valid.name());
        sc_trace(o_vcd, i_resp_ctrl_addr, i_resp_ctrl_addr.name());
        sc_trace(o_vcd, i_resp_ctrl_data, i_resp_ctrl_data.name());
        sc_trace(o_vcd, i_resp_ctrl_load_fault, i_resp_ctrl_load_fault.name());
        sc_trace(o_vcd, i_resp_ctrl_executable, i_resp_ctrl_executable.name());
        sc_trace(o_vcd, o_resp_ctrl_ready, o_resp_ctrl_ready.name());
        sc_trace(o_vcd, i_req_data_ready, i_req_data_ready.name());
        sc_trace(o_vcd, o_req_data_valid, o_req_data_valid.name());
        sc_trace(o_vcd, o_req_data_type, o_req_data_type.name());
        sc_trace(o_vcd, o_req_data_addr, o_req_data_addr.name());
        sc_trace(o_vcd, o_req_data_wdata, o_req_data_wdata.name());
        sc_trace(o_vcd, o_req_data_wstrb, o_req_data_wstrb.name());
        sc_trace(o_vcd, o_req_data_size, o_req_data_size.name());
        sc_trace(o_vcd, i_resp_data_valid, i_resp_data_valid.name());
        sc_trace(o_vcd, i_resp_data_addr, i_resp_data_addr.name());
        sc_trace(o_vcd, i_resp_data_data, i_resp_data_data.name());
        sc_trace(o_vcd, i_resp_data_fault_addr, i_resp_data_fault_addr.name());
        sc_trace(o_vcd, i_resp_data_load_fault, i_resp_data_load_fault.name());
        sc_trace(o_vcd, i_resp_data_store_fault, i_resp_data_store_fault.name());
        sc_trace(o_vcd, i_resp_data_er_mpu_load, i_resp_data_er_mpu_load.name());
        sc_trace(o_vcd, i_resp_data_er_mpu_store, i_resp_data_er_mpu_store.name());
        sc_trace(o_vcd, o_resp_data_ready, o_resp_data_ready.name());
        sc_trace(o_vcd, i_irq_pending, i_irq_pending.name());
        sc_trace(o_vcd, o_mpu_region_we, o_mpu_region_we.name());
        sc_trace(o_vcd, o_mpu_region_idx, o_mpu_region_idx.name());
        sc_trace(o_vcd, o_mpu_region_addr, o_mpu_region_addr.name());
        sc_trace(o_vcd, o_mpu_region_mask, o_mpu_region_mask.name());
        sc_trace(o_vcd, o_mpu_region_flags, o_mpu_region_flags.name());
        sc_trace(o_vcd, i_haltreq, i_haltreq.name());
        sc_trace(o_vcd, i_resumereq, i_resumereq.name());
        sc_trace(o_vcd, i_dport_req_valid, i_dport_req_valid.name());
        sc_trace(o_vcd, i_dport_type, i_dport_type.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, i_dport_size, i_dport_size.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, i_dport_resp_ready, i_dport_resp_ready.name());
        sc_trace(o_vcd, o_dport_resp_valid, o_dport_resp_valid.name());
        sc_trace(o_vcd, o_dport_resp_error, o_dport_resp_error.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, i_progbuf, i_progbuf.name());
        sc_trace(o_vcd, o_halted, o_halted.name());
        sc_trace(o_vcd, o_flush_address, o_flush_address.name());
        sc_trace(o_vcd, o_flush_valid, o_flush_valid.name());
        sc_trace(o_vcd, o_data_flush_address, o_data_flush_address.name());
        sc_trace(o_vcd, o_data_flush_valid, o_data_flush_valid.name());
        sc_trace(o_vcd, i_data_flush_end, i_data_flush_end.name());
    }

    if (fetch0) {
        fetch0->generateVCD(i_vcd, o_vcd);
    }
    if (dec0) {
        dec0->generateVCD(i_vcd, o_vcd);
    }
    if (exec0) {
        exec0->generateVCD(i_vcd, o_vcd);
    }
    if (mem0) {
        mem0->generateVCD(i_vcd, o_vcd);
    }
    if (immu0) {
        immu0->generateVCD(i_vcd, o_vcd);
    }
    if (dmmu0) {
        dmmu0->generateVCD(i_vcd, o_vcd);
    }
    if (predic0) {
        predic0->generateVCD(i_vcd, o_vcd);
    }
    if (iregs0) {
        iregs0->generateVCD(i_vcd, o_vcd);
    }
    if (iccsr0) {
        iccsr0->generateVCD(i_vcd, o_vcd);
    }
    if (csr0) {
        csr0->generateVCD(i_vcd, o_vcd);
    }
    if (trace0) {
        trace0->generateVCD(i_vcd, o_vcd);
    }
    if (dbg0) {
        dbg0->generateVCD(i_vcd, o_vcd);
    }
}

void Processor::comb() {
    sc_uint<CFG_CPU_ADDR_BITS> vb_flush_address;

    vb_flush_address = 0;

    w_mem_resp_error = (i_resp_data_load_fault || i_resp_data_store_fault || i_resp_data_er_mpu_store || i_resp_data_er_mpu_load);
    w_writeback_ready = (!w.e.reg_wena);
    if (w.e.reg_wena.read() == 1) {
        w_reg_wena = w.e.reg_wena;
        wb_reg_waddr = w.e.reg_waddr;
        wb_reg_wdata = w.e.reg_wdata;
        wb_reg_wtag = w.e.reg_wtag;
        w_reg_inorder = 0;                                 // Executor can overwrite memory loading before it was loaded
    } else {
        w_reg_wena = w.w.wena;
        wb_reg_waddr = w.w.waddr;
        wb_reg_wdata = w.w.wdata;
        wb_reg_wtag = w.w.wtag;
        w_reg_inorder = 1;                                 // Cannot write loaded from memory value if it was overwritten
    }
    w_flush_pipeline = (w.e.flushi || csr.flushi_ena);
    if (w.e.flushi.read() == 1) {
        // fencei or ebreak instructions
        vb_flush_address = w.e.flushi_addr;
    } else {
        // request through debug interface to clear cache
        vb_flush_address = csr.flushi_addr;
    }
    w_immu_core_req_fetch = 0;
    w_dmmu_core_req_fetch = 1;
    unused_immu_core_req_type = 0;
    unused_immu_core_req_wdata = 0;
    unused_immu_core_req_wstrb = 0;
    unused_immu_core_req_size = 0;
    unused_immu_mem_resp_store_fault = 0;
    unused_immu_fence_addr = 0;
    o_flush_valid = w_flush_pipeline;
    o_flush_address = vb_flush_address;
    o_data_flush_address = ~0ull;
    o_data_flush_valid = w.m.flushd;
    o_halted = w.e.halted;
}

}  // namespace debugger

