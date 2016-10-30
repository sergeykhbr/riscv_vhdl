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
    sensitive << i_rdata1;
    sensitive << i_rdata2;
    sensitive << i_csr_rdata;
    sensitive << r.valid;
    sensitive << r.hazard_hold;
    sensitive << r.hazard_addr[0];
    sensitive << r.hazard_addr[1];

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
        sc_trace(vcd, o_memop_load, "/top/proc0/exec0/o_memop_load");
        sc_trace(vcd, o_memop_store, "/top/proc0/exec0/o_memop_store");
        sc_trace(vcd, o_memop_size, "/top/proc0/exec0/o_memop_size");
        sc_trace(vcd, o_csr_addr, "/top/proc0/exec0/o_csr_addr");
        sc_trace(vcd, o_csr_wena, "/top/proc0/exec0/o_csr_wena");
        sc_trace(vcd, i_csr_rdata, "/top/proc0/exec0/i_csr_rdata");
        sc_trace(vcd, o_csr_wdata, "/top/proc0/exec0/o_csr_wena");

        sc_trace(vcd, w_hazard_detected, "/top/proc0/exec0/w_hazard_detected");
        sc_trace(vcd, r.hazard_hold, "/top/proc0/exec0/r_hazard_hold");
        sc_trace(vcd, r.hazard_addr[0], "/top/proc0/exec0/r_hazard_addr(0)");
        sc_trace(vcd, r.hazard_addr[1], "/top/proc0/exec0/r_hazard_addr(1)");
    }
};


