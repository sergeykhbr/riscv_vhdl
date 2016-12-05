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
    sensitive << i_pipeline_hold;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data;
    sensitive << i_e_npc_valid;
    sensitive << i_e_npc;
    sensitive << i_predict_npc;
    sensitive << r.pc_z1;
    sensitive << r.raddr_not_resp_yet;
    sensitive << r.wait_resp;
    sensitive << r.pipeline_init;

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
        sc_trace(vcd, o_predict_miss, "/top/proc0/fetch0/o_predict_miss");
        sc_trace(vcd, o_valid, "/top/proc0/fetch0/o_valid");
        sc_trace(vcd, o_pc, "/top/proc0/fetch0/o_pc");
        sc_trace(vcd, o_instr, "/top/proc0/fetch0/o_instr");
        sc_trace(vcd, r.pc_z1, "/top/proc0/fetch0/r.pc_z1");
        sc_trace(vcd, r.wait_resp, "/top/proc0/fetch0/r.wait_resp");
        sc_trace(vcd, r.raddr_not_resp_yet, "/top/proc0/fetch0/r_raddr_not_resp_yet");
    }
};


void InstrFetch::comb() {
    sc_uint<BUS_ADDR_WIDTH> wb_addr_req;
    bool w_wrong_address;
    bool w_predict_miss;
    bool w_req_fire;
    bool w_resp_fire;
    bool w_o_hold;
    bool w_o_mem_resp_ready;

    v = r;

    w_req_fire = i_nrst.read() && !i_pipeline_hold.read() 
                && i_mem_req_ready.read();

    w_wrong_address = 1;
    if (i_e_npc == r.pc_z1 || i_e_npc == i_mem_data_addr.read()
       || i_e_npc == i_predict_npc || i_e_npc == r.raddr_not_resp_yet) {
        w_wrong_address = 0;
    }

    w_predict_miss = 0;
    if (w_wrong_address) {
        wb_addr_req = i_e_npc.read();
        w_predict_miss = w_req_fire;
    } else {
        wb_addr_req = i_predict_npc.read();
    }
    
    if (w_req_fire) {
        v.raddr_not_resp_yet = wb_addr_req; // Address already requested but probably not responded yet.
                                            // Avoid marking such request as 'miss'.
    }

    w_resp_fire = i_mem_data_valid.read() && !i_pipeline_hold.read();
    if (w_req_fire) {
        v.wait_resp = 1;
        v.pc_z1 = i_mem_data_addr;
        v.pipeline_init = (r.pipeline_init.read() << 1) | 1;
    } else if (i_mem_data_valid.read() && !w_req_fire) {
        v.wait_resp = 0;
    }

    w_o_hold = !i_mem_req_ready.read() 
            || (r.wait_resp.read() && !i_mem_data_valid.read());
    // Signal 'i_mem_req_ready' is also used to hold-on pipeline, so
    // don't accept response if cannot send request. Maybe improved.
    w_o_mem_resp_ready = !i_pipeline_hold.read() && i_mem_req_ready.read();

    if (!i_nrst.read()) {
        v.wait_resp = 0;
        v.pipeline_init = 0;
        v.pc_z1 = 0;
        v.raddr_not_resp_yet = 0;
    }

    o_mem_addr_valid = w_req_fire;
    o_mem_addr = wb_addr_req;
    o_valid = w_resp_fire;
    o_pc = i_mem_data_addr;
    o_instr = i_mem_data;
    o_predict_miss = w_predict_miss;
    o_mem_resp_ready = w_o_mem_resp_ready;
    o_hold = w_o_hold;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

