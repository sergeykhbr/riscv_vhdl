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
    o_req_data_write("o_req_data_write"),
    o_req_data_addr("o_req_data_addr"),
    o_req_data_wdata("o_req_data_wdata"),
    o_req_data_wstrb("o_req_data_wstrb"),
    i_resp_data_valid("i_resp_data_valid"),
    i_resp_data_addr("i_resp_data_addr"),
    i_resp_data_data("i_resp_data_data"),
    i_resp_data_store_fault_addr("i_resp_data_store_fault_addr"),
    i_resp_data_load_fault("i_resp_data_load_fault"),
    i_resp_data_store_fault("i_resp_data_store_fault"),
    i_resp_data_er_mpu_load("i_resp_data_er_mpu_load"),
    i_resp_data_er_mpu_store("i_resp_data_er_mpu_store"),
    o_resp_data_ready("o_resp_data_ready"),
    i_ext_irq("i_ext_irq"),
    o_time("o_time"),
    o_exec_cnt("o_exec_cnt"),
    o_mpu_region_we("o_mpu_region_we"),
    o_mpu_region_idx("o_mpu_region_idx"),
    o_mpu_region_addr("o_mpu_region_addr"),
    o_mpu_region_mask("o_mpu_region_mask"),
    o_mpu_region_flags("o_mpu_region_flags"),
    i_dport_valid("i_dport_valid"),
    i_dport_write("i_dport_write"),
    i_dport_region("i_dport_region"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_ready("o_dport_ready"),
    o_dport_rdata("o_dport_rdata"),
    o_halted("o_halted"),
    o_flush_address("o_flush_address"),
    o_flush_valid("o_flush_valid"),
    o_data_flush_address("o_data_flush_address"),
    o_data_flush_valid("o_data_flush_valid"),
    i_data_flush_end("i_data_flush_end"),
    i_istate("i_istate"),
    i_dstate("i_dstate"),
    i_cstate("i_cstate") {
    fpu_ena_ = fpu_ena;
    tracer_ena_ = tracer_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_ctrl_valid;
    sensitive << w.f.pipeline_hold;
    sensitive << w.e.npc;
    sensitive << w.e.valid;
    sensitive << w.e.d_ready;
    sensitive << w.e.wena;
    sensitive << w.e.whazard;
    sensitive << w.e.waddr;
    sensitive << w.e.wdata;
    sensitive << w.e.wtag;
    sensitive << w.w.wena;
    sensitive << w.w.waddr;
    sensitive << w.w.wdata;
    sensitive << w.w.wtag;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.valid;
    sensitive << dbg.clock_cnt;
    sensitive << dbg.executed_cnt;
    sensitive << dbg.csr_addr;
    sensitive << dbg.reg_addr;
    sensitive << dbg.halt;
    sensitive << dbg.core_wdata;
    sensitive << csr.break_event;
    sensitive << dbg.flush_valid;
    sensitive << dbg.flush_address;

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
    fetch0->i_e_fencei(w.e.flushi);
    fetch0->i_predict_npc(bp.npc);
    fetch0->o_mem_req_fire(w.f.req_fire);
    fetch0->o_instr_load_fault(w.f.instr_load_fault);
    fetch0->o_instr_executable(w.f.instr_executable);
    fetch0->o_valid(w.f.valid);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);
    fetch0->o_hold(w.f.pipeline_hold);
    fetch0->i_br_fetch_valid(dbg.br_fetch_valid);
    fetch0->i_br_address_fetch(dbg.br_address_fetch);
    fetch0->i_br_instr_fetch(dbg.br_instr_fetch);

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
    dec0->o_imm(w.d.imm);
    dec0->i_e_ready(w.e.d_ready);
    dec0->i_e_fencei(w.e.flushi);
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
    dec0->o_f64(w.d.f64);
    dec0->o_isa_type(w.d.isa_type);
    dec0->o_instr_vec(w.d.instr_vec);
    dec0->o_exception(w.d.exception);
    dec0->o_instr_load_fault(w.d.instr_load_fault);
    dec0->o_instr_executable(w.d.instr_executable);

    exec0 = new InstrExecute("exec0", async_reset, fpu_ena);
    exec0->i_clk(i_clk);
    exec0->i_nrst(i_nrst);
    exec0->i_d_valid(w.d.instr_valid);
    exec0->i_d_pc(w.d.pc);
    exec0->i_d_instr(w.d.instr);
    exec0->i_d_radr1(w.d.radr1);
    exec0->i_d_radr2(w.d.radr2);
    exec0->i_d_waddr(w.d.waddr);
    exec0->i_d_imm(w.d.imm);
    exec0->i_wb_waddr(w.w.waddr);
    exec0->i_memop_store(w.d.memop_store);
    exec0->i_memop_load(w.d.memop_load);
    exec0->i_memop_sign_ext(w.d.memop_sign_ext);
    exec0->i_memop_size(w.d.memop_size);
    exec0->i_unsigned_op(w.d.unsigned_op);
    exec0->i_rv32(w.d.rv32);
    exec0->i_compressed(w.d.compressed);
    exec0->i_f64(w.d.f64);
    exec0->i_isa_type(w.d.isa_type);
    exec0->i_ivec(w.d.instr_vec);
    exec0->i_unsup_exception(w.d.exception);
    exec0->i_instr_load_fault(w.d.instr_load_fault);
    exec0->i_instr_executable(w.d.instr_executable);
    exec0->i_dport_npc_write(dbg.npc_write);
    exec0->i_dport_npc(wb_exec_dport_npc);
    exec0->i_rdata1(ireg.rdata1);
    exec0->i_rhazard1(ireg.rhazard1);
    exec0->i_rdata2(ireg.rdata2);
    exec0->i_rhazard2(ireg.rhazard2);
    exec0->i_wtag(ireg.wtag);
    exec0->o_wena(w.e.wena);
    exec0->o_waddr(w.e.waddr);
    exec0->o_whazard(w.e.whazard);
    exec0->o_wdata(w.e.wdata);
    exec0->o_wtag(w.e.wtag);
    exec0->o_d_ready(w.e.d_ready);
    exec0->o_csr_addr(w.e.csr_addr);
    exec0->o_csr_wena(w.e.csr_wena);
    exec0->i_csr_rdata(csr.rdata);
    exec0->o_csr_wdata(w.e.csr_wdata);
    exec0->i_trap_valid(csr.trap_valid);
    exec0->i_trap_pc(csr.trap_pc);
    exec0->o_ex_npc(w.e.ex_npc);
    exec0->o_ex_instr_load_fault(w.e.ex_instr_load_fault);
    exec0->o_ex_instr_not_executable(w.e.ex_instr_not_executable);
    exec0->o_ex_illegal_instr(w.e.ex_illegal_instr);
    exec0->o_ex_unalign_store(w.e.ex_unalign_store);
    exec0->o_ex_unalign_load(w.e.ex_unalign_load);
    exec0->o_ex_breakpoint(w.e.ex_breakpoint);
    exec0->o_ex_ecall(w.e.ex_ecall);
    exec0->o_ex_fpu_invalidop(w.e.ex_fpu_invalidop);
    exec0->o_ex_fpu_divbyzero(w.e.ex_fpu_divbyzero);
    exec0->o_ex_fpu_overflow(w.e.ex_fpu_overflow);
    exec0->o_ex_fpu_underflow(w.e.ex_fpu_underflow);
    exec0->o_ex_fpu_inexact(w.e.ex_fpu_inexact);
    exec0->o_fpu_valid(w.e.fpu_valid);
    exec0->o_memop_sign_ext(w.e.memop_sign_ext);
    exec0->o_memop_load(w.e.memop_load);
    exec0->o_memop_store(w.e.memop_store);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_memop_addr(w.e.memop_addr);
    exec0->o_memop_wdata(w.e.memop_wdata);
    exec0->o_memop_waddr(w.e.memop_waddr);
    exec0->o_memop_wtag(w.e.memop_wtag);
    exec0->i_memop_ready(w.m.memop_ready);
    exec0->o_trap_ready(w.e.trap_ready);
    exec0->o_valid(w.e.valid);
    exec0->o_pc(w.e.pc);
    exec0->o_npc(w.e.npc);
    exec0->o_instr(w.e.instr);
    exec0->i_flushd_end(i_data_flush_end);
    exec0->o_flushd(w.e.flushd);
    exec0->o_flushi(w.e.flushi);
    exec0->o_call(w.e.call);
    exec0->o_ret(w.e.ret);
    exec0->o_mret(w.e.mret);
    exec0->o_uret(w.e.uret);
    exec0->o_multi_ready(w.e.multi_ready);

    mem0 = new MemAccess("mem0", async_reset);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_e_valid(w.e.valid);
    mem0->i_e_pc(w.e.pc);
    mem0->i_e_instr(w.e.instr);
    mem0->i_e_flushd(w.e.flushd);
    mem0->o_flushd(w.m.flushd);
    mem0->i_memop_waddr(w.e.memop_waddr);
    mem0->i_memop_wtag(w.e.memop_wtag);
    mem0->i_memop_wdata(w.e.memop_wdata);
    mem0->i_memop_sign_ext(w.e.memop_sign_ext);
    mem0->i_memop_load(w.e.memop_load);
    mem0->i_memop_store(w.e.memop_store);
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
    mem0->o_mem_write(o_req_data_write);
    mem0->o_mem_addr(o_req_data_addr);
    mem0->o_mem_wdata(o_req_data_wdata);
    mem0->o_mem_wstrb(o_req_data_wstrb);
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
    iregs0->i_radr1(w.d.radr1);
    iregs0->o_rdata1(ireg.rdata1);
    iregs0->o_rhazard1(ireg.rhazard1);
    iregs0->i_radr2(w.d.radr2);
    iregs0->o_rdata2(ireg.rdata2);
    iregs0->o_rhazard2(ireg.rhazard2);
    iregs0->i_waddr(wb_reg_waddr);
    iregs0->i_wena(w_reg_wena);
    iregs0->i_whazard(w_reg_whazard);
    iregs0->i_wtag(wb_reg_wtag);
    iregs0->i_wdata(wb_reg_wdata);
    iregs0->o_wtag(ireg.wtag);
    iregs0->i_dport_addr(dbg.reg_addr);
    iregs0->i_dport_ena(dbg.ireg_ena);
    iregs0->i_dport_write(dbg.ireg_write);
    iregs0->i_dport_wdata(dbg.core_wdata);
    iregs0->o_dport_rdata(ireg.dport_rdata);
    iregs0->o_ra(ireg.ra);
    iregs0->o_sp(ireg.sp);

/*    if (fpu_ena_) {
        fregs0 = new RegFloatBank("fregs0", async_reset);
        fregs0->i_clk(i_clk);
        fregs0->i_nrst(i_nrst);
        fregs0->i_radr1(w.d.radr1);
        fregs0->o_rdata1(freg.rdata1);
        fregs0->i_radr2(w.d.radr2);
        fregs0->o_rdata2(freg.rdata2);
        fregs0->i_waddr(w.w.waddr);
        fregs0->i_wena(w.w.wena);
        fregs0->i_wdata(w.w.wdata);
        fregs0->i_dport_addr(wb_freg_dport_addr);
        fregs0->i_dport_ena(dbg.freg_ena);
        fregs0->i_dport_write(dbg.freg_write);
        fregs0->i_dport_wdata(dbg.core_wdata);
        fregs0->o_dport_rdata(freg.dport_rdata);
    } else {
        freg.rdata1 = 0;
        freg.rdata2 = 0;
        freg.dport_rdata = 0;
    }*/

    csr0 = new CsrRegs("csr0", hartid, async_reset);
    csr0->i_clk(i_clk);
    csr0->i_nrst(i_nrst);
    csr0->i_mret(w.e.mret);
    csr0->i_uret(w.e.uret);
    csr0->i_sp(ireg.sp);
    csr0->i_addr(w.e.csr_addr);
    csr0->i_wena(w.e.csr_wena);
    csr0->i_wdata(w.e.csr_wdata);
    csr0->o_rdata(csr.rdata);
    csr0->i_trap_ready(w.e.trap_ready);
    csr0->i_ex_pc(w.e.npc);
    csr0->i_ex_npc(w.e.ex_npc);
    csr0->i_ex_data_addr(i_resp_data_addr);
    csr0->i_ex_data_load_fault(i_resp_data_load_fault);
    csr0->i_ex_data_store_fault(i_resp_data_store_fault);
    csr0->i_ex_data_store_fault_addr(i_resp_data_store_fault_addr);
    csr0->i_ex_instr_load_fault(w.e.ex_instr_load_fault);
    csr0->i_ex_instr_not_executable(w.e.ex_instr_not_executable);
    csr0->i_ex_illegal_instr(w.e.ex_illegal_instr);
    csr0->i_ex_unalign_store(w.e.ex_unalign_store);
    csr0->i_ex_unalign_load(w.e.ex_unalign_load);
    csr0->i_ex_breakpoint(w.e.ex_breakpoint);
    csr0->i_ex_ecall(w.e.ex_ecall);
    csr0->i_ex_fpu_invalidop(w.e.ex_fpu_invalidop);
    csr0->i_ex_fpu_divbyzero(w.e.ex_fpu_divbyzero);
    csr0->i_ex_fpu_overflow(w.e.ex_fpu_overflow);
    csr0->i_ex_fpu_underflow(w.e.ex_fpu_underflow);
    csr0->i_ex_fpu_inexact(w.e.ex_fpu_inexact);
    csr0->i_fpu_valid(w.e.fpu_valid);
    csr0->i_irq_external(i_ext_irq);
    csr0->o_trap_valid(csr.trap_valid);
    csr0->o_trap_pc(csr.trap_pc);
    csr0->i_break_mode(dbg.break_mode);
    csr0->o_break_event(csr.break_event);
    csr0->o_mpu_region_we(o_mpu_region_we);
    csr0->o_mpu_region_idx(o_mpu_region_idx);
    csr0->o_mpu_region_addr(o_mpu_region_addr);
    csr0->o_mpu_region_mask(o_mpu_region_mask);
    csr0->o_mpu_region_flags(o_mpu_region_flags);
    csr0->i_dport_ena(dbg.csr_ena);
    csr0->i_dport_write(dbg.csr_write);
    csr0->i_dport_addr(dbg.csr_addr);
    csr0->i_dport_wdata(dbg.core_wdata);
    csr0->o_dport_rdata(csr.dport_rdata);

    dbg0 = new DbgPort("dbg0", async_reset);
    dbg0->i_clk(i_clk);
    dbg0->i_nrst(i_nrst);
    dbg0->i_dport_valid(i_dport_valid);
    dbg0->i_dport_write(i_dport_write);
    dbg0->i_dport_region(i_dport_region);
    dbg0->i_dport_addr(i_dport_addr);
    dbg0->i_dport_wdata(i_dport_wdata);
    dbg0->o_dport_ready(o_dport_ready);
    dbg0->o_dport_rdata(o_dport_rdata);
    dbg0->o_csr_addr(dbg.csr_addr);
    dbg0->o_reg_addr(dbg.reg_addr);
    dbg0->o_core_wdata(dbg.core_wdata);
    dbg0->o_csr_ena(dbg.csr_ena);
    dbg0->o_csr_write(dbg.csr_write);
    dbg0->i_csr_rdata(csr.dport_rdata);
    dbg0->o_ireg_ena(dbg.ireg_ena);
    dbg0->o_ireg_write(dbg.ireg_write);
    dbg0->o_npc_write(dbg.npc_write);
    dbg0->i_ireg_rdata(ireg.dport_rdata);
    dbg0->i_pc(w.e.pc);
    dbg0->i_npc(w.e.npc);
    dbg0->i_e_call(w.e.call);
    dbg0->i_e_ret(w.e.ret);
    dbg0->i_e_next_ready(w.e.trap_ready);
    dbg0->i_e_valid(w.e.valid);
    dbg0->o_clock_cnt(dbg.clock_cnt);
    dbg0->o_executed_cnt(dbg.executed_cnt);
    dbg0->o_halt(dbg.halt);
    dbg0->i_ebreak(csr.break_event);
    dbg0->o_break_mode(dbg.break_mode);
    dbg0->o_br_fetch_valid(dbg.br_fetch_valid);
    dbg0->o_br_address_fetch(dbg.br_address_fetch);
    dbg0->o_br_instr_fetch(dbg.br_instr_fetch);
    dbg0->i_istate(i_istate);
    dbg0->i_dstate(i_dstate);
    dbg0->i_cstate(i_cstate);
    dbg0->o_flush_address(dbg.flush_address);
    dbg0->o_flush_valid(dbg.flush_valid);

    trace0 = 0;
    if (tracer_ena) {
        trace0 = new Tracer("trace0", async_reset, "trace_river_sysc.log");
        trace0->i_clk(i_clk);
        trace0->i_nrst(i_nrst);
        trace0->i_dbg_executed_cnt(dbg.executed_cnt);
        trace0->i_e_valid(w.e.valid);
        trace0->i_e_pc(w.e.pc);
        trace0->i_e_instr(w.e.instr);
        trace0->i_e_multi_ready(w.e.multi_ready);
        trace0->i_e_wena(w.e.wena);
        trace0->i_e_whazard(w.e.whazard);
        trace0->i_e_waddr(w.e.waddr);
        trace0->i_e_wdata(w.e.wdata);
        trace0->i_e_memop_store(w.e.memop_store);
        trace0->i_e_memop_load(w.e.memop_load);
        trace0->i_e_memop_addr(w.e.memop_addr);
        trace0->i_e_memop_wdata(w.e.memop_wdata);
        trace0->i_m_wena(w.w.wena);
        trace0->i_m_waddr(w.w.waddr);
        trace0->i_m_wdata(w.w.wdata);
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
    if (trace0) {
        trace0->generateVCD(i_vcd, o_vcd);
    }
}

void Processor::comb() {
    w_fetch_pipeline_hold = !w.e.d_ready | dbg.halt;
    w_any_pipeline_hold = w.f.pipeline_hold | !w.e.d_ready | dbg.halt;

    wb_exec_dport_npc = dbg.core_wdata.read()(CFG_CPU_ADDR_BITS-1, 0);

    w_writeback_ready = !w.e.wena.read();
    if (w.e.wena.read() == 1) {
        w_reg_wena = w.e.wena;
        w_reg_whazard = w.e.whazard;
        wb_reg_waddr = w.e.waddr;
        wb_reg_wdata = w.e.wdata;
        wb_reg_wtag = w.e.wtag;
    } else {
        w_reg_wena = w.w.wena;
        w_reg_whazard = 0;
        wb_reg_waddr = w.w.waddr;
        wb_reg_wdata = w.w.wdata;
        wb_reg_wtag = w.w.wtag;
    }

    o_req_ctrl_valid = w.f.imem_req_valid;
    o_req_ctrl_addr = w.f.imem_req_addr;
    o_time = dbg.clock_cnt;
    o_exec_cnt = dbg.executed_cnt;

    o_flush_valid = w.e.flushi.read() || dbg.flush_valid.read()
                || csr.break_event.read();
    if (w.e.flushi.read() == 1) {
        o_flush_address = ~0ull;
    } else if (csr.break_event.read()) {
        o_flush_address = w.e.npc;
    } else {
        o_flush_address = dbg.flush_address;
    }
    o_data_flush_address = ~0ull;
    o_data_flush_valid = w.m.flushd;

    o_halted = dbg.halt;
}

}  // namespace debugger

