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
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    i0 = new ICache("i0", vcd);
    i0->i_clk(i_clk);
    i0->i_nrst(i_nrst);
    i0->i_req_ctrl_valid(i_req_ctrl_valid);
    i0->o_req_ctrl_ready(o_req_ctrl_ready);
    i0->i_req_ctrl_addr(i_req_ctrl_addr);
    i0->o_resp_ctrl_valid(o_resp_ctrl_valid);
    i0->o_resp_ctrl_addr(o_resp_ctrl_addr);
    i0->o_resp_ctrl_data(o_resp_ctrl_data);
    i0->o_req_mem_valid(o_req_mem_valid);
    i0->o_req_mem_write(o_req_mem_write);
    i0->o_req_mem_addr(o_req_mem_addr);
    i0->o_req_mem_strob(o_req_mem_strob);
    i0->o_req_mem_data(o_req_mem_data);
    i0->i_resp_mem_data_valid(i_resp_mem_data_valid);
    i0->i_resp_mem_data(i_resp_mem_data);


    if (vcd) {
        sc_trace(vcd, r.state, "/top/cache0/r.state");
    }
};

CacheTop::~CacheTop() {
    delete i0;
}


void CacheTop::comb() {
    // Instruction cache:
    bool w_mem_valid = false;
    bool w_mem_write = false;
    sc_uint<AXI_ADDR_WIDTH> wb_mem_addr;
    sc_uint<AXI_DATA_BYTES> wb_mem_strob;
    sc_uint<AXI_DATA_WIDTH> wb_mem_data;

    
    v = r;

    switch (r.state.read()) {
    case State_Idle:
        if (i_req_data_valid.read()) {
            v.state = State_DMem;
        } else if (i_req_ctrl_valid.read()) {
            v.state = State_IMem;
            w_mem_valid = true;
            wb_mem_addr = i_req_ctrl_addr.read() & ~0x7;
            wb_mem_strob = 0;
            wb_mem_data = 0;
        }
        break;
    case State_DMem:
        break;
    case State_IMem:
        if (i_req_data_valid.read()) {
            v.state = State_DMem;
            // todo:
        } else if (i_req_ctrl_valid.read()) {
            w_mem_valid = true;
            wb_mem_addr = i_req_ctrl_addr.read() & ~0x7;
        } else {
            v.state = State_Idle;
        }
        break;
    default:;
    }

    if (!i_nrst.read()) {
        v.state = State_Idle;
    }

    /*o_req_mem_valid = w_mem_valid;
    o_req_mem_write = w_mem_write;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_strob = wb_mem_strob;
    o_req_mem_data = wb_mem_data;

    o_req_ctrl_addr_ready = i_req_ctrl_addr_valid;
    o_resp_ctrl_data_valid = w_ivalid;
    o_resp_ctrl_data = wb_idata;*/
}

void CacheTop::registers() {
    r = v;
}

}  // namespace debugger

