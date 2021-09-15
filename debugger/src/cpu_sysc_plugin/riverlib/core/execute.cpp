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
    i_dbg_progbuf_ena("i_dbg_progbuf_ena"),
    i_wb_waddr("i_wb_waddr"),
    i_memop_store("i_memop_store"),
    i_memop_load("i_memop_load"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_size("i_memop_size"),
    i_unsigned_op("i_unsigned_op"),
    i_rv32("i_rv32"),
    i_compressed("i_compressed"),
    i_f64("i_f64"),
    i_isa_type("i_isa_type"),
    i_ivec("i_ivec"),
    i_unsup_exception("i_unsup_exception"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_executable("i_instr_executable"),
    i_dport_npc_write("i_dport_npc_write"),
    i_dport_npc("i_dport_npc"),
    i_rdata1("i_rdata1"),
    i_rtag1("i_rtag1"),
    i_rdata2("i_rdata2"),
    i_rtag2("i_rtag2"),
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
    i_trap_valid("i_trap_valid"),
    i_trap_pc("i_trap_pc"),
    o_ex_npc("o_ex_npc"),
    o_ex_instr_load_fault("o_ex_instr_load_fault"),
    o_ex_instr_not_executable("o_ex_instr_not_executable"),
    o_ex_illegal_instr("o_ex_illegal_instr"),
    o_ex_unalign_store("o_ex_unalign_store"),
    o_ex_unalign_load("o_ex_unalign_load"),
    o_ex_breakpoint("o_ex_breakpoint"),
    o_ex_ecall("o_ex_ecall"),
    o_ex_fpu_invalidop("o_ex_fpu_invalidop"),
    o_ex_fpu_divbyzero("o_ex_fpu_divbyzero"),
    o_ex_fpu_overflow("o_ex_fpu_overflow"),
    o_ex_fpu_underflow("o_ex_fpu_underflow"),
    o_ex_fpu_inexact("o_ex_fpu_inexact"),
    o_fpu_valid("o_fpu_valid"),
    o_memop_valid("o_memop_valid"),
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_type("o_memop_type"),
    o_memop_size("o_memop_size"),
    o_memop_memaddr("o_memop_memaddr"),
    o_memop_wdata("o_memop_wdata"),
    i_memop_ready("i_memop_ready"),
    o_trap_ready("o_trap_ready"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_npc("o_npc"),
    o_instr("o_instr"),
    i_flushd_end("i_flushd_end"),
    o_flushd("o_flushd"),
    o_flushi("o_flushi"),
    o_call("o_call"),
    o_ret("o_ret"),
    o_multi_ready("o_multi_ready") {
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
    sensitive << i_f64;
    sensitive << i_isa_type;
    sensitive << i_ivec;
    sensitive << i_unsup_exception;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_executable;
    sensitive << i_dport_npc_write;
    sensitive << i_dport_npc;
    sensitive << i_rdata1;
    sensitive << i_rtag1;
    sensitive << i_rdata2;
    sensitive << i_rtag2;
    sensitive << i_csr_req_ready;
    sensitive << i_csr_resp_valid;
    sensitive << i_csr_resp_data;
    sensitive << i_trap_valid;
    sensitive << i_trap_pc;
    sensitive << i_flushd_end;
    sensitive << r.state;
    sensitive << r.csrstate;
    sensitive << r.pc;
    sensitive << r.npc;
    sensitive << r.instr;
    sensitive << r.hold_radr1;
    sensitive << r.hold_radr2;
    sensitive << r.hold_waddr;
    sensitive << r.hold_rdata1;
    sensitive << r.hold_ivec;
    sensitive << r.tagcnt_rd;
    sensitive << r.tagcnt_wr;
    sensitive << r.select;
    sensitive << r.reg_waddr;
    sensitive << r.reg_wtag;
    sensitive << r.csr_req_rmw;
    sensitive << r.csr_req_pc;
    sensitive << r.csr_req_type;
    sensitive << r.csr_req_addr;
    sensitive << r.csr_req_data;
    sensitive << r.memop_valid;
    sensitive << r.memop_type;
    sensitive << r.memop_memaddr;
    sensitive << r.memop_wdata;

    sensitive << r.res_reg2;
    sensitive << r.res_npc;
    sensitive << r.res_ra;
    sensitive << r.res_csr;

    sensitive << r.flushi;
    sensitive << r.flushd;
    sensitive << r.reg_write;
    sensitive << r.valid;
    sensitive << r.call;
    sensitive << r.ret;
    sensitive << r.hold_fencei;
#ifdef UPDT2
    for (int i = 0; i < Res_Total; i++) {
        sensitive << wb_select.ena[i];
        sensitive << wb_select.valid[i];
        sensitive << wb_select.res[i];
    }
#else
    sensitive << w_arith_valid[Multi_MUL];
    sensitive << w_arith_valid[Multi_DIV];
    sensitive << w_arith_valid[Multi_FPU];
    sensitive << w_arith_busy[Multi_MUL];
    sensitive << w_arith_busy[Multi_DIV];
    sensitive << w_arith_busy[Multi_FPU];
#endif
    sensitive << wb_shifter_a1;
    sensitive << wb_shifter_a2;
    sensitive << wb_sll;
    sensitive << wb_sllw;
    sensitive << wb_srl;
    sensitive << wb_srlw;
    sensitive << wb_sra;
    sensitive << wb_sraw;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    alu0 = new AluLogic("alu0", async_reset);
    alu0->i_clk(i_clk);
    alu0->i_nrst(i_nrst);
    alu0->i_mode(wb_alu_mode);
    alu0->i_a1(wb_rdata1);
    alu0->i_a2(wb_rdata2);
    alu0->o_res(wb_select.res[Res_Alu]);

    addsub0 = new IntAddSub("addsub0", async_reset);
    addsub0->i_clk(i_clk);
    addsub0->i_nrst(i_nrst);
    addsub0->i_mode(wb_addsub_mode);
    addsub0->i_a1(wb_rdata1);
    addsub0->i_a2(wb_rdata2);
    addsub0->o_res(wb_select.res[Res_AddSub]);

    mul0 = new IntMul("mul0", async_reset);
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(wb_select.ena[Res_IMul]);
    mul0->i_unsigned(i_unsigned_op);
    mul0->i_hsu(w_mul_hsu);
    mul0->i_high(w_arith_residual_high);
    mul0->i_rv32(i_rv32);
    mul0->i_a1(wb_rdata1);
    mul0->i_a2(wb_rdata2);
    mul0->o_res(wb_select.res[Res_IMul]);
    mul0->o_valid(wb_select.valid[Res_IMul]);

    div0 = new IntDiv("div0", async_reset);
    div0->i_clk(i_clk);
    div0->i_nrst(i_nrst);
    div0->i_ena(wb_select.ena[Res_IDiv]);
    div0->i_unsigned(i_unsigned_op);
    div0->i_residual(w_arith_residual_high);
    div0->i_rv32(i_rv32);
    div0->i_a1(wb_rdata1);
    div0->i_a2(wb_rdata2);
    div0->o_res(wb_select.res[Res_IDiv]);
    div0->o_valid(wb_select.valid[Res_IDiv]);

    sh0 = new Shifter("sh0", async_reset);
    sh0->i_clk(i_clk);
    sh0->i_nrst(i_nrst);
    sh0->i_mode(wb_shifter_mode);
    sh0->i_a1(wb_shifter_a1);
    sh0->i_a2(wb_shifter_a2);
    sh0->o_res(wb_select.res[Res_Shifter]);

    if (fpu_ena_) {
        fpu0 = new FpuTop("fpu0", async_reset);
        fpu0->i_clk(i_clk);
        fpu0->i_nrst(i_nrst);
        fpu0->i_ena(wb_select.ena[Res_FPU]);
        fpu0->i_ivec(wb_fpu_vec);
        fpu0->i_a(wb_rdata1);
        fpu0->i_b(wb_rdata2);
        fpu0->o_res(wb_select.res[Res_FPU]);
        fpu0->o_ex_invalidop(o_ex_fpu_invalidop);
        fpu0->o_ex_divbyzero(o_ex_fpu_divbyzero);
        fpu0->o_ex_overflow(o_ex_fpu_overflow);
        fpu0->o_ex_underflow(o_ex_fpu_underflow);
        fpu0->o_ex_inexact(o_ex_fpu_inexact);
        fpu0->o_valid(wb_select.valid[Res_FPU]);
    } else {
        wb_select.res[Res_FPU] = 0;
        wb_select.valid[Res_FPU] = 0;
        o_fpu_valid = 0;
        o_ex_fpu_invalidop = 0;
        o_ex_fpu_divbyzero = 0;
        o_ex_fpu_overflow = 0;
        o_ex_fpu_underflow = 0;
        o_ex_fpu_inexact = 0;
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
        sc_trace(o_vcd, i_unsup_exception, i_unsup_exception.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_executable, i_instr_executable.name());
        sc_trace(o_vcd, i_rdata1, i_rdata1.name());
        sc_trace(o_vcd, i_rtag1, i_rtag1.name());
        sc_trace(o_vcd, i_rdata2, i_rdata2.name());
        sc_trace(o_vcd, i_rtag2, i_rtag2.name());
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
        sc_trace(o_vcd, i_trap_valid, i_trap_valid.name());
        sc_trace(o_vcd, i_trap_pc, i_trap_pc.name());
        sc_trace(o_vcd, i_flushd_end, i_flushd_end.name());
        sc_trace(o_vcd, o_ex_npc, o_ex_npc.name());
        sc_trace(o_vcd, o_memop_valid, o_memop_valid.name());
        sc_trace(o_vcd, o_memop_type, o_memop_type.name());
        sc_trace(o_vcd, o_memop_size, o_memop_size.name());
        sc_trace(o_vcd, o_memop_memaddr, o_memop_memaddr.name());
        sc_trace(o_vcd, o_memop_wdata, o_memop_wdata.name());

        sc_trace(o_vcd, o_flushd, o_flushd.name());
        sc_trace(o_vcd, o_flushi, o_flushi.name());
        sc_trace(o_vcd, i_memop_ready, i_memop_ready.name());
        sc_trace(o_vcd, o_trap_ready, o_trap_ready.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_npc, o_npc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_call, o_call.name());
        sc_trace(o_vcd, o_ret, o_ret.name());
        sc_trace(o_vcd, o_ex_instr_not_executable, o_ex_instr_not_executable.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_select.ena[Res_IMul], pn + ".w_arith_ena(5)");
        sc_trace(o_vcd, wb_select.res[Res_IMul], pn + ".wb_arith_res(5)");
        sc_trace(o_vcd, wb_select.ena[Res_IDiv], pn + ".w_arith_ena(6)");
        sc_trace(o_vcd, wb_select.res[Res_IDiv], pn + ".wb_arith_res(6)");
        sc_trace(o_vcd, wb_select.ena[Res_FPU], pn + ".w_arith_ena(7)");
        sc_trace(o_vcd, wb_select.res[Res_FPU], pn + ".wb_arith_res(7)");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.csrstate, pn + ".r_csrstate");
        sc_trace(o_vcd, r.hold_fencei, pn + ".r_hold_fencei");
        sc_trace(o_vcd, r.tagcnt_rd, pn + ".r_tagcnt_rd");
        sc_trace(o_vcd, r.tagcnt_wr, pn + ".r_tagcnt_wr");
        sc_trace(o_vcd, r.select, pn + ".r_select");
        sc_trace(o_vcd, r.reg_waddr, pn + ".r_reg_waddr");
        sc_trace(o_vcd, r.reg_wtag, pn + ".r_reg_wtag");
        sc_trace(o_vcd, r.hold_rdata1, pn + ".r_hold_rdata1");
        sc_trace(o_vcd, r.flushd, pn + ".r_flushd");
#if!defined(UPDT2)
        sc_trace(o_vcd, w_next_ready, pn + ".w_next_ready");
#endif
        sc_trace(o_vcd, w_multi_ena, pn + ".w_multi_ena");
        sc_trace(o_vcd, w_multi_busy, pn + ".w_multi_busy");
        sc_trace(o_vcd, w_multi_ready, pn + ".w_multi_ready");
        sc_trace(o_vcd, w_hold_memop, pn + ".w_hold_memop");
        sc_trace(o_vcd, w_hold_multi, pn + ".w_hold_multi");
        sc_trace(o_vcd, w_hold_hazard, pn + ".w_hold_hazard");
        sc_trace(o_vcd, w_test_hazard1, pn + ".w_test_hazard1");
        sc_trace(o_vcd, w_test_hazard2, pn + ".w_test_hazard2");
        sc_trace(o_vcd, tag_expected[0xA], pn + ".tag_expected0x0A");
        sc_trace(o_vcd, tag_expected[0xb], pn + ".tag_expected0x0B");
        sc_trace(o_vcd, tag_expected[0xf], pn + ".tag_expected0x0F");
        sc_trace(o_vcd, wb_select.res[Res_AddSub], pn + ".t_res_AddSub");
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
    bool v_fence_d;
    bool v_fence_i;
    bool v_mret;
    bool v_uret;
    bool v_csr_req_valid;
    bool v_csr_resp_ready;
    sc_uint<RISCV_ARCH> vb_csr_wdata;
    sc_uint<RISCV_ARCH> vb_res;
    sc_uint<CFG_CPU_ADDR_BITS> vb_prog_npc;
    sc_uint<CFG_CPU_ADDR_BITS> vb_npc_incr;
    sc_uint<CFG_CPU_ADDR_BITS> vb_npc;
    sc_uint<RISCV_ARCH> vb_off;
    sc_uint<RISCV_ARCH> vb_sum64;
    sc_uint<RISCV_ARCH> vb_sum32;
    sc_uint<RISCV_ARCH> vb_sub64;
    sc_uint<RISCV_ARCH> vb_sub32;
    sc_uint<RISCV_ARCH> vb_and64;
    sc_uint<RISCV_ARCH> vb_or64;
    sc_uint<RISCV_ARCH> vb_xor64;
    sc_uint<CFG_CPU_ADDR_BITS> vb_memop_memaddr;
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
    sc_uint<RISCV_ARCH> vb_i_rdata1;
    sc_uint<RISCV_ARCH> vb_i_rdata2;
    sc_uint<RISCV_ARCH> vb_rdata1;
    sc_uint<RISCV_ARCH> vb_rdata2;
    bool v_check_tag1;
    bool v_check_tag2;
    bool v_o_valid;
    sc_uint<RISCV_ARCH> vb_o_wdata;
    bool v_hold_exec;
    bool v_multi_ena;
    bool v_next_mul_ready;
    bool v_next_div_ready;
    bool v_next_fpu_ready;
    bool v_wena;
    bool v_whazard;
    sc_uint<6> vb_waddr;
    bool v_next_normal;
    bool v_next_progbuf;
    sc_uint<Res_Total> vb_select;
    sc_biguint<CFG_REG_TAG_WITH*REGS_TOTAL> vb_tagcnt_wr;
    sc_biguint<CFG_REG_TAG_WITH*REGS_TOTAL> vb_tagcnt_rd;
    bool v_d_ready;
    bool v_latch_input;
    bool v_memop_ena;
    bool v_reg_ena;
    sc_uint<6> vb_reg_waddr;
    bool v_csr_cmdpc_ena;
    sc_uint<12> vb_csr_cmdpc_addr;

    v = r;

    v_d_ready = 0;
    v_csr_req_valid = 0;
    v_csr_resp_ready = 0;
    vb_csr_wdata = 0;
    vb_res = 0;
    vb_off = 0;
    vb_memop_memaddr = 0;
    wv = i_ivec.read();
    v_call = 0;
    v_ret = 0;
    v.valid = 0;
    v.call = 0;
    v.ret = 0;
#ifdef UPDT2
    v.reg_write = 0;
    v.flushd = 0;
    v.flushi = 0;
#endif
    vb_rdata1 = 0;
    vb_rdata2 = 0;
    vb_select = 0;
    for (int i = 0; i < Res_Total; i++) {
        wb_select.ena[i] = 0;
    }
    v_csr_cmdpc_ena = 0;
    vb_csr_cmdpc_addr = 0;

    vb_i_rdata1 = i_rdata1;
    vb_i_rdata2 = i_rdata2;
    v_check_tag1 = 0;
    v_check_tag2 = 0;

    if (i_isa_type.read()[ISA_R_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    } else if (i_isa_type.read()[ISA_I_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = i_d_imm;
        v_check_tag1 = 1;
    } else if (i_isa_type.read()[ISA_SB_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        vb_off = i_d_imm;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        vb_rdata1 = i_d_pc;
        vb_off = i_d_imm;
        v_check_tag1 = 1;
    } else if (i_isa_type.read()[ISA_U_type]) {
        vb_rdata1 = i_d_pc;
        vb_rdata2 = i_d_imm;
    } else if (i_isa_type.read()[ISA_S_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        vb_off = i_d_imm;
        v_check_tag1 = 1;
        v_check_tag2 = 1;
    }

#ifdef UPDT2
    // Check that registers tags are equal to expeted ones
    vb_tagcnt_wr = r.tagcnt_wr;
    vb_tagcnt_rd = r.tagcnt_rd;

    int t_d_radr1 = i_d_radr1.read().to_int();
    int t_d_radr2 = i_d_radr2.read().to_int();

    w_test_hazard1 = 0;
    if (r.tagcnt_rd.read()(CFG_REG_TAG_WITH * t_d_radr1 + (CFG_REG_TAG_WITH - 1),
                           CFG_REG_TAG_WITH * t_d_radr1) != i_rtag1.read()) {
        w_test_hazard1 = v_check_tag1;
    }
    w_test_hazard2 = 0;
    if (r.tagcnt_rd.read()(CFG_REG_TAG_WITH * t_d_radr2 + (CFG_REG_TAG_WITH - 1),
                           CFG_REG_TAG_WITH * t_d_radr2) != i_rtag2.read()) {
        w_test_hazard2 = v_check_tag2;
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
#endif

    wb_fpu_vec = wv.range(Instr_FSUB_D, Instr_FADD_D);  // directly connected i_ivec
#ifdef UPDT2
    v_fence_d = wv[Instr_FENCE].to_bool();
    v_fence_i = wv[Instr_FENCE_I].to_bool();
#else
    w_multi_busy = w_arith_busy[Multi_MUL] | w_arith_busy[Multi_DIV]
                  | w_arith_busy[Multi_FPU];

    w_multi_ready = w_arith_valid[Multi_MUL] | w_arith_valid[Multi_DIV]
                  | w_arith_valid[Multi_FPU];


    /** Hold signals:
            1. hazard
            2. memaccess not ready to accept next memop operation (or flush request)
            3. multi instruction
            4. Flushing $D on flush.i instruction
     */
    w_hold_hazard = i_rhazard1.read() || i_rhazard2.read();

    w_hold_memop = (i_memop_load.read() || i_memop_store.read()
                    || wv[Instr_FENCE] || wv[Instr_FENCE_I])
                && !i_memop_ready.read();

    w_hold_multi = w_multi_busy | w_multi_ready;

    v_hold_exec = w_hold_hazard || w_hold_memop || w_hold_multi
                || r.hold_fencei.read();

    v_next_normal = 0;
    if (i_d_pc.read() == r.npc.read() && i_dbg_progbuf_ena.read() == 0
        && i_d_progbuf_ena.read() == 0) {
        v_next_normal = 1;
    }

    v_next_progbuf = 0;
    if (i_d_pc.read() == r.progbuf_npc.read() && i_dbg_progbuf_ena.read() == 1
        && i_d_progbuf_ena.read() == 1) {
        v_next_progbuf = 1;
    }

    w_next_ready = 0;
    if (i_d_valid.read() == 1 && (v_next_normal|v_next_progbuf) == 1
        && v_hold_exec == 0) {
        w_next_ready = 1;
    }

    v_fence = wv[Instr_FENCE].to_bool() & w_next_ready;
    v_fencei = wv[Instr_FENCE_I].to_bool() & w_next_ready;
    v_fenced = v_fence | v_fencei;
    v_mret = wv[Instr_MRET].to_bool() & w_next_ready;
    v_uret = wv[Instr_URET].to_bool() & w_next_ready;

    v_next_mul_ready = (wv[Instr_MUL] || wv[Instr_MULW]
                     || wv[Instr_MULH]|| wv[Instr_MULHSU]
                     || wv[Instr_MULHU]) && w_next_ready;
    v_next_div_ready = (wv[Instr_DIV] || wv[Instr_DIVU]
                            || wv[Instr_DIVW] || wv[Instr_DIVUW]
                            || wv[Instr_REM] || wv[Instr_REMU]
                            || wv[Instr_REMW] || wv[Instr_REMUW]) && w_next_ready;
    v_next_fpu_ready = 0;
    if (fpu_ena_) {
        if (i_f64.read() && !(wv[Instr_FSD] | wv[Instr_FLD]).to_bool()) {
            v_next_fpu_ready = w_next_ready;
        }
    }
#endif

    w_arith_residual_high = (wv[Instr_REM] || wv[Instr_REMU]
                          || wv[Instr_REMW] || wv[Instr_REMUW]
                          || wv[Instr_MULH] || wv[Instr_MULHSU] || wv[Instr_MULHU]);

    w_mul_hsu = wv[Instr_MULHSU].to_bool();

#if!defined(UPDT2)
    v_multi_ena = v_next_mul_ready || v_next_div_ready || v_next_fpu_ready;

    w_arith_ena[Multi_MUL] = v_next_mul_ready;
    w_arith_ena[Multi_DIV] = v_next_div_ready;
    w_arith_ena[Multi_FPU] = v_next_fpu_ready;
#endif

    if (i_memop_load) {
        vb_memop_memaddr =
            vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_rdata2(CFG_CPU_ADDR_BITS-1, 0);
    } else if (i_memop_store) {
        vb_memop_memaddr = 
            vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    } else if ((wv[Instr_FENCE] || wv[Instr_FENCE_I]) == 1) {
        vb_memop_memaddr = ~0ull;
    } else if (wv[Instr_EBREAK] == 1) {
        vb_memop_memaddr = i_d_pc.read();
    }

    w_exception_store = 0;
    w_exception_load = 0;

    if ((wv[Instr_LD] && vb_memop_memaddr(2, 0) != 0)
        || ((wv[Instr_LW] || wv[Instr_LWU]) && vb_memop_memaddr(1, 0) != 0)
        || ((wv[Instr_LH] || wv[Instr_LHU]) && vb_memop_memaddr[0] != 0)) {
        w_exception_load = 1;
    }
    if ((wv[Instr_SD] && vb_memop_memaddr(2, 0) != 0)
        || (wv[Instr_SW] && vb_memop_memaddr(1, 0) != 0)
        || (wv[Instr_SH] && vb_memop_memaddr[0] != 0)) {
        w_exception_store = 1;
    }


#if!defined (UPDT2)
    // parallel ALU:
    vb_sum64 = vb_rdata1 + vb_rdata2;
    vb_sum32(31, 0) = vb_rdata1(31, 0) + vb_rdata2(31, 0);
    if (vb_sum32[31]) {
        vb_sum32(63, 32) = ~0;
    }
    vb_sub64 = vb_rdata1 - vb_rdata2;
    vb_sub32(31, 0) = vb_rdata1(31, 0) - vb_rdata2(31, 0);
    if (vb_sub32[31]) {
        vb_sub32(63, 32) = ~0;
    }
    vb_and64 = vb_rdata1 & vb_rdata2;
    vb_or64 = vb_rdata1 | vb_rdata2;
    vb_xor64 = vb_rdata1 ^ vb_rdata2;

    wb_shifter_a1 = vb_rdata1;
    wb_shifter_a2 = vb_rdata2(5, 0);

    v_ltu = 0;
    v_geu = 0;
    if (vb_rdata1 < vb_rdata2) {
        v_ltu = 1;
    }
    if (vb_rdata1 >= vb_rdata2) {
        v_geu = 1;
    }

    // Relative Branch on some condition:
    v_pc_branch = 0;
    if ((wv[Instr_BEQ].to_bool() & (vb_sub64 == 0))
        || (wv[Instr_BGE].to_bool() & (vb_sub64[63] == 0))
        || (wv[Instr_BGEU].to_bool() & (v_geu))
        || (wv[Instr_BLT].to_bool() & (vb_sub64[63] == 1))
        || (wv[Instr_BLTU].to_bool() & (v_ltu))
        || (wv[Instr_BNE].to_bool() & (vb_sub64 != 0))) {
        v_pc_branch = 1;
    }
#endif

    opcode_len = 4;
    if (i_compressed.read()) {
        opcode_len = 2;
    }
    vb_npc_incr = i_d_pc.read() + opcode_len;

    if (v_pc_branch) {
        vb_prog_npc = i_d_pc.read() + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        vb_prog_npc = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_off(CFG_CPU_ADDR_BITS-1, 0);
    } else if (wv[Instr_JALR].to_bool()) {
        vb_prog_npc = vb_rdata1(CFG_CPU_ADDR_BITS-1, 0) + vb_rdata2(CFG_CPU_ADDR_BITS-1, 0);
        vb_prog_npc[0] = 0;
    } else {
        vb_prog_npc = vb_npc_incr;
    }

    v_csr_cmdpc_ena = i_unsup_exception.read() || wv[Instr_ECALL];
    if (i_unsup_exception.read()) {
        vb_csr_cmdpc_addr = CsrReq_PcCmd_UnsupInstruction;
    } else if (wv[Instr_ECALL]) {
        vb_csr_cmdpc_addr = CsrReq_PcCmd_EnvCall;
    }

    if (i_trap_valid.read()) {
        vb_npc = i_trap_pc.read();
    } else {
        vb_npc = vb_prog_npc;
    }

#ifdef UPDT2
    vb_select[Res_Reg2] = i_memop_store || wv[Instr_LUI];
    vb_select[Res_Npc] = 0;
    vb_select[Res_Ra] = v_pc_branch || wv[Instr_JAL] || wv[Instr_JALR]
                        || wv[Instr_MRET] || wv[Instr_URET]
                        || i_trap_valid;
    vb_select[Res_Csr] = wv[Instr_CSRRC] || wv[Instr_CSRRCI] || wv[Instr_CSRRS]
                        || wv[Instr_CSRRSI] || wv[Instr_CSRRW] || wv[Instr_CSRRWI];
    vb_select[Res_Alu] = wv[Instr_AND] || wv[Instr_ANDI]
                      || wv[Instr_OR] || wv[Instr_ORI]
                      || wv[Instr_XOR] || wv[Instr_XORI];
    vb_select[Res_AddSub] = wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]
                         || wv[Instr_ADDW] || wv[Instr_ADDIW]
                         || wv[Instr_SUB] || wv[Instr_SUBW]
                         || wv[Instr_SLT] || wv[Instr_SLTI]
                         || wv[Instr_SLTU] || wv[Instr_SLTIU];
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
        vb_select[Res_FPU] = i_f64.read() && !(wv[Instr_FSD] | wv[Instr_FLD]).to_bool();
    }
    vb_select[Res_Zero] = !vb_select(Res_Total-1, Res_Zero+1).or_reduce();  // load memory, fence


    if ((wv[Instr_JAL] || wv[Instr_JALR]) && i_d_waddr.read() == Reg_ra) {
        v_call = 1;
    }
    if (wv[Instr_JALR] && vb_rdata2 == 0 
        && i_d_waddr.read() != Reg_ra && i_d_radr1.read() == Reg_ra) {
        v_ret = 1;
    }

    wb_select.res[Res_Zero] = 0;
    wb_select.res[Res_Reg2] = r.res_reg2;
    wb_select.res[Res_Csr] = r.res_csr;
    wb_select.res[Res_Npc] = r.res_npc;
    wb_select.res[Res_Ra] = r.res_ra;

    vb_csr_wdata = 0;
    if (r.hold_ivec.read()[Instr_CSRRC]) {
        vb_csr_wdata = i_csr_resp_data.read() & ~r.hold_rdata1.read();
    } else if (r.hold_ivec.read()[Instr_CSRRCI]) {
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_resp_data.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_resp_data.read()(4, 0) & ~r.hold_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (r.hold_ivec.read()[Instr_CSRRS]) {
        vb_csr_wdata = i_csr_resp_data.read() | r.hold_rdata1.read();
    } else if (r.hold_ivec.read()[Instr_CSRRSI]) {
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_resp_data.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_resp_data.read()(4, 0) | r.hold_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (r.hold_ivec.read()[Instr_CSRRW]) {
        vb_csr_wdata = r.hold_rdata1.read();
    } else if (r.hold_ivec.read()[Instr_CSRRWI]) {
        vb_csr_wdata(4, 0) = r.hold_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    }

    // Select result:
    if (r.select.read()[Res_Reg2]) {
        vb_res = wb_select.res[Res_Reg2];
    } else if (r.select.read()[Res_Npc]) {
        vb_res = wb_select.res[Res_Npc];
    } else if (r.select.read()[Res_Ra]) {
        vb_res = wb_select.res[Res_Ra];
    } else if (r.select.read()[Res_Csr]) {
        vb_res = wb_select.res[Res_Csr];
    } else if (r.select.read()[Res_Alu]) {
        vb_res = wb_select.res[Res_Alu];
    } else if (r.select.read()[Res_AddSub]) {
        vb_res = wb_select.res[Res_AddSub];
    } else if (r.select.read()[Res_Shifter]) {
        vb_res = wb_select.res[Res_Shifter];
    } else if (r.select.read()[Res_IMul]) {
        vb_res = wb_select.res[Res_IMul];
    } else if (r.select.read()[Res_IDiv]) {
        vb_res = wb_select.res[Res_IDiv];
    } else if (r.select.read()[Res_FPU]) {
        vb_res = wb_select.res[Res_FPU];
    } else {
        vb_res = 0;
    }

#else
    // ALU block selector:
    if (w_arith_valid[Multi_MUL]) {
        vb_res = wb_res.arr[Res_IMul];
    } else if (w_arith_valid[Multi_DIV]) {
        vb_res = wb_res.arr[Res_IDiv];
    } else if (w_arith_valid[Multi_FPU]) {
        vb_res = wb_res.arr[Res_FPU];
    } else if (i_memop_load) {
        vb_res = 0;
    } else if (i_memop_store) {
        vb_res = vb_rdata2;
    } else if (wv[Instr_JAL].to_bool()) {
        vb_res = vb_npc_incr;
        if (i_d_waddr.read() == Reg_ra) {
            v_call = 1;
        }
    } else if (wv[Instr_JALR].to_bool()) {
        vb_res = vb_npc_incr;
        if (i_d_waddr.read() == Reg_ra) {
            v_call = 1;
        } else if (vb_rdata2 == 0 && i_d_radr1.read() == Reg_ra) {
            v_ret = 1;
        }
    } else if (wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]) {
        vb_res = vb_sum64;
    } else if (wv[Instr_ADDW] || wv[Instr_ADDIW]) {
        vb_res = vb_sum32;
    } else if (wv[Instr_SUB]) {
        vb_res = vb_sub64;
    } else if (wv[Instr_SUBW]) {
        vb_res = vb_sub32;
    } else if (wv[Instr_SLL] || wv[Instr_SLLI]) {
        vb_res = wb_sll;
    } else if (wv[Instr_SLLW] || wv[Instr_SLLIW]) {
        vb_res = wb_sllw;
    } else if (wv[Instr_SRL] || wv[Instr_SRLI]) {
        vb_res = wb_srl;
    } else if (wv[Instr_SRLW] || wv[Instr_SRLIW]) {
        vb_res = wb_srlw;
    } else if (wv[Instr_SRA] || wv[Instr_SRAI]) {
        vb_res = wb_sra;
    } else if (wv[Instr_SRAW] || wv[Instr_SRAW] || wv[Instr_SRAIW]) {
        vb_res = wb_sraw;
    } else if (wv[Instr_AND] || wv[Instr_ANDI]) {
        vb_res = vb_and64;
    } else if (wv[Instr_OR] || wv[Instr_ORI]) {
        vb_res = vb_or64;
    } else if (wv[Instr_XOR] || wv[Instr_XORI]) {
        vb_res = vb_xor64;
    } else if (wv[Instr_SLT] || wv[Instr_SLTI]) {
        vb_res = vb_sub64[63];
    } else if (wv[Instr_SLTU] || wv[Instr_SLTIU]) {
        vb_res = v_ltu;
    } else if (wv[Instr_LUI]) {
        vb_res = vb_rdata2;
    } else if (wv[Instr_CSRRC]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata = i_csr_rdata.read() & ~vb_rdata1;
    } else if (wv[Instr_CSRRCI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) & ~i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRS]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata = i_csr_rdata.read() | vb_rdata1;
    } else if (wv[Instr_CSRRSI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) | i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRW]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata = vb_rdata1;
    } else if (wv[Instr_CSRRWI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_wdata(4, 0) = i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    }
#endif

#ifdef UPDT2
    v_latch_input = 0;
    v_o_valid = 0;
    v_reg_ena = 0;
    vb_reg_waddr = 0;
    v_memop_ena = 0;
    switch (r.state.read()) {
    case State_Idle:
        v_d_ready = 1;
        if (i_d_pc.read() == r.npc.read() && i_dbg_progbuf_ena.read() == 0
            && i_d_progbuf_ena.read() == 0) {
            v_latch_input = 1;
            if (w_test_hazard1 == 0 && w_test_hazard2 == 0) {
                if (v_csr_cmdpc_ena) {
                    v.state = State_Csr;
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = CsrReq_PcCmd;
                    v.csr_req_addr = vb_csr_cmdpc_addr;
                    v.csr_req_data = 0;
                    v.csr_req_pc = 1;
                } else if (vb_select[Res_IMul] || vb_select[Res_IDiv] || vb_select[Res_FPU]) {
                    v.state = State_WaitMulti;
                } else if (i_memop_load.read() || i_memop_store.read()) {
                    v_memop_ena = 1;
                    vb_reg_waddr = i_d_waddr;       // write back register address
                    if (!i_memop_ready.read()) {
                        // Wait cycles until FIFO to memoryaccess becomes available
                        v.state = State_WaitMemAcces;
                    } else {
                        v_o_valid = 1;
                    }
                } else if (v_fence_i || v_fence_d) {
                    if (!i_memop_ready.read()) {
                        v.state = State_WaitFlushingAccept;
                    } else if (v_fence_i) {
                        v.state = State_Flushing_I;     // Flushing D no need to wait ending
                    } else {
                        v_o_valid = 1;
                    }
                } else if (vb_select[Res_Csr] == 1) {
                    v.state = State_Csr;
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = CsrReq_ReadCmd;
                    v.csr_req_addr = i_d_csr_addr;
                    v.csr_req_data = 0;
                    v.csr_req_rmw = 1;  // read/modify/write
                } else if (wv[Instr_MRET] || wv[Instr_URET]) {
                    v.state = State_Csr;
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = (CsrReq_ReadCmd | CsrReq_ChaneModeCmd);
                    if (wv[Instr_MRET]) {
                        v.csr_req_addr = CSR_mepc;
                    } else {
                        v.csr_req_addr = CSR_uepc;
                    }
                    v.csr_req_data = 0;
                    v.csr_req_pc = 1;
                } else {
                    v_o_valid = 1;
                    v_reg_ena = i_d_waddr.read().or_reduce() && !i_memop_load.read(); // should be written by memaccess, but tag must be updated
                    vb_reg_waddr = i_d_waddr;
                }
            } else {
                v_latch_input = 0;
                v_d_ready = 0;
            }
        }
        break;
    case State_WaitMemAcces:
        // Fifo exec => memacess is full
        if (i_memop_ready.read()) {
            v.state = State_Idle;
            v_o_valid = 1;
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
                if (r.csr_req_pc.read()) {
                    v.npc = i_csr_resp_data;
                    v.state = State_Idle;
                    v_o_valid = 1;
                } else if (r.csr_req_rmw.read()) {
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = CsrReq_WriteCmd;
                    v.csr_req_data = vb_csr_wdata;
                    v.csr_req_rmw = 0;

                    // Store result int cpu register on next clock
                    v.res_csr = i_csr_resp_data.read();
                    v_reg_ena = r.hold_waddr.read().or_reduce();
                    vb_reg_waddr = r.hold_waddr;
                } else {
                    v.state = State_Idle;
                    v_o_valid = 1;
                }
                // TODO: check access right exception
            }
            break;
        default:;
        }
        break;
    case State_WaitMulti:
        // Wait end of multiclock instructions
        if (wb_select.valid[Res_IMul]
          | wb_select.valid[Res_IDiv]
          | wb_select.valid[Res_FPU]) {
            v.state = State_Idle;
            v_reg_ena = r.hold_waddr.read().or_reduce();
            vb_reg_waddr = r.hold_waddr;
            v_o_valid = 1;
        }
        break;
    case State_WaitFlushingAccept:
        // Fifo exec => memacess is full
        if (i_memop_ready.read()) {
            if (r.hold_ivec.read()[Instr_FENCE] == 1) {
                // no need to wait ending of D-flashing
                v.state = State_Idle;
                v_o_valid = 1;
            } else {
                v.state = State_Flushing_I;
            }
        }
        break;
    case State_Flushing_I:
        // Flushing DataCache could take much more time than flushing I
        // so that we should wait D-cache finish before requesting new
        // instruction to avoid reading obsolete data.
        if (i_flushd_end.read() == 1) {
            v.state = State_Idle;
            v_o_valid = 1;
        }
        break;
    default:;
    }

    // Next tags:
    int t_waddr = vb_reg_waddr.to_int();
    int t_tagcnt_wr = r.tagcnt_wr.read()(CFG_REG_TAG_WITH*t_waddr + (CFG_REG_TAG_WITH - 1),
                                         CFG_REG_TAG_WITH*t_waddr).to_int();

    vb_tagcnt_wr(CFG_REG_TAG_WITH*t_waddr+(CFG_REG_TAG_WITH-1), CFG_REG_TAG_WITH*t_waddr) = t_tagcnt_wr + 1;
    vb_tagcnt_wr(CFG_REG_TAG_WITH - 1, 0) = 0;
    vb_tagcnt_rd(CFG_REG_TAG_WITH*t_waddr+(CFG_REG_TAG_WITH-1), CFG_REG_TAG_WITH*t_waddr) = t_tagcnt_wr;
    vb_tagcnt_rd(CFG_REG_TAG_WITH - 1, 0) = 0;

    // Latch decoder's data into internal registers:
    if (v_latch_input) {
        if (i_dbg_progbuf_ena.read() == 0) {
            v.pc = i_d_pc;
            v.npc = vb_npc;
        } else {
            v.progbuf_npc = vb_npc_incr;
        }
        v.hold_radr1 = i_d_radr1;
        v.hold_radr2 = i_d_radr2;
        v.hold_waddr = i_d_waddr;
        v.hold_rdata1 = vb_rdata1;
        v.hold_ivec = i_ivec;
        v.instr = i_d_instr;
        v.flushd = v_fence_d;
        v.flushi = v_fence_i;
        v.call = v_call;
        v.ret = v_ret;
        v.res_reg2 = vb_rdata2;
        v.res_npc = vb_npc;
        v.res_ra = vb_npc_incr;

        wb_select.ena[Res_IMul] = vb_select[Res_IMul];
        wb_select.ena[Res_IDiv] = vb_select[Res_IDiv];
        wb_select.ena[Res_FPU] = vb_select[Res_FPU];
        v.select = vb_select;
    }
    if (v_reg_ena) {
        v.reg_write = 1;
        v.tagcnt_wr = vb_tagcnt_wr;
        v.tagcnt_rd = vb_tagcnt_rd;
        v.reg_waddr = vb_reg_waddr;
        v.reg_wtag = t_tagcnt_wr;
    }
    if (v_memop_ena) {
        v.memop_valid = 1;
        v.memop_type = i_memop_store;
        v.memop_sign_ext = i_memop_sign_ext;
        v.memop_size = i_memop_size;
        v.memop_memaddr = vb_memop_memaddr;
        v.memop_wdata = vb_rdata2;
        if (i_memop_load.read() == 1) {
            v.tagcnt_wr = vb_tagcnt_wr;
            v.tagcnt_rd = vb_tagcnt_rd;
            v.reg_waddr = vb_reg_waddr;
            v.reg_wtag = t_tagcnt_wr;
        }
    } else if (i_memop_ready.read()) {
        v.memop_valid = 0;
    }

    v.valid = v_o_valid;
#else

    // Latch ready result
    v_wena = 0;
    v_whazard = 0;
    vb_waddr = i_d_waddr.read();
    vb_o_wdata = vb_res;
    if (w_next_ready == 1) {
        v.valid = 1;

        if (i_dbg_progbuf_ena.read() == 0) {
            v.pc = i_d_pc;
            v.npc = vb_npc;
        } else {
            v.progbuf_npc = vb_npc_incr;
        }
        v.instr = i_d_instr;
        v.memop_valid = i_memop_load | i_memop_store;
        v.memop_load = i_memop_load;
        v.memop_sign_ext = i_memop_sign_ext;
        v.memop_store = i_memop_store;
        v.memop_size = i_memop_size;
        v.memop_addr = vb_memop_addr;
        v.memop_wdata = vb_res;

        v.memop_waddr = i_d_waddr.read();
        v.memop_wtag = i_wtag.read() + 1;
        v_whazard = i_memop_load;
        v_wena = i_d_waddr.read().or_reduce() && !v_multi_ena;

        v.wval = vb_res;
        v.call = v_call;
        v.ret = v_ret;
        v.flushd = v_fenced;
        v.hold_fencei = v_fencei;
    }

    if (i_dbg_progbuf_ena.read() == 0) {
        v.progbuf_npc = 0;
    }

    if (i_flushd_end.read() == 1) {
        v.hold_fencei = 0;
    }
    
    if (w_multi_ready == 1) {
        v_wena = r.memop_waddr.read().or_reduce();
        vb_waddr = r.memop_waddr;
    }

    v_o_valid = (r.valid && !w_multi_busy) || w_multi_ready;
#endif

    if (i_dport_npc_write.read()) {
        v.npc = i_dport_npc.read();
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    wb_rdata1 = vb_rdata1;
    wb_rdata2 = vb_rdata2;

#ifdef UPDT2
    sc_uint<3> t_alu_mode;
    t_alu_mode[2] = wv[Instr_XOR] || wv[Instr_XORI];
    t_alu_mode[1] = wv[Instr_OR] || wv[Instr_ORI];
    t_alu_mode[0] = wv[Instr_AND] || wv[Instr_ANDI];
    wb_alu_mode = t_alu_mode;

    sc_uint<5> t_addsub_mode;
    t_addsub_mode[4] = wv[Instr_SLT] || wv[Instr_SLTI];
    t_addsub_mode[3] = wv[Instr_SLTU] || wv[Instr_SLTIU];
    t_addsub_mode[2] = wv[Instr_SUB] || wv[Instr_SUBW];
    t_addsub_mode[1] = wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_ADDW] || wv[Instr_ADDIW] || wv[Instr_AUIPC];
    t_addsub_mode[0] = i_rv32.read();
    wb_addsub_mode = t_addsub_mode;

    sc_uint<4> t_shifter_mode;
    t_shifter_mode[3] = wv[Instr_SRA] || wv[Instr_SRAI] || wv[Instr_SRAW] || wv[Instr_SRAW] || wv[Instr_SRAIW];
    t_shifter_mode[2] = wv[Instr_SRL] || wv[Instr_SRLI] || wv[Instr_SRLW] || wv[Instr_SRLIW];
    t_shifter_mode[1] = wv[Instr_SLL] || wv[Instr_SLLI] || wv[Instr_SLLW] || wv[Instr_SLLIW];
    t_shifter_mode[0] = i_rv32.read();
    wb_shifter_mode = t_shifter_mode;

    wb_shifter_a1 = vb_rdata1;
    wb_shifter_a2 = vb_rdata2(5, 0);


    o_trap_ready = r.valid;

    o_ex_instr_load_fault = 0;//i_instr_load_fault.read() & r.valid;
    o_ex_instr_not_executable = 0;//!i_instr_executable.read() & r.valid;
    o_ex_illegal_instr = 0;//i_unsup_exception.read() & r.valid;
    o_ex_unalign_store = 0;//w_exception_store & r.valid;
    o_ex_unalign_load = 0;//w_exception_load & r.valid;
    o_ex_breakpoint = 0;//wv[Instr_EBREAK].to_bool() & r.valid;
    o_ex_ecall = 0;//wv[Instr_ECALL].to_bool() & r.valid;

    o_reg_wena = r.reg_write;
    o_reg_waddr = r.reg_waddr;
    o_reg_wtag = r.reg_wtag;
    o_reg_wdata = vb_res;
    o_d_ready = v_d_ready;

    o_csr_req_valid = v_csr_req_valid;
    o_csr_req_type = r.csr_req_type;
    o_csr_req_addr = r.csr_req_addr;
    o_csr_req_data = r.csr_req_data;
    o_csr_resp_ready = v_csr_resp_ready;
    o_ex_npc = vb_prog_npc;

    // Debug rtl only:!!
    for (int i = 0; i < Reg_Total; i++) {
        tag_expected[i] = r.tagcnt_rd.read()(CFG_REG_TAG_WITH*i+(CFG_REG_TAG_WITH-1), CFG_REG_TAG_WITH*i).to_int();
    }

#else
    w_multi_ena = v_multi_ena;

    o_trap_ready = w_next_ready;

    o_ex_instr_load_fault = i_instr_load_fault.read() & w_next_ready;
    o_ex_instr_not_executable = !i_instr_executable.read() & w_next_ready;
    o_ex_illegal_instr = i_unsup_exception.read() & w_next_ready;
    o_ex_unalign_store = w_exception_store & w_next_ready;
    o_ex_unalign_load = w_exception_load & w_next_ready;
    o_ex_breakpoint = wv[Instr_EBREAK].to_bool() & w_next_ready;
    o_ex_ecall = wv[Instr_ECALL].to_bool() & w_next_ready;

    o_wena = v_wena;
    o_whazard = v_whazard;
    o_waddr = vb_waddr;
    o_rtag = r.rtag;
    o_wdata = vb_o_wdata;
    o_wtag = i_wtag;
    o_d_ready = !v_hold_exec;

    o_csr_wena = v_csr_wena && w_next_ready;
    o_csr_wdata = vb_csr_wdata;
    o_ex_npc = vb_prog_npc;
#endif

    o_memop_valid = r.memop_valid;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_type = r.memop_type;
    o_memop_size = r.memop_size;
    o_memop_memaddr = r.memop_memaddr;
    o_memop_wdata = r.memop_wdata;
    
#ifdef UPDT2
    o_valid = r.valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
    o_flushd = r.flushd;    // must be post in a memory queue to avoid to early flushing
    o_flushi = r.flushi;
    o_call = r.call;
    o_ret = r.ret;
#else
    o_valid = v_o_valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
    o_flushd = r.flushd;    // must be post in a memory queue to avoid to early flushing
    o_flushi = v_fencei || v_fence;
    o_call = r.call;
    o_ret = r.ret;
    o_mret = v_mret;
    o_uret = v_uret;
    o_fpu_valid = wb_select.valid[Res_FPU];
    // Tracer only:
    o_multi_ready = w_multi_ready;
#endif
}

void InstrExecute::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

