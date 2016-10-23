/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Cache Top level.
 */

#include "cache_top.h"

namespace debugger {

CacheTop::CacheTop(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(proc0);
    sensitive << i_resp_mem_ready;
    sensitive << i_resp_mem_data;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (vcd) {
        //sc_trace(vcd, r.f.pc, "r.f.pc");
    }
};


void CacheTop::proc0() {
    o_req_mem_valid = i_req_ctrl_valid;
    o_req_mem_write.write(false);
    o_req_mem_addr = i_req_ctrl_addr;
    o_req_mem_strob.write(0);
    o_req_mem_data.write(0);

    o_resp_ctrl_ready = i_resp_mem_ready;
    uint64_t t1 = i_resp_mem_data.read();
    o_resp_ctrl_data.write(t1);
}

void CacheTop::registers() {
    if (!i_nrst.read()) {
    } else {
        //r = rin;
    }
}

}  // namespace debugger

