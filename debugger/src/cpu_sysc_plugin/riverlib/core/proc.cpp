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

#include "proc.h"
#include "api_core.h"

namespace debugger {

Processor::Processor(sc_module_name name_, uint32_t hartid, bool async_reset,
    bool fpu_ena, bool tracer_ena) : sc_module(name_),
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
    i_ext_irq("i_ext_irq"),
    o_mpu_region_we("o_mpu_region_we"),
    o_mpu_region_idx("o_mpu_region_idx"),
    o_mpu_region_addr("o_mpu_region_addr"),
    o_mpu_region_mask("o_mpu_region_mask"),
    o_mpu_region_flags("o_mpu_region_flags"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_write("i_dport_write"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_rdata("o_dport_rdata"),
    o_halted("o_halted"),
    o_flush_address("o_flush_address"),
    o_flush_valid("o_flush_valid"),
    o_data_flush_address("o_data_flush_address"),
    o_data_flush_valid("o_data_flush_valid"),
    i_data_flush_end("i_data_flush_end") {
    fpu_ena_ = fpu_ena;
    tracer_ena_ = tracer_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_ctrl_valid;
    sensitive << w.f.pipeline_hold;
    sensitive << w.e.npc;
    sensitive << w.e.valid;
    sensitive << w.e.d_ready;
    sensitive << w.e.reg_wena;
    sensitive << w.e.reg_waddr;
    sensitive << w.e.reg_wtag;
    sensitive << w.e.reg_wdata;
    sensitive << w.e.flushi;
    sensitive << w.e.flushi_addr;
    sensitive << w.e.halted;
    sensitive << w.w.wena;
    sensitive << w.w.waddr;
    sensitive << w.w.wdata;
    sensitive << w.w.wtag;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.valid;
    sensitive << w.m.flushd;
    sensitive << csr.executed_cnt;
    sensitive << dbg.reg_addr;
    sensitive << csr.halt;
    sensitive << dbg.core_wdata;
    sensitive << csr.flushi_ena;
    sensitive << csr.flushi_addr;
    sensitive << w_fetch_pipeline_hold;
    sensitive << w_any_pipeline_hold;
    sensitive << w_flush_pipeline;

    fetch0 = new InstrFetch("fetch0", async_reset);
    fetch0->i_clk(i_clk);
    fetch0->i_nrst(i_nrst);
    fetch0->i_pipeline_hold(w_fetch_pipeline_hold);
    fetch0->i_mem_req_ready(i_req_ctrl_ready);
    fetch0->o_mem_addr_valid(w.f.imem_req_valid);
    fetch0->o_mem_addr(w.f.imem_req_addr);
    fetch0->i_mem_data_valid(i_resp_ctrl_valid);
    fetch0->i_mem_data_addr(i_resp_ctrl_addr);
    fetch0->i_mem_data(i_resp_ctrl_data);
    fetch0->i_mem_load_fault(i_resp_ctrl_load_fault);
    fetch0->i_mem_executable(i_resp_ctrl_executable);
    fetch0->o_mem_resp_ready(o_resp_ctrl_ready);
    fetch0->i_flush_pipeline(w_flush_pipeline);
    fetch0->i_progbuf_ena(csr.progbuf_ena);
    fetch0->i_progbuf_pc(csr.progbuf_pc);
    fetch0->i_progbuf_data(csr.progbuf_data);
    fetch0->i_predict_npc(bp.npc);
    fetch0->o_mem_req_fire(w.f.req_fire);
    fetch0->o_instr_load_fault(w.f.instr_load_fault);
    fetch0->o_instr_executable(w.f.instr_executable);
    fetch0->o_valid(w.f.valid);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);
    fetch0->o_hold(w.f.pipeline_hold);

    dec0 = new InstrDecoder("dec0", async_reset, fpu_ena);
    dec0->i_clk(i_clk);
    dec0->i_nrst(i_nrst);
    dec0->i_any_hold(w_any_pipeline_hold);
    dec0->i_f_valid(w.f.valid);
    dec0->i_f_pc(w.f.pc);
    dec0->i_f_instr(w.f.instr);
    dec0->i_instr_load_fault(w.f.instr_load_fault);
    dec0->i_instr_executable(w.f.instr_executable);
    dec0->o_radr1(w.d.radr1);
    dec0->o_radr2(w.d.radr2);
    dec0->o_waddr(w.d.waddr);
    dec0->o_csr_addr(w.d.csr_addr),
    dec0->o_imm(w.d.imm);
    dec0->i_e_ready(w.e.d_ready);
    dec0->i_flush_pipeline(w_flush_pipeline);
    dec0->i_progbuf_ena(csr.progbuf_ena);
    dec0->o_valid(w.d.instr_valid);
    dec0->o_pc(w.d.pc);
    dec0->o_instr(w.d.instr);
    dec0->o_memop_store(w.d.memop_store);
    dec0->o_memop_load(w.d.memop_load);
    dec0->o_memop_sign_ext(w.d.memop_sign_ext);
    dec0->o_memop_size(w.d.memop_size);
    dec0->o_unsigned_op(w.d.unsigned_op);
    dec0->o_rv32(w.d.rv32);
    dec0->o_compressed(w.d.compressed);
    dec0->o_amo(w.d.amo);
    dec0->o_f64(w.d.f64);
    dec0->o_isa_type(w.d.isa_type);
    dec0->o_instr_vec(w.d.instr_vec);
    dec0->o_exception(w.d.exception);
    dec0->o_instr_load_fault(w.d.instr_load_fault);
    dec0->o_instr_executable(w.d.instr_executable);
    dec0->o_progbuf_ena(w.d.progbuf_ena);

    exec0 = new InstrExecute("exec0", async_reset, fpu_ena);
    exec0->i_clk(i_clk);
    exec0->i_nrst(i_nrst);
    exec0->i_d_valid(w.d.instr_valid);
    exec0->i_d_pc(w.d.pc);
    exec0->i_d_instr(w.d.instr);
    exec0->i_d_progbuf_ena(w.d.progbuf_ena);
    exec0->i_dbg_progbuf_ena(csr.progbuf_ena);
    exec0->i_d_radr1(w.d.radr1);
    exec0->i_d_radr2(w.d.radr2);
    exec0->i_d_waddr(w.d.waddr);
    exec0->i_d_csr_addr(w.d.csr_addr);
    exec0->i_d_imm(w.d.imm);
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
    exec0->i_unsup_exception(w.d.exception);
    exec0->i_instr_load_fault(w.d.instr_load_fault);
    exec0->i_instr_executable(w.d.instr_executable);
    exec0->i_mem_ex_load_fault(i_resp_data_load_fault);
    exec0->i_mem_ex_store_fault(i_resp_data_store_fault);
    exec0->i_mem_ex_mpu_store(i_resp_data_er_mpu_store);
    exec0->i_mem_ex_mpu_load(i_resp_data_er_mpu_load);
    exec0->i_mem_ex_addr(i_resp_data_fault_addr);
    exec0->i_irq_software(csr.irq_software);
    exec0->i_irq_timer(csr.irq_timer);
    exec0->i_irq_external(csr.irq_external);
    exec0->i_halt(csr.halt);
    exec0->i_dport_npc_write(csr.dbg_pc_write);
    exec0->i_dport_npc(csr.dbg_pc);
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
    exec0->o_d_ready(w.e.d_ready);
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
    exec0->o_memop_sign_ext(w.e.memop_sign_ext);
    exec0->o_memop_type(w.e.memop_type);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_memop_memaddr(w.e.memop_addr);
    exec0->o_memop_wdata(w.e.memop_wdata);
    exec0->i_memop_ready(w.m.memop_ready);
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
    exec0->o_halted(w.e.halted);

    mem0 = new MemAccess("mem0", async_reset);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_e_pc(w.e.pc);
    mem0->i_e_instr(w.e.instr);
    mem0->i_e_flushd(w.e.flushd);
    mem0->o_flushd(w.m.flushd);
    mem0->i_reg_waddr(w.e.reg_waddr);
    mem0->i_reg_wtag(w.e.reg_wtag);
    mem0->i_memop_valid(w.e.memop_valid);
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
    mem0->i_mem_req_ready(i_req_data_ready);
    mem0->o_mem_valid(o_req_data_valid);
    mem0->o_mem_type(o_req_data_type);
    mem0->o_mem_addr(o_req_data_addr);
    mem0->o_mem_wdata(o_req_data_wdata);
    mem0->o_mem_wstrb(o_req_data_wstrb);
    mem0->o_mem_size(o_req_data_size);
    mem0->i_mem_data_valid(i_resp_data_valid);
    mem0->i_mem_data_addr(i_resp_data_addr);
    mem0->i_mem_data(i_resp_data_data);
    mem0->o_mem_resp_ready(o_resp_data_ready);

    predic0 = new BranchPredictor("predic0", async_reset);
    predic0->i_clk(i_clk);
    predic0->i_nrst(i_nrst);
    predic0->i_req_mem_fire(w.f.req_fire);
    predic0->i_resp_mem_valid(i_resp_ctrl_valid);
    predic0->i_resp_mem_addr(i_resp_ctrl_addr);
    predic0->i_resp_mem_data(i_resp_ctrl_data);
    predic0->i_e_npc(w.e.npc);
    predic0->i_ra(ireg.ra);
    predic0->o_npc_predict(bp.npc);

    iregs0 = new RegIntBank("iregs0", async_reset, fpu_ena);
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
    iregs0->i_dport_addr(dbg.reg_addr);
    iregs0->i_dport_ena(dbg.ireg_ena);
    iregs0->i_dport_write(dbg.ireg_write);
    iregs0->i_dport_wdata(dbg.core_wdata);
    iregs0->o_dport_rdata(ireg.dport_rdata);
    iregs0->o_ra(ireg.ra);
    iregs0->o_sp(ireg.sp);

    iccsr0 = new ic_csr_m2_s1("icscr0", async_reset);
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

    csr0 = new CsrRegs("csr0", hartid, async_reset);
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
    csr0->i_e_pc(w.e.pc);
    csr0->i_e_npc(w.e.npc);
    csr0->i_irq_external(i_ext_irq);
    csr0->o_irq_software(csr.irq_software);
    csr0->o_irq_timer(csr.irq_timer);
    csr0->o_irq_external(csr.irq_external);
    csr0->i_e_valid(w.e.valid);
    csr0->o_executed_cnt(csr.executed_cnt);
    csr0->o_dbg_pc_write(csr.dbg_pc_write);
    csr0->o_dbg_pc(csr.dbg_pc);
    csr0->o_progbuf_ena(csr.progbuf_ena);
    csr0->o_progbuf_pc(csr.progbuf_pc);
    csr0->o_progbuf_data(csr.progbuf_data);
    csr0->o_flushi_ena(csr.flushi_ena);
    csr0->o_flushi_addr(csr.flushi_addr);
    csr0->o_mpu_region_we(o_mpu_region_we);
    csr0->o_mpu_region_idx(o_mpu_region_idx);
    csr0->o_mpu_region_addr(o_mpu_region_addr);
    csr0->o_mpu_region_mask(o_mpu_region_mask);
    csr0->o_mpu_region_flags(o_mpu_region_flags);
    csr0->o_halt(csr.halt);

    dbg0 = new DbgPort("dbg0", async_reset);
    dbg0->i_clk(i_clk);
    dbg0->i_nrst(i_nrst);
    dbg0->i_dport_req_valid(i_dport_req_valid);
    dbg0->i_dport_write(i_dport_write);
    dbg0->i_dport_addr(i_dport_addr);
    dbg0->i_dport_wdata(i_dport_wdata);
    dbg0->o_dport_req_ready(o_dport_req_ready);
    dbg0->i_dport_resp_ready(i_dport_resp_ready);
    dbg0->o_dport_resp_valid(o_dport_resp_valid);
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
    dbg0->o_reg_addr(dbg.reg_addr);
    dbg0->o_core_wdata(dbg.core_wdata);
    dbg0->o_ireg_ena(dbg.ireg_ena);
    dbg0->o_ireg_write(dbg.ireg_write);
    dbg0->i_ireg_rdata(ireg.dport_rdata);
    dbg0->i_pc(w.e.pc);
    dbg0->i_npc(w.e.npc);
    dbg0->i_e_call(w.e.call);
    dbg0->i_e_ret(w.e.ret);

    trace0 = 0;
    if (tracer_ena) {
        trace0 = new Tracer("trace0", async_reset, "trace_river_sysc.log");
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
        trace0->i_e_memop_addr(w.e.memop_addr);
        trace0->i_e_memop_wdata(w.e.memop_wdata);
        trace0->i_m_memop_ready(w.m.memop_ready);
        trace0->i_m_wena(w.w.wena);
        trace0->i_m_waddr(w.w.waddr);
        trace0->i_m_wdata(w.w.wdata);
        trace0->i_reg_ignored(w_reg_ignored);
    }
};

Processor::~Processor() {
    delete fetch0;
    delete dec0;
    delete exec0;
    delete mem0;
    delete predic0;
    delete iregs0;
    if (trace0) {
        delete trace0;
    }
    delete csr0;
    delete dbg0;
    delete iccsr0;
}

void Processor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    predic0->generateVCD(i_vcd, o_vcd);
    csr0->generateVCD(i_vcd, o_vcd);
    dbg0->generateVCD(i_vcd, o_vcd);
    dec0->generateVCD(i_vcd, o_vcd);
    exec0->generateVCD(i_vcd, o_vcd);
    fetch0->generateVCD(i_vcd, o_vcd);
    mem0->generateVCD(i_vcd, o_vcd);
    iregs0->generateVCD(i_vcd, o_vcd);
    iccsr0->generateVCD(i_vcd, o_vcd);
    if (trace0) {
        trace0->generateVCD(i_vcd, o_vcd);
    }
}

void Processor::comb() {
    w_fetch_pipeline_hold = !w.e.d_ready | csr.halt;
    w_any_pipeline_hold = w.f.pipeline_hold | !w.e.d_ready | csr.halt;

    w_writeback_ready = !w.e.reg_wena.read();
    if (w.e.reg_wena.read() == 1) {
        w_reg_wena = w.e.reg_wena;
        wb_reg_waddr = w.e.reg_waddr;
        wb_reg_wdata = w.e.reg_wdata;
        wb_reg_wtag = w.e.reg_wtag;
        w_reg_inorder = 0;      // Executor can overwrite memory loading before it was loaded
    } else {
        w_reg_wena = w.w.wena;
        wb_reg_waddr = w.w.waddr;
        wb_reg_wdata = w.w.wdata;
        wb_reg_wtag = w.w.wtag;
        w_reg_inorder = 1;      // Cannot write loaded from memory value if it was overwritten
    }

    w_flush_pipeline = w.e.flushi.read() || csr.flushi_ena.read();
    o_flush_valid = w_flush_pipeline;
    if (w.e.flushi.read() == 1) {
        // fencei or ebreak instructions
        o_flush_address = w.e.flushi_addr;
    } else {
        // request through debug interface to clear cache
        o_flush_address = csr.flushi_addr;
    }
    o_data_flush_address = ~0ull;
    o_data_flush_valid = w.m.flushd;

    o_req_ctrl_valid = w.f.imem_req_valid;
    o_req_ctrl_addr = w.f.imem_req_addr;
    o_halted = w.e.halted;
}

}  // namespace debugger

