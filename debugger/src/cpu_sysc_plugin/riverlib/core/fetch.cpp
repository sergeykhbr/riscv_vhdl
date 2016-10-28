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
    sensitive << i_hold;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_e_npc_valid;
    sensitive << i_e_npc;
    sensitive << i_predict_npc;
    sensitive << r.pc[0];

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_mem_data_valid, "/top/proc0/fetch0/i_mem_data_valid");
        sc_trace(vcd, i_mem_data_addr, "/top/proc0/fetch0/i_mem_data_addr");
        sc_trace(vcd, i_e_npc_valid, "/top/proc0/fetch0/i_e_npc_valid");
        sc_trace(vcd, i_e_npc, "/top/proc0/fetch0/i_e_npc");
        sc_trace(vcd, i_predict_npc, "/top/proc0/fetch0/i_predict_npc");
        sc_trace(vcd, o_mem_addr_valid, "/top/proc0/fetch0/o_mem_addr_valid");
        sc_trace(vcd, o_mem_addr, "/top/proc0/fetch0/o_mem_addr");
        sc_trace(vcd, o_valid, "/top/proc0/fetch0/o_valid");
        sc_trace(vcd, o_pc, "/top/proc0/fetch0/o_pc");
        sc_trace(vcd, o_instr, "/top/proc0/fetch0/o_instr");
        sc_trace(vcd, r.pc[2], "/top/proc0/fetch0/r.pc(2)");
        sc_trace(vcd, r.pc[1], "/top/proc0/fetch0/r.pc(1)");
        sc_trace(vcd, r.pc[0], "/top/proc0/fetch0/r.pc(0)");
        sc_trace(vcd, o_predict_miss, "/top/proc0/fetch0/o_predict_miss");
    }
};


void InstrFetch::comb() {
    v = r;

    w_mem_addr_valid = i_nrst.read();

    bool wrong_address = (i_e_npc.read() != r.pc[1].read()) 
                    && (i_e_npc.read() != r.pc[0].read())
                    && (i_e_npc.read() != i_predict_npc.read())
                    && (i_e_npc.read() != r.raddr_not_resp_yet.read());

    v.predict_miss = 0;
    if (i_e_npc_valid.read() && wrong_address) {
        wb_addr_req = (i_e_npc.read()) % 0x2000;
        v.predict_miss = 1;
    } else {
        wb_addr_req = (i_predict_npc.read()) % 0x2000;    // !!! DEBUG for a while
    }
    
    v.f_valid = 0;
    v.raddr_not_resp_yet = wb_addr_req; // Address already requested but probably not responded yet.
                                        // Avoid marking such request as 'miss'.
    if (i_mem_data_valid.read()) {
        v.f_valid = 1;
        v.instr = i_mem_data;
        v.pc[1] = r.pc[0];
        v.pc[0] = i_mem_data_addr.read();
    }

    if (!i_nrst.read()) {
        v.f_valid = 0;
        v.pc[0] = 0;
        v.pc[1] = 0;
        v.predict_miss = 0;
        v.raddr_not_resp_yet = 0;
    }

    o_mem_addr_valid = w_mem_addr_valid;
    o_mem_addr = wb_addr_req;
    o_valid = r.f_valid;
    o_pc = r.pc[0];
    o_instr = r.instr;
    o_predict_miss = r.predict_miss;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

