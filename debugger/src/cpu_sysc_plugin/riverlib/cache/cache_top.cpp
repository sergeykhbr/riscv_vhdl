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
    sensitive << w_ctrl_req_mem_valid;
    sensitive << w_ctrl_req_mem_write;
    sensitive << wb_ctrl_req_mem_addr;
    sensitive << wb_ctrl_req_mem_strob;
    sensitive << wb_ctrl_req_mem_data;
    sensitive << w_data_req_mem_valid;
    sensitive << w_data_req_mem_write;
    sensitive << wb_data_req_mem_addr;
    sensitive << wb_data_req_mem_strob;
    sensitive << wb_data_req_mem_data;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    i0 = new ICache("i0", vcd);
    i0->i_clk(i_clk);
    i0->i_nrst(i_nrst);
    i0->i_req_ctrl_valid(i_req_ctrl_valid);
    i0->i_req_ctrl_addr(i_req_ctrl_addr);
    i0->o_resp_ctrl_valid(o_resp_ctrl_valid);
    i0->o_resp_ctrl_addr(o_resp_ctrl_addr);
    i0->o_resp_ctrl_data(o_resp_ctrl_data);
    i0->o_req_mem_valid(w_ctrl_req_mem_valid);
    i0->o_req_mem_write(w_ctrl_req_mem_write);
    i0->o_req_mem_addr(wb_ctrl_req_mem_addr);
    i0->o_req_mem_strob(wb_ctrl_req_mem_strob);
    i0->o_req_mem_data(wb_ctrl_req_mem_data);
    i0->i_resp_mem_data_valid(w_ctrl_resp_mem_data_valid);
    i0->i_resp_mem_data(wb_ctrl_resp_mem_data);

    d0 = new DCache("d0", vcd);
    d0->i_clk(i_clk);
    d0->i_nrst(i_nrst);
    d0->i_req_data_valid(i_req_data_valid);
    d0->i_req_data_write(i_req_data_write);
    d0->i_req_data_sz(i_req_data_size);
    d0->i_req_data_addr(i_req_data_addr);
    d0->i_req_data_data(i_req_data_data);
    d0->o_resp_data_valid(o_resp_data_valid);
    d0->o_resp_data_addr(o_resp_data_addr);
    d0->o_resp_data_data(o_resp_data_data);
    d0->o_req_mem_valid(w_data_req_mem_valid);
    d0->o_req_mem_write(w_data_req_mem_write);
    d0->o_req_mem_addr(wb_data_req_mem_addr);
    d0->o_req_mem_strob(wb_data_req_mem_strob);
    d0->o_req_mem_data(wb_data_req_mem_data);
    d0->i_resp_mem_data_valid(w_data_resp_mem_data_valid);
    d0->i_resp_mem_data(wb_data_resp_mem_data);


    if (vcd) {
        sc_trace(vcd, w_ctrl_req_mem_valid, "/top/cache0/i_ctrl_req_mem_valid");
        sc_trace(vcd, w_ctrl_req_mem_write, "/top/cache0/i_ctrl_req_mem_write");
        sc_trace(vcd, wb_ctrl_req_mem_addr, "/top/cache0/i_ctrl_req_mem_addr");
        sc_trace(vcd, wb_ctrl_req_mem_strob, "/top/cache0/i_ctrl_req_mem_strob");
        sc_trace(vcd, wb_ctrl_req_mem_data, "/top/cache0/i_ctrl_req_mem_data");
        sc_trace(vcd, i_req_data_valid, "/top/cache0/i_req_data_valid");
        sc_trace(vcd, i_req_data_write, "/top/cache0/i_req_data_write");
        sc_trace(vcd, i_req_data_addr, "/top/cache0/i_req_data_addr");
        sc_trace(vcd, i_req_data_data, "/top/cache0/i_req_data_data");

        sc_trace(vcd, w_data_resp_mem_data_valid, "/top/cache0/w_data_resp_mem_data_valid");
        sc_trace(vcd, wb_data_resp_mem_data, "/top/cache0/wb_data_resp_mem_data");
        sc_trace(vcd, w_ctrl_resp_mem_data_valid, "/top/cache0/w_ctrl_resp_mem_data_valid");
        sc_trace(vcd, wb_ctrl_resp_mem_data, "/top/cache0/wb_ctrl_resp_mem_data");

        sc_trace(vcd, o_req_mem_valid, "/top/cache0/o_req_mem_valid");
        sc_trace(vcd, o_req_mem_write, "/top/cache0/o_req_mem_write");
        sc_trace(vcd, o_req_mem_addr, "/top/cache0/o_req_mem_addr");
        sc_trace(vcd, o_req_mem_strob, "/top/cache0/o_req_mem_strob");
        sc_trace(vcd, o_req_mem_data, "/top/cache0/o_req_mem_data");
        sc_trace(vcd, i_resp_mem_data_valid, "/top/cache0/i_resp_mem_data_valid");
        sc_trace(vcd, i_resp_mem_data, "/top/cache0/i_resp_mem_data");
        sc_trace(vcd, o_hold, "/top/cache0/o_hold");

        sc_trace(vcd, r.state, "/top/cache0/r.state");
    }
};

CacheTop::~CacheTop() {
    delete i0;
}


void CacheTop::comb() {
    bool w_mem_valid;
    bool w_mem_write;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<BUS_DATA_BYTES> wb_mem_strob;
    sc_uint<BUS_DATA_WIDTH> wb_mem_wdata;
    bool w_hold;

    v = r;

    w_data_resp_mem_data_valid = 0;
    wb_data_resp_mem_data = 0;
    w_ctrl_resp_mem_data_valid = 0;
    wb_ctrl_resp_mem_data = 0;
    switch (r.state.read()) {
    case State_Idle:
        break;
    case State_DMem:
        w_data_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_data_resp_mem_data = i_resp_mem_data;
        break;
    case State_IMem:
        w_ctrl_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_ctrl_resp_mem_data = i_resp_mem_data;
        break;
    default:;
    }

    w_mem_valid = w_ctrl_req_mem_valid.read() | w_data_req_mem_valid.read();
    if (w_data_req_mem_valid.read()) {
        v.state = State_DMem;
        w_mem_write = w_data_req_mem_write;
        wb_mem_addr = wb_data_req_mem_addr;
        wb_mem_strob = wb_data_req_mem_strob;
        wb_mem_wdata = wb_data_req_mem_data;
    } else if (w_ctrl_req_mem_valid.read()) {
        v.state = State_IMem;
        w_mem_write = w_ctrl_req_mem_write;
        wb_mem_addr = wb_ctrl_req_mem_addr;
        wb_mem_strob = wb_ctrl_req_mem_strob;
        wb_mem_wdata = wb_ctrl_req_mem_data;
    } else {
        v.state = State_Idle;
        w_mem_write = 0;
        wb_mem_addr = 0;
        wb_mem_strob = 0;
        wb_mem_wdata = 0;
    }

    w_hold = 0;
    if ((!i_resp_mem_data_valid.read() && w_mem_valid && r.state.read() != State_Idle) 
        || (w_ctrl_req_mem_valid.read() && w_data_req_mem_valid.read())) {
        w_hold = 1;
    }

    if (!i_nrst.read()) {
        v.state = State_Idle;
    }

    o_req_mem_valid = w_mem_valid;
    o_req_mem_write = w_mem_write;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_strob = wb_mem_strob;
    o_req_mem_data = wb_mem_wdata;
    o_hold = w_hold;
}

void CacheTop::registers() {
    r = v;
}

}  // namespace debugger

