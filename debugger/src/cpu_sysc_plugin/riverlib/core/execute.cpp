/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Execution stage.
 */

#include "execute.h"

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
    sensitive << i_isa_type;
    sensitive << i_ivec;
    sensitive << i_rdata1;
    sensitive << i_rdata2;
    sensitive << i_csr_rdata;
    sensitive << r.valid;
    sensitive << r.npc;
    sensitive << r.hazard_depth;
    sensitive << r.hazard_addr[0];
    sensitive << r.hazard_addr[1];
    sensitive << w_hazard_detected;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
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
        sc_trace(vcd, o_csr_wdata, "/top/proc0/exec0/o_csr_wena");

        sc_trace(vcd, w_hazard_detected, "/top/proc0/exec0/w_hazard_detected");
        sc_trace(vcd, r.hazard_depth, "/top/proc0/exec0/r_hazard_depth");
        sc_trace(vcd, r.hazard_addr[0], "/top/proc0/exec0/r_hazard_addr(0)");
        sc_trace(vcd, r.hazard_addr[1], "/top/proc0/exec0/r_hazard_addr(1)");
    }
};


void InstrExecute::comb() {
    sc_uint<5> wb_radr1;
    sc_uint<RISCV_ARCH> wb_rdata1;
    sc_uint<5> wb_radr2;
    sc_uint<RISCV_ARCH> wb_rdata2;
    bool w_csr_wena = 0;
    sc_uint<5> wb_res_addr = 0;
    sc_uint<12> wb_csr_addr = 0;
    sc_uint<RISCV_ARCH> wb_csr_wdata = 0;
    sc_uint<RISCV_ARCH> wb_res = 0;
    sc_uint<AXI_ADDR_WIDTH> wb_npc;
    sc_uint<RISCV_ARCH> wb_off;
    sc_uint<RISCV_ARCH> wb_mask_i31;    // Bits depending instr[31] bits
    sc_uint<RISCV_ARCH> wb_sum64;
    sc_uint<RISCV_ARCH> wb_sub64;
    sc_uint<RISCV_ARCH> wb_and64;
    sc_uint<RISCV_ARCH> wb_or64;
    sc_uint<RISCV_ARCH> wb_xor64;
    sc_uint<RISCV_ARCH> wb_mul64;
    sc_uint<RISCV_ARCH> wb_sll64;
    sc_uint<RISCV_ARCH> wb_srl64;
    sc_uint<32> wb_srl32;
    sc_uint<2> wb_memop_size;
    bool w_memop_store;
    bool w_memop_load;

    bool w_w32;
    bool w_unsigned;
    bool w_res_wena;
    bool w_pc_jump;

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
    if (i_d_pc.read() == 0x1444) {
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
        uint64_t x4 = wb_rdata2;
        bool st = true;
    } else if (i_isa_type.read()[ISA_SB_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
        wb_off(RISCV_ARCH-1, 12) = wb_mask_i31(RISCV_ARCH-1, 12);
        uint64_t x1 = wb_off[12] = i_d_instr.read()[31];
        uint64_t x2 = wb_off[11] = i_d_instr.read()[7];
        uint64_t x3 = wb_off(10, 5) = i_d_instr.read()(30, 25);
        uint64_t x4 = wb_off(4, 1) = i_d_instr.read()(11, 8);
        uint64_t x5 = wb_off[0] = 0;
        bool st = true;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        wb_radr1 = 0;
        uint64_t x2, x1 = wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_off(RISCV_ARCH-1, 20) = wb_mask_i31(RISCV_ARCH-1, 20);
        wb_off(19, 12) = i_d_instr.read()(19, 12);
        wb_off[11] = i_d_instr.read()[20];
        wb_off(10, 1) = i_d_instr.read()(30, 21);
        wb_off[0] = 0;
        x2 = wb_off;
        bool st = true;
    } else if (i_isa_type.read()[ISA_U_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_rdata2(31, 0) = i_d_instr.read().range(31, 12) << 12;
        wb_off(RISCV_ARCH-1, 32) = wb_mask_i31(RISCV_ARCH-1, 32);
        uint64_t x2 = wb_rdata2;
        bool stop = true;
    } else if (i_isa_type.read()[ISA_S_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        int a2 = wb_radr2 = i_d_instr.read().range(24, 20);
        uint64_t rv2 = wb_rdata2 = i_rdata2;

        wb_off(RISCV_ARCH-1, 12) = wb_mask_i31(RISCV_ARCH-1, 12);
        wb_off(11, 5) = i_d_instr.read()(31, 25);
        wb_off(4, 0) = i_d_instr.read()(11, 7);
        uint64_t x2 = wb_off;
        bool stop = true;
    }

    // Don't modify registers on conditional jumps:
    w_res_wena = !(wv[Instr_BEQ] | wv[Instr_BGE] | wv[Instr_BGEU]
               | wv[Instr_BLT] | wv[Instr_BLTU] | wv[Instr_BNE]
               | wv[Instr_SD] | wv[Instr_SW] | wv[Instr_SH] | wv[Instr_SB]
               | wv[Instr_MRET]).to_bool();
    if (w_res_wena) {
        wb_res_addr = i_d_instr.read().range(11, 7);
    } else {
        wb_res_addr = 0;
    }

    // parallel ALU:
    uint64_t t_sum64 = wb_sum64 = wb_rdata1 + wb_rdata2;
    wb_sub64 = wb_rdata1 - wb_rdata2;
    wb_and64 = wb_rdata1 & wb_rdata2;
    wb_or64 = wb_rdata1 | wb_rdata2;
    wb_xor64 = wb_rdata1 ^ wb_rdata2;
    wb_mul64 = wb_rdata1 * wb_rdata2;
    wb_sll64 = wb_rdata1 << wb_rdata2;
    wb_srl64 = wb_rdata1 >> wb_rdata2;
    wb_srl32 = wb_rdata1(31,0) >> wb_rdata2;

    // Relative Jumps on some condition:
    w_pc_jump = (wv[Instr_BEQ] & (wb_sub64 == 0))
              || (wv[Instr_BGE] & (wb_sub64[63] == 0))
              || (wv[Instr_BGEU] & (wb_sub64[63] == wb_rdata1[63]))
              || (wv[Instr_BLT] & (wb_sub64[63] == 1))
              || (wv[Instr_BLTU] & (wb_sub64[63] != wb_rdata1[63]))
              || (wv[Instr_BEQ] & (wb_sub64 != 0));

    if (w_pc_jump) {
        wb_npc = i_d_pc.read() + wb_off(AXI_ADDR_WIDTH-1, 0);
    } else if (wv[Instr_JAL].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        uint64_t x1 = wb_npc = wb_rdata1 + wb_off;
        // TODO: execptions and traps:
        bool st=true;
    } else if (wv[Instr_JALR].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        uint64_t x1 = wb_npc = wb_rdata1 + wb_rdata2;
        bool st=true;
    } else if (wv[Instr_MRET].to_bool()) {
        wb_res = i_d_pc.read() + 4;
        w_csr_wena = 0;
        wb_csr_addr = 0x341;
        uint64_t x1 = wb_npc = i_csr_rdata;
        bool st=true;
    } else {
        wb_npc = i_d_pc.read() + 4;
    }

    // RV32 instructions list (MOVE TO DECODER):
    w_w32 = (wv[Instr_ADDW] | wv[Instr_ADDIW] 
        | wv[Instr_SLLW] | wv[Instr_SLLIW] | wv[Instr_SRAW] | wv[Instr_SRAIW]
        | wv[Instr_SRLW] | wv[Instr_SRLIW] | wv[Instr_SUBW] 
        | wv[Instr_DIVW] | wv[Instr_DIVUW] | wv[Instr_MULW]
        | wv[Instr_REMW] | wv[Instr_REMUW]).to_bool();
    w_unsigned = (wv[Instr_DIVUW] | wv[Instr_REMUW]).to_bool();

    if (wv[Instr_LD] || wv[Instr_SD]) {
        wb_memop_size = MEMOP_8B;
    } else if (wv[Instr_LW] || wv[Instr_LWU] || wv[Instr_SW]) {
        wb_memop_size = MEMOP_4B;
    } else if (wv[Instr_LH] || wv[Instr_LHU] || wv[Instr_SH]) {
        wb_memop_size = MEMOP_2B;
    } else {
        wb_memop_size = MEMOP_1B;
    }
    w_memop_store = (wv[Instr_SD] | wv[Instr_SW] | wv[Instr_SH] | wv[Instr_SB]).to_bool();
    w_memop_load = (wv[Instr_LD] | wv[Instr_LW] | wv[Instr_LH] | wv[Instr_LB]
                  | wv[Instr_LWU] | wv[Instr_LHU] | wv[Instr_LBU]).to_bool();
    v.memop_sign_ext = (wv[Instr_LD] | wv[Instr_LW] | wv[Instr_LH] | wv[Instr_LB]).to_bool();

    // ALU block selector:
    if (wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]) {
        wb_res = wb_sum64;
    } else if (wv[Instr_ADDW] || wv[Instr_ADDIW]) {
        wb_res(31, 0) = wb_sum64(31, 0);
        if (wb_sum64[31]) {
            wb_res(63, 32) = ~0;
        }
    } else if (wv[Instr_SLL] || wv[Instr_SLLI]) {
        wb_res = wb_sll64;
    } else if (wv[Instr_SRL] || wv[Instr_SRLI]) {
        wb_res = wb_srl64;
    } else if (wv[Instr_SRLW] || wv[Instr_SRLIW]) {
        wb_res(31, 0) = wb_srl32;
        wb_res(63, 32) = 0;
    /*} else if (wv[Instr_SRA] || wv[Instr_SRAI]) {
        wb_res = wb_srl64;
    } else if (wv[Instr_SRAW] || wv[Instr_SRAW]) {
        wb_res(31, 0) = wb_srl32;
        if (wb_srl32[31]) {
            wb_res(63, 32) = ~0;
        }*/
    } else if (wv[Instr_AND] || wv[Instr_ANDI]) {
        wb_res = wb_and64;
    } else if (wv[Instr_OR] || wv[Instr_ORI]) {
        wb_res = wb_or64;
    } else if (wv[Instr_XOR] || wv[Instr_XORI]) {
        wb_res = wb_xor64;
    } else if (w_memop_load) {
        uint64_t a0 = wb_rdata1;
        uint64_t a1 = wb_rdata2;
        v.memop_addr = wb_rdata1 + wb_rdata2;
        uint64_t x1 = v.memop_addr.read();
        v.memop_load = !w_hazard_detected.read();
        v.memop_size = wb_memop_size;
    } else if (w_memop_store) {
        uint64_t a0 = wb_rdata1;
        uint64_t a1 = wb_off;
        v.memop_addr = wb_rdata1 + wb_off;
        uint64_t x1 = v.memop_addr.read();
        uint64_t x2 = wb_rdata2;
        uint64_t x3 = i_rdata2.read();
        v.memop_store = !w_hazard_detected.read();
        v.memop_size = wb_memop_size;
        wb_res = wb_rdata2;
    } else if (wv[Instr_LUI]) {
        uint64_t x1 = wb_res = wb_rdata2;
        bool stop = true;
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

    bool w_valid = 0;

    if (!i_cache_hold.read() && i_d_valid.read() 
                && i_d_pc.read() == r.npc.read()) {
        w_valid = 1;
        v.pc = i_d_pc;
        v.instr = i_d_instr;
        v.npc = wb_npc;
        v.res_addr = wb_res_addr;
        v.res_val = wb_res;
    }


    v.valid = w_valid;

    if (w_valid) {
        v.hazard_addr[1] = r.hazard_addr[0];
        v.hazard_addr[0] = wb_res_addr;
    }

    if (w_valid && !i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() + 1;
        v.hazard_addr[0] = wb_res_addr;
    } else if (!w_valid && i_wb_done.read()) {
        v.hazard_depth = r.hazard_depth.read() - 1;
    }
    switch (r.hazard_depth.read()) {
    case 1:
        w_hazard_detected = (wb_radr1 != 0 && (wb_radr1 == r.hazard_addr[0]))
                            || (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr[0]));
        break;
    case 2:
        w_hazard_detected = (wb_radr1 != 0 && (wb_radr1 == r.hazard_addr[0]
                                            || wb_radr1 == r.hazard_addr[1]))
                            || (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr[0]
                                            || wb_radr2 == r.hazard_addr[1]));
        break;
    default:
        w_hazard_detected = 0;
    }

    if (!i_nrst.read()) {
        v.valid = false;
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
    }

    o_radr1 = wb_radr1;
    o_radr2 = wb_radr2;
    o_res_addr = r.res_addr;
    o_res_data = r.res_val;
    o_hazard_hold = w_hazard_detected;

    o_csr_wena = w_csr_wena;
    o_csr_addr = wb_csr_addr;
    o_csr_wdata = wb_csr_wdata;

    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_size = r.memop_size;
    o_memop_addr = r.memop_addr;

    o_valid = r.valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
}

void InstrExecute::registers() {
    r = v;
}

}  // namespace debugger

