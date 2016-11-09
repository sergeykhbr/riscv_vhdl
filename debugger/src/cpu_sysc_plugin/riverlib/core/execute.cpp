/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Execution stage.
 */

#include "execute.h"
#include "riscv-isa.h"

namespace debugger {

InstrExecute::InstrExecute(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_cache_hold;
    sensitive << i_d_valid;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
    sensitive << i_wb_done;
    sensitive << i_memop_load;
    sensitive << i_memop_store;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_size;
    sensitive << i_unsigned_op;
    sensitive << i_rv32;;
    sensitive << i_isa_type;
    sensitive << i_ivec;
    sensitive << i_rdata1;
    sensitive << i_rdata2;
    sensitive << i_csr_rdata;
    sensitive << r.d_valid;
    sensitive << r.npc;
    sensitive << r.hazard_depth;
    sensitive << r.hazard_addr[0];
    sensitive << r.hazard_addr[1];
    sensitive << r.multiclock_cnt;
    sensitive << r.multiclock_instr;
    sensitive << r.postponed_valid;
    sensitive << r.res_val;
    sensitive << w_hazard_detected;
    sensitive << r.multi_ena[Multi_MUL];
    sensitive << r.multi_ena[Multi_DIV];
    sensitive << wb_arith_res[Multi_MUL];
    sensitive << wb_arith_res[Multi_DIV];

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    mul0 = new IntMul("mul0", vcd);
    mul0->i_clk(i_clk);
    mul0->i_nrst(i_nrst);
    mul0->i_ena(r.multi_ena[Multi_MUL]);
    mul0->i_unsigned(r.multi_unsigned);
    mul0->i_rv32(r.multi_rv32);
    mul0->i_a1(r.multi_a1);
    mul0->i_a2(r.multi_a2);
    mul0->o_res(wb_arith_res[Multi_MUL]);

    div0 = new IntDiv("div0", vcd);
    div0->i_clk(i_clk);
    div0->i_nrst(i_nrst);
    div0->i_ena(r.multi_ena[Multi_DIV]);
    div0->i_unsigned(r.multi_unsigned);
    div0->i_residual(r.multi_residual);
    div0->i_rv32(r.multi_rv32);
    div0->i_a1(r.multi_a1);
    div0->i_a2(r.multi_a2);
    div0->o_res(wb_arith_res[Multi_DIV]);

    if (vcd) {
        sc_trace(vcd, i_ext_irq, "/top/proc0/exec0/i_ext_irq");
        sc_trace(vcd, i_cache_hold, "/top/proc0/exec0/i_cache_hold");
        sc_trace(vcd, i_d_valid, "/top/proc0/exec0/i_d_valid");
        sc_trace(vcd, i_d_pc, "/top/proc0/exec0/i_d_pc");
        sc_trace(vcd, i_d_instr, "/top/proc0/exec0/i_d_instr");
        sc_trace(vcd, i_wb_done, "/top/proc0/exec0/i_wb_done");
        sc_trace(vcd, i_rdata1, "/top/proc0/exec0/i_rdata1");
        sc_trace(vcd, i_rdata2, "/top/proc0/exec0/i_rdata2");
        sc_trace(vcd, o_valid, "/top/proc0/exec0/o_valid");
        sc_trace(vcd, o_npc, "/top/proc0/exec0/o_npc");
        sc_trace(vcd, o_pc, "/top/proc0/exec0/o_pc");
        sc_trace(vcd, o_radr1, "/top/proc0/exec0/o_radr1");
        sc_trace(vcd, o_radr2, "/top/proc0/exec0/o_radr2");
        sc_trace(vcd, o_res_addr, "/top/proc0/exec0/o_res_addr");
        sc_trace(vcd, o_res_data, "/top/proc0/exec0/o_res_data");
        sc_trace(vcd, o_memop_addr, "/top/proc0/exec0/o_memop_addr");
        sc_trace(vcd, o_memop_load, "/top/proc0/exec0/o_memop_load");
        sc_trace(vcd, o_memop_store, "/top/proc0/exec0/o_memop_store");
        sc_trace(vcd, o_memop_size, "/top/proc0/exec0/o_memop_size");
        sc_trace(vcd, o_csr_addr, "/top/proc0/exec0/o_csr_addr");
        sc_trace(vcd, o_csr_wena, "/top/proc0/exec0/o_csr_wena");
        sc_trace(vcd, i_csr_rdata, "/top/proc0/exec0/i_csr_rdata");
        sc_trace(vcd, o_csr_wdata, "/top/proc0/exec0/o_csr_wdata");
        sc_trace(vcd, o_pipeline_hold, "/top/proc0/exec0/o_pipeline_hold");

        sc_trace(vcd, w_hazard_detected, "/top/proc0/exec0/w_hazard_detected");
        sc_trace(vcd, r.hazard_depth, "/top/proc0/exec0/r_hazard_depth");
        sc_trace(vcd, r.hazard_addr[0], "/top/proc0/exec0/r_hazard_addr(0)");
        sc_trace(vcd, r.hazard_addr[1], "/top/proc0/exec0/r_hazard_addr(1)");
        sc_trace(vcd, r.multiclock_cnt, "/top/proc0/exec0/r_multiclock_cnt");
        sc_trace(vcd, r.multi_ena[Multi_DIV], "/top/proc0/exec0/r_multi_ena(1)");
        sc_trace(vcd, wb_arith_res[Multi_DIV], "/top/proc0/exec0/wb_arith_res(1)");

        sc_trace(vcd, w_trap, "/top/proc0/exec0/w_trap");
        sc_trace(vcd, r.trap_ena, "/top/proc0/exec0/r_trap_ena");
        sc_trace(vcd, r.trap_pc, "/top/proc0/exec0/r_trap_pc");
        sc_trace(vcd, r.trap_code, "/top/proc0/exec0/r_trap_code");
        sc_trace(vcd, r.trap_code_waiting, "/top/proc0/exec0/r_trap_code_waiting");
    }
};

InstrExecute::~InstrExecute() {
    delete mul0;
    delete div0;
}

void InstrExecute::comb() {
    sc_uint<5> wb_radr1;
    sc_uint<RISCV_ARCH> wb_rdata1;
    sc_uint<5> wb_radr2;
    sc_uint<RISCV_ARCH> wb_rdata2;
    bool w_xret = 0;
    bool w_csr_wena = 0;
    sc_uint<5> wb_res_addr = 0;
    sc_uint<12> wb_csr_addr = 0;
    sc_uint<RISCV_ARCH> wb_csr_wdata = 0;
    sc_uint<RISCV_ARCH> wb_res = 0;
    sc_uint<AXI_ADDR_WIDTH> wb_npc;
    sc_uint<RISCV_ARCH> wb_off;
    sc_uint<RISCV_ARCH> wb_mask_i31;    // Bits depending instr[31] bits
    sc_uint<RISCV_ARCH> wb_sum64;
    sc_uint<RISCV_ARCH> wb_sum32;
    sc_uint<RISCV_ARCH> wb_sub64;
    sc_uint<RISCV_ARCH> wb_sub32;
    sc_uint<RISCV_ARCH> wb_and64;
    sc_uint<RISCV_ARCH> wb_or64;
    sc_uint<RISCV_ARCH> wb_xor64;
    sc_uint<RISCV_ARCH> wb_sll64;
    sc_uint<RISCV_ARCH> wb_srl64;
    sc_uint<RISCV_ARCH> wb_srl32;
    sc_uint<7> wb_multiclock_cnt;       // up to 127 clocks per one instruction (maybe insreased)

    bool w_res_wena;
    bool w_pc_branch;

    sc_bv<Instr_Total> wv = i_ivec.read();

    v = r;

    v.memop_load = 0;
    v.memop_store = 0;
    v.memop_size = 0;
    v.memop_addr = 0;
#if 1
    int t_pc = i_d_pc.read();
    int t_instr = i_d_instr.read();
    int tinstr_idx = -1;
    int check_unqiue_cnt = 0;
    int tisa = i_isa_type.read()[ISA_R_type];
    for (int i = 0; i < Instr_Total; i++) {
        if (wv[i].to_bool()) {
            tinstr_idx = i;
            check_unqiue_cnt++;
        }
    }
    if (i_d_pc.read() == 0x10001e94) {
        bool st = true;
    }
#endif

    wb_mask_i31 = 0;
    if (i_d_instr.read()[31]) {
        wb_mask_i31 = ~0ull;
    }

    if (i_isa_type.read()[ISA_R_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
    } else if (i_isa_type.read()[ISA_I_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = 0;
        wb_rdata2 = (wb_mask_i31(63, 12), i_d_instr.read().range(31, 20));
    } else if (i_isa_type.read()[ISA_SB_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
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
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
        wb_off(RISCV_ARCH-1, 12) = wb_mask_i31(RISCV_ARCH-1, 12);
        wb_off(11, 5) = i_d_instr.read()(31, 25);
        wb_off(4, 0) = i_d_instr.read()(11, 7);
    }

    // Don't modify registers on conditional jumps:
    w_res_wena = !(wv[Instr_BEQ] | wv[Instr_BGE] | wv[Instr_BGEU]
               | wv[Instr_BLT] | wv[Instr_BLTU] | wv[Instr_BNE]
               | wv[Instr_SD] | wv[Instr_SW] | wv[Instr_SH] | wv[Instr_SB]
               | wv[Instr_MRET] | wv[Instr_URET]).to_bool();
    if (w_res_wena) {
        wb_res_addr = i_d_instr.read().range(11, 7);
    } else {
        wb_res_addr = 0;
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
    wb_sll64 = wb_rdata1 << wb_rdata2;
    wb_srl64 = wb_rdata1 >> wb_rdata2;
    wb_srl32(31, 0) = wb_rdata1(31, 0) >> wb_rdata2;
    if (wb_srl32[31]) {
        wb_srl32(RISCV_ARCH - 1, 32) = ~0;
    }

    // Relative Branch on some condition:
    w_pc_branch = (wv[Instr_BEQ] & (wb_sub64 == 0))
              || (wv[Instr_BGE] & (wb_sub64[63] == 0))
              || (wv[Instr_BGEU] & (wb_sub64[63] == wb_rdata1[63]))
              || (wv[Instr_BLT] & (wb_sub64[63] == 1))
              || (wv[Instr_BLTU] & (wb_sub64[63] != wb_rdata1[63]))
              || (wv[Instr_BNE] & (wb_sub64 != 0));

    if (w_pc_branch) {
        wb_npc = i_d_pc.read() + wb_off(AXI_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        wb_npc = wb_rdata1 + wb_off;
    } else if (wv[Instr_JALR].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        wb_npc = wb_rdata1 + wb_rdata2;
        wb_npc[0] = 0;
    } else if (wv[Instr_MRET].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        w_xret = 1;
        w_csr_wena = 0;
        wb_csr_addr = CSR_mepc;
        wb_npc = i_csr_rdata;
    } else {
        wb_npc = i_d_pc.read() + 4;
    }

    v.memop_addr = 0;
    v.memop_load = 0;
    v.memop_store = 0;
    v.memop_sign_ext = 0;
    v.memop_size = 0;

    /** Default number of cycles per instruction = 0 (1 clock per instr)
     *  If instruction is multicycle then modify this value.
     */
    wb_multiclock_cnt = 0;
    v.multi_ena[Multi_MUL] = 0;
    v.multi_ena[Multi_DIV] = 0;
    v.multi_rv32 = i_rv32;
    v.multi_unsigned = i_unsigned_op;
    v.multi_residual = 0;
    //v.multi_type = 0; // This value mustn't be changed during computation
    v.multi_a1 = i_rdata1;
    v.multi_a2 = i_rdata2;

    // ALU block selector:
    if (i_memop_load) {
        v.memop_addr = wb_rdata1 + wb_rdata2;
        v.memop_load = !w_hazard_detected.read();
        v.memop_sign_ext = i_memop_sign_ext;
        v.memop_size = i_memop_size;
    } else if (i_memop_store) {
        v.memop_addr = wb_rdata1 + wb_off;
        v.memop_store = !w_hazard_detected.read();
        v.memop_size = i_memop_size;
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
        wb_res = wb_sll64;
    } else if (wv[Instr_SLLW] || wv[Instr_SLLIW]) {
        wb_res(31, 0) = wb_sll64(31, 0);
        wb_res(63, 32) = 0;
    } else if (wv[Instr_SRL] || wv[Instr_SRLI]) {
        wb_res = wb_srl64;
    } else if (wv[Instr_SRLW] || wv[Instr_SRLIW]) {
        wb_res = wb_srl32;
    } else if (wv[Instr_SRA] || wv[Instr_SRAI]) {
        wb_res = wb_srl64;
    } else if (wv[Instr_SRAW] || wv[Instr_SRAW]) {
        wb_res = wb_srl32;
    } else if (wv[Instr_AND] || wv[Instr_ANDI]) {
        wb_res = wb_and64;
    } else if (wv[Instr_OR] || wv[Instr_ORI]) {
        wb_res = wb_or64;
    } else if (wv[Instr_XOR] || wv[Instr_XORI]) {
        wb_res = wb_xor64;
    } else if (wv[Instr_SLTU] || wv[Instr_SLTIU]) {
        wb_res = wb_sub64[63] ^ wb_rdata1[63];
    } else if (wv[Instr_LUI]) {
        uint64_t x1 = wb_res = wb_rdata2;
        bool stop = true;
    } else if (wv[Instr_MUL] || wv[Instr_MULW]) {
        v.multi_ena[Multi_MUL] = !r.multiclock_instr;
        v.multi_type = Multi_MUL;
        wb_multiclock_cnt = IMUL_EXEC_DURATION_CYCLES;
    } else if (wv[Instr_DIV] || wv[Instr_DIVU]
            || wv[Instr_DIVW] || wv[Instr_DIVUW]) {
        v.multi_ena[Multi_DIV] = !r.multiclock_instr;
        v.multi_type = Multi_DIV;
        wb_multiclock_cnt = IDIV_EXEC_DURATION_CYCLES;
    } else if (wv[Instr_REM] || wv[Instr_REMU]
            || wv[Instr_REMW] || wv[Instr_REMUW]) {
        v.multi_ena[Multi_DIV] = !r.multiclock_instr;
        v.multi_residual = 1;
        v.multi_type = Multi_DIV;
        wb_multiclock_cnt = IDIV_EXEC_DURATION_CYCLES;
    } else if (wv[Instr_CSRRC]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() & ~i_rdata1.read();
    } else if (wv[Instr_CSRRCI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() & ~wb_radr1;  // extending to 64-bits
    } else if (wv[Instr_CSRRS]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() | i_rdata1.read();
    } else if (wv[Instr_CSRRSI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_csr_rdata.read() | wb_radr1;  // extending to 64-bits
    } else if (wv[Instr_CSRRW]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = i_rdata1;
    } else if (wv[Instr_CSRRWI]) {
        wb_res = i_csr_rdata;
        w_csr_wena = 1;
        wb_csr_addr = wb_rdata2.range(11, 0);
        wb_csr_wdata = wb_radr1;  // extending to 64-bits
    }

    v.ext_irq_pulser = i_ext_irq & i_ie;
    w_trap = !i_cache_hold & i_d_valid
            & (i_d_pc.read() == r.npc.read())
            & r.trap_code_waiting  & !r.multiclock_instr;

    if (i_ext_irq & i_ie & !r.ext_irq_pulser) { // Maskable traps (interrupts)
        v.trap_code_waiting[4] = 1;
        v.trap_code_waiting(3, 0) = 11;
    } else if (i_unsup_exception.read()) {      // Unmaskable traps (exceptions)
        v.trap_code_waiting[4] = 1;
        v.trap_code_waiting(3, 0) = EXCEPTION_InstrIllegal;
    } else if (w_trap) {
        v.trap_code_waiting = 0;
    }

    bool w_d_valid = !i_cache_hold.read() && i_d_valid.read()
                        && i_d_pc.read() == r.npc.read()
                        && !w_trap.read();

    v.postponed_valid = 0;
    if (w_d_valid && wb_multiclock_cnt != 0) {
        v.multiclock_cnt = wb_multiclock_cnt;
        v.multiclock_instr = 1;
    } else if (r.multiclock_cnt.read() != 0) {
        v.res_val = wb_arith_res[r.multi_type.read()];
        v.multiclock_cnt = r.multiclock_cnt.read() - 1;
        if (r.multiclock_cnt.read() == 1) {
            v.multiclock_instr = 0;
            v.postponed_valid = 1;
        }
    }

    v.trap_ena = 0;
    if (w_trap) {
        v.trap_ena = 1;
        v.trap_pc = i_d_pc;
        v.trap_code = r.trap_code_waiting;
        v.npc = i_mtvec;
    } else if (w_d_valid) {
        v.pc = i_d_pc;
        v.instr = i_d_instr;
        v.npc = wb_npc;
        v.res_addr = wb_res_addr;
        v.res_val = wb_res;

        v.hazard_addr[1] = r.hazard_addr[0];
        v.hazard_addr[0] = wb_res_addr;
    }

    v.d_valid = w_d_valid;

    if (w_d_valid && !i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() + 1;
        v.hazard_addr[0] = wb_res_addr;
    } else if (!w_d_valid && i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() - 1;
    }
    bool w_hazard_lvl1 = (wb_radr1 != 0 && (wb_radr1 == r.hazard_addr[0]))
                      || (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr[0]));
    bool w_hazard_lvl2 = (wb_radr1 != 0 && (wb_radr1 == r.hazard_addr[1]))
                      || (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr[1]));

    if (r.hazard_depth.read() == 1) {
        w_hazard_detected = w_hazard_lvl1;
    } else if (r.hazard_depth.read() == 2) {
        w_hazard_detected = w_hazard_lvl1 | w_hazard_lvl2;
    } else {
        w_hazard_detected = 0;
    }

    if (!i_nrst.read()) {
        v.d_valid = false;
        v.pc = 0;
        v.npc = RESET_VECTOR;
        v.instr = 0;
        v.res_addr = 0;
        v.res_val = 0;
        v.memop_load = 0;
        v.memop_store = 0;
        v.memop_size = 0;
        v.memop_addr = 0;
        v.hazard_depth = 0;
        v.hazard_addr[0] = 0;
        v.hazard_addr[1] = 0;
        v.multiclock_cnt = 0;
        v.multiclock_instr = 0;

        v.multi_ena[Multi_MUL] = 0;
        v.multi_ena[Multi_DIV] = 0;
        v.multi_rv32 = 0;
        v.multi_unsigned = 0;
        v.multi_residual = 0;
        v.multi_type = 0;
        v.multi_a1 = 0;
        v.multi_a2 = 0;

        v.ext_irq_pulser = 0;
        v.trap_code_waiting = 0;
        v.trap_ena = 0;
        v.trap_code = 0;
        v.trap_pc = 0;
    }

    o_radr1 = wb_radr1;
    o_radr2 = wb_radr2;
    o_res_addr = r.res_addr;
    o_res_data = r.res_val;
    o_pipeline_hold = w_hazard_detected | r.multiclock_instr;

    o_xret = w_xret;
    o_csr_wena = w_csr_wena;
    o_csr_addr = wb_csr_addr;
    o_csr_wdata = wb_csr_wdata;
    o_trap_ena = r.trap_ena;
    o_trap_code = r.trap_code;
    o_trap_pc = r.trap_pc;

    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_size = r.memop_size;
    o_memop_addr = r.memop_addr;

    o_valid = (r.d_valid.read() & !r.multiclock_instr) | r.postponed_valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
}

void InstrExecute::registers() {
    r = v;
}

}  // namespace debugger

