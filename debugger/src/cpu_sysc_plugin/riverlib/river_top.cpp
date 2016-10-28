/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      "River" CPU Top level.
 */

#include "river_top.h"

namespace debugger {

RiverTop::RiverTop(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << r.timer;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    proc0 = new Processor("proc0", vcd);
    proc0->i_clk(i_clk);
    proc0->i_nrst(i_nrst);
    proc0->i_hold(w_cache_hold);
    proc0->o_req_ctrl_valid(w_req_ctrl_valid);
    proc0->o_req_ctrl_addr(wb_req_ctrl_addr);
    proc0->i_resp_ctrl_valid(w_resp_ctrl_valid);
    proc0->i_resp_ctrl_addr(wb_resp_ctrl_addr);
    proc0->i_resp_ctrl_data(wb_resp_ctrl_data);
    proc0->o_req_data_valid(w_req_data_valid);
    proc0->o_req_data_write(w_req_data_write);
    proc0->o_req_data_addr(wb_req_data_addr);
    proc0->o_req_data_size(wb_req_data_size);
    proc0->o_req_data_data(wb_req_data_data);
    proc0->i_resp_data_valid(w_resp_data_valid);
    proc0->i_resp_data_addr(wb_resp_data_addr);
    proc0->i_resp_data_data(wb_resp_data_data);

    cache0 = new CacheTop("cache0", vcd);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_ctrl_valid(w_req_ctrl_valid);
    cache0->i_req_ctrl_addr(wb_req_ctrl_addr);
    cache0->o_resp_ctrl_valid(w_resp_ctrl_valid);
    cache0->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    cache0->o_resp_ctrl_data(wb_resp_ctrl_data);
    cache0->i_req_data_valid(w_req_data_valid);
    cache0->i_req_data_write(w_req_data_write);
    cache0->i_req_data_addr(wb_req_data_addr);
    cache0->i_req_data_size(wb_req_data_size);
    cache0->i_req_data_data(wb_req_data_data);
    cache0->o_resp_data_valid(w_resp_data_valid);
    cache0->o_resp_data_addr(wb_resp_data_addr);
    cache0->o_resp_data_data(wb_resp_data_data);
    cache0->o_req_mem_valid(o_req_mem_valid);
    cache0->o_req_mem_write(o_req_mem_write);
    cache0->o_req_mem_addr(o_req_mem_addr);
    cache0->o_req_mem_strob(o_req_mem_strob);
    cache0->o_req_mem_data(o_req_mem_data);
    cache0->i_resp_mem_data_valid(i_resp_mem_data_valid);
    cache0->i_resp_mem_data(i_resp_mem_data);
    cache0->o_hold(w_cache_hold);

    if (vcd) {
        sc_trace(vcd, i_clk, "/top/i_clk");
        sc_trace(vcd, i_nrst, "/top/i_nrst");
        sc_trace(vcd, o_timer, "/top/o_timer");
        sc_trace(vcd, o_req_mem_valid, "/top/o_req_mem_valid");
        sc_trace(vcd, o_req_mem_addr, "/top/o_req_mem_addr");
        sc_trace(vcd, i_resp_mem_data_valid, "/top/i_resp_mem_data_valid");
        sc_trace(vcd, i_resp_mem_data, "/top/i_resp_mem_data");
        sc_trace(vcd, w_cache_hold, "/top/w_cache_hold");
    }
};

RiverTop::~RiverTop() {
    delete cache0;
    delete proc0;
}

void RiverTop::comb() {
    v.timer = r.timer.read() + 1;

    if (!i_nrst.read()) {
        v.timer = 0;
    }

    o_timer = r.timer;
}

void RiverTop::registers() {
    r = v;
}

}  // namespace debugger

