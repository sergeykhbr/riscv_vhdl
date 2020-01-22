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
    i_d_imm("i_d_imm"),
    i_d_pc("i_d_pc"),
    i_d_instr("i_d_instr"),
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
    i_rhazard1("i_rhazard1"),
    i_rdata2("i_rdata2"),
    i_rhazard2("i_rhazard2"),
    i_wtag("i_wtag"),
    o_wena("o_wena"),
    o_waddr("o_waddr"),
    o_whazard("o_whazard"),
    o_wdata("o_wdata"),
    o_wtag("o_wtag"),
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
    o_memop_wdata("o_memop_wdata"),
    o_memop_waddr("o_memop_waddr"),
    o_memop_wtag("o_memop_wtag"),
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
    o_mret("o_mret"),
    o_uret("o_uret"),
    o_multi_ready("o_multi_ready") {
    async_reset_ = async_reset;
    fpu_ena_ = fpu_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_d_valid;
    sensitive << i_d_radr1;
    sensitive << i_d_radr2;
    sensitive << i_d_waddr;
    sensitive << i_d_imm;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
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
    sensitive << i_rhazard1;
    sensitive << i_rdata2;
    sensitive << i_rhazard2;
    sensitive << i_wtag;
    sensitive << i_csr_rdata;
    sensitive << i_trap_valid;
    sensitive << i_trap_pc;
    sensitive << i_flushd_end;
    sensitive << r.pc;
    sensitive << r.npc;
    sensitive << r.instr;
    sensitive << r.memop_waddr;
    sensitive << r.memop_wtag;
    sensitive << r.wval;
    sensitive << r.memop_load;
    sensitive << r.memop_store;
    sensitive << r.memop_addr;
    sensitive << r.valid;
    sensitive << r.call;
    sensitive << r.ret;
    sensitive << r.hold_fencei;
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

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    mul0 = new IntMul("mul0", async_reset);
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(w_arith_ena[Multi_MUL]);
    mul0->i_unsigned(i_unsigned_op);
    mul0->i_hsu(w_mul_hsu);
    mul0->i_high(w_arith_residual_high);
    mul0->i_rv32(i_rv32);
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

    if (fpu_ena_) {
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
    if (fpu_ena_) {
        delete fpu0;
    }
}

void InstrExecute::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_d_valid, i_d_valid.name());
        sc_trace(o_vcd, i_d_pc, i_d_pc.name());
        sc_trace(o_vcd, i_d_instr, i_d_instr.name());
        sc_trace(o_vcd, i_d_waddr, i_d_waddr.name());
        sc_trace(o_vcd, i_wb_waddr, i_wb_waddr.name());
        sc_trace(o_vcd, i_f64, i_f64.name());
        sc_trace(o_vcd, i_unsup_exception, i_unsup_exception.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_executable, i_instr_executable.name());
        sc_trace(o_vcd, i_rdata1, i_rdata1.name());
        sc_trace(o_vcd, i_rhazard1, i_rhazard1.name());
        sc_trace(o_vcd, i_rdata2, i_rdata2.name());
        sc_trace(o_vcd, i_rhazard2, i_rhazard2.name());
        sc_trace(o_vcd, o_wena, o_wena.name());
        sc_trace(o_vcd, o_whazard, o_whazard.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_wdata, o_wdata.name());
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
        sc_trace(o_vcd, o_memop_wdata, o_memop_wdata.name());
        sc_trace(o_vcd, o_memop_waddr, o_memop_waddr.name());
        sc_trace(o_vcd, o_memop_wtag, o_memop_wtag.name());

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
        sc_trace(o_vcd, r.hold_fencei, pn + ".r_hold_fencei");
        sc_trace(o_vcd, r.memop_waddr, pn + ".r_memop_waddr");
        sc_trace(o_vcd, r.memop_wtag, pn + ".r_memop_wtag");
        sc_trace(o_vcd, w_next_ready, pn + ".w_next_ready");
        sc_trace(o_vcd, w_multi_ena, pn + ".w_multi_ena");
        sc_trace(o_vcd, w_multi_busy, pn + ".w_multi_busy");
        sc_trace(o_vcd, w_multi_ready, pn + ".w_multi_ready");
        sc_trace(o_vcd, w_hold_memop, pn + ".w_hold_memop");
        sc_trace(o_vcd, w_hold_multi, pn + ".w_hold_multi");
        sc_trace(o_vcd, w_hold_hazard, pn + ".w_hold_hazard");
    }
    mul0->generateVCD(i_vcd, o_vcd);
    div0->generateVCD(i_vcd, o_vcd);
    if (fpu_ena_) {
        fpu0->generateVCD(i_vcd, o_vcd);
    }
}

