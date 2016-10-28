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
    sensitive << i_d_valid;
    sensitive << i_d_pc;
    sensitive << i_d_instr;
    sensitive << i_rdata1;
    sensitive << i_rdata2;
    sensitive << r.valid;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_d_valid, "/top/proc0/exec0/i_d_valid");
        sc_trace(vcd, i_d_pc, "/top/proc0/exec0/i_d_pc");
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
    }
};


void InstrExecute::comb() {
    v = r;

    sc_uint<5> wb_radr1;
    sc_uint<RISCV_ARCH> wb_rdata1;
    sc_uint<5> wb_radr2;
    sc_uint<RISCV_ARCH> wb_rdata2;
    sc_uint<5> wb_res_addr = 0;
    sc_uint<RISCV_ARCH> wb_res = 0;
    sc_uint<AXI_ADDR_WIDTH> wb_npc = i_d_pc.read() + 4;
    sc_uint<AXI_ADDR_WIDTH> wb_off;

    bool w_memop_load = 0;
    bool w_memop_store = 0;
    sc_uint<2> wb_memop_size = 0;


    if (i_isa_type[ISA_R_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
        wb_res_addr = i_d_instr.read().range(11, 7);
    } else if (i_isa_type[ISA_I_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = 0;
        wb_rdata2 = i_d_instr.read().range(31, 20);
        if (wb_rdata2.bit(11)) {
            wb_rdata2(31, 12) = ~0;
        }
    } else if (i_isa_type[ISA_SB_type]) {
        wb_radr1 = i_d_instr.read().range(19, 15);
        wb_rdata1 = i_rdata1;
        wb_radr2 = i_d_instr.read().range(24, 20);
        wb_rdata2 = i_rdata2;
        if (i_d_instr[31]) {
            wb_off(63, 12) = ~0;
        } else {
            wb_off(63, 12) = 0;
        }
        wb_off[11] = i_d_instr[7];
        wb_off(10, 5) = i_d_instr.read()(30, 25);
        wb_off(4, 1) = i_d_instr.read()(11, 8);
        wb_off[0] = 0;
    } else if (i_isa_type[ISA_U_type]) {
        wb_radr1 = 0;
        wb_rdata1 = i_d_pc;
        wb_radr2 = 0;
        wb_rdata2(31, 0) = i_d_instr.read().range(31, 12) << 12;
        if (wb_rdata2.bit(31)) {
            wb_rdata2(63, 32) = ~0;
        }
        wb_res_addr = i_d_instr.read().range(11, 7);
    }

    if (i_ivec[Instr_ADD] || i_ivec[Instr_ADDI] || i_ivec[Instr_AUIPC]) {
        wb_res = wb_rdata1 + wb_rdata2;
    } else  if (i_ivec[Instr_ADDW] || i_ivec[Instr_ADDIW]) {
        wb_res(31, 0) = wb_rdata1(31, 0) + wb_rdata2(31, 0);
        if (wb_res[31]) {
            wb_res(63, 32) = ~0;
        }
    } else if (i_ivec[Instr_ADD] || i_ivec[Instr_ADDI]) {
        wb_res = wb_rdata1 & wb_rdata2;
    } else if (i_ivec[Instr_BEQ]) {
        if (wb_rdata1 == wb_rdata2) {
            wb_npc = i_d_pc.read() + wb_off;
        }
    } else if (i_ivec[Instr_LD]) {
        wb_res_addr = wb_rdata1 + wb_rdata2;
        w_memop_load = 1;
        wb_memop_size = MEMOP_8B;
    } 


    v.valid = 0;
    if (i_d_valid.read() && i_d_pc.read() == r.npc.read()) {
        v.valid = 1;
        v.pc = i_d_pc;
        v.instr = i_d_instr;
        v.npc = wb_npc;
    }


    if (!i_nrst.read()) {
        v.valid = false;
        v.pc = 0;
        v.npc = RESET_VECTOR;
        v.instr = 0;
    }

    o_radr1 = wb_radr1;
    o_radr2 = wb_radr2;
    o_res_addr = wb_res_addr;
    o_res_data = wb_res;

    o_memop_load = w_memop_load;
    o_memop_store = w_memop_store;
    o_memop_size = wb_memop_size;

    o_valid = r.valid;
    o_pc = r.pc;
    o_npc = r.npc;
    o_instr = r.instr;
}

void InstrExecute::registers() {
    r = v;
}

}  // namespace debugger

