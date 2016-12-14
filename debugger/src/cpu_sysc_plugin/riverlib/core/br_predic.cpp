/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Branch Predictor.
 * @details    This module gives about 5% of performance improvement (CPI)
 */

#include "br_predic.h"

namespace debugger {

BranchPredictor::BranchPredictor(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_hold;
    sensitive << i_req_mem_fire;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_addr;
    sensitive << i_resp_mem_data;
    sensitive << i_f_predic_miss;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << r.npc;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void BranchPredictor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_hold, "/top/proc0/bp0/i_hold");
        sc_trace(o_vcd, i_req_mem_fire, "/top/proc0/bp0/i_req_mem_fire");
        sc_trace(o_vcd, i_resp_mem_valid, "/top/proc0/bp0/i_resp_mem_valid");
        sc_trace(o_vcd, i_resp_mem_addr, "/top/proc0/bp0/i_resp_mem_addr");
        sc_trace(o_vcd, i_resp_mem_data, "/top/proc0/bp0/i_resp_mem_data");
        sc_trace(o_vcd, i_f_predic_miss, "/top/proc0/bp0/i_f_predic_miss");
        sc_trace(o_vcd, i_e_npc, "/top/proc0/bp0/i_e_npc");
        sc_trace(o_vcd, i_ra, "/top/proc0/bp0/i_ra");

        sc_trace(o_vcd, o_npc_predict, "/top/proc0/bp0/o_npc_predict");
        sc_trace(o_vcd, r.npc, "/top/proc0/bp0/r_npc");
    }
}

void BranchPredictor::comb() {
    v = r;
    sc_uint<32> wb_tmp;
    sc_uint<BUS_ADDR_WIDTH> wb_npc;
    sc_uint<BUS_ADDR_WIDTH> wb_off;

    wb_tmp = i_resp_mem_data.read();
    if (i_f_predic_miss.read()) {
        wb_npc = i_e_npc.read() + 4;
    } else {
        wb_npc = r.npc.read() + 4;
    }


    if (wb_tmp[31]) {
        wb_off(BUS_ADDR_WIDTH-1, 20) = ~0;
    } else {
        wb_off(BUS_ADDR_WIDTH-1, 20) = 0;
    }
    wb_off(19, 12) = wb_tmp(19, 12);
    wb_off[11] = wb_tmp[20];
    wb_off(10, 1) = wb_tmp(30, 21);
    wb_off[0] = 0;

    if (i_resp_mem_valid.read()) {
        if (wb_tmp == 0x00008067) {
            // ret pseudo-instruction: Dhry score 34816 -> 35136
            v.npc = i_ra.read()(BUS_ADDR_WIDTH-1, 0);
        } else if (wb_tmp(6, 0) == 0x6f) {
            // jal instruction: Dhry score 35136 -> 36992
            v.npc = i_resp_mem_addr.read() + wb_off;
        } else if (i_req_mem_fire.read()) {
            v.npc = wb_npc;
        }
    } else if (i_req_mem_fire.read()) {
        v.npc = wb_npc;
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

