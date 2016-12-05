/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Branch Predictor.
 */

#include "br_predic.h"

namespace debugger {

BranchPredictor::BranchPredictor(sc_module_name name_, sc_trace_file *vcd)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_hold;
    sensitive << i_f_mem_request;
    sensitive << i_f_predic_miss;
    sensitive << i_f_instr_valid;
    sensitive << i_f_instr;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << r.npc;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_hold, "/top/proc0/bp0/i_hold");
        sc_trace(vcd, i_f_mem_request, "/top/proc0/bp0/i_f_mem_request");
        sc_trace(vcd, i_f_predic_miss, "/top/proc0/bp0/i_f_predic_miss");
        sc_trace(vcd, i_f_instr_valid, "/top/proc0/bp0/i_f_instr_valid");
        sc_trace(vcd, i_f_instr, "/top/proc0/bp0/i_f_instr");
        sc_trace(vcd, i_e_npc, "/top/proc0/bp0/i_e_npc");

        sc_trace(vcd, o_npc_predict, "/top/proc0/bp0/o_npc_predict");
        sc_trace(vcd, r.npc, "/top/proc0/bp0/r_npc");
    }
};


void BranchPredictor::comb() {
    v = r;
    if (i_f_mem_request.read()) {
        if (i_f_predic_miss.read() && !i_hold.read()) {
            v.npc = i_e_npc.read() + 4;
        } else {
            // todo: JAL and JALR ra (return)
            v.npc = r.npc.read() + 4;
        }
    }

    if (!i_nrst.read()) {
        v.npc = RESET_VECTOR;
    }

    o_npc_predict = r.npc;
}

void BranchPredictor::registers() {
    r = v;
}

}  // namespace debugger

