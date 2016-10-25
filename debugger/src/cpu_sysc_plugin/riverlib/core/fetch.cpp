/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Fetch Instruction stage.
 */

#include "fetch.h"

namespace debugger {

InstrFetch::InstrFetch(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mem_addr_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_jump_valid;
    sensitive << r.mem_addr_valid;
    sensitive << r.addr_req;
    sensitive << r.pc;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, o_mem_addr_valid, "/top/proc0/fetch0/o_mem_addr_valid");
        sc_trace(vcd, o_mem_addr, "/top/proc0/fetch0/o_mem_addr");
        sc_trace(vcd, o_f_valid, "/top/proc0/fetch0/o_f_valid");
        sc_trace(vcd, o_f_pc, "/top/proc0/fetch0/o_f_pc");
        sc_trace(vcd, o_f_instr, "/top/proc0/fetch0/o_f_instr");
    }
};


void InstrFetch::comb() {
    v = r;

    v.mem_addr_valid = true; // todo: halt and others.
    bool w_hold = false;
    if (r.mem_addr_valid_z.read() && !i_mem_data_valid.read()) {
        w_hold = true;
    }
    v.mem_addr_valid_z = r.mem_addr_valid;

    if (i_mem_addr_ready.read()) {
        if (i_jump_valid.read()) {
            v.addr_req = i_jump_pc;
        } else if (r.post_jump_valid.read()) {
            v.addr_req = r.post_jump_pc;
        } else {
            v.addr_req = (r.addr_req.read() + 4) % 0x2000;    // !!! DEBUG for a while
        }
    }

    v.f_valid = false;
    if (i_mem_data_valid.read()) {
        v.instr = i_mem_data;
        v.f_valid = true;
        v.post_jump_valid = false;
        v.pc = i_mem_data_addr;
    } else if (i_jump_valid.read()) {
        v.post_jump_valid = true;
        v.post_jump_pc = i_jump_pc;
    }

    if (!i_nrst.read()) {
        v.mem_addr_valid = false;
        v.mem_addr_valid_z = false;
        v.f_valid = false;
        v.pc = 0;
        v.addr_req = RESET_VECTOR;
        v.post_jump_valid = false;
        v.post_jump_pc = 0;
    }

    o_mem_addr_valid = r.mem_addr_valid;
    o_mem_addr = r.addr_req;
    o_f_valid = r.f_valid;
    o_f_pc = r.pc;
    o_f_instr = r.instr;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