void InstrExecute::comb() {
    sc_uint<5> wb_radr1;
    sc_uint<RISCV_ARCH> wb_rdata1;
    sc_uint<5> wb_radr2;
    sc_uint<RISCV_ARCH> wb_rdata2;
    sc_uint<5> wb_res_addr = 0;
    sc_uint<RISCV_ARCH> wb_res = 0;
    sc_uint<AXI_ADDR_WIDTH> wb_npc;
    sc_uint<AXI_ADDR_WIDTH> wb_off;
    sc_uint<RISCV_ARCH> wb_sum64;
    sc_uint<RISCV_ARCH> wb_sub64;
    sc_uint<RISCV_ARCH> wb_and64;
    sc_uint<RISCV_ARCH> wb_or64;
    sc_uint<RISCV_ARCH> wb_xor64;
    sc_uint<RISCV_ARCH> wb_mul64;

    sc_logic w_w32;
    sc_logic w_res_wena;
    sc_logic w_pc_jump;

    sc_bv<Instr_Total> wv = i_ivec.read();

    v = r;

    v.memop_load = 0;
    v.memop_store = 0;
    v.memop_size = 0;
    v.memop_addr = 0;
#if 1
    int tinstr_idx = -1;
    int tisa = i_isa_type.read()[ISA_R_type];//.to_int();
    for (int i = 0; i < Instr_Total; i++) {
        if (wv[i].to_bool()) {
            tinstr_idx = i;
            break;
        }
    }
    if (i_d_pc.read() >= 0x1010) {
        bool st = true;
    }
#endif

    if (i_isa_type.read()[ISA_R_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
    } else if (i_isa_type.read()[ISA_I_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = 0;
        wb_rdata2 = i_d_instr.read().range(31, 20);
        if (wb_rdata2.bit(11)) {
            wb_rdata2(31, 12) = ~0;
        }
    } else if (i_isa_type.read()[ISA_SB_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
        if (i_d_instr.read()[31]) {
            wb_off(31, 20) = ~0;
        } else {
            wb_off(31, 20) = 0;
        }
        wb_off(19, 12) = i_d_instr.read()(19, 12);
        wb_off[11] = i_d_instr.read()[20];
        wb_off(10, 1) = i_d_instr.read()(30, 21);
        wb_off[0] = 0;
    } else if (i_isa_type.read()[ISA_UJ_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        if (wb_rdata2.bit(31)) {
            wb_rdata2(63, 32) = ~0;
        }
    } else if (i_isa_type.read()[ISA_U_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_rdata2(31, 0) = i_d_instr.read().range(31, 12) << 12;
        if (wb_rdata2.bit(31)) {
            wb_rdata2(63, 32) = ~0;
        }
    }

    // Don't modify registers on conditional jumps:
    w_res_wena = ~(wv[Instr_BEQ] | wv[Instr_BGE] | wv[Instr_BGEU]
               | wv[Instr_BLT] | wv[Instr_BLTU] | wv[Instr_BNE]);
    if (w_res_wena.to_bool()) {
        wb_res_addr = i_d_instr.read().range(11, 7);
    } else {
        wb_res_addr = 0;
    }

    // parallel ALU:
    wb_sum64 = wb_rdata1 + wb_rdata2;
    wb_sub64 = wb_rdata1 - wb_rdata2;
    wb_and64 = wb_rdata1 & wb_rdata2;
    wb_or64 = wb_rdata1 | wb_rdata2;
    wb_xor64 = wb_rdata1 ^ wb_rdata2;
    wb_mul64 = wb_rdata1 * wb_rdata2;

    // Relative Jumps on some condition:
    w_pc_jump = (wv[Instr_BEQ] & (wb_sub64 == 0))
              || (wv[Instr_BGE] & (wb_sub64[63] == 0))
              || (wv[Instr_BGEU] & (wb_sub64[63] == wb_rdata1[63]))
              || (wv[Instr_BLT] & (wb_sub64[63] == 1))
              || (wv[Instr_BLTU] & (wb_sub64[63] != wb_rdata1[63]))
              || (wv[Instr_BEQ] & (wb_sub64 != 0))
              || wv[Instr_JAL];

    if (w_pc_jump.to_bool()) {
        wb_npc = i_d_pc.read() + wb_off;
    } else if (wv[Instr_JALR].to_bool()) {
        wb_npc = wb_rdata1 + wb_off;
        // TODO: execptions and traps:
    } else {
        wb_npc = i_d_pc.read() + 4;
    }

    // RV32 instructions list:
    w_w32 = wv[Instr_ADDW] | wv[Instr_ADDIW] 
        | wv[Instr_SLLW] | wv[Instr_SLLIW] | wv[Instr_SRAW] | wv[Instr_SRAIW]
        | wv[Instr_SRLW] | wv[Instr_SRLIW] | wv[Instr_SUBW] 
        | wv[Instr_DIVW] | wv[Instr_DIVUW] | wv[Instr_MULW]
        | wv[Instr_REMW] | wv[Instr_REMUW];


    // ALU block selector:
    if (wv[Instr_ADD] || wv[Instr_ADDI] || wv[Instr_AUIPC]) {
        wb_res = wb_sum64;
    } else if (wv[Instr_ADDW] || wv[Instr_ADDIW]) {
        wb_res(31, 0) = wb_sum64(31, 0);
        if (wb_sum64[31]) {
            wb_res(63, 32) = ~0;
        }
    } else if (wv[Instr_AND] || wv[Instr_ANDI]) {
        wb_res = wb_rdata1 & wb_rdata2;
    } else if (wv[Instr_LD]) {
        v.memop_addr = wb_rdata1 + wb_rdata2;
        v.memop_load = 1;
        v.memop_size = MEMOP_8B;
    } else if (wv[Instr_LUI]) {
        wb_res = wb_rdata2;
    } 

#if 1
    int t1 = i_cache_hold.read();
    int t2 = i_d_valid.read();
    int t3 = i_d_pc.read();
    int t4 = r.npc.read();
    if(t3 == 0x1000) {
        bool st = true;
    }
#endif
    w_hazard_detected = (wb_radr1 != 0 && (wb_radr1 == r.hazard_addr[0]
                                        || wb_radr1 == r.hazard_addr[1]))
                     || (wb_radr2 != 0 && (wb_radr2 == r.hazard_addr[0]
                                        || wb_radr2 == r.hazard_addr[1]));
    v.hazard_hold = w_hazard_detected;

    v.valid = 0;
    if (w_hazard_detected) {
        // Wait 1 or 2 clocks while register's value will be updated:
        if (i_wb_done.read()) {
            if (!r.hazard_hold.read()) {
                v.hazard_addr[1] = 0;
            }
            if (r.hazard_hold.read()) {
                v.hazard_addr[0] = 0;
            }
        }
    } else if (!i_cache_hold.read() && i_d_valid.read() 
        && i_d_pc.read() == r.npc.read()) {
        v.valid = 1;
        v.pc = i_d_pc;
        v.instr = i_d_instr;
        v.npc = wb_npc;
        v.res_addr = wb_res_addr;
        v.res_val = wb_res;
        v.hazard_addr[1] = r.hazard_addr[0];
        v.hazard_addr[0] = wb_res_addr;
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
        v.hazard_hold = 0;
        v.hazard_addr[0] = 0;
        v.hazard_addr[1] = 0;
    }

    o_radr1 = wb_radr1;
    o_radr2 = wb_radr2;
    o_res_addr = r.res_addr;
    o_res_data = r.res_val;
    o_hazard_hold = w_hazard_detected;

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