void InstrExecute::comb() {
    bool v_fence;
    bool v_fencei;
    bool v_mret;
    bool v_uret;
    bool v_csr_wena;
    sc_uint<12> vb_csr_addr;
    sc_uint<RISCV_ARCH> vb_csr_wdata;
    sc_uint<RISCV_ARCH> vb_res;
    sc_uint<BUS_ADDR_WIDTH> vb_prog_npc;
    sc_uint<BUS_ADDR_WIDTH> vb_npc_incr;
    sc_uint<BUS_ADDR_WIDTH> vb_npc;
    sc_uint<RISCV_ARCH> vb_off;
    sc_uint<RISCV_ARCH> vb_sum64;
    sc_uint<RISCV_ARCH> vb_sum32;
    sc_uint<RISCV_ARCH> vb_sub64;
    sc_uint<RISCV_ARCH> vb_sub32;
    sc_uint<RISCV_ARCH> vb_and64;
    sc_uint<RISCV_ARCH> vb_or64;
    sc_uint<RISCV_ARCH> vb_xor64;
    sc_uint<BUS_ADDR_WIDTH> vb_memop_addr;
    sc_bv<Instr_Total> wv;
    int opcode_len;
    bool v_call;
    bool v_ret;
    bool v_pc_branch;
    bool v_less;
    bool v_gr_equal;
    sc_uint<RISCV_ARCH> vb_i_rdata1;
    sc_uint<RISCV_ARCH> vb_i_rdata2;
    sc_uint<RISCV_ARCH> vb_rdata1;
    sc_uint<RISCV_ARCH> vb_rdata2;
    sc_uint<RISCV_ARCH> vb_rfdata1;
    sc_uint<RISCV_ARCH> vb_rfdata2;
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

    v = r;

    v_csr_wena = 0;
    vb_csr_addr = 0;
    vb_csr_wdata = 0;
    vb_res = 0;
    vb_off = 0;
    vb_memop_addr = 0;
    wv = i_ivec.read();
    v_call = 0;
    v_ret = 0;
    v.valid = 0;
    v.call = 0;
    v.ret = 0;
    vb_rdata1 = 0;
    vb_rdata2 = 0;

    vb_i_rdata1 = i_rdata1;
    vb_i_rdata2 = i_rdata2;

    if (i_isa_type.read()[ISA_R_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
    } else if (i_isa_type.read()[ISA_I_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = i_d_imm;
    } else if (i_isa_type.read()[ISA_SB_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        vb_off = i_d_imm;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        vb_rdata1 = i_d_pc;
        vb_off = i_d_imm;
    } else if (i_isa_type.read()[ISA_U_type]) {
        vb_rdata1 = i_d_pc;
        vb_rdata2 = i_d_imm;
    } else if (i_isa_type.read()[ISA_S_type]) {
        vb_rdata1 = vb_i_rdata1;
        vb_rdata2 = vb_i_rdata2;
        vb_off = i_d_imm;
    }

    wb_fpu_vec = wv.range(Instr_FSUB_D, Instr_FADD_D);  // directly connected i_ivec
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

    w_next_ready = 0;
    if (i_d_valid.read() == 1 && i_d_pc.read() == r.npc.read()
        && v_hold_exec == 0) {
        w_next_ready = 1;
    }

    v_fence = wv[Instr_FENCE].to_bool() & w_next_ready;
    v_fencei = wv[Instr_FENCE_I].to_bool() & w_next_ready;
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

    w_arith_residual_high = (wv[Instr_REM] || wv[Instr_REMU]
                          || wv[Instr_REMW] || wv[Instr_REMUW]
                          || wv[Instr_MULH] || wv[Instr_MULHSU] || wv[Instr_MULHU]);

    w_mul_hsu = wv[Instr_MULHSU].to_bool();

    v_multi_ena = v_next_mul_ready || v_next_div_ready || v_next_fpu_ready;

    w_arith_ena[Multi_MUL] = v_next_mul_ready;
    w_arith_ena[Multi_DIV] = v_next_div_ready;
    w_arith_ena[Multi_FPU] = v_next_fpu_ready;

    if (i_memop_load) {
        vb_memop_addr =
            vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_rdata2(BUS_ADDR_WIDTH-1, 0);
    } else if (i_memop_store) {
        vb_memop_addr = 
            vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_off(BUS_ADDR_WIDTH-1, 0);
    }

    w_exception_store = 0;
    w_exception_load = 0;

    if ((wv[Instr_LD] && vb_memop_addr(2, 0) != 0)
        || ((wv[Instr_LW] || wv[Instr_LWU]) && vb_memop_addr(1, 0) != 0)
        || ((wv[Instr_LH] || wv[Instr_LHU]) && vb_memop_addr[0] != 0)) {
        w_exception_load = 1;
    }
    if ((wv[Instr_SD] && vb_memop_addr(2, 0) != 0)
        || (wv[Instr_SW] && vb_memop_addr(1, 0) != 0)
        || (wv[Instr_SH] && vb_memop_addr[0] != 0)) {
        w_exception_store = 1;
    }


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

    v_less = 0;
    v_gr_equal = 0;
    if (vb_rdata1 < vb_rdata2) {
        v_less = 1;
    }
    if (vb_rdata1 >= vb_rdata2) {
        v_gr_equal = 1;
    }

    // Relative Branch on some condition:
    v_pc_branch = 0;
    if ((wv[Instr_BEQ].to_bool() & (vb_sub64 == 0))
        || (wv[Instr_BGE].to_bool() & (vb_sub64[63] == 0))
        || (wv[Instr_BGEU].to_bool() & (v_gr_equal))
        || (wv[Instr_BLT].to_bool() & (vb_sub64[63] == 1))
        || (wv[Instr_BLTU].to_bool() & (v_less))
        || (wv[Instr_BNE].to_bool() & (vb_sub64 != 0))) {
        v_pc_branch = 1;
    }

    opcode_len = 4;
    if (i_compressed.read()) {
        opcode_len = 2;
    }
    vb_npc_incr = i_d_pc.read() + opcode_len;

    if (v_pc_branch) {
        vb_prog_npc = i_d_pc.read() + vb_off(BUS_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        vb_prog_npc = vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_off(BUS_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JALR].to_bool()) {
        vb_prog_npc = vb_rdata1(BUS_ADDR_WIDTH-1, 0) + vb_rdata2(BUS_ADDR_WIDTH-1, 0);
        vb_prog_npc[0] = 0;
    } else if (wv[Instr_MRET].to_bool()) {
        vb_prog_npc = i_csr_rdata;
    } else if (wv[Instr_URET].to_bool()) {
        vb_prog_npc = i_csr_rdata;
    } else {
        vb_prog_npc = vb_npc_incr;
    }

    if (i_trap_valid.read()) {
        vb_npc = i_trap_pc.read();
    } else {
        vb_npc = vb_prog_npc;
    }

    // ALU block selector:
    if (w_arith_valid[Multi_MUL]) {
        vb_res = wb_arith_res.arr[Multi_MUL];
    } else if (w_arith_valid[Multi_DIV]) {
        vb_res = wb_arith_res.arr[Multi_DIV];
    } else if (w_arith_valid[Multi_FPU]) {
        vb_res = wb_arith_res.arr[Multi_FPU];
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
        vb_res = v_less;
    } else if (wv[Instr_LUI]) {
        vb_res = vb_rdata2;
    } else if (wv[Instr_CSRRC]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata = i_csr_rdata.read() & ~vb_rdata1;
    } else if (wv[Instr_CSRRCI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) & ~i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRS]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata = i_csr_rdata.read() | vb_rdata1;
    } else if (wv[Instr_CSRRSI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        vb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) | i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRW]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata = vb_rdata1;
    } else if (wv[Instr_CSRRWI]) {
        vb_res = i_csr_rdata;
        v_csr_wena = 1;
        vb_csr_addr = vb_rdata2.range(11, 0);
        vb_csr_wdata(4, 0) = i_d_radr1.read()(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_MRET].to_bool()) {
        vb_res = vb_npc_incr;
        v_csr_wena = 0;
        vb_csr_addr = CSR_mepc;
    } else if (wv[Instr_URET].to_bool()) {
        vb_res = vb_npc_incr;
        v_csr_wena = 0;
        vb_csr_addr = CSR_uepc;
    }


    // Latch ready result
    v_wena = 0;
    v_whazard = 0;
    vb_waddr = i_d_waddr.read();
    vb_o_wdata = vb_res;
    if (w_next_ready == 1) {
        v.valid = 1;

        v.pc = i_d_pc;
        v.instr = i_d_instr;
        v.npc = vb_npc;
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
        v.flushd = v_fencei || v_fence;
        v.hold_fencei = v_fencei;
    }

    if (i_flushd_end.read() == 1) {
        v.hold_fencei = 0;
    }
    
    if (w_multi_ready == 1) {
        v_wena = r.memop_waddr.read().or_reduce();
        vb_waddr = r.memop_waddr;
    }

    v_o_valid = (r.valid && !w_multi_busy) || w_multi_ready;

    if (i_dport_npc_write.read()) {
        v.npc = i_dport_npc.read();
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    wb_rdata1 = vb_rdata1;
    wb_rdata2 = vb_rdata2;

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
    o_wdata = vb_o_wdata;
    o_wtag = i_wtag;
    o_d_ready = !v_hold_exec;

    o_csr_wena = v_csr_wena && w_next_ready;
    o_csr_addr = vb_csr_addr;
    o_csr_wdata = vb_csr_wdata;
    o_ex_npc = vb_prog_npc;

    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_size = r.memop_size;
    o_memop_addr = r.memop_addr;
    o_memop_wdata = r.memop_wdata;
    o_memop_waddr = r.memop_waddr;
    o_memop_wtag = r.memop_wtag;
    
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
    o_fpu_valid = w_arith_valid[Multi_FPU];
    // Tracer only:
    o_multi_ready = w_multi_ready;
}

void InstrExecute::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

