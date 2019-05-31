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
    sensitive << i_mem_load_fault;
    sensitive << i_e_npc;
    sensitive << i_predict_npc;
    sensitive << i_predict;
    sensitive << i_minus2;
    sensitive << i_minus4;
    sensitive << i_br_fetch_valid;
    sensitive << i_br_address_fetch;
    sensitive << i_br_instr_fetch;
    sensitive << r.pc_z1;
    sensitive << r.raddr_not_resp_yet;
    sensitive << r.wait_resp;
    sensitive << r.pipeline_init;
    sensitive << r.br_address;
    sensitive << r.br_instr;
    sensitive << r.resp_address;
    sensitive << r.resp_data;
    sensitive << r.resp_valid;
    sensitive << r.resp_address_z;
    sensitive << r.resp_data_z;
    sensitive << r.minus2;
    sensitive << r.minus4;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void InstrFetch::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mem_data_valid, "/top/proc0/fetch0/i_mem_data_valid");
        sc_trace(o_vcd, i_mem_data_addr, "/top/proc0/fetch0/i_mem_data_addr");
        sc_trace(o_vcd, i_mem_data, "/top/proc0/fetch0/i_mem_data");
        sc_trace(o_vcd, o_mem_resp_ready, "/top/proc0/fetch0/o_mem_resp_ready");
        sc_trace(o_vcd, i_e_npc, "/top/proc0/fetch0/i_e_npc");
        sc_trace(o_vcd, i_predict_npc, "/top/proc0/fetch0/i_predict_npc");
        sc_trace(o_vcd, i_pipeline_hold, "/top/proc0/fetch0/i_pipeline_hold");
        sc_trace(o_vcd, o_mem_addr_valid, "/top/proc0/fetch0/o_mem_addr_valid");
        sc_trace(o_vcd, o_mem_addr, "/top/proc0/fetch0/o_mem_addr");
        sc_trace(o_vcd, i_mem_req_ready, "/top/proc0/fetch0/i_mem_req_ready");
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
    bool w_o_req_valid;
    bool w_o_req_fire;
    bool w_o_hold;
    sc_uint<BUS_ADDR_WIDTH> wb_o_pc;
    sc_uint<32> wb_o_instr;

    v = r;

    w_o_req_valid = i_nrst.read() & !i_pipeline_hold.read()
            & !(r.wait_resp.read() & !i_mem_data_valid.read());
    w_o_req_fire =  i_mem_req_ready.read() && w_o_req_valid;

    w_o_hold = !(r.wait_resp.read() && i_mem_data_valid.read());
    

    // Debug last fetched instructions buffer:
    sc_biguint<DBG_FETCH_TRACE_SIZE*64> wb_instr_buf = r.instr_buf;
    if (w_o_req_fire) {
        v.wait_resp = 1;
    } else if (i_mem_data_valid.read() == 1 && i_pipeline_hold.read() == 0) {
        v.wait_resp = 0;
    }

    if (i_br_fetch_valid.read()) {
        v.br_address = i_br_address_fetch;
        v.br_instr = i_br_instr_fetch;
    }

    if (i_mem_data_valid.read() && r.wait_resp.read() && !i_pipeline_hold.read()) {
        v.resp_valid = 1;
        v.minus2 = i_minus2.read();
        v.minus4 = i_minus4.read();
//        if i_mem_data_addr = r.br_address then
//            v.resp_address := r.br_address;
//            v.resp_data := r.br_instr;
//            v.br_address := (others => '1');
//        else
            v.resp_address = i_mem_data_addr.read();
            v.resp_data = i_mem_data.read();
//        end if;
        v.resp_address_z = r.resp_address.read();
        v.resp_data_z = r.resp_data.read();
    }

    if (i_br_fetch_valid.read() == 1) {
         v.br_address = i_br_address_fetch.read();
         v.br_instr = i_br_instr_fetch.read();
    }

    if (r.minus4.read() == 1) {
        wb_o_pc = r.resp_address_z.read();
        wb_o_instr = r.resp_data_z.read();
    } else if (r.minus2.read() == 1) {
        wb_o_pc = r.resp_address.read() - 2;
        wb_o_instr = (r.resp_data.read().range(15, 0) << 16) 
                    | r.resp_data_z.read().range(31, 16);
    } else {
        wb_o_pc = r.resp_address.read();
        wb_o_instr = r.resp_data.read();
    }

    if (!i_nrst.read()) {
        R_RESET(v);
    }

    o_mem_addr_valid = w_o_req_valid;
    o_mem_addr = i_predict_npc.read();
    o_mem_req_fire = w_o_req_fire;
    o_ex_load_fault = 0;        // todo:
    o_valid = r.resp_valid.read() && !(i_pipeline_hold.read() || w_o_hold);
    o_pc = wb_o_pc;
    o_instr = wb_o_instr;
    o_mem_resp_ready = r.wait_resp.read() && !i_pipeline_hold.read();
    o_hold = w_o_hold;
    o_instr_buf = r.instr_buf;
}

void InstrFetch::registers() {
    r = v;
}

}  // namespace debugger

