/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
    : sc_module(name_) {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_pipeline_hold;
    sensitive << i_d_valid;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
    sensitive << i_wb_done;
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
    sensitive << i_dport_npc_write;
    sensitive << i_dport_npc;
    sensitive << i_rdata1;
    sensitive << i_rdata2;
    sensitive << i_rfdata1;
    sensitive << i_rfdata2;
    sensitive << i_csr_rdata;
    sensitive << i_trap_valid;
    sensitive << i_trap_pc;
    sensitive << r.d_valid;
    sensitive << r.pc;
    sensitive << r.npc;
    sensitive << r.instr;
    sensitive << r.res_val;
    sensitive << r.memop_load;
    sensitive << r.memop_store;
    sensitive << r.memop_addr;
    sensitive << r.multi_res_addr;
    sensitive << r.multi_pc;
    sensitive << r.multi_npc;
    sensitive << r.multi_instr;
    sensitive << r.multi_ena[Multi_MUL];
    sensitive << r.multi_ena[Multi_DIV];
    sensitive << r.multi_ena[Multi_FPU];
    sensitive << r.multi_rv32;
    sensitive << r.multi_f64;
    sensitive << r.multi_unsigned;
    sensitive << r.multi_residual_high;
    sensitive << r.multiclock_ena;
    sensitive << r.multi_ivec_fpu;
    sensitive << r.multi_a1;
    sensitive << r.multi_a2;
    sensitive << r.hazard_addr0;
    sensitive << r.hazard_addr1;
    sensitive << r.hazard_depth;
    sensitive << r.call;
    sensitive << r.ret;
    sensitive << w_hazard_detected;
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
    sensitive << i_clk.pos();

    mul0 = new IntMul("mul0");
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(r.multi_ena[Multi_MUL]);
    mul0->i_unsigned(r.multi_unsigned);
    mul0->i_rv32(r.multi_rv32);
    mul0->i_high(r.multi_residual_high);
    mul0->i_a1(r.multi_a1);
    mul0->i_a2(r.multi_a2);
    mul0->o_res(wb_arith_res.arr[Multi_MUL]);
    mul0->o_valid(w_arith_valid[Multi_MUL]);
    mul0->o_busy(w_arith_busy[Multi_MUL]);

    div0 = new IntDiv("div0");
    div0->i_clk(i_clk);
    div0->i_nrst(i_nrst);
    div0->i_ena(r.multi_ena[Multi_DIV]);
    div0->i_unsigned(r.multi_unsigned);
    div0->i_residual(r.multi_residual_high);
    div0->i_rv32(r.multi_rv32);
    div0->i_a1(r.multi_a1);
    div0->i_a2(r.multi_a2);
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
        fpu0->i_ena(r.multi_ena[Multi_FPU]);
        fpu0->i_ivec(r.multi_ivec_fpu);
        fpu0->i_a(r.multi_a1);
        fpu0->i_b(r.multi_a2);
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
        sc_trace(o_vcd, i_pipeline_hold, "/top/proc0/exec0/i_pipeline_hold");
        sc_trace(o_vcd, i_d_valid, "/top/proc0/exec0/i_d_valid");
        sc_trace(o_vcd, i_d_pc, "/top/proc0/exec0/i_d_pc");
        sc_trace(o_vcd, i_d_instr, "/top/proc0/exec0/i_d_instr");
        sc_trace(o_vcd, i_wb_done, "/top/proc0/exec0/i_wb_done");
        sc_trace(o_vcd, i_rdata1, "/top/proc0/exec0/i_rdata1");
        sc_trace(o_vcd, i_rdata2, "/top/proc0/exec0/i_rdata2");
        sc_trace(o_vcd, i_rfdata1, "/top/proc0/exec0/i_rfdata1");
        sc_trace(o_vcd, i_rfdata2, "/top/proc0/exec0/i_rfdata2");
        sc_trace(o_vcd, i_f64, "/top/proc0/exec0/i_f64");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/o_valid");
        sc_trace(o_vcd, o_npc, "/top/proc0/exec0/o_npc");
        sc_trace(o_vcd, o_pc, "/top/proc0/exec0/o_pc");
        sc_trace(o_vcd, o_radr1, "/top/proc0/exec0/o_radr1");
        sc_trace(o_vcd, o_radr2, "/top/proc0/exec0/o_radr2");
        sc_trace(o_vcd, o_res_addr, "/top/proc0/exec0/o_res_addr");
        sc_trace(o_vcd, o_res_data, "/top/proc0/exec0/o_res_data");
        sc_trace(o_vcd, o_memop_addr, "/top/proc0/exec0/o_memop_addr");
        sc_trace(o_vcd, o_memop_load, "/top/proc0/exec0/o_memop_load");
        sc_trace(o_vcd, o_memop_store, "/top/proc0/exec0/o_memop_store");
        sc_trace(o_vcd, o_memop_size, "/top/proc0/exec0/o_memop_size");
        sc_trace(o_vcd, o_csr_addr, "/top/proc0/exec0/o_csr_addr");
        sc_trace(o_vcd, o_csr_wena, "/top/proc0/exec0/o_csr_wena");
        sc_trace(o_vcd, i_csr_rdata, "/top/proc0/exec0/i_csr_rdata");
        sc_trace(o_vcd, o_csr_wdata, "/top/proc0/exec0/o_csr_wdata");
        sc_trace(o_vcd, o_pipeline_hold, "/top/proc0/exec0/o_pipeline_hold");
        sc_trace(o_vcd, o_call, "/top/proc0/exec0/o_call");
        sc_trace(o_vcd, o_ret, "/top/proc0/exec0/o_ret");

        sc_trace(o_vcd, w_hazard_detected, "/top/proc0/exec0/w_hazard_detected");
        sc_trace(o_vcd, r.hazard_depth, "/top/proc0/exec0/r_hazard_depth");
        sc_trace(o_vcd, r.hazard_addr0, "/top/proc0/exec0/r_hazard_addr0");
        sc_trace(o_vcd, r.hazard_addr1, "/top/proc0/exec0/r_hazard_addr1");
        sc_trace(o_vcd, r.multiclock_ena, "/top/proc0/exec0/r_multiclock_ena");
        sc_trace(o_vcd, r.multi_ena[Multi_MUL], "/top/proc0/exec0/r_multi_ena(0)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_MUL], "/top/proc0/exec0/wb_arith_res(0)");
        sc_trace(o_vcd, r.multi_ena[Multi_DIV], "/top/proc0/exec0/r_multi_ena(1)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_DIV], "/top/proc0/exec0/wb_arith_res(1)");
        sc_trace(o_vcd, r.multi_ena[Multi_FPU], "/top/proc0/exec0/r_multi_ena(2)");
        sc_trace(o_vcd, wb_arith_res.arr[Multi_FPU], "/top/proc0/exec0/wb_arith_res(2)");
        sc_trace(o_vcd, r.multi_res_addr, "/top/proc0/exec0/r_multi_res_addr");
        sc_trace(o_vcd, r.multi_a1, "/top/proc0/exec0/multi_a1");
        sc_trace(o_vcd, r.multi_a2, "/top/proc0/exec0/multi_a2");
        sc_trace(o_vcd, r.multi_ivec_fpu, "/top/proc0/exec0/r_multi_ivec_fpu");
    }
    mul0->generateVCD(i_vcd, o_vcd);
    div0->generateVCD(i_vcd, o_vcd);
    if (CFG_HW_FPU_ENABLE) {
        fpu0->generateVCD(i_vcd, o_vcd);
    }
}

void InstrExecute::comb() {
    sc_uint<6> wb_radr1;        // [5] 0=Integer bank; 1=FPU bank
    sc_uint<RISCV_ARCH> wb_rdata1;
    sc_uint<6> wb_radr2;
    sc_uint<RISCV_ARCH> wb_rdata2;
    bool w_mret;
    bool w_uret;
    bool w_csr_wena;
    sc_uint<6> wb_res_addr;
    sc_uint<12> wb_csr_addr;
    sc_uint<RISCV_ARCH> wb_csr_wdata;
    sc_uint<RISCV_ARCH> wb_res;
    sc_uint<BUS_ADDR_WIDTH> wb_npc;
    sc_uint<BUS_ADDR_WIDTH> wb_ex_npc;
    sc_uint<RISCV_ARCH> wb_off;
    sc_uint<RISCV_ARCH> wb_mask_i31;    // Bits depending instr[31] bits
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

    bool w_pc_valid;
    bool w_d_acceptable;
    bool w_multi_valid;
    bool w_multi_ena;
    bool w_fpu_ena;
    bool w_res_wena;
    bool w_pc_branch;
    bool w_hazard_lvl1;
    bool w_hazard_lvl2;
    bool w_d_valid;
    bool w_o_valid;
    bool w_o_pipeline_hold;
    bool w_less;
    bool w_gr_equal;

    wb_radr1 = 0;
    wb_radr2 = 0;
    w_mret = 0;
    w_uret = 0;
    w_csr_wena = 0;
    wb_res_addr = 0;
    wb_csr_addr = 0;
    wb_csr_wdata = 0;
    wb_res = 0;
    wb_off = 0;
    wb_rdata1 = 0;
    wb_rdata2 = 0;
    w_memop_load = 0;
    w_memop_store = 0;
    w_memop_sign_ext = 0;
    wb_memop_size = 0;
    wb_memop_addr = 0;
    wv = i_ivec.read();

    v = r;

    wb_mask_i31 = 0;
    if (i_d_instr.read()[31]) {
        wb_mask_i31 = ~0ull;
    }

    w_pc_valid = 0;
    if (i_d_pc.read() == r.npc.read()) {
        w_pc_valid = 1;
    }
    w_d_acceptable = (!i_pipeline_hold) & i_d_valid 
                          & w_pc_valid & (!r.multiclock_ena);

    if (i_isa_type.read()[ISA_R_type]) {
        wb_radr1 = (0, i_d_instr.read().range(19, 15));
        wb_rdata1 = i_rdata1;
        wb_radr2 = (0, i_d_instr.read().range(24, 20));
        wb_rdata2 = i_rdata2;
        if (CFG_HW_FPU_ENABLE && i_f64.read() == 1) {
            if ((wv[Instr_FMOV_D_X] |
                wv[Instr_FCVT_D_L] | wv[Instr_FCVT_D_LU] |
                wv[Instr_FCVT_D_W] | wv[Instr_FCVT_D_WU]).to_bool() == 0) {
                wb_radr1 = (1, i_d_instr.read().range(19, 15));
                wb_rdata1 = i_rfdata1;
            }
            if (wv[Instr_FMOV_X_D].to_bool() == 0) {
                wb_radr2 = (1, i_d_instr.read().range(24, 20));
                wb_rdata2 = i_rfdata2;
            }
        }
    } else if (i_isa_type.read()[ISA_I_type]) {
        wb_radr1 = (0, i_d_instr.read().range(19, 15));
        wb_rdata1 = i_rdata1;
        wb_radr2 = 0;
        wb_rdata2 = (wb_mask_i31(63, 12), i_d_instr.read().range(31, 20));
    } else if (i_isa_type.read()[ISA_SB_type]) {
        wb_radr1 = (0, i_d_instr.read().range(19, 15));
        wb_rdata1 = i_rdata1;
        wb_radr2 = (0, i_d_instr.read().range(24, 20));
        wb_rdata2 = i_rdata2;
        wb_off(RISCV_ARCH-1, 12) = wb_mask_i31(RISCV_ARCH-1, 12);
        wb_off[12] = i_d_instr.read()[31];
        wb_off[11] = i_d_instr.read()[7];
        wb_off(10, 5) = i_d_instr.read()(30, 25);
        wb_off(4, 1) = i_d_instr.read()(11, 8);
        wb_off[0] = 0;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_off(RISCV_ARCH-1, 20) = wb_mask_i31(RISCV_ARCH-1, 20);
        wb_off(19, 12) = i_d_instr.read()(19, 12);
        wb_off[11] = i_d_instr.read()[20];
        wb_off(10, 1) = i_d_instr.read()(30, 21);
        wb_off[0] = 0;
    } else if (i_isa_type.read()[ISA_U_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_rdata2(31, 0) = i_d_instr.read().range(31, 12) << 12;
        wb_rdata2(RISCV_ARCH-1, 32) = wb_mask_i31(RISCV_ARCH-1, 32);
    } else if (i_isa_type.read()[ISA_S_type]) {
        wb_radr1 = (0, i_d_instr.read().range(19, 15));
        wb_rdata1 = i_rdata1;
        wb_radr2 = (0, i_d_instr.read().range(24, 20));
        wb_rdata2 = i_rdata2;
        wb_off(RISCV_ARCH-1, 12) = wb_mask_i31(RISCV_ARCH-1, 12);
        wb_off(11, 5) = i_d_instr.read()(31, 25);
        wb_off(4, 0) = i_d_instr.read()(11, 7);
        if (CFG_HW_FPU_ENABLE && wv[Instr_FSD].to_bool()) {
            wb_radr2 = (1, i_d_instr.read().range(24, 20));
            wb_rdata2 = i_rfdata2;
        }
    }

    // parallel ALU:
    wb_sum64 = wb_rdata1 + wb_rdata2;
    wb_sum32(31, 0) = wb_rdata1(31, 0) + wb_rdata2(31, 0);
    if (wb_sum32[31]) {
        wb_sum32(63, 32) = ~0;
    }
    wb_sub64 = wb_rdata1 - wb_rdata2;
    wb_sub32(31, 0) = wb_rdata1(31, 0) - wb_rdata2(31, 0);
    if (wb_sub32[31]) {
        wb_sub32(63, 32) = ~0;
    }
    wb_and64 = wb_rdata1 & wb_rdata2;
    wb_or64 = wb_rdata1 | wb_rdata2;
    wb_xor64 = wb_rdata1 ^ wb_rdata2;

    wb_shifter_a1 = wb_rdata1;
    wb_shifter_a2 = wb_rdata2(5, 0);

    w_multi_valid = w_arith_valid[Multi_MUL] | w_arith_valid[Multi_DIV]
                  | w_arith_valid[Multi_FPU];

    // Don't modify registers on conditional jumps:
    w_res_wena = !(wv[Instr_BEQ] | wv[Instr_BGE] | wv[Instr_BGEU]
               | wv[Instr_BLT] | wv[Instr_BLTU] | wv[Instr_BNE]
               | wv[Instr_SD] | wv[Instr_SW] | wv[Instr_SH] | wv[Instr_SB]
               | wv[Instr_FSD]
               | wv[Instr_MRET] | wv[Instr_URET]
               | wv[Instr_ECALL] | wv[Instr_EBREAK]).to_bool();

    if (w_multi_valid) {
        wb_res_addr = r.multi_res_addr;
        v.multiclock_ena = 0;
    } else if (w_res_wena) {
        wb_res_addr = (0, i_d_instr.read().range(11, 7));
        if (CFG_HW_FPU_ENABLE) {
            if (i_f64.read() == 1 && wv[Instr_FLD] == 1) {
                wb_res_addr |= 0x20;
            }
        }
    } else {
        wb_res_addr = 0;
    }
    w_less = 0;
    w_gr_equal = 0;
    if (wb_rdata1 < wb_rdata2) {
        w_less = 1;
    }
    if (wb_rdata1 >= wb_rdata2) {
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
        wb_npc = wb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_off(BUS_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JALR].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        wb_npc = wb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_rdata2(BUS_ADDR_WIDTH-1, 0);
        wb_npc[0] = 0;
    } else if (wv[Instr_MRET].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        w_mret = i_d_valid.read() && w_pc_valid;
        w_csr_wena = 0;
        wb_csr_addr = CSR_mepc;
        wb_npc = i_csr_rdata;
    } else if (wv[Instr_URET].to_bool()) {
        wb_res = i_d_pc.read() + opcode_len;
        w_uret = i_d_valid.read() && w_pc_valid;
        w_csr_wena = 0;
        wb_csr_addr = CSR_uepc;
        wb_npc = i_csr_rdata;
    } else {
        // Instr_HRET, Instr_SRET, Instr_FENCE, Instr_FENCE_I:
        wb_npc = i_d_pc.read() + opcode_len;
    }

    if (i_memop_load) {
        wb_memop_addr =
            wb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_rdata2(BUS_ADDR_WIDTH-1, 0);
    } else if (i_memop_store) {
        wb_memop_addr = 
            wb_rdata1(BUS_ADDR_WIDTH-1, 0) + wb_off(BUS_ADDR_WIDTH-1, 0);
    }

    v.memop_addr = 0;
    v.memop_load = 0;
    v.memop_store = 0;
    v.memop_sign_ext = 0;
    v.memop_size = 0;
    w_exception_store = 0;
    w_exception_load = 0;

    if ((wv[Instr_LD] && wb_memop_addr(2, 0) != 0)
        || ((wv[Instr_LW] || wv[Instr_LWU]) && wb_memop_addr(1, 0) != 0)
        || ((wv[Instr_LH] || wv[Instr_LHU]) && wb_memop_addr[0] != 0)) {
        w_exception_load = !w_hazard_detected.read();
    }
    if ((wv[Instr_SD] && wb_memop_addr(2, 0) != 0)
        || (wv[Instr_SW] && wb_memop_addr(1, 0) != 0)
        || (wv[Instr_SH] && wb_memop_addr[0] != 0)) {
        w_exception_store = !w_hazard_detected.read();
    }


    /** Default number of cycles per instruction = 0 (1 clock per instr)
     *  If instruction is multicycle then modify this value.
     */
    w_fpu_ena = 0;
    if (CFG_HW_FPU_ENABLE) {
        if (i_f64.read() && !(wv[Instr_FSD] | wv[Instr_FLD]).to_bool()) {
            w_fpu_ena = 1;
        }
    }

    v.multi_ena[Multi_MUL] = 0;
    v.multi_ena[Multi_DIV] = 0;
    v.multi_ena[Multi_FPU] = 0;
    v.multi_rv32 = i_rv32;
    v.multi_f64 = i_f64;
    v.multi_unsigned = i_unsigned_op;
    v.multi_residual_high = 0;
    if (w_fpu_ena == 1) {
        v.multi_a1 = wb_rdata1;
        v.multi_a2 = wb_rdata2;
    } else {
        v.multi_a1 = i_rdata1;
        v.multi_a2 = i_rdata2;
    }

    w_multi_ena = (wv[Instr_MUL] | wv[Instr_MULW] | wv[Instr_DIV] 
                    | wv[Instr_DIVU] | wv[Instr_DIVW] | wv[Instr_DIVUW]
                    | wv[Instr_REM] | wv[Instr_REMU] | wv[Instr_REMW]
                    | wv[Instr_REMUW]).to_bool() || w_fpu_ena;
    if (w_multi_ena & w_d_acceptable) {
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
        v.multi_npc = wb_npc;
    }

    // ALU block selector:
    if (w_arith_valid[Multi_MUL]) {
        wb_res = wb_arith_res.arr[Multi_MUL];
    } else if (w_arith_valid[Multi_DIV]) {
        wb_res = wb_arith_res.arr[Multi_DIV];
    } else if (w_arith_valid[Multi_FPU]) {
        wb_res = wb_arith_res.arr[Multi_FPU];
    } else if (i_memop_load) {
        w_memop_load = !w_hazard_detected.read();
        w_memop_sign_ext = i_memop_sign_ext;
        wb_memop_size = i_memop_size;
    } else if (i_memop_store) {
        w_memop_store = !w_hazard_detected.read();
        wb_memop_size = i_memop_size;
        wb_res = wb_rdata2;
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
        wb_res = wb_rdata2;
    } else if (wv[Instr_MUL] || wv[Instr_MULW]) {
        v.multi_ena[Multi_MUL] = w_d_acceptable;
    } else if (wv[Instr_DIV] || wv[Instr_DIVU]
            || wv[Instr_DIVW] || wv[Instr_DIVUW]) {
        v.multi_ena[Multi_DIV] = w_d_acceptable;
    } else if (wv[Instr_REM] || wv[Instr_REMU]
            || wv[Instr_REMW] || wv[Instr_REMUW]) {
        v.multi_ena[Multi_DIV] = w_d_acceptable;
        v.multi_residual_high = 1;
    } else if (w_fpu_ena == 1) {
        v.multi_ena[Multi_FPU] = w_d_acceptable;
    } else if (wv[Instr_CSRRC]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() & ~i_rdata1.read();
    } else if (wv[Instr_CSRRCI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        wb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) & ~wb_radr1(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRS]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() | i_rdata1.read();
    } else if (wv[Instr_CSRRSI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = i_csr_rdata.read()(RISCV_ARCH-1, 5);
        wb_csr_wdata(4, 0) = i_csr_rdata.read()(4, 0) | wb_radr1(4, 0);  // zero-extending 5 to 64-bits
    } else if (wv[Instr_CSRRW]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_rdata1;
    } else if (wv[Instr_CSRRWI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata(RISCV_ARCH-1, 5) = 0;
        wb_csr_wdata(4, 0) = wb_radr1(4, 0);  // zero-extending 5 to 64-bits
    }

    w_d_valid = (w_d_acceptable && !w_multi_ena) || w_multi_valid;
    o_pre_valid = w_d_valid;

    o_ex_illegal_instr = (i_unsup_exception.read() & w_pc_valid) && w_d_valid;
    o_ex_unalign_store = w_exception_store && w_d_valid;
    o_ex_unalign_load = w_exception_load && w_d_valid;
    o_ex_breakpoint = wv[Instr_EBREAK].to_bool() && w_d_valid;
    o_ex_ecall = wv[Instr_ECALL].to_bool() && w_d_valid;

    v.call = 0;
    v.ret = 0;
    wb_ex_npc = 0;
    if (i_dport_npc_write.read()) {
        v.npc = i_dport_npc.read();
    } else if (w_d_valid) {
        if (w_multi_valid) {
            v.pc = r.multi_pc;
            v.instr = r.multi_instr;
            if (i_trap_valid.read()) {
                v.npc = i_trap_pc.read();
                wb_ex_npc = r.multi_npc;
            } else {
                v.npc = r.multi_npc;
            }
            v.memop_load = 0;
            v.memop_sign_ext = 0;
            v.memop_store = 0;
            v.memop_size = 0;
            v.memop_addr = 0;
        } else {
            v.pc = i_d_pc;
            v.instr = i_d_instr;
            if (i_trap_valid.read()) {
                v.npc = i_trap_pc.read();
                wb_ex_npc = wb_npc;
            } else {
                v.npc = wb_npc;
            }
            v.memop_load = w_memop_load;
            v.memop_sign_ext = w_memop_sign_ext;
            v.memop_store = w_memop_store;
            v.memop_size = wb_memop_size;
            v.memop_addr = wb_memop_addr;
        }
        v.res_addr = wb_res_addr;
        v.res_val = wb_res;

        v.hazard_addr1 = r.hazard_addr0;
        v.hazard_addr0 = wb_res_addr;

        if (wv[Instr_JAL] && wb_res_addr == Reg_ra) {
            v.call = 1;
        }
        if (wv[Instr_JALR]) {
            if (wb_res_addr == Reg_ra) {
                v.call = 1;
            } else if (wb_rdata2 == 0 && wb_radr1 == Reg_ra) {
                v.ret = 1;
            }
        }
    }

    v.d_valid = w_d_valid;

    if (w_d_valid && !i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() + 1;
        v.hazard_addr0 = wb_res_addr;
    } else if (!w_d_valid && i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() - 1;
    }
    w_hazard_lvl1 = 0;
    if ((wb_radr1 != 0 && (wb_radr1 == r.hazard_addr0)) ||
        (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr0))) {
        w_hazard_lvl1 = 1;
    }
    w_hazard_lvl2 = 0;
    if ((wb_radr1 != 0 && (wb_radr1 == r.hazard_addr1)) ||
        (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr1))) {
        w_hazard_lvl2 = 1;
    }

    if (r.hazard_depth.read() == 1) {
        w_hazard_detected = w_hazard_lvl1;
    } else if (r.hazard_depth.read() == 2) {
        w_hazard_detected = w_hazard_lvl1 | w_hazard_lvl2;
    } else {
        w_hazard_detected = 0;
    }

    w_o_valid = r.d_valid.read();
    w_o_pipeline_hold = w_hazard_detected | r.multiclock_ena;

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_radr1 = wb_radr1;
    o_radr2 = wb_radr2;
    o_res_addr = r.res_addr;
    o_res_data = r.res_val;
    o_pipeline_hold = w_o_pipeline_hold;

    o_csr_wena = w_csr_wena & w_pc_valid & !w_hazard_detected;
    o_csr_addr = wb_csr_addr;
    o_csr_wdata = wb_csr_wdata;
    o_ex_npc = wb_ex_npc;

    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_size = r.memop_size;
    o_memop_addr = r.memop_addr;

    o_valid = w_o_valid;
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
    }
}

}  // namespace debugger

