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
    }
};


void BranchPredictor::comb() {
    v = r;
    if (i_f_mem_request.read() && !i_hold.read()) {
        if (i_f_predic_miss.read()) {
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

