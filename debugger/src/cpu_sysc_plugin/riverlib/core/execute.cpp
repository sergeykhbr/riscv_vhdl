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

#include "execute.h"
#include "riscv-isa.h"

namespace debugger {

InstrExecute::InstrExecute(sc_module_name name_, bool async_reset,
    bool fpu_ena) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_d_valid("i_d_valid"),
    i_d_radr1("i_d_radr1"),
    i_d_radr2("i_d_radr2"),
    i_d_waddr("i_d_waddr"),
    i_d_csr_addr("i_d_csr_addr"),
    i_d_imm("i_d_imm"),
    i_d_pc("i_d_pc"),
    i_d_instr("i_d_instr"),
    i_d_progbuf_ena("i_d_progbuf_ena"),
    i_wb_waddr("i_wb_waddr"),
    i_memop_store("i_memop_store"),
    i_memop_load("i_memop_load"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_size("i_memop_size"),
    i_unsigned_op("i_unsigned_op"),
    i_rv32("i_rv32"),
    i_compressed("i_compressed"),
    i_amo("i_amo"),
    i_f64("i_f64"),
    i_isa_type("i_isa_type"),
    i_ivec("i_ivec"),
    i_stack_overflow("i_stack_overflow"),
    i_stack_underflow("i_stack_underflow"),
    i_unsup_exception("i_unsup_exception"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_executable("i_instr_executable"),
    i_mem_ex_debug("i_mem_ex_debug"),
    i_mem_ex_load_fault("i_mem_ex_load_fault"),
    i_mem_ex_store_fault("i_mem_ex_store_fault"),
    i_mem_ex_mpu_store("i_mem_ex_mpu_store"),
    i_mem_ex_mpu_load("i_mem_ex_mpu_load"),
    i_mem_ex_addr("i_mem_ex_addr"),
    i_irq_software("i_irq_software"),
    i_irq_timer("i_irq_timer"),
    i_irq_external("i_irq_external"),
    i_haltreq("i_haltreq"),
    i_resumereq("i_resumereq"),
    i_step("i_step"),
    i_dbg_progbuf_ena("i_dbg_progbuf_ena"),
    i_rdata1("i_rdata1"),
    i_rtag1("i_rtag1"),
    i_rdata2("i_rdata2"),
    i_rtag2("i_rtag2"),
    o_radr1("o_radr1"),
    o_radr2("o_radr2"),
    o_reg_wena("o_reg_wena"),
    o_reg_waddr("o_reg_waddr"),
    o_reg_wtag("o_reg_wtag"),
    o_reg_wdata("o_reg_wdata"),
    o_d_ready("o_d_ready"),
    o_csr_req_valid("o_csr_req_valid"),
    i_csr_req_ready("i_csr_req_ready"),
    o_csr_req_type("o_csr_req_type"),
    o_csr_req_addr("o_csr_req_addr"),
    o_csr_req_data("o_csr_req_data"),
    i_csr_resp_valid("i_csr_resp_valid"),
    o_csr_resp_ready("o_csr_resp_ready"),
    i_csr_resp_data("i_csr_resp_data"),
    i_csr_resp_exception("i_csr_resp_exception"),
    o_memop_valid("o_memop_valid"),
    o_memop_debug("o_memop_debug"),
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_type("o_memop_type"),
    o_memop_size("o_memop_size"),
    o_memop_memaddr("o_memop_memaddr"),
    o_memop_wdata("o_memop_wdata"),
    i_memop_ready("i_memop_ready"),
    i_dbg_mem_req_valid("i_dbg_mem_req_valid"),
    i_dbg_mem_req_write("i_dbg_mem_req_write"),
    i_dbg_mem_req_size("i_dbg_mem_req_size"),
    i_dbg_mem_req_addr("i_dbg_mem_req_addr"),
    i_dbg_mem_req_wdata("i_dbg_mem_req_wdata"),
    o_dbg_mem_req_ready("o_dbg_mem_req_ready"),
    o_dbg_mem_req_error("o_dbg_mem_req_error"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_npc("o_npc"),
    o_instr("o_instr"),
    i_flushd_end("i_flushd_end"),
    o_flushd("o_flushd"),
    o_flushi("o_flushi"),
    o_flushi_addr("o_flushi_addr"),
    o_call("o_call"),
    o_ret("o_ret"),
    o_halted("o_halted") {
    async_reset_ = async_reset;
    fpu_ena_ = fpu_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_d_valid;
    sensitive << i_d_radr1;
    sensitive << i_d_radr2;
    sensitive << i_d_waddr;
    sensitive << i_d_csr_addr;
    sensitive << i_d_imm;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
    sensitive << i_d_progbuf_ena;
    sensitive << i_dbg_progbuf_ena;
    sensitive << i_wb_waddr;
    sensitive << i_memop_ready;
    sensitive << i_memop_store;
    sensitive << i_memop_load;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_size;
    sensitive << i_unsigned_op;
    sensitive << i_rv32;
    sensitive << i_compressed;
    sensitive << i_amo;
    sensitive << i_f64;
    sensitive << i_isa_type;
    sensitive << i_ivec;
    sensitive << i_stack_overflow;
    sensitive << i_stack_underflow;
    sensitive << i_unsup_exception;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_executable;
    sensitive << i_mem_ex_debug;
    sensitive << i_mem_ex_load_fault;
    sensitive << i_mem_ex_store_fault;
    sensitive << i_mem_ex_mpu_store;
    sensitive << i_mem_ex_mpu_load;
    sensitive << i_mem_ex_addr;
    sensitive << i_irq_software;
    sensitive << i_irq_timer;
    sensitive << i_irq_external;
    sensitive << i_haltreq;
    sensitive << i_resumereq;
    sensitive << i_step;
    sensitive << i_rdata1;
    sensitive << i_rtag1;
    sensitive << i_rdata2;
    sensitive << i_rtag2;
    sensitive << i_csr_req_ready;
    sensitive << i_csr_resp_valid;
    sensitive << i_csr_resp_data;
    sensitive << i_csr_resp_exception;
    sensitive << i_flushd_end;
    sensitive << i_dbg_mem_req_valid;
    sensitive << i_dbg_mem_req_write;
    sensitive << i_dbg_mem_req_size;
    sensitive << i_dbg_mem_req_addr;
    sensitive << i_dbg_mem_req_wdata;
    sensitive << r.state;
    sensitive << r.csrstate;
    sensitive << r.amostate;
    sensitive << r.pc;
    sensitive << r.npc;
    sensitive << r.dnpc;
    sensitive << r.instr;
    sensitive << r.radr1;
    sensitive << r.radr2;
    sensitive << r.waddr;
    sensitive << r.rdata1;
    sensitive << r.rdata2;
    sensitive << r.ivec;
    sensitive << r.isa_type;
    sensitive << r.imm;
    sensitive << r.tagcnt;
    sensitive << r.select;
    sensitive << r.reg_waddr;
    sensitive << r.reg_wtag;
    sensitive << r.csr_req_rmw;
    sensitive << r.csr_req_type;
    sensitive << r.csr_req_addr;
    sensitive << r.csr_req_data;
    sensitive << r.memop_valid;
    sensitive << r.memop_debug;
    sensitive << r.memop_halted;
    sensitive << r.memop_type;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.memop_memaddr;
    sensitive << r.memop_wdata;

    sensitive << r.unsigned_op;
    sensitive << r.rv32;
    sensitive << r.compressed;
    sensitive << r.f64;
    sensitive << r.stack_overflow;
    sensitive << r.stack_underflow;
    sensitive << r.mem_ex_load_fault;
    sensitive << r.mem_ex_store_fault;
    sensitive << r.mem_ex_mpu_store;
    sensitive << r.mem_ex_mpu_load;
    sensitive << r.mem_ex_addr;
    sensitive << r.res_npc;
    sensitive << r.res_ra;
    sensitive << r.res_csr;

    sensitive << r.flushi;
    sensitive << r.flushi_addr;
    sensitive << r.flushd;
    sensitive << r.reg_write;
    sensitive << r.valid;
    sensitive << r.call;
    sensitive << r.ret;
    sensitive << r.stepdone;
    for (int i = 0; i < Res_Total; i++) {
        sensitive << wb_select[i].ena;
        sensitive << wb_select[i].valid;
        sensitive << wb_select[i].res;
    }
    sensitive << wb_shifter_a1;
    sensitive << wb_shifter_a2;
    sensitive << w_ex_fpu_invalidop;
    sensitive << w_ex_fpu_divbyzero;
    sensitive << w_ex_fpu_overflow;
    sensitive << w_ex_fpu_underflow;
    sensitive << w_ex_fpu_inexact;


    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    alu0 = new AluLogic("alu0", async_reset);
    alu0->i_clk(i_clk);
    alu0->i_nrst(i_nrst);
    alu0->i_mode(wb_alu_mode);
    alu0->i_a1(wb_rdata1);
    alu0->i_a2(wb_rdata2);
    alu0->o_res(wb_select[Res_Alu].res);

    addsub0 = new IntAddSub("addsub0", async_reset);
    addsub0->i_clk(i_clk);
    addsub0->i_nrst(i_nrst);
    addsub0->i_mode(wb_addsub_mode);
    addsub0->i_a1(wb_rdata1);
    addsub0->i_a2(wb_rdata2);
    addsub0->o_res(wb_select[Res_AddSub].res);

    mul0 = new IntMul("mul0", async_reset);
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(wb_select[Res_IMul].ena);
    mul0->i_unsigned(i_unsigned_op);
    mul0->i_hsu(w_mul_hsu);
    mul0->i_high(w_arith_residual_high);
    mul0->i_rv32(i_rv32);
    mul0->i_a1(wb_rdata1);
    mul0->i_a2(wb_rdata2);
    mul0->o_res(wb_select[Res_IMul].res);
    mul0->o_valid(wb_select[Res_IMul].valid);

    div0 = new IntDiv("div0", async_reset);
    div0->i_clk(i_clk);
    div0->i_nrst(i_nrst);
    div0->i_ena(wb_select[Res_IDiv].ena);
    div0->i_unsigned(i_unsigned_op);
    div0->i_residual(w_arith_residual_high);
    div0->i_rv32(i_rv32);
    div0->i_a1(wb_rdata1);
    div0->i_a2(wb_rdata2);
    div0->o_res(wb_select[Res_IDiv].res);
    div0->o_valid(wb_select[Res_IDiv].valid);

    sh0 = new Shifter("sh0", async_reset);
    sh0->i_clk(i_clk);
    sh0->i_nrst(i_nrst);
    sh0->i_mode(wb_shifter_mode);
    sh0->i_a1(wb_shifter_a1);
    sh0->i_a2(wb_shifter_a2);
    sh0->o_res(wb_select[Res_Shifter].res);

    if (fpu_ena_) {
        fpu0 = new FpuTop("fpu0", async_reset);
        fpu0->i_clk(i_clk);
        fpu0->i_nrst(i_nrst);
        fpu0->i_ena(wb_select[Res_FPU].ena);
        fpu0->i_ivec(wb_fpu_vec);
        fpu0->i_a(wb_rdata1);
        fpu0->i_b(wb_rdata2);
        fpu0->o_res(wb_select[Res_FPU].res);
        fpu0->o_ex_invalidop(w_ex_fpu_invalidop);
        fpu0->o_ex_divbyzero(w_ex_fpu_divbyzero);
        fpu0->o_ex_overflow(w_ex_fpu_overflow);
        fpu0->o_ex_underflow(w_ex_fpu_underflow);
        fpu0->o_ex_inexact(w_ex_fpu_inexact);
        fpu0->o_valid(wb_select[Res_FPU].valid);
    } else {
        wb_select[Res_FPU].res = 0;
        wb_select[Res_FPU].valid = 0;
        w_ex_fpu_invalidop = 0;
        w_ex_fpu_divbyzero = 0;
        w_ex_fpu_overflow = 0;
        w_ex_fpu_underflow = 0;
        w_ex_fpu_inexact = 0;
    }
};

InstrExecute::~InstrExecute() {
    delete alu0;
    delete addsub0;
    delete mul0;
    delete div0;
    delete sh0;
    if (fpu_ena_) {
        delete fpu0;
    }
}

void InstrExecute::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, i_nrst.name());
        sc_trace(o_vcd, i_clk, i_clk.name());
        sc_trace(o_vcd, i_d_valid, i_d_valid.name());
        sc_trace(o_vcd, i_d_pc, i_d_pc.name());
        sc_trace(o_vcd, i_d_instr, i_d_instr.name());
        sc_trace(o_vcd, i_d_radr1, i_d_radr1.name());
        sc_trace(o_vcd, i_d_radr2, i_d_radr2.name());
        sc_trace(o_vcd, i_d_waddr, i_d_waddr.name());
        sc_trace(o_vcd, i_d_csr_addr, i_d_csr_addr.name());
        sc_trace(o_vcd, i_wb_waddr, i_wb_waddr.name());
        sc_trace(o_vcd, i_f64, i_f64.name());
        sc_trace(o_vcd, i_stack_overflow, i_stack_overflow.name());
        sc_trace(o_vcd, i_stack_underflow, i_stack_underflow.name());
        sc_trace(o_vcd, i_unsup_exception, i_unsup_exception.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_executable, i_instr_executable.name());
        sc_trace(o_vcd, i_mem_ex_debug, i_mem_ex_debug.name());
        sc_trace(o_vcd, i_mem_ex_load_fault, i_mem_ex_load_fault.name());
        sc_trace(o_vcd, i_mem_ex_store_fault, i_mem_ex_store_fault.name());
        sc_trace(o_vcd, i_mem_ex_mpu_store, i_mem_ex_mpu_store.name());
        sc_trace(o_vcd, i_mem_ex_mpu_load, i_mem_ex_mpu_load.name());
        sc_trace(o_vcd, i_mem_ex_addr, i_mem_ex_addr.name());
        sc_trace(o_vcd, i_irq_software, i_irq_software.name());
        sc_trace(o_vcd, i_irq_timer, i_irq_timer.name());
        sc_trace(o_vcd, i_irq_external, i_irq_external.name());
        sc_trace(o_vcd, i_haltreq, i_haltreq.name());
        sc_trace(o_vcd, i_resumereq, i_resumereq.name());
        sc_trace(o_vcd, i_dbg_progbuf_ena, i_dbg_progbuf_ena.name());
        sc_trace(o_vcd, i_step, i_step.name());
        sc_trace(o_vcd, i_rdata1, i_rdata1.name());
        sc_trace(o_vcd, i_rtag1, i_rtag1.name());
        sc_trace(o_vcd, i_rdata2, i_rdata2.name());
        sc_trace(o_vcd, i_rtag2, i_rtag2.name());
        sc_trace(o_vcd, o_radr1, o_radr1.name());
        sc_trace(o_vcd, o_radr2, o_radr2.name());
        sc_trace(o_vcd, o_reg_wena, o_reg_wena.name());
        sc_trace(o_vcd, o_reg_waddr, o_reg_waddr.name());
        sc_trace(o_vcd, o_reg_wtag, o_reg_wtag.name());
        sc_trace(o_vcd, o_reg_wdata, o_reg_wdata.name());
        sc_trace(o_vcd, o_d_ready, o_d_ready.name());
        sc_trace(o_vcd, o_csr_req_valid, o_csr_req_valid.name());
        sc_trace(o_vcd, i_csr_req_ready, i_csr_req_ready.name());
        sc_trace(o_vcd, o_csr_req_type, o_csr_req_type.name());
        sc_trace(o_vcd, o_csr_req_addr, o_csr_req_addr.name());
        sc_trace(o_vcd, o_csr_req_data, o_csr_req_data.name());
        sc_trace(o_vcd, i_csr_resp_valid, i_csr_resp_valid.name());
        sc_trace(o_vcd, o_csr_resp_ready, o_csr_resp_ready.name());
        sc_trace(o_vcd, i_csr_resp_data, i_csr_resp_data.name());
        sc_trace(o_vcd, i_csr_resp_exception, i_csr_resp_exception.name());
        sc_trace(o_vcd, i_flushd_end, i_flushd_end.name());
        sc_trace(o_vcd, o_memop_valid, o_memop_valid.name());
        sc_trace(o_vcd, o_memop_debug, o_memop_debug.name());
        sc_trace(o_vcd, o_memop_type, o_memop_type.name());
        sc_trace(o_vcd, o_memop_size, o_memop_size.name());
        sc_trace(o_vcd, o_memop_memaddr, o_memop_memaddr.name());
        sc_trace(o_vcd, o_memop_wdata, o_memop_wdata.name());

        sc_trace(o_vcd, o_flushd, o_flushd.name());
        sc_trace(o_vcd, o_flushi, o_flushi.name());
        sc_trace(o_vcd, o_flushi_addr, o_flushi_addr.name());
        sc_trace(o_vcd, i_memop_ready, i_memop_ready.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_npc, o_npc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_call, o_call.name());
        sc_trace(o_vcd, o_ret, o_ret.name());
        sc_trace(o_vcd, o_halted, o_halted.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_select[Res_IMul].ena, pn + ".w_arith_ena(5)");
        sc_trace(o_vcd, wb_select[Res_IMul].res, pn + ".wb_arith_res(5)");
        sc_trace(o_vcd, wb_select[Res_IDiv].ena, pn + ".w_arith_ena(6)");
        sc_trace(o_vcd, wb_select[Res_IDiv].res, pn + ".wb_arith_res(6)");
        sc_trace(o_vcd, wb_select[Res_FPU].ena, pn + ".w_arith_ena(7)");
        sc_trace(o_vcd, wb_select[Res_FPU].res, pn + ".wb_arith_res(7)");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.csrstate, pn + ".r_csrstate");
        sc_trace(o_vcd, r.csr_req_type, pn + ".r_csr_req_type");
        sc_trace(o_vcd, r.csr_req_addr, pn + ".r_csr_req_addr");
        sc_trace(o_vcd, r.csr_req_data, pn + ".r_csr_req_data");
        sc_trace(o_vcd, r.amostate, pn + ".r_amostate");
        sc_trace(o_vcd, r.tagcnt, pn + ".r_tagcnt");
        sc_trace(o_vcd, r.select, pn + ".r_select");
        sc_trace(o_vcd, r.reg_waddr, pn + ".r_reg_waddr");
        sc_trace(o_vcd, r.reg_wtag, pn + ".r_reg_wtag");
        sc_trace(o_vcd, r.rdata1, pn + ".r_rdata1");
        sc_trace(o_vcd, r.radr1, pn + ".r_radr1");
        sc_trace(o_vcd, r.radr2, pn + ".r_radr2");
        sc_trace(o_vcd, r.flushd, pn + ".r_flushd");
        sc_trace(o_vcd, r.stepdone, pn + ".r_stepdone");
        sc_trace(o_vcd, r.memop_debug, pn + ".r_memop_debug");
        sc_trace(o_vcd, r.memop_halted, pn + ".r_memop_halted");
        sc_trace(o_vcd, r.mem_ex_store_fault, pn + ".r_mem_ex_store_fault");
        sc_trace(o_vcd, r.npc, pn + ".r_npc");
        sc_trace(o_vcd, r.dnpc, pn + ".r_dnpc");
        sc_trace(o_vcd, w_hazard1, pn + ".w_hazard1");
        sc_trace(o_vcd, w_hazard2, pn + ".w_hazard2");
        sc_trace(o_vcd, tag_expected[0x8], pn + ".tag_expected0x08");
        sc_trace(o_vcd, tag_expected[0x9], pn + ".tag_expected0x09");
        sc_trace(o_vcd, tag_expected[0xA], pn + ".tag_expected0x0A");
        sc_trace(o_vcd, tag_expected[0xb], pn + ".tag_expected0x0B");
        sc_trace(o_vcd, tag_expected[0xc], pn + ".tag_expected0x0C");
        sc_trace(o_vcd, tag_expected[0xd], pn + ".tag_expected0x0D");
        sc_trace(o_vcd, tag_expected[0xe], pn + ".tag_expected0x0E");
        sc_trace(o_vcd, tag_expected[0xf], pn + ".tag_expected0x0F");
        sc_trace(o_vcd, wb_select[Res_AddSub].res, pn + ".t_res_AddSub");
        sc_trace(o_vcd, t_tagcnt_wr, pn + ".t_tagcnt_wr");
        sc_trace(o_vcd, t_waddr, pn + ".t_waddr");
    }
    alu0->generateVCD(i_vcd, o_vcd);
    addsub0->generateVCD(i_vcd, o_vcd);
    sh0->generateVCD(i_vcd, o_vcd);
    mul0->generateVCD(i_vcd, o_vcd);
    div0->generateVCD(i_vcd, o_vcd);
    if (fpu_ena_) {
        fpu0->generateVCD(i_vcd, o_vcd);
    }
}

