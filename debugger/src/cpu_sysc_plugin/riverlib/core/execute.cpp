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

InstrExecute::InstrExecute(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_d_valid("i_d_valid"),
    i_d_radr1("i_d_radr1"),
    i_d_radr2("i_d_radr2"),
    i_d_waddr("i_d_waddr"),
    i_d_imm("i_d_imm"),
    i_d_pc("i_d_pc"),
    i_d_instr("i_d_instr"),
    i_wb_valid("i_wb_valid"),
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
    i_rdata2("i_rdata2"),
    i_rfdata1("i_rfdata1"),
    i_rfdata2("i_rfdata2"),
    o_res_addr("o_res_addr"),
    o_res_data("o_res_data"),
    o_d_ready("o_d_ready"),
    o_csr_addr("o_csr_addr"),
    o_csr_wena("o_csr_wena"),
    i_csr_rdata("i_csr_rdata"),
    o_csr_wdata("o_csr_wdata"),
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
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_load("o_memop_load"),
    o_memop_store("o_memop_store"),
    o_memop_size("o_memop_size"),
    o_memop_addr("o_memop_addr"),
    i_memop_ready("i_memop_ready"),
    o_trap_ready("o_trap_ready"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_npc("o_npc"),
    o_instr("o_instr"),
    o_call("o_call"),
    o_ret("o_ret"),
    o_mret("o_mret"),
    o_uret("o_uret") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_d_valid;
    sensitive << i_d_radr1;
    sensitive << i_d_radr2;
    sensitive << i_d_waddr;
    sensitive << i_d_imm;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
    sensitive << i_wb_valid;
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
    sensitive << i_rdata2;
    sensitive << i_rfdata1;
    sensitive << i_rfdata2;
    sensitive << i_csr_rdata;
    sensitive << i_trap_valid;
    sensitive << i_trap_pc;
    sensitive << r.pc;
    sensitive << r.npc;
    sensitive << r.instr;
    sensitive << r.waddr;
    sensitive << r.wval;
    sensitive << r.memop_load;
    sensitive << r.memop_store;
    sensitive << r.memop_addr;
    sensitive << r.valid;
    sensitive << r.call;
    sensitive << r.ret;
    sensitive << wb_arith_res.arr[Multi_MUL];
    sensitive << wb_arith_res.arr[Multi_DIV];
    sensitive << wb_arith_res.arr[Multi_FPU];
    sensitive << w_arith_valid[Multi_MUL];
    sensitive << w_arith_valid[Multi_DIV];
    sensitive << w_arith_valid[Multi_FPU];
    sensitive << w_arith_busy[Multi_MUL];
    sensitive << w_arith_busy[Multi_DIV];
    sensitive << w_arith_busy[Multi_FPU];
    sensitive << wb_shifter_a1;
    sensitive << wb_shifter_a2;
    sensitive << wb_sll;
    sensitive << wb_sllw;
    sensitive << wb_srl;
    sensitive << wb_srlw;
    sensitive << wb_sra;
    sensitive << wb_sraw;
    for (int i = 0; i < SCOREBOARD_SIZE; i++) {
        sensitive << r_scoreboard[i].status;
        sensitive << r_scoreboard[i].forward;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    mul0 = new IntMul("mul0", async_reset);
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(w_arith_ena[Multi_MUL]);
    mul0->i_unsigned(i_unsigned_op);
    mul0->i_rv32(i_rv32);
    mul0->i_high(w_arith_residual_high);
    mul0->i_a1(wb_rdata1);
    mul0->i_a2(wb_rdata2);
    mul0->o_res(wb_arith_res.arr[Multi_MUL]);
    mul0->o_valid(w_arith_valid[Multi_MUL]);
    mul0->o_busy(w_arith_busy[Multi_MUL]);

    div0 = new IntDiv("div0", async_reset);
    div0->i_clk(i_clk);
    div0->i_nrst(i_nrst);
    div0->i_ena(w_arith_ena[Multi_DIV]);
    div0->i_unsigned(i_unsigned_op);
    div0->i_residual(w_arith_residual_high);
    div0->i_rv32(i_rv32);
    div0->i_a1(wb_rdata1);
    div0->i_a2(wb_rdata2);
    div0->o_res(wb_arith_res.arr[Multi_DIV]);
    div0->o_valid(w_arith_valid[Multi_DIV]);
    div0->o_busy(w_arith_busy[Multi_DIV]);

    sh0 = new Shifter("sh0");
    sh0->i_a1(wb_shifter_a1);
    sh0->i_a2(wb_shifter_a2);
    sh0->o_sll(wb_sll);
    sh0->o_sllw(wb_sllw);
    sh0->o_srl(wb_srl);
    sh0->o_sra(wb_sra);
    sh0->o_srlw(wb_srlw);
    sh0->o_sraw(wb_sraw);

    if (CFG_HW_FPU_ENABLE) {
        fpu0 = new FpuTop("fpu0", async_reset);
        fpu0->i_clk(i_clk);
        fpu0->i_nrst(i_nrst);
        fpu0->i_ena(w_arith_ena[Multi_FPU]);
        fpu0->i_ivec(wb_fpu_vec);
        fpu0->i_a(wb_rdata1);
        fpu0->i_b(wb_rdata2);
        fpu0->o_res(wb_arith_res.arr[Multi_FPU]);
        fpu0->o_ex_invalidop(o_ex_fpu_invalidop);
        fpu0->o_ex_divbyzero(o_ex_fpu_divbyzero);
        fpu0->o_ex_overflow(o_ex_fpu_overflow);
        fpu0->o_ex_underflow(o_ex_fpu_underflow);
        fpu0->o_ex_inexact(o_ex_fpu_inexact);
        fpu0->o_valid(w_arith_valid[Multi_FPU]);
        fpu0->o_busy(w_arith_busy[Multi_FPU]);
    } else {
        wb_arith_res.arr[Multi_FPU] = 0;
        w_arith_valid[Multi_FPU] = 0;
        w_arith_busy[Multi_FPU] = 0;
        o_fpu_valid = 0;
        o_ex_fpu_invalidop = 0;
        o_ex_fpu_divbyzero = 0;
        o_ex_fpu_overflow = 0;
        o_ex_fpu_underflow = 0;
        o_ex_fpu_inexact = 0;
    }
};

InstrExecute::~InstrExecute() {
    delete mul0;
    delete div0;
    delete sh0;
    if (CFG_HW_FPU_ENABLE) {
        delete fpu0;
    }
}

void InstrExecute::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_d_valid, i_d_valid.name());
        sc_trace(o_vcd, i_d_pc, i_d_pc.name());
        sc_trace(o_vcd, i_d_instr, i_d_instr.name());
        sc_trace(o_vcd, i_wb_valid, i_wb_valid.name());
        sc_trace(o_vcd, i_wb_waddr, i_wb_waddr.name());
        sc_trace(o_vcd, i_f64, i_f64.name());
        sc_trace(o_vcd, i_unsup_exception, i_unsup_exception.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_executable, i_instr_executable.name());
        sc_trace(o_vcd, i_rdata1, i_rdata1.name());
        sc_trace(o_vcd, i_rdata2, i_rdata2.name());
        sc_trace(o_vcd, i_rfdata1, i_rfdata1.name());
        sc_trace(o_vcd, i_rfdata2, i_rfdata2.name());
        sc_trace(o_vcd, o_res_addr, o_res_addr.name());
        sc_trace(o_vcd, o_res_data, o_res_data.name());
        sc_trace(o_vcd, o_d_ready, o_d_ready.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_csr_wena, o_csr_wena.name());
        sc_trace(o_vcd, i_csr_rdata, i_csr_rdata.name());
        sc_trace(o_vcd, o_csr_wdata, o_csr_wdata.name());
        sc_trace(o_vcd, i_trap_valid, i_trap_valid.name());
        sc_trace(o_vcd, i_trap_pc, i_trap_pc.name());
        sc_trace(o_vcd, o_ex_npc, o_ex_npc.name());
        sc_trace(o_vcd, o_memop_load, o_memop_load.name());
        sc_trace(o_vcd, o_memop_store, o_memop_store.name());
        sc_trace(o_vcd, o_memop_size, o_memop_size.name());
        sc_trace(o_vcd, o_memop_addr, o_memop_addr.name());
        sc_trace(o_vcd, i_memop_ready, i_memop_ready.name());
        sc_trace(o_vcd, o_trap_ready, o_trap_ready.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_npc, o_npc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_call, o_call.name());
        sc_trace(o_vcd, o_ret, o_ret.name());
        sc_trace(o_vcd, o_mret, o_mret.name());
        sc_trace(o_vcd, o_uret, o_uret.name());
        sc_trace(o_vcd, o_ex_instr_not_executable, o_ex_instr_not_executable.name());

        std::string pn(name());
        sc_trace(o_vcd, w_arith_ena[Multi_MUL], pn + ".w_arith_ena(0)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_MUL], pn + ".wb_arith_res(0)");
        sc_trace(o_vcd, w_arith_ena[Multi_DIV], pn + ".w_arith_ena(1)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_DIV], pn + ".wb_arith_res(1)");
        sc_trace(o_vcd, w_arith_ena[Multi_FPU], pn + ".w_arith_ena(2)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_FPU], pn + ".wb_arith_res(2)");
        sc_trace(o_vcd, r.waddr, pn + ".r_waddr");
        sc_trace(o_vcd, w_next_ready, pn + ".w_next_ready");
        sc_trace(o_vcd, w_multi_ena, pn + ".w_multi_ena");
        sc_trace(o_vcd, w_multi_busy, pn + ".w_multi_busy");
        sc_trace(o_vcd, w_multi_ready, pn + ".w_multi_ready");
        
        sc_trace(o_vcd, r_scoreboard[1].status, pn + ".scoreboard1_status");
        sc_trace(o_vcd, r_scoreboard[2].status, pn + ".scoreboard2_status");
        sc_trace(o_vcd, r_scoreboard[3].status, pn + ".scoreboard3_status");
        sc_trace(o_vcd, r_scoreboard[4].status, pn + ".scoreboard4_status");
        sc_trace(o_vcd, r_scoreboard[5].status, pn + ".scoreboard5_status");
        sc_trace(o_vcd, r_scoreboard[6].status, pn + ".scoreboard6_status");
        sc_trace(o_vcd, r_scoreboard[7].status, pn + ".scoreboard7_status");
        sc_trace(o_vcd, r_scoreboard[8].status, pn + ".scoreboard8_status");
        sc_trace(o_vcd, r_scoreboard[9].status, pn + ".scoreboard9_status");
        sc_trace(o_vcd, r_scoreboard[10].status, pn + ".scoreboard10_status");
        sc_trace(o_vcd, r_scoreboard[11].status, pn + ".scoreboard11_status");
        sc_trace(o_vcd, r_scoreboard[12].status, pn + ".scoreboard12_status");
        sc_trace(o_vcd, r_scoreboard[13].status, pn + ".scoreboard13_status");
        sc_trace(o_vcd, r_scoreboard[14].status, pn + ".scoreboard14_status");
        sc_trace(o_vcd, r_scoreboard[15].status, pn + ".scoreboard15_status");
        sc_trace(o_vcd, r_scoreboard[16].status, pn + ".scoreboard16_status");
        sc_trace(o_vcd, r_scoreboard[17].status, pn + ".scoreboard17_status");
        sc_trace(o_vcd, r_scoreboard[18].status, pn + ".scoreboard18_status");
        sc_trace(o_vcd, r_scoreboard[19].status, pn + ".scoreboard19_status");
        sc_trace(o_vcd, r_scoreboard[20].status, pn + ".scoreboard20_status");
        sc_trace(o_vcd, r_scoreboard[21].status, pn + ".scoreboard21_status");
        sc_trace(o_vcd, r_scoreboard[22].status, pn + ".scoreboard22_status");
        sc_trace(o_vcd, r_scoreboard[23].status, pn + ".scoreboard23_status");
        sc_trace(o_vcd, r_scoreboard[24].status, pn + ".scoreboard24_status");
        sc_trace(o_vcd, r_scoreboard[25].status, pn + ".scoreboard25_status");
        sc_trace(o_vcd, r_scoreboard[26].status, pn + ".scoreboard26_status");
        sc_trace(o_vcd, r_scoreboard[27].status, pn + ".scoreboard27_status");
        sc_trace(o_vcd, r_scoreboard[28].status, pn + ".scoreboard28_status");
        sc_trace(o_vcd, r_scoreboard[29].status, pn + ".scoreboard29_status");
        sc_trace(o_vcd, r_scoreboard[30].status, pn + ".scoreboard30_status");
        sc_trace(o_vcd, r_scoreboard[31].status, pn + ".scoreboard31_status");
        sc_trace(o_vcd, w_hold_memop, pn + ".w_hold_memop");
        sc_trace(o_vcd, w_hold_multi, pn + ".w_hold_multi");
        sc_trace(o_vcd, w_hold_hazard, pn + ".w_hold_hazard");
    }
    mul0->generateVCD(i_vcd, o_vcd);
    div0->generateVCD(i_vcd, o_vcd);
    if (CFG_HW_FPU_ENABLE) {
        fpu0->generateVCD(i_vcd, o_vcd);
    }
}

void InstrExecute::comb() {
    bool w_mret;
    bool w_uret;
    bool w_csr_wena;
    sc_uint<12> wb_csr_addr;
    sc_uint<RISCV_ARCH> wb_csr_wdata;
    sc_uint<RISCV_ARCH> wb_res;
    sc_uint<BUS_ADDR_WIDTH> wb_npc;
    sc_uint<BUS_ADDR_WIDTH> wb_ex_npc;
    sc_uint<RISCV_ARCH> wb_off;
    sc_uint<RISCV_ARCH> wb_sum64;
    sc_uint<RISCV_ARCH> wb_sum32;
    sc_uint<RISCV_ARCH> wb_sub64;
    sc_uint<RISCV_ARCH> wb_sub32;
    sc_uint<RISCV_ARCH> wb_and64;
    sc_uint<RISCV_ARCH> wb_or64;
    sc_uint<RISCV_ARCH> wb_xor64;
    bool w_memop_load;
    bool w_memop_store;
    bool w_memop_sign_ext;
    sc_uint<2> wb_memop_size;
    sc_uint<BUS_ADDR_WIDTH> wb_memop_addr;
    sc_bv<Instr_Total> wv;
    int opcode_len;
    bool v_call;
    bool v_ret;

    bool w_pc_branch;
    bool w_less;
    bool w_gr_equal;
    sc_uint<RISCV_ARCH> vb_i_rdata1;
    sc_uint<RISCV_ARCH> vb_i_rdata2;
    sc_uint<RISCV_ARCH> vb_rdata1;
    sc_uint<RISCV_ARCH> vb_rdata2;
    sc_uint<RISCV_ARCH> vb_rfdata1;
    sc_uint<RISCV_ARCH> vb_rfdata2;
    bool v_o_valid;
    sc_uint<RISCV_ARCH> vb_o_wdata;

    v = r;
    for (int i = 0; i < SCOREBOARD_SIZE; i++) {
        v_scoreboard[i].forward = r_scoreboard[i].forward;
        v_scoreboard[i].status = r_scoreboard[i].status;
    }


    w_mret = 0;
    w_uret = 0;
    w_csr_wena = 0;
    wb_csr_addr = 0;
    wb_csr_wdata = 0;
    wb_res = 0;
    wb_off = 0;
    w_memop_load = 0;
    w_memop_store = 0;
    w_memop_sign_ext = 0;
    wb_memop_size = 0;
    wb_memop_addr = 0;
    wv = i_ivec.read();
    v_call = 0;
    v_ret = 0;
    wb_ex_npc = 0;
    v.valid = 0;
    v.call = 0;
    v.ret = 0;


    //vb_rdata1 = i_rdata1.read();
    //vb_rdata2 = i_rdata2.read();
    //vb_rfdata1 = i_rfdata1.read();
    //vb_rfdata2 = i_rfdata2.read();
    if (i_d_radr1.read() != 0 &&
        r_scoreboard[i_d_radr1.read().to_int()].status.read() == RegForward) {
        vb_i_rdata1 = r_scoreboard[i_d_radr1.read().to_int()].forward;
    } else {
        vb_i_rdata1 = i_rdata1;
    }

    if (i_d_radr2.read() != 0 &&
        r_scoreboard[i_d_radr2.read().to_int()].status.read() == RegForward) {
        vb_i_rdata2 = r_scoreboard[i_d_radr2.read().to_int()].forward;
    } else {
        vb_i_rdata2 = i_rdata2;
    }

    if (i_isa_type.read()[ISA_R_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
    } else if (i_isa_type.read()[ISA_I_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = i_d_imm;
    } else if (i_isa_type.read()[ISA_SB_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        wb_off = i_d_imm;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        vb_rdata1 = i_d_pc;
        wb_off = i_d_imm;
    } else if (i_isa_type.read()[ISA_U_type]) {
        vb_rdata1 = i_d_pc;
        vb_rdata2 = i_d_imm;
    } else if (i_isa_type.read()[ISA_S_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        wb_off = i_d_imm;
    }

    /** Default number of cycles per instruction = 0 (1 clock per instr)
     *  If instruction is multicycle then modify this value.
     */
    w_arith_ena[Multi_FPU] = 0;
    if (CFG_HW_FPU_ENABLE) {
        if (i_f64.read() && !(wv[Instr_FSD] | wv[Instr_FLD]).to_bool()) {
            w_arith_ena[Multi_FPU] = 1;
        }
    }

    wb_fpu_vec = wv.range(Instr_FSUB_D, Instr_FADD_D);
    w_arith_ena[Multi_MUL] = (wv[Instr_MUL] || wv[Instr_MULW]) && w_next_ready;
    w_arith_ena[Multi_DIV] = (wv[Instr_DIV] || wv[Instr_DIVU]
                            || wv[Instr_DIVW] || wv[Instr_DIVUW]
                            || wv[Instr_REM] || wv[Instr_REMU]
                            || wv[Instr_REMW] || wv[Instr_REMUW]) && w_next_ready;
    w_arith_residual_high = (wv[Instr_REM] || wv[Instr_REMU]
                          || wv[Instr_REMW] || wv[Instr_REMUW]);


    w_multi_ena = w_arith_ena[Multi_MUL] | w_arith_ena[Multi_DIV]
                | w_arith_ena[Multi_FPU];

    w_multi_busy = w_arith_busy[Multi_MUL] | w_arith_busy[Multi_DIV]
                  | w_arith_busy[Multi_FPU];

    w_multi_ready = w_arith_valid[Multi_MUL] | w_arith_valid[Multi_DIV]
                  | w_arith_valid[Multi_FPU];


    /** Hold signals:
            1. hazard
            2. memaccess not ready to accept next memop operation
            3. multi instruction
     */
    w_hold_hazard = 0;
    if ((i_d_radr1.read() != 0 &&
        r_scoreboard[i_d_radr1.read().to_int()].status.read() == RegHazard) ||
        (i_d_radr2.read() != 0 &&
        r_scoreboard[i_d_radr2.read().to_int()].status.read() == RegHazard)) {
        w_hold_hazard = 1;
    }

    w_hold_memop = (i_memop_load.read() || i_memop_store.read())
                && !i_memop_ready.read();

    w_hold_multi = w_multi_busy | w_multi_ready;

    bool v_hold_exec = w_hold_hazard || w_hold_memop || w_hold_multi;

    w_next_ready = 0;
    if (i_d_valid.read() == 1 && i_d_pc.read() == r.npc.read()
        && v_hold_exec == 0) {
        w_next_ready = 1;
    }


    if (i_memop_load) {
        wb_memop_addr =
            vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_rdata2(BUS_ADDR_WIDTH-1, 0);
    } else if (i_memop_store) {
        wb_memop_addr = 
            vb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_off(BUS_ADDR_WIDTH-1, 0);
    }

    w_exception_store = 0;
    w_exception_load = 0;

    if ((wv[Instr_LD] && wb_memop_addr(2, 0) != 0)
        || ((wv[Instr_LW] || wv[Instr_LWU]) && wb_memop_addr(1, 0) != 0)
        || ((wv[Instr_LH] || wv[Instr_LHU]) && wb_memop_addr[0] != 0)) {
        w_exception_load = 1;
    }
    if ((wv[Instr_SD] && wb_memop_addr(2, 0) != 0)
        || (wv[Instr_SW] && wb_memop_addr(1, 0) != 0)
        || (wv[Instr_SH] && wb_memop_addr[0] != 0)) {
        w_exception_store = 1;
    }


    // parallel ALU:
    wb_sum64 = vb_rdata1 + vb_rdata2;
    wb_sum32(31, 0) = vb_rdata1(31, 0) + vb_rdata2(31, 0);
    if (wb_sum32[31]) {
        wb_sum32(63, 32) = ~0;
    }
    wb_sub64 = vb_rdata1 - vb_rdata2;
    wb_sub32(31, 0) = vb_rdata1(31, 0) - vb_rdata2(31, 0);
    if (wb_sub32[31]) {
        wb_sub32(63, 32) = ~0;
    }
    wb_and64 = vb_rdata1 & vb_rdata2;
    wb_or64 = vb_rdata1 | vb_rdata2;
    wb_xor64 = vb_rdata1 ^ vb_rdata2;

    wb_shifter_a1 = vb_rdata1;
    wb_shifter_a2 = vb_rdata2(5, 0);

    w_less = 0;
    w_gr_equal = 0;
    if (vb_rdata1 < vb_rdata2) {
        w_less = 1;
    }
    if (vb_rdata1 >= vb_rdata2) {
        w_gr_equal = 1;
    }

    // Relative Branch on some condition:
    w_pc_branch = 0;
    if ((wv[Instr_BEQ].to_bool() & (wb_sub64 == 0))
        || (wv[Instr_BGE].to_bool() & (wb_sub64[63] == 0))
        || (wv[Instr_BGEU].to_bool() & (w_gr_equal))
        || (wv[Instr_BLT].to_bool() & (wb_sub64[63] == 1))
        || (wv[Instr_BLTU].to_bool() & (w_less))
        || (wv[Instr_BNE].to_bool() & (wb_sub64 != 0))) {
        w_pc_branch = 1;
    }

    opcode_len = 4;
    if (i_compressed.read()) {
        opcode_len = 2;
    }

    if (w_pc_branch) {
        wb_npc = i_d_pc.read() + wb_off(BUS_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        wb_npc = vb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_off(BUS_ADDR_WIDTH-1, 0);
        if (i_d_waddr.read() == Reg_ra) {
            v_call = 1;
        }
    } else if (wv[Instr_JALR].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        wb_npc = vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_rdata2(BUS_ADDR_WIDTH-1, 0);
        wb_npc[0] = 0;
        if (i_d_waddr.read() == Reg_ra) {
            v_call = 1;
        } else if (vb_rdata2 == 0 && i_d_radr1.read() == Reg_ra) {
            v_ret = 1;
        }
    } else if (wv[Instr_MRET].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        w_mret = 1;
        w_csr_wena = 0;
        wb_csr_addr = CSR_mepc;
        wb_npc = i_csr_rdata;
    } else if (wv[Instr_URET].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        w_uret = 1;
        w_csr_wena = 0;
        wb_csr_addr = CSR_uepc;
        wb_npc = i_csr_rdata;
    } else {
        // Instr_HRET, Instr_SRET, Instr_FENCE, Instr_FENCE_I:
        wb_npc = i_d_pc.read() + opcode_len;
    }

    if (w_arith_valid[Multi_MUL]) {
        wb_res = wb_arith_res.arr[Multi_MUL];
    } else if (w_arith_valid[Multi_DIV]) {
        wb_res = wb_arith_res.arr[Multi_DIV];
    } else if (w_arith_valid[Multi_FPU]) {
        wb_res = wb_arith_res.arr[Multi_FPU];
    } else if (i_memop_load) {
        w_memop_load = 1;
        w_memop_sign_ext = i_memop_sign_ext;
        wb_memop_size = i_memop_size;
    } else if (i_memop_store) {
        w_memop_store = 1;
        wb_memop_size = i_memop_size;
        wb_res = vb_rdata2;
    } else if (wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]) {
        wb_res = wb_sum64;
    } else if (wv[Instr_ADDW] || wv[Instr_ADDIW]) {
        wb_res = wb_sum32;
    } else if (wv[Instr_SUB]) {
        wb_res = wb_sub64;
    } else if (wv[Instr_SUBW]) {
        wb_res = wb_sub32;
    } else if (wv[Instr_SLL] || wv[Instr_SLLI]) {
        wb_res = wb_sll;
    } else if (wv[Instr_SLLW] || wv[Instr_SLLIW]) {
        wb_res = wb_sllw;
    } else if (wv[Instr_SRL] || wv[Instr_SRLI]) {
        wb_res = wb_srl;
    } else if (wv[Instr_SRLW] || wv[Instr_SRLIW]) {
        wb_res = wb_srlw;
    } else if (wv[Instr_SRA] || wv[Instr_SRAI]) {
        wb_res = wb_sra;
    } else if (wv[Instr_SRAW] || wv[Instr_SRAW] || wv[Instr_SRAIW]) {
        wb_res = wb_sraw;
    } else if (wv[Instr_AND] || wv[Instr_ANDI]) {
        wb_res = wb_and64;
    } else if (wv[Instr_OR] || wv[Instr_ORI]) {
        wb_res = wb_or64;
    } else if (wv[Instr_XOR] || wv[Instr_XORI]) {
        wb_res = wb_xor64;
    } else if (wv[Instr_SLT] || wv[Instr_SLTI]) {
        wb_res = wb_sub64[63];
    } else if (wv[Instr_SLTU] || wv[Instr_SLTIU]) {
        wb_res = w_less;
    } else if (wv[Instr_LUI]) {
        wb_res = vb_rdata2;
    } else if (wv[Instr_CSRRC]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() & ~vb_rdata1;
    } else if (wv[Instr_CSRRCI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        wb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) & ~i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRS]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() | vb_rdata1;
    } else if (wv[Instr_CSRRSI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        wb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) | i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRW]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata = vb_rdata1;
    } else if (wv[Instr_CSRRWI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = vb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = 0;
        wb_csr_wdata(4, 0) = i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    }



/*    if (w_multi_ena & w_next_ready) {
        v.multiclock_ena = 1;
        v.multi_res_addr = wb_res_addr;
        if (CFG_HW_FPU_ENABLE) {
            v.multi_ivec_fpu = wv.range(Instr_FSUB_D, Instr_FADD_D);
            if (w_fpu_ena == 1 && (wv[Instr_FMOV_X_D] | wv[Instr_FEQ_D]
                | wv[Instr_FLT_D] | wv[Instr_FLE_D]
                | wv[Instr_FCVT_LU_D] | wv[Instr_FCVT_L_D]
                | wv[Instr_FCVT_WU_D] | wv[Instr_FCVT_W_D]).to_bool() == 0) {
                v.multi_res_addr = 0x20 | wb_res_addr;
            }
        }
        v.multi_pc = i_d_pc;
        v.multi_instr = i_d_instr;
        if (i_trap_valid.read() == 1) {
            v.multi_npc = i_trap_pc.read();
        } else {
            v.multi_npc = wb_npc;
        }
    }
    */

    // ALU block selector:


    if (i_wb_valid.read() == 1) {
        v_scoreboard[i_wb_waddr.read().to_int()].status = RegValid;
    }


    // Latch ready result
    if (w_next_ready == 1) {
        v.valid = 1;

        v.pc = i_d_pc;
        v.instr = i_d_instr;
        if (i_trap_valid.read()) {
            v.npc = i_trap_pc.read();
            wb_ex_npc = wb_npc;
        } else {
            v.npc = wb_npc;
        }
        v.memop_load = i_memop_load;
        v.memop_sign_ext = i_memop_sign_ext;
        v.memop_store = i_memop_store;
        v.memop_size = i_memop_size;
        v.memop_addr = wb_memop_addr;

        v.waddr = i_d_waddr.read();
        v.wval = wb_res;
        v.call = v_call;
        v.ret = v_ret;

        if (i_d_waddr.read() != 0) {
            int tdx = i_d_waddr.read().to_int();
            v_scoreboard[tdx].forward = wb_res;
            if (w_memop_load == 1 || r_scoreboard[tdx].status.read() == RegHazard) {
                v_scoreboard[tdx].status = RegHazard;
            } else {
                v_scoreboard[tdx].status = RegHazard;
            }
        }
    }

    if (w_multi_ready == 1) {
        v_o_valid = 1;
        vb_o_wdata = wb_res;
        v_scoreboard[r.waddr.read().to_int()].forward = wb_res;
    } else if (w_multi_busy == 1) {
        v_o_valid = 0;
    } else {
        v_o_valid = r.valid;
        vb_o_wdata = r.wval;
    }

    if (i_dport_npc_write.read()) {
        v.npc = i_dport_npc.read();
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
        for (int i = 0; i < SCOREBOARD_SIZE; i++) {
            v_scoreboard[i].status = RegValid;
            v_scoreboard[i].forward = 0;
        }
    }

    wb_rdata1 = vb_rdata1;
    wb_rdata2 = vb_rdata2;

    o_trap_ready = w_next_ready;

    o_ex_instr_load_fault = i_instr_load_fault.read() & w_next_ready;
    o_ex_instr_not_executable = !i_instr_executable.read() & w_next_ready;
    o_ex_illegal_instr = i_unsup_exception.read() & w_next_ready;
    o_ex_unalign_store = w_exception_store & w_next_ready;
    o_ex_unalign_load = w_exception_load & w_next_ready;
    o_ex_breakpoint = wv[Instr_EBREAK].to_bool() & w_next_ready;
    o_ex_ecall = wv[Instr_ECALL].to_bool() & w_next_ready;

    o_res_addr = r.waddr;
    o_res_data = vb_o_wdata;
    o_d_ready = !v_hold_exec;

    o_csr_wena = w_csr_wena && w_next_ready;
    o_csr_addr = wb_csr_addr;
    o_csr_wdata = wb_csr_wdata;
    o_ex_npc = wb_ex_npc;

    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_size = r.memop_size;
    o_memop_addr = r.memop_addr;

    o_valid = v_o_valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
    o_call = r.call;
    o_ret = r.ret;
    o_mret = w_mret;
    o_uret = w_uret;
    o_fpu_valid = w_arith_valid[Multi_FPU];
}

void InstrExecute::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
        for (int i = 0; i < SCOREBOARD_SIZE; i++) {
            r_scoreboard[i].forward = v_scoreboard[i].forward;
            r_scoreboard[i].status = v_scoreboard[i].status;
        }
    }
}

}  // namespace debugger

