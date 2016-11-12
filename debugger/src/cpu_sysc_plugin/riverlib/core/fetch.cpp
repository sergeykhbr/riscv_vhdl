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
    sensitive << i_cache_hold;
    sensitive << i_pipeline_hold;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_e_npc_valid;
    sensitive << i_e_npc;
    sensitive << i_predict_npc;
    sensitive << r.pc_z0;
    sensitive << r.is_postponed;
    sensitive << r.wait_resp;
    sensitive << w_mem_addr_valid;
    //sensitive << wb_addr_req;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_mem_data_valid, "/top/proc0/fetch0/i_mem_data_valid");
        sc_trace(vcd, i_mem_data_addr, "/top/proc0/fetch0/i_mem_data_addr");
        sc_trace(vcd, i_mem_data, "/top/proc0/fetch0/i_mem_data");
        sc_trace(vcd, i_e_npc_valid, "/top/proc0/fetch0/i_e_npc_valid");
        sc_trace(vcd, i_e_npc, "/top/proc0/fetch0/i_e_npc");
        sc_trace(vcd, i_predict_npc, "/top/proc0/fetch0/i_predict_npc");
        sc_trace(vcd, i_pipeline_hold, "/top/proc0/fetch0/i_pipeline_hold");
        sc_trace(vcd, o_mem_addr_valid, "/top/proc0/fetch0/o_mem_addr_valid");
        sc_trace(vcd, o_mem_addr, "/top/proc0/fetch0/o_mem_addr");
        sc_trace(vcd, o_valid, "/top/proc0/fetch0/o_valid");
        sc_trace(vcd, o_pc, "/top/proc0/fetch0/o_pc");
        sc_trace(vcd, o_instr, "/top/proc0/fetch0/o_instr");
        sc_trace(vcd, r.pc_z1, "/top/proc0/fetch0/r.pc_z1");
        sc_trace(vcd, r.pc_z0, "/top/proc0/fetch0/r.pc_z0");
        sc_trace(vcd, r.wait_resp, "/top/proc0/fetch0/r.wait_resp");
        sc_trace(vcd, o_predict_miss, "/top/proc0/fetch0/o_predict_miss");
        sc_trace(vcd, w_mem_addr_valid, "/top/proc0/fetch0/w_mem_addr_valid");
    }
};


void InstrFetch::comb() {
    bool w_wrong_address;
    bool w_predict_miss;
    bool w_o_valid;

    v = r;

    if (!i_nrst.read() || i_pipeline_hold.read() ||
         (!i_mem_data_valid.read() && r.wait_resp.read())) {
        // Do not request new data:
        w_mem_addr_valid = 0;
    } else {
        w_mem_addr_valid = 1;
    }
    v.wait_resp = w_mem_addr_valid;

    w_wrong_address = 1;
    if (i_e_npc == r.pc_z1 || i_e_npc == r.pc_z0 || i_e_npc == i_predict_npc
       || i_e_npc == r.raddr_not_resp_yet) {
        w_wrong_address = 0;
    }

    w_predict_miss = 0;
    if (w_wrong_address) {
        wb_addr_req = i_e_npc.read();
        w_predict_miss = 1;
    } else {
        wb_addr_req = i_predict_npc.read();
    }
    
    v.raddr_not_resp_yet = wb_addr_req; // Address already requested but probably not responded yet.
                                        // Avoid marking such request as 'miss'.

    w_any_hold = i_cache_hold.read() || i_pipeline_hold.read();
    v.is_postponed = r.is_postponed & w_any_hold;
    if (i_mem_data_valid.read()) {
        v.f_valid = 1;
        if (!w_any_hold) {
            // direct transition:
            v.instr = i_mem_data;
            v.pc_z1 = r.pc_z0;
            v.pc_z0 = i_mem_data_addr.read();
        } else {
            // Postpone recieved data when gold signal down:
            v.is_postponed = 1;
            v.postponed_pc = i_mem_data_addr.read();
            v.postponed_instr = i_mem_data;
        }
    } else if (!w_any_hold) {
        if (r.is_postponed) {
            v.instr = r.postponed_instr;
            v.pc_z1 = r.pc_z0;
            v.pc_z0 = r.postponed_pc;
        } else {
            v.f_valid = 0;
        }
    }
    w_o_valid = r.f_valid.read() && !w_any_hold;

    if (!i_nrst.read()) {
        v.f_valid = 0;
        v.pc_z0 = 0;
        v.pc_z1 = 0;
        v.raddr_not_resp_yet = 0;
        v.is_postponed = 0;
        v.postponed_pc = 0;
        v.postponed_instr = 0;
        v.wait_resp = 0;
    }

    o_mem_addr_valid = w_mem_addr_valid;
    o_mem_addr = wb_addr_req;
    o_valid = w_o_valid;
    o_pc = r.pc_z0;
    o_instr = r.instr;
    o_predict_miss = w_predict_miss;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

