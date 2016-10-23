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
    SC_METHOD(proc0);
    sensitive << i_resp_mem_ready;
    sensitive << i_resp_mem_data;
    sensitive << rb_timer;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    core0 = new Processor("core0", vcd);
    core0->i_clk(i_clk);
    core0->i_nrst(i_nrst);
    core0->o_req_ctrl_valid(w_req_ctrl_valid);
    core0->o_req_ctrl_addr(wb_req_ctrl_addr);
    core0->i_resp_ctrl_ready(w_resp_ctrl_ready);
    core0->i_resp_ctrl_data(wb_resp_ctrl_data);
    core0->o_req_data_valid(w_req_data_valid);
    core0->o_req_data_write(w_req_data_write);
    core0->o_req_data_addr(wb_req_data_addr);
    core0->o_req_data_size(wb_req_data_size);
    core0->o_req_data_data(wb_req_data_data);
    core0->i_resp_data_ready(w_resp_data_ready);
    core0->i_resp_data_data(wb_resp_data_data);

    cache0 = new CacheTop("cache0", vcd);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_ctrl_valid(w_req_ctrl_valid);
    cache0->i_req_ctrl_addr(wb_req_ctrl_addr);
    cache0->o_resp_ctrl_ready(w_resp_ctrl_ready);
    cache0->o_resp_ctrl_data(wb_resp_ctrl_data);
    cache0->i_req_data_valid(w_req_data_valid);
    cache0->i_req_data_write(w_req_data_write);
    cache0->i_req_data_addr(wb_req_data_addr);
    cache0->i_req_data_size(wb_req_data_size);
    cache0->i_req_data_data(wb_req_data_data);
    cache0->o_resp_data_ready(w_resp_data_ready);
    cache0->o_resp_data_data(wb_resp_data_data);
    cache0->o_req_mem_valid(o_req_mem_valid);
    cache0->o_req_mem_write(o_req_mem_write);
    cache0->o_req_mem_addr(o_req_mem_addr);
    cache0->o_req_mem_strob(o_req_mem_strob);
    cache0->o_req_mem_data(o_req_mem_data);
    cache0->i_resp_mem_ready(i_resp_mem_ready);
    cache0->i_resp_mem_data(i_resp_mem_data);


    if (vcd) {
        sc_trace(vcd, rb_timer, "rb_timer");
    }
};

RiverTop::~RiverTop() {
    delete cache0;
    delete core0;
}

void RiverTop::proc0() {

    o_timer = rb_timer;
}

void RiverTop::registers() {
    if (!i_nrst.read()) {
        rb_timer.write(0);
    } else {
        rb_timer.write(rb_timer.read() + 1);
    }
}

}  // namespace debugger

