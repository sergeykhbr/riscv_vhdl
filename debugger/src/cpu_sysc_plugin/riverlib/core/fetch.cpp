/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Fetch Instruction stage.
 */

#include "fetch.h"

namespace debugger {

InstrFetch::InstrFetch(sc_module_name name_) : sc_module(name_) {
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
};

void InstrFetch::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mem_data_valid, "/top/proc0/fetch0/i_mem_data_valid");
        sc_trace(o_vcd, i_mem_data_addr, "/top/proc0/fetch0/i_mem_data_addr");
        sc_trace(o_vcd, i_mem_data, "/top/proc0/fetch0/i_mem_data");
        sc_trace(o_vcd, o_mem_resp_ready, "/top/proc0/fetch0/o_mem_resp_ready");
        sc_trace(o_vcd, i_e_npc_valid, "/top/proc0/fetch0/i_e_npc_valid");
        sc_trace(o_vcd, i_e_npc, "/top/proc0/fetch0/i_e_npc");
        sc_trace(o_vcd, i_predict_npc, "/top/proc0/fetch0/i_predict_npc");
        sc_trace(o_vcd, i_pipeline_hold, "/top/proc0/fetch0/i_pipeline_hold");
        sc_trace(o_vcd, o_mem_addr_valid, "/top/proc0/fetch0/o_mem_addr_valid");
        sc_trace(o_vcd, o_mem_addr, "/top/proc0/fetch0/o_mem_addr");
        sc_trace(o_vcd, i_mem_req_ready, "/top/proc0/fetch0/i_mem_req_ready");
        sc_trace(o_vcd, o_predict_miss, "/top/proc0/fetch0/o_predict_miss");
        sc_trace(o_vcd, o_hold, "/top/proc0/fetch0/o_hold");
        sc_trace(o_vcd, o_valid, "/top/proc0/fetch0/o_valid");
        sc_trace(o_vcd, o_pc, "/top/proc0/fetch0/o_pc");
        sc_trace(o_vcd, o_instr, "/top/proc0/fetch0/o_instr");
        sc_trace(o_vcd, r.pc_z1, "/top/proc0/fetch0/r.pc_z1");
        sc_trace(o_vcd, r.wait_resp, "/top/proc0/fetch0/r.wait_resp");
        sc_trace(o_vcd, r.raddr_not_resp_yet, "/top/proc0/fetch0/r_raddr_not_resp_yet");
    }
}

void InstrFetch::comb() {
    sc_uint<BUS_ADDR_WIDTH> wb_o_addr_req;
    bool w_predict_miss;
    bool w_o_req_valid;
    bool w_o_req_fire;
    bool w_resp_fire;
    bool w_o_mem_resp_ready;

    v = r;

    w_o_req_valid = i_nrst.read() & !i_pipeline_hold.read()
            & !(r.wait_resp.read() & !i_mem_data_valid.read());
    w_o_req_fire = w_o_req_valid && i_mem_req_ready.read();

    w_o_mem_resp_ready = !i_pipeline_hold.read();
    w_resp_fire = i_mem_data_valid.read() && w_o_mem_resp_ready;

    w_predict_miss = 1;
    if (i_e_npc == r.pc_z1 
       || i_e_npc == i_predict_npc || i_e_npc == r.raddr_not_resp_yet) {
        w_predict_miss = 0;
    }

    if (w_predict_miss) {
        wb_o_addr_req = i_e_npc.read();
    } else {
        wb_o_addr_req = i_predict_npc.read();
    }
    

    if (w_o_req_fire) {
        v.wait_resp = 1;
        v.pc_z1 = r.raddr_not_resp_yet;
        v.raddr_not_resp_yet = wb_o_addr_req;
        v.pipeline_init = (r.pipeline_init.read() << 1) | 1;
    } else if (i_mem_data_valid.read() && !w_o_req_fire && !i_pipeline_hold.read()) {
        v.wait_resp = 0;
    }


    if (!i_nrst.read()) {
        v.wait_resp = 0;
        v.pipeline_init = 0;
        v.pc_z1 = 0;
        v.raddr_not_resp_yet = 0;
    }

    o_mem_addr_valid = w_o_req_valid;
    o_mem_addr = wb_o_addr_req;
    o_mem_req_fire = w_o_req_fire;
    o_valid = w_resp_fire;
    o_pc = i_mem_data_addr;
    o_instr = i_mem_data;
    o_predict_miss = w_predict_miss;
    o_mem_resp_ready = w_o_mem_resp_ready;
    o_hold = !w_resp_fire;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

