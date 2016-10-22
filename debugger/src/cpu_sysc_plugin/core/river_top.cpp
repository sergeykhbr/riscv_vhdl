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
    sensitive << rb_pc;
    sensitive << rb_timer;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, rb_pc, "r.pc");
    }
};


void RiverTop::proc0() {
    if (i_resp_mem_ready.read()) {
        wb_pc = rb_pc.read() + 4;
    }
    bool request_ena = 0;
    if ((rb_timer.read() % 8) == 7) {
        request_ena = 1;
    }

    o_timer.write(rb_timer);
    o_req_mem_valid.write(request_ena);
    o_req_mem_write.write(false);
    o_req_mem_addr.write(rb_pc.read());
    o_req_mem_strob.write(0);
    o_req_mem_data.write(0);
}

void RiverTop::registers() {
    if (!i_nrst.read()) {
        rb_pc.write(0);
        rb_timer.write(0);
    } else {
        rb_pc.write(wb_pc);
        rb_timer.write(rb_timer.read() + 1);
    }
}

}  // namespace debugger