void InstrExecute::comb() {
    bool v_d_valid;
    bool v_fence_d;
    bool v_fence_i;
    bool v_csr_req_valid;
    bool v_csr_resp_ready;
    sc_uint<RISCV_ARCH> vb_csr_cmd_wdata;
    sc_uint<RISCV_ARCH> vb_res;
    sc_uint<CFG_CPU_ADDR_BITS> vb_prog_npc;
    sc_uint<CFG_CPU_ADDR_BITS> vb_npc_incr;
    sc_uint<RISCV_ARCH> vb_off;
    sc_uint<RISCV_ARCH> vb_sub64;
    sc_uint<CFG_CPU_ADDR_BITS> vb_memop_memaddr;
    sc_uint<CFG_CPU_ADDR_BITS> vb_memop_memaddr_load;
    sc_uint<CFG_CPU_ADDR_BITS> vb_memop_memaddr_store;
    sc_uint<RISCV_ARCH> vb_memop_wdata;
    sc_bv<Instr_Total> wv;
    int opcode_len;
    bool v_call;
    bool v_ret;
    bool v_pc_branch;
    bool v_eq;      // equal
    bool v_ge;      // greater/equal signed
    bool v_geu;     // greater/equal unsigned
    bool v_lt;      // less signed
    bool v_ltu;     // less unsigned
    bool v_neq;     // not equal
    sc_uint<RISCV_ARCH> vb_rdata1;
    sc_uint<RISCV_ARCH> vb_rdata2;
    bool v_check_tag1;
    bool v_check_tag2;
    sc_uint<Res_Total> vb_select;
    sc_biguint<CFG_REG_TAG_WIDTH*REGS_TOTAL> vb_tagcnt_next;
    bool v_d_ready;
    bool v_latch_input;
    bool v_memop_ena;
    bool v_memop_debug;
    bool v_reg_ena;
    sc_uint<6> vb_reg_waddr;
    bool v_instr_misaligned;
    bool v_store_misaligned;
    bool v_load_misaligned;
    bool v_debug_misaligned;  // from the debug interface
    bool v_csr_cmd_ena;
    bool v_mem_ex;
    sc_uint<12> vb_csr_cmd_addr;
    sc_uint<CsrReq_TotalBits> vb_csr_cmd_type;
    bool v_dbg_mem_req_ready;
    bool v_dbg_mem_req_error;
    input_mux_type mux;
    sc_uint<CFG_CPU_ADDR_BITS> vb_o_npc;

    v = r;

    v_d_valid = 0;
    v_d_ready = 0;
    v_csr_req_valid = 0;
    v_csr_resp_ready = 0;
    vb_res = 0;
    vb_off = 0;
    vb_memop_memaddr = 0;
    vb_memop_wdata = 0;
    v_call = 0;
    v_ret = 0;
    v.valid = 0;
    v.call = 0;
    v.ret = 0;
    v.reg_write = 0;
    v.flushi_addr = 0;
    vb_rdata1 = 0;
    vb_rdata2 = 0;
    vb_select = 0;
    for (int i = 0; i < Res_Total; i++) {
        wb_select[i].ena = 0;
    }
    v_instr_misaligned = 0;
    v_store_misaligned = 0;
    v_load_misaligned = 0;
    v_debug_misaligned = 0;
    v_csr_cmd_ena = 0;
    v_mem_ex = 0;
    vb_csr_cmd_addr = 0;
    vb_csr_cmd_type = 0;
    vb_csr_cmd_wdata = 0;
    v_latch_input = 0;
    v_reg_ena = 0;
    v_memop_ena = 0;
    v_memop_debug = 0;
    v_dbg_mem_req_ready = 0;
    v_dbg_mem_req_error = 0;

    vb_reg_waddr = i_d_waddr;

    if (r.state.read() == State_Idle) {
        mux.radr1 = i_d_radr1;
        mux.radr2 = i_d_radr2;
        mux.waddr = i_d_waddr;
        mux.imm = i_d_imm;
        mux.pc = i_d_pc;
        mux.instr = i_d_instr;
        mux.memop_type[MemopType_Store] = i_memop_store;
        mux.memop_type[MemopType_Locked] = i_amo;
        mux.memop_type[MemopType_Reserve] =
                (i_ivec.read()[Instr_LR_D] | i_ivec.read()[Instr_LR_W]).to_bool();
        mux.memop_type[MemopType_Release] =
                (i_ivec.read()[Instr_SC_D] | i_ivec.read()[Instr_SC_W]).to_bool();
        mux.memop_sign_ext = i_memop_sign_ext;
        mux.memop_size = i_memop_size;
        mux.unsigned_op = i_unsigned_op;
        mux.rv32 = i_rv32;
        mux.compressed = i_compressed;
        mux.f64 = i_f64;
        mux.ivec = i_ivec;
        mux.isa_type = i_isa_type;
    } else {
        mux.radr1 = r.radr1;
        mux.radr2 = r.radr2;
        mux.waddr = r.waddr;
        mux.imm = r.imm;
        mux.pc = r.pc;
        mux.instr = r.instr;
        mux.memop_type = r.memop_type;
        mux.memop_sign_ext = r.memop_sign_ext;
        mux.memop_size = r.memop_size;
        mux.unsigned_op = r.unsigned_op;
        mux.rv32 = r.rv32;
        mux.compressed = r.compressed;
        mux.f64 = r.f64;
        mux.ivec = r.ivec;
        mux.isa_type = r.isa_type;
    }
    wv = mux.ivec;

    v_check_tag1 = 0;
    v_check_tag2 = 0;

    if (mux.isa_type[ISA_R_type]) {
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    } else if (mux.isa_type[ISA_I_type]) {
        vb_rdata1 = i_rdata1;
        vb_rdata2 = mux.imm;
        v_check_tag1 = 1;
    } else if (mux.isa_type[ISA_SB_type]) {
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        vb_off = mux.imm;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    } else if (mux.isa_type[ISA_UJ_type]) {
        vb_rdata1 = mux.pc;
        vb_off = mux.imm;
        v_check_tag1 = 1;
    } else if (mux.isa_type[ISA_U_type]) {
        vb_rdata1 = mux.pc;
        vb_rdata2 = mux.imm;
    } else if (mux.isa_type[ISA_S_type]) {
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        vb_off = mux.imm;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    }

    vb_memop_memaddr_load = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0)
                          + vb_rdata2(CFG_CPU_ADDR_BITS-1, 0);
    vb_memop_memaddr_store = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0)
                           + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    if (mux.memop_type[MemopType_Store] == 0) {
        vb_memop_memaddr = vb_memop_memaddr_load;
    } else {
        vb_memop_memaddr = vb_memop_memaddr_store;
    }


    // Check that registers tags are equal to expeted ones
    int t_radr1 = mux.radr1.to_int();
    int t_radr2 = mux.radr2.to_int();

    w_hazard1 = 0;
    if (r.tagcnt.read()(CFG_REG_TAG_WIDTH * t_radr1 + (CFG_REG_TAG_WIDTH - 1),
                        CFG_REG_TAG_WIDTH * t_radr1) != i_rtag1.read()) {
        w_hazard1 = v_check_tag1;
    }
    w_hazard2 = 0;
    if (r.tagcnt.read()(CFG_REG_TAG_WIDTH * t_radr2 + (CFG_REG_TAG_WIDTH - 1),
                        CFG_REG_TAG_WIDTH * t_radr2) != i_rtag2.read()) {
        w_hazard2 = v_check_tag2;
    }

    // Compute branch conditions:
    vb_sub64 = vb_rdata1 - vb_rdata2;
    v_eq = !vb_sub64.or_reduce();   // equal
    v_ge = !vb_sub64[63];           // greater/equal (signed)
    v_geu = 0;
    if (vb_rdata1 >= vb_rdata2) {
        v_geu = 1;                  // greater/equal (unsigned)
    }
    v_lt = vb_sub64[63];            // less (signed)
    v_ltu = 0;
    if (vb_rdata1 < vb_rdata2) {
        v_ltu = 1;                  // less (unsiged)
    }
    v_neq = vb_sub64.or_reduce();   // not equal

    // Relative Branch on some condition:
    v_pc_branch = 0;
    if ((wv[Instr_BEQ].to_bool() & v_eq)
        || (wv[Instr_BGE].to_bool() & v_ge)
        || (wv[Instr_BGEU].to_bool() & v_geu)
        || (wv[Instr_BLT].to_bool() & v_lt)
        || (wv[Instr_BLTU].to_bool() & v_ltu)
        || (wv[Instr_BNE].to_bool() & v_neq)) {
        v_pc_branch = 1;
    }

    wb_fpu_vec = wv.range(Instr_FSUB_D, Instr_FADD_D);  // directly connected i_ivec
    v_fence_d = wv[Instr_FENCE].to_bool();
    v_fence_i = wv[Instr_FENCE_I].to_bool();
    w_arith_residual_high = (wv[Instr_REM] || wv[Instr_REMU]
                          || wv[Instr_REMW] || wv[Instr_REMUW]
                          || wv[Instr_MULH] || wv[Instr_MULHSU] || wv[Instr_MULHU]);

    w_mul_hsu = wv[Instr_MULHSU].to_bool();

    v_instr_misaligned = mux.pc[0];
    if ((wv[Instr_LD] && vb_memop_memaddr_load(2, 0) != 0)
        || ((wv[Instr_LW] || wv[Instr_LWU]) && vb_memop_memaddr_load(1, 0) != 0)
        || ((wv[Instr_LH] || wv[Instr_LHU]) && vb_memop_memaddr_load[0] != 0)) {
        v_load_misaligned = 1;
    }
    if ((wv[Instr_SD] && vb_memop_memaddr_store(2, 0) != 0)
        || (wv[Instr_SW] && vb_memop_memaddr_store(1, 0) != 0)
        || (wv[Instr_SH] && vb_memop_memaddr_store[0] != 0)) {
        v_store_misaligned = 1;
    }
    if ((i_dbg_mem_req_size.read() == 3 && i_dbg_mem_req_addr.read()(2, 0) != 0)
        || (i_dbg_mem_req_size.read() == 2 && i_dbg_mem_req_addr.read()(1, 0) != 0)
        || (i_dbg_mem_req_size.read() == 1 && i_dbg_mem_req_addr.read()[0] != 0)) {
        v_debug_misaligned = 1;
    }
    if (i_stack_overflow.read() == 1) {
        v.stack_overflow = 1;
    }
    if (i_stack_underflow.read() == 1) {
        v.stack_underflow = 1;
    }
    if (i_mem_ex_load_fault.read() == 1 && i_mem_ex_debug.read() == 0) {
        v.mem_ex_load_fault = 1;
        v.mem_ex_addr = i_mem_ex_addr;
    }
    if (i_mem_ex_store_fault.read() == 1 && i_mem_ex_debug.read() == 0) {
        v.mem_ex_store_fault = 1;
        v.mem_ex_addr = i_mem_ex_addr;
    }
    if (i_mem_ex_mpu_store.read() == 1 && i_mem_ex_debug.read() == 0) {
        v.mem_ex_mpu_store = 1;
        v.mem_ex_addr = i_mem_ex_addr;
    }
    if (i_mem_ex_mpu_load.read() == 1 && i_mem_ex_debug.read() == 0) {
        v.mem_ex_mpu_load = 1;
        v.mem_ex_addr = i_mem_ex_addr;
    }



    opcode_len = 4;
    if (mux.compressed) {
        opcode_len = 2;
    }
    vb_npc_incr = mux.pc + opcode_len;

    if (v_pc_branch) {
        vb_prog_npc = mux.pc + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        vb_prog_npc = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    } else if (wv[Instr_JALR].to_bool()) {
        vb_prog_npc = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_rdata2(CFG_CPU_ADDR_BITS-1, 0);
        vb_prog_npc[0] = 0;
    } else {
        vb_prog_npc = vb_npc_incr;
    }

    vb_select[Res_Reg2] = mux.memop_type[MemopType_Store] || wv[Instr_LUI];
    vb_select[Res_Npc] = 0;
    vb_select[Res_Ra] = v_pc_branch || wv[Instr_JAL] || wv[Instr_JALR]
                        || wv[Instr_MRET] || wv[Instr_URET];
    vb_select[Res_Csr] = wv[Instr_CSRRC] || wv[Instr_CSRRCI] || wv[Instr_CSRRS]
                        || wv[Instr_CSRRSI] || wv[Instr_CSRRW] || wv[Instr_CSRRWI];
    vb_select[Res_Alu] = wv[Instr_AND] || wv[Instr_ANDI]
                      || wv[Instr_OR] || wv[Instr_ORI]
                      || wv[Instr_XOR] || wv[Instr_XORI]
                      || wv[Instr_AMOOR_D] || wv[Instr_AMOOR_W]
                      || wv[Instr_AMOAND_D] || wv[Instr_AMOAND_W]
                      || wv[Instr_AMOXOR_D] || wv[Instr_AMOXOR_W];
    vb_select[Res_AddSub] = wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]
                         || wv[Instr_ADDW] || wv[Instr_ADDIW]
                         || wv[Instr_SUB] || wv[Instr_SUBW]
                         || wv[Instr_SLT] || wv[Instr_SLTI]
                         || wv[Instr_SLTU] || wv[Instr_SLTIU]
                         || wv[Instr_AMOADD_D] || wv[Instr_AMOADD_W]
                         || wv[Instr_AMOMIN_D] || wv[Instr_AMOMIN_W]
                         || wv[Instr_AMOMAX_D] || wv[Instr_AMOMAX_W]
                         || wv[Instr_AMOMINU_D] || wv[Instr_AMOMINU_W]
                         || wv[Instr_AMOMAXU_D] || wv[Instr_AMOMAXU_W]
                         || wv[Instr_AMOSWAP_D] || wv[Instr_AMOSWAP_W];
    vb_select[Res_Shifter] = wv[Instr_SLL] || wv[Instr_SLLI]
                          || wv[Instr_SLLW] || wv[Instr_SLLIW]
                          || wv[Instr_SRL] || wv[Instr_SRLI]
                          || wv[Instr_SRLW] || wv[Instr_SRLIW]
                          || wv[Instr_SRA] || wv[Instr_SRAI]
                          || wv[Instr_SRAW] || wv[Instr_SRAW] || wv[Instr_SRAIW];
    vb_select[Res_IMul] = wv[Instr_MUL] || wv[Instr_MULW]
                     || wv[Instr_MULH]|| wv[Instr_MULHSU]
                     || wv[Instr_MULHU];
    vb_select[Res_IDiv] = wv[Instr_DIV] || wv[Instr_DIVU]
                            || wv[Instr_DIVW] || wv[Instr_DIVUW]
                            || wv[Instr_REM] || wv[Instr_REMU]
                            || wv[Instr_REMW] || wv[Instr_REMUW];
    if (fpu_ena_) {
        vb_select[Res_FPU] = mux.f64 && !(wv[Instr_FSD] | wv[Instr_FLD]).to_bool();
    }
    vb_select[Res_Zero] = !vb_select(Res_Total-1, Res_Zero+1).or_reduce();  // load memory, fence


    if ((wv[Instr_JAL] || wv[Instr_JALR]) && mux.waddr == Reg_ra) {
        v_call = 1;
    }
    if (wv[Instr_JALR] && vb_rdata2 == 0 
        && mux.waddr != Reg_ra && mux.radr1 == Reg_ra) {
        v_ret = 1;
    }

    v_mem_ex = r.mem_ex_load_fault || r.mem_ex_store_fault
            || r.mem_ex_mpu_store || r.mem_ex_mpu_load;
    v_csr_cmd_ena = i_haltreq || (i_step && r.stepdone) || i_unsup_exception
                || i_instr_load_fault || v_mem_ex || !i_instr_executable.read()
                || r.stack_overflow || r.stack_underflow
                || v_instr_misaligned || v_load_misaligned || v_store_misaligned
                || i_irq_software || i_irq_timer || i_irq_external
                || wv[Instr_WFI]
                || wv[Instr_EBREAK] || wv[Instr_ECALL]
                || wv[Instr_MRET] || wv[Instr_URET]
                || wv[Instr_CSRRC] || wv[Instr_CSRRCI] || wv[Instr_CSRRS]
                || wv[Instr_CSRRSI] || wv[Instr_CSRRW] || wv[Instr_CSRRWI];
    if (i_haltreq == 1) {
        vb_csr_cmd_type = CsrReq_HaltCmd;
        vb_csr_cmd_addr = HALT_CAUSE_HALTREQ;
    } else if (i_step == 1 && r.stepdone == 1) {
        vb_csr_cmd_type = CsrReq_HaltCmd;
        vb_csr_cmd_addr = HALT_CAUSE_STEP;
    } else if (v_instr_misaligned == 1) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_InstrMisalign;          // Instruction address misaligned
        vb_csr_cmd_wdata = mux.pc;
    } else if (i_instr_load_fault == 1 || i_instr_executable == 0) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_InstrFault;             // Instruction access fault
        vb_csr_cmd_wdata = mux.pc;
    } else if (i_unsup_exception) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_InstrIllegal;           // Illegal instruction
        vb_csr_cmd_wdata = mux.instr;
    } else if (wv[Instr_EBREAK]) {
        vb_csr_cmd_type = CsrReq_BreakpointCmd;
        vb_csr_cmd_addr = EXCEPTION_Breakpoint;
    } else if (v_load_misaligned == 1) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_LoadMisalign;           // Load address misaligned
        vb_csr_cmd_wdata = vb_memop_memaddr_load;
    } else if (r.mem_ex_load_fault || r.mem_ex_mpu_load) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_LoadFault;              // Load access fault
        vb_csr_cmd_wdata = r.mem_ex_addr;
    } else if (v_store_misaligned == 1) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_StoreMisalign;          // Store/AMO address misaligned
        vb_csr_cmd_wdata = vb_memop_memaddr_store;
    } else if (r.mem_ex_store_fault || r.mem_ex_mpu_store) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_StoreFault;             // Store/AMO access fault
        vb_csr_cmd_wdata = r.mem_ex_addr;
    } else if (r.stack_overflow) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_StackOverflow;          // Stack overflow
    } else if (r.stack_underflow) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_StackUnderflow;         // Stack Underflow
    } else if (wv[Instr_ECALL]) {
        vb_csr_cmd_type = CsrReq_ExceptionCmd;
        vb_csr_cmd_addr = EXCEPTION_CallFromXMode;          // Environment call
    } else if (i_irq_software) {
        vb_csr_cmd_type = CsrReq_InterruptCmd;
        vb_csr_cmd_addr = INTERRUPT_XSoftware;              // Software interrupt request
    } else if (i_irq_timer) {
        vb_csr_cmd_type = CsrReq_InterruptCmd;
        vb_csr_cmd_addr = INTERRUPT_XTimer;                 // Timer interrupt request
    } else if (i_irq_external) {
        vb_csr_cmd_type = CsrReq_InterruptCmd;
        vb_csr_cmd_addr = INTERRUPT_XExternal;              // PLIC interrupt request
    } else if (wv[Instr_WFI]) {
        vb_csr_cmd_type = CsrReq_WfiCmd;
        vb_csr_cmd_addr =  mux.instr(14,12);                // PRIV field
    } else if (wv[Instr_MRET]) {
        vb_csr_cmd_type = CsrReq_TrapReturnCmd;
        vb_csr_cmd_addr = CSR_mepc;
    } else if (wv[Instr_URET]) {
        vb_csr_cmd_type = CsrReq_TrapReturnCmd;
        vb_csr_cmd_addr = CSR_uepc;
    } else if (wv[Instr_CSRRC]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = i_csr_resp_data.read() & ~r.rdata1.read();
    } else if (wv[Instr_CSRRCI]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata(RISCV_ARCH-1, 5) = i_csr_resp_data.read()(RISCV_ARCH-1, 5);
        vb_csr_cmd_wdata(4, 0) = i_csr_resp_data.read()(4, 0) & ~r.radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRS]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = i_csr_resp_data.read() | r.rdata1.read();
    } else if (wv[Instr_CSRRSI]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata(RISCV_ARCH-1, 5) = i_csr_resp_data.read()(RISCV_ARCH-1, 5);
        vb_csr_cmd_wdata(4, 0) = i_csr_resp_data.read()(4, 0) | r.radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRW]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = r.rdata1.read();
    } else if (wv[Instr_CSRRWI]) {
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata(4, 0) = r.radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    }

    wb_select[Res_Zero].res = 0;
    wb_select[Res_Reg2].res = r.rdata2;
    wb_select[Res_Csr].res = r.res_csr;
    wb_select[Res_Npc].res = r.res_npc;
    wb_select[Res_Ra].res = r.res_ra;

    // Select result:
    if (r.select.read()[Res_Reg2]) {
        vb_res = wb_select[Res_Reg2].res;
    } else if (r.select.read()[Res_Npc]) {
        vb_res = wb_select[Res_Npc].res;
    } else if (r.select.read()[Res_Ra]) {
        vb_res = wb_select[Res_Ra].res;
    } else if (r.select.read()[Res_Csr]) {
        vb_res = wb_select[Res_Csr].res;
    } else if (r.select.read()[Res_Alu]) {
        vb_res = wb_select[Res_Alu].res;
    } else if (r.select.read()[Res_AddSub]) {
        vb_res = wb_select[Res_AddSub].res;
    } else if (r.select.read()[Res_Shifter]) {
        vb_res = wb_select[Res_Shifter].res;
    } else if (r.select.read()[Res_IMul]) {
        vb_res = wb_select[Res_IMul].res;
    } else if (r.select.read()[Res_IDiv]) {
        vb_res = wb_select[Res_IDiv].res;
    } else if (r.select.read()[Res_FPU]) {
        vb_res = wb_select[Res_FPU].res;
    } else {
        vb_res = 0;
    }

    if ((i_d_pc == r.npc.read()
        && i_d_progbuf_ena.read() == 0 && i_dbg_progbuf_ena.read() == 0)
        || (i_d_pc == r.dnpc.read()
        && i_d_progbuf_ena.read() == 1 && i_dbg_progbuf_ena.read() == 1)) {
        v_d_valid = 1;
    }

    switch (r.state.read()) {
    case State_Idle:
        if (r.memop_valid && !i_memop_ready) {
            // Do nothing, previous memaccess request wasn't accepted. queue is full.
        } else if (v_d_valid == 1 && w_hazard1 == 0 && w_hazard2 == 0) {
            v_d_ready = 1;
            v_latch_input = 1;
            // opencocd doesn't clear 'step' value in dcsr after step has been done
            v.stepdone = i_step & !i_dbg_progbuf_ena;
            if (i_dbg_mem_req_valid.read()) {
                v_dbg_mem_req_ready = 1;
                v_dbg_mem_req_error = v_debug_misaligned;
                sc_uint<MemopType_Total> t_type = 0;
                if (v_debug_misaligned) {
                    v.state = State_DebugMemError;
                } else {
                    v.state = State_DebugMemRequest;
                }
                v.memop_halted = 0;
                v.memop_sign_ext = 0;
                t_type[MemopType_Store] = i_dbg_mem_req_write;
                v.memop_type = t_type;
                v.memop_size = i_dbg_mem_req_size;
            } else if (v_csr_cmd_ena) {
                v.state = State_Csr;
                v.csrstate = CsrState_Req;
                v.csr_req_type = vb_csr_cmd_type;
                v.csr_req_addr = vb_csr_cmd_addr;
                v.csr_req_data = vb_csr_cmd_wdata;
                v.csr_req_rmw = vb_csr_cmd_type[CsrReq_ReadBit];  // read/modify/write
                v.mem_ex_load_fault = 0;
                v.mem_ex_store_fault = 0;
                v.mem_ex_mpu_store = 0;
                v.mem_ex_mpu_load = 0;
                v.stack_overflow = 0;
                v.stack_underflow = 0;
            } else if (vb_select[Res_IMul] || vb_select[Res_IDiv] || vb_select[Res_FPU]) {
                v.state = State_WaitMulti;
            } else if (i_amo.read() == 1) {
                v_memop_ena = 1;
                vb_memop_memaddr = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0);
                vb_memop_wdata = vb_rdata2;
                v.state = State_Amo;
                if (!i_memop_ready.read()) {
                    v.amostate = AmoState_WaitMemAccess;
                } else {
                    v.amostate = AmoState_Read;
                }
            } else if (i_memop_load || i_memop_store) {
                v_memop_ena = 1;
                vb_memop_wdata = vb_rdata2;
                if (!i_memop_ready.read()) {
                    // Wait cycles until FIFO to memoryaccess becomes available
                    v.state = State_WaitMemAcces;
                } else {
                    v.valid = 1;
                }
            } else if (v_fence_i || v_fence_d) {
                vb_memop_memaddr = ~0ull;
                if (!i_memop_ready.read()) {
                    v.state = State_WaitFlushingAccept;
                } else if (v_fence_i) {
                    v.state = State_Flushing_I;     // Flushing I need to wait ending of flashing D
                } else {
                    v.valid = 1;
                }
            } else {
                v.valid = 1;
                v_reg_ena = i_d_waddr.read().or_reduce() && !i_memop_load; // should be written by memaccess, but tag must be updated
            }
        } else {
            v_d_ready = 1;
        }
        break;
    case State_WaitMemAcces:
        // Fifo exec => memacess is full
        vb_memop_memaddr = r.memop_memaddr;
        if (i_memop_ready.read()) {
            v.state = State_Idle;
            v.valid = 1;
        }
        break;
    case State_Csr:
        // Request throught CSR bus
        switch (r.csrstate.read()) {
        case CsrState_Req:
            v_csr_req_valid = 1;
            if (i_csr_req_ready.read() == 1) {
                v.csrstate = CsrState_Resp;
            }
            break;
        case CsrState_Resp:
            v_csr_resp_ready = 1;
            if (i_csr_resp_valid.read() == 1) {
                v.csrstate = CsrState_Idle;
                if (i_csr_resp_exception == 1) {
                    if (i_dbg_progbuf_ena.read() == 1) {
                        v.valid = 0;
                        v.state = State_Halted;
                    } else {
                        // Invalid access rights
                        v.csrstate = CsrState_Req;
                        v.csr_req_type = CsrReq_ExceptionCmd;
                        v.csr_req_addr = EXCEPTION_InstrIllegal;
                        v.csr_req_data = mux.instr;
                        v.csr_req_rmw = 0;
                    }
                } else if (r.csr_req_type.read()[CsrReq_HaltBit]) {
                    v.valid = 0;
                    v.state = State_Halted;
                } else if (r.csr_req_type.read()[CsrReq_BreakpointBit]) {
                    v.valid = 0;
                    if (i_csr_resp_data.read()[0] == 1) {
                        // ebreakm is set
                        v.state = State_Halted;
                    } else {
                        v.state = State_Idle;
                        if (i_dbg_progbuf_ena.read() == 0) {
                            v.npc = i_csr_resp_data;
                        }
                    }
                } else if (r.csr_req_type.read()[CsrReq_ExceptionBit]
                         | r.csr_req_type.read()[CsrReq_InterruptBit]
                         | r.csr_req_type.read()[CsrReq_ResumeBit]) {
                    v.valid = 0;                // No valid strob should be generated
                    v.state = State_Idle;
                    if (i_dbg_progbuf_ena.read() == 0) {
                        v.npc = i_csr_resp_data;
                    }
                } else if (r.csr_req_type.read()[CsrReq_WfiBit]) {
                    if (i_csr_resp_data.read()[0]) {
                        // Invalid WFI instruction in current mode
                        v.csrstate = CsrState_Req;
                        v.csr_req_type = CsrReq_ExceptionCmd;
                        v.csr_req_addr = EXCEPTION_InstrIllegal;
                        v.csr_req_data = mux.instr;
                        v.csr_req_rmw = 0;
                    } else {
                        v.valid = 1;
                        v.state = State_Wfi;
                    }
                } else if (r.csr_req_type.read()[CsrReq_TrapReturnBit]) {
                    v.valid = 1;
                    v.state = State_Idle;
                    if (i_dbg_progbuf_ena.read() == 0) {
                        v.npc = i_csr_resp_data;
                    }
                } else if (r.csr_req_rmw.read()) {
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = CsrReq_WriteCmd;
                    v.csr_req_data = vb_csr_cmd_wdata;
                    v.csr_req_rmw = 0;

                    // Store result int cpu register on next clock
                    v.res_csr = i_csr_resp_data.read();
                    v_reg_ena = r.waddr.read().or_reduce();
                    vb_reg_waddr = r.waddr;
                } else {
                    v.state = State_Idle;
                    v.valid = 1;
                }
            }
            break;
        default:;
        }
        break;
    case State_Amo:
        switch (r.amostate.read()) {
        case AmoState_WaitMemAccess:
            // No need to make memop_valid active
            if (i_memop_ready.read()) {
                v.amostate = AmoState_Read;
            }
            break;
        case AmoState_Read:
            v.amostate = AmoState_Modify;
            if (wv[Instr_AMOSWAP_D] || wv[Instr_AMOSWAP_W]) {
                v.radr1 = 0;
            } else {
                v.radr1 = r.waddr;
            }
            break;
        case AmoState_Modify:
            if (w_hazard1 == 0 && w_hazard2 == 0) {
                // Need to wait 1 clock to latch addsub/alu output
                v.amostate = AmoState_Write;
                mux.memop_type[MemopType_Store] = 1;    // no need to do this in rtl, just assign to v.memop_type[0]
                v.memop_type = mux.memop_type; 
            }
            break;
        case AmoState_Write:
            v_memop_ena = 1;
            vb_memop_memaddr = r.memop_memaddr;
            vb_memop_wdata = vb_res;
            if (i_memop_ready.read()) {
                v.state = State_Idle;
                v.amostate = AmoState_WaitMemAccess;
                v.valid = 1;
            }
            break;
        default:;
        }
        break;
    case State_WaitMulti:
        // Wait end of multiclock instructions
        if (wb_select[Res_IMul].valid
          | wb_select[Res_IDiv].valid
          | wb_select[Res_FPU].valid) {
            v.state = State_Idle;
            v_reg_ena = r.waddr.read().or_reduce();
            vb_reg_waddr = r.waddr;
            v.valid = 1;
        }
        break;
    case State_WaitFlushingAccept:
        // Fifo exec => memacess is full
        vb_memop_memaddr = ~0ull;
        if (i_memop_ready.read()) {
            v.flushd = 0;
            v.flushi = 0;
            if (mux.ivec[Instr_FENCE] == 1) {
                // no need to wait ending of D-flashing
                v.state = State_Idle;
                v.valid = 1;
            } else {
                v.state = State_Flushing_I;
            }
        }
        break;
    case State_Flushing_I:
        // Flushing DataCache could take much more time than flushing I
        // so that we should wait D-cache finish before requesting new
        // instruction to avoid reading obsolete data.
        v.flushd = 0;
        v.flushi = 0;
        if (i_flushd_end.read() == 1) {
            v.state = State_Idle;
            v.valid = 1;
        }
        break;
    case State_Halted:
        v.stepdone = 0;
        if (i_resumereq.read() == 1 || i_dbg_progbuf_ena.read() == 1) {
            v.state = State_Csr;
            v.csrstate = CsrState_Req;
            v.csr_req_type = CsrReq_ResumeCmd;
            v.csr_req_addr = 0;
            v.csr_req_data = 0;
        } else if (i_dbg_mem_req_valid.read()) {
            v_dbg_mem_req_ready = 1;
            v_dbg_mem_req_error = v_debug_misaligned;
            sc_uint<MemopType_Total> t_type = 0;
            if (v_debug_misaligned) {
                v.state = State_DebugMemError;
            } else {
                v.state = State_DebugMemRequest;
            }
            v.memop_halted = 1;
            v.memop_sign_ext = 0;
            t_type[MemopType_Store] = i_dbg_mem_req_write;
            v.memop_type = t_type;
            v.memop_size = i_dbg_mem_req_size;
        }
        break;
    case State_DebugMemRequest:
        v_memop_ena = 1;
        v_memop_debug = 1;
        vb_memop_memaddr = i_dbg_mem_req_addr;
        vb_memop_wdata = i_dbg_mem_req_wdata;
        if (i_memop_ready.read()) {
            if (r.memop_halted) {
                v.state = State_Halted;
            } else {
                v.state = State_Idle;
            }
        }
        break;
    case State_DebugMemError:
        if (r.memop_halted) {
            v.state = State_Halted;
        } else {
            v.state = State_Idle;
        }
        break;
    case State_Wfi:
        if (i_haltreq || i_irq_external || i_irq_timer) {
            v.state = State_Idle;
        }
        break;
    default:;
    }

    // Next tags:
    t_waddr = vb_reg_waddr.to_int();

    t_tagcnt_wr = r.tagcnt.read()(CFG_REG_TAG_WIDTH*t_waddr + (CFG_REG_TAG_WIDTH - 1),
                                         CFG_REG_TAG_WIDTH*t_waddr).to_int() + 1;

    vb_tagcnt_next = r.tagcnt;
    vb_tagcnt_next(CFG_REG_TAG_WIDTH*t_waddr+(CFG_REG_TAG_WIDTH-1), CFG_REG_TAG_WIDTH*t_waddr) = t_tagcnt_wr;
    vb_tagcnt_next(CFG_REG_TAG_WIDTH - 1, 0) = 0;      // r0 always 0

    if (i_dbg_progbuf_ena.read() == 0) {
        v.dnpc = 0;
    }

    // Latch decoder's data into internal registers:
    if (v_latch_input) {
        if (i_dbg_progbuf_ena.read() == 1) {
            v.dnpc = r.dnpc.read() + opcode_len;
        } else {
            v.dnpc = 0;
            v.pc = i_d_pc;
            v.npc = vb_prog_npc;        // Actually this value will be restored on resume request
        }
        v.radr1 = i_d_radr1;
        v.radr2 = i_d_radr2;
        v.waddr = i_d_waddr;
        v.rdata1 = vb_rdata1;
        v.rdata2 = vb_rdata2;
        v.imm = i_d_imm;
        v.ivec = i_ivec;
        v.isa_type = i_isa_type;
        v.unsigned_op = i_unsigned_op;
        v.rv32 = i_rv32;
        v.compressed = i_compressed;
        v.f64 = i_f64;
        v.instr = i_d_instr;
        v.flushd = v_fence_i || v_fence_d;
        v.flushi = v_fence_i;
        if (v_fence_i) {
            v.flushi_addr = ~0ull;
        } else if (wv[Instr_EBREAK]) {
            v.flushi_addr = i_d_pc;
        }
        v.call = v_call;
        v.ret = v_ret;
        v.res_npc = vb_prog_npc;
        v.res_ra = vb_npc_incr;

        wb_select[Res_IMul].ena = vb_select[Res_IMul];
        wb_select[Res_IDiv].ena = vb_select[Res_IDiv];
        wb_select[Res_FPU].ena = vb_select[Res_FPU];
        v.select = vb_select;
    }
    if (v_reg_ena) {
        v.reg_write = 1;
        v.tagcnt = vb_tagcnt_next;
        v.reg_waddr = vb_reg_waddr;
        v.reg_wtag = t_tagcnt_wr;
    }
    if (v_memop_ena) {
        v.memop_valid = 1;
        v.memop_debug = v_memop_debug;
        v.memop_type = mux.memop_type;
        v.memop_sign_ext = mux.memop_sign_ext;
        v.memop_size = mux.memop_size;
        v.memop_memaddr = vb_memop_memaddr;
        v.memop_wdata = vb_memop_wdata;
        if (v_memop_debug == 0 
            && (mux.memop_type[MemopType_Store] == 0
                || mux.memop_type[MemopType_Release])) {
            // Error code of the instruction SC (store with release) should
            // be written into register
            v.tagcnt = vb_tagcnt_next;
            v.reg_waddr = vb_reg_waddr;
            v.reg_wtag = t_tagcnt_wr;
        }
    } else if (i_memop_ready.read()) {
        v.memop_valid = 0;
        v.memop_debug = 0;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    wb_rdata1 = vb_rdata1;
    wb_rdata2 = vb_rdata2;

    sc_uint<3> t_alu_mode;
    t_alu_mode[2] = wv[Instr_XOR] || wv[Instr_XORI]
                 || wv[Instr_AMOXOR_D] || wv[Instr_AMOXOR_W];
    t_alu_mode[1] = wv[Instr_OR] || wv[Instr_ORI]
                 || wv[Instr_AMOOR_D] || wv[Instr_AMOOR_W];
    t_alu_mode[0] = wv[Instr_AND] || wv[Instr_ANDI]
                 || wv[Instr_AMOAND_D] || wv[Instr_AMOAND_W];
    wb_alu_mode = t_alu_mode;

    sc_uint<7> t_addsub_mode;
    t_addsub_mode[6] = wv[Instr_AMOMAX_D] || wv[Instr_AMOMAX_W]
                    || wv[Instr_AMOMAXU_D] || wv[Instr_AMOMAXU_W];
    t_addsub_mode[5] = wv[Instr_AMOMIN_D] || wv[Instr_AMOMIN_W]
                    || wv[Instr_AMOMINU_D] || wv[Instr_AMOMINU_W];
    t_addsub_mode[4] = wv[Instr_SLT] || wv[Instr_SLTI]
                    || wv[Instr_SLTU] || wv[Instr_SLTIU];
    t_addsub_mode[3] = wv[Instr_SUB] || wv[Instr_SUBW];
    t_addsub_mode[2] = wv[Instr_ADD] || wv[Instr_ADDI]
                    || wv[Instr_ADDW] || wv[Instr_ADDIW]
                    || wv[Instr_AUIPC]
                    || wv[Instr_AMOADD_D] || wv[Instr_AMOADD_W]
                    || wv[Instr_AMOSWAP_D] || wv[Instr_AMOSWAP_W];
    t_addsub_mode[1] = wv[Instr_SLTU] || wv[Instr_SLTIU]
                    || wv[Instr_AMOMINU_D] || wv[Instr_AMOMINU_W]
                    || wv[Instr_AMOMAXU_D] || wv[Instr_AMOMAXU_W];   // unsigned
    t_addsub_mode[0] = mux.rv32;
    wb_addsub_mode = t_addsub_mode;

    sc_uint<4> t_shifter_mode;
    t_shifter_mode[3] = wv[Instr_SRA] || wv[Instr_SRAI] || wv[Instr_SRAW] || wv[Instr_SRAW] || wv[Instr_SRAIW];
    t_shifter_mode[2] = wv[Instr_SRL] || wv[Instr_SRLI] || wv[Instr_SRLW] || wv[Instr_SRLIW];
    t_shifter_mode[1] = wv[Instr_SLL] || wv[Instr_SLLI] || wv[Instr_SLLW] || wv[Instr_SLLIW];
    t_shifter_mode[0] = mux.rv32;
    wb_shifter_mode = t_shifter_mode;

    wb_shifter_a1 = vb_rdata1;
    wb_shifter_a2 = vb_rdata2(5, 0);

    o_radr1 = mux.radr1;
    o_radr2 = mux.radr2;

    o_reg_wena = r.reg_write;
    o_reg_waddr = r.reg_waddr;
    o_reg_wtag = r.reg_wtag;
    o_reg_wdata = vb_res;
    o_d_ready = v_d_ready;

    o_memop_valid = r.memop_valid;
    o_memop_debug = r.memop_debug;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_type = r.memop_type;
    o_memop_size = r.memop_size;
    o_memop_memaddr = r.memop_memaddr;
    o_memop_wdata = r.memop_wdata;

    o_csr_req_valid = v_csr_req_valid;
    o_csr_req_type = r.csr_req_type;
    o_csr_req_addr = r.csr_req_addr;
    o_csr_req_data = r.csr_req_data;
    o_csr_resp_ready = v_csr_resp_ready;

    o_dbg_mem_req_ready = v_dbg_mem_req_ready;
    o_dbg_mem_req_error = v_dbg_mem_req_error;

    if (i_dbg_progbuf_ena.read() == 1) {
        vb_o_npc = r.dnpc;
    } else {
        vb_o_npc = r.npc;
    }

    o_valid = r.valid;
    o_pc = r.pc;
    o_npc = vb_o_npc;
    o_instr = r.instr;
    o_flushd = r.flushd;    // must be post in a memory queue to avoid to early flushing
    o_flushi = r.flushi;
    o_flushi_addr = r.flushi_addr;
    o_call = r.call;
    o_ret = r.ret;
    o_halted = r.state.read() == State_Halted ? 1: 0;

    // Debug rtl only:!!
    for (int i = 0; i < Reg_Total; i++) {
        tag_expected[i] = r.tagcnt.read()(CFG_REG_TAG_WIDTH*i+(CFG_REG_TAG_WIDTH-1), CFG_REG_TAG_WIDTH*i).to_int();
    }
}

void InstrExecute::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

