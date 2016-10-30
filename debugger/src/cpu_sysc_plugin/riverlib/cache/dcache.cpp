/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Data Cache.
 */

#include "dcache.h"

namespace debugger {

DCache::DCache(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_data_valid;
    sensitive << i_req_data_write;
    sensitive << i_req_data_sz;
    sensitive << i_req_data_addr;
    sensitive << i_req_data_data;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
    }
};


void DCache::comb() {
    if (o_req_mem_valid.read()) {
        v.req_addr = i_req_data_addr;
        v.req_size = i_req_data_sz;
    }

    if (!i_nrst.read()) {
        v.req_addr = 0;
        v.req_size = 0;
    }

    o_req_mem_valid = i_req_data_valid;
    o_req_mem_write = i_req_data_write;
    o_req_mem_addr = i_req_data_addr;
    o_req_mem_strob = 0xff;             // TODO!!!!!!!!
    o_req_mem_data = i_req_data_data;

    o_resp_data_valid = i_resp_mem_data_valid;
    o_resp_data_data = i_resp_mem_data;
    o_resp_data_addr = r.req_addr;
}

void DCache::registers() {
    r = v;
}

}  // namespace debugger

