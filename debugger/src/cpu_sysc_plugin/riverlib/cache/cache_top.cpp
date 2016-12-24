/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Cache Top level.
 */

#include "cache_top.h"

namespace debugger {

CacheTop::CacheTop(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_ready;
    sensitive << i.req_mem_valid;
    sensitive << i.req_mem_write;
    sensitive << i.req_mem_addr;
    sensitive << i.req_mem_strob;
    sensitive << i.req_mem_wdata;
    sensitive << d.req_mem_valid;
    sensitive << d.req_mem_write;
    sensitive << d.req_mem_addr;
    sensitive << d.req_mem_strob;
    sensitive << d.req_mem_wdata;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    i0 = new ICache("i0");
    i0->i_clk(i_clk);
    i0->i_nrst(i_nrst);
    i0->i_req_ctrl_valid(i_req_ctrl_valid);
    i0->i_req_ctrl_addr(i_req_ctrl_addr);
    i0->o_req_ctrl_ready(o_req_ctrl_ready);
    i0->o_resp_ctrl_valid(o_resp_ctrl_valid);
    i0->o_resp_ctrl_addr(o_resp_ctrl_addr);
    i0->o_resp_ctrl_data(o_resp_ctrl_data);
    i0->i_resp_ctrl_ready(i_resp_ctrl_ready);
    i0->i_req_mem_ready(w_ctrl_req_ready);
    i0->o_req_mem_valid(i.req_mem_valid);
    i0->o_req_mem_write(i.req_mem_write);
    i0->o_req_mem_addr(i.req_mem_addr);
    i0->o_req_mem_strob(i.req_mem_strob);
    i0->o_req_mem_data(i.req_mem_wdata);
    i0->i_resp_mem_data_valid(w_ctrl_resp_mem_data_valid);
    i0->i_resp_mem_data(wb_ctrl_resp_mem_data);

    d0 = new DCache("d0");
    d0->i_clk(i_clk);
    d0->i_nrst(i_nrst);
    d0->i_req_data_valid(i_req_data_valid);
    d0->i_req_data_write(i_req_data_write);
    d0->i_req_data_sz(i_req_data_size);
    d0->i_req_data_addr(i_req_data_addr);
    d0->i_req_data_data(i_req_data_data);
    d0->o_req_data_ready(o_req_data_ready);
    d0->o_resp_data_valid(o_resp_data_valid);
    d0->o_resp_data_addr(o_resp_data_addr);
    d0->o_resp_data_data(o_resp_data_data);
    d0->i_resp_data_ready(i_resp_data_ready);
    d0->i_req_mem_ready(w_data_req_ready);
    d0->o_req_mem_valid(d.req_mem_valid);
    d0->o_req_mem_write(d.req_mem_write);
    d0->o_req_mem_addr(d.req_mem_addr);
    d0->o_req_mem_strob(d.req_mem_strob);
    d0->o_req_mem_data(d.req_mem_wdata);
    d0->i_resp_mem_data_valid(w_data_resp_mem_data_valid);
    d0->i_resp_mem_data(wb_data_resp_mem_data);
};

void CacheTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_data_valid, "/top/cache0/i_req_data_valid");
        sc_trace(o_vcd, i_req_data_write, "/top/cache0/i_req_data_write");
        sc_trace(o_vcd, i_req_data_addr, "/top/cache0/i_req_data_addr");
        sc_trace(o_vcd, i_req_data_data, "/top/cache0/i_req_data_data");

        sc_trace(o_vcd, w_data_resp_mem_data_valid, "/top/cache0/w_data_resp_mem_data_valid");
        sc_trace(o_vcd, wb_data_resp_mem_data, "/top/cache0/wb_data_resp_mem_data");
        sc_trace(o_vcd, w_ctrl_resp_mem_data_valid, "/top/cache0/w_ctrl_resp_mem_data_valid");
        sc_trace(o_vcd, wb_ctrl_resp_mem_data, "/top/cache0/wb_ctrl_resp_mem_data");

        sc_trace(o_vcd, i_req_mem_ready, "/top/cache0/i_req_mem_ready");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_write, "/top/cache0/o_req_mem_write");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/o_req_mem_addr");
        sc_trace(o_vcd, o_req_mem_strob, "/top/cache0/o_req_mem_strob");
        sc_trace(o_vcd, o_req_mem_data, "/top/cache0/o_req_mem_data");
        sc_trace(o_vcd, i_resp_mem_data_valid, "/top/cache0/i_resp_mem_data_valid");
        sc_trace(o_vcd, i_resp_mem_data, "/top/cache0/i_resp_mem_data");

        sc_trace(o_vcd, r.state, "/top/cache0/r.state");
    }
    i0->generateVCD(i_vcd, o_vcd);
    d0->generateVCD(i_vcd, o_vcd);
}

CacheTop::~CacheTop() {
    delete i0;
    delete d0;
}


void CacheTop::comb() {
    bool w_mem_write;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<BUS_DATA_BYTES> wb_mem_strob;
    sc_uint<BUS_DATA_WIDTH> wb_mem_wdata;

    v = r;

    w_mem_write = 0;
    wb_mem_addr = 0;
    wb_mem_strob = 0;
    wb_mem_wdata = 0;

    w_data_req_ready = 0;
    w_data_resp_mem_data_valid = 0;
    wb_data_resp_mem_data = 0;
    w_ctrl_req_ready = 0;
    w_ctrl_resp_mem_data_valid = 0;
    wb_ctrl_resp_mem_data = 0;

    switch (r.state.read()) {
    case State_Idle:
        if (d.req_mem_valid.read()) {
            w_data_req_ready = i_req_mem_ready;
            w_mem_write = d.req_mem_write;
            wb_mem_addr = d.req_mem_addr;
            wb_mem_strob = d.req_mem_strob;
            wb_mem_wdata = d.req_mem_wdata;
            if (i_req_mem_ready.read()) {
                v.state = State_DMem;
            }
        } else if (i.req_mem_valid.read()) {
            w_ctrl_req_ready = i_req_mem_ready;
            w_mem_write = i.req_mem_write;
            wb_mem_addr = i.req_mem_addr;
            wb_mem_strob = i.req_mem_strob;
            wb_mem_wdata = i.req_mem_wdata;
            if (i_req_mem_ready.read()) {
                v.state = State_IMem;
            }
        }
        break;

    case State_DMem:
        w_data_req_ready = i_req_mem_ready;
        w_mem_write = d.req_mem_write;
        wb_mem_addr = d.req_mem_addr;
        wb_mem_strob = d.req_mem_strob;
        wb_mem_wdata = d.req_mem_wdata;
        if (i_resp_mem_data_valid.read()) {
            if (!d.req_mem_valid.read() && i.req_mem_valid.read()) {
                v.state = State_IMem;
                w_data_req_ready = 0;
                w_ctrl_req_ready = i_req_mem_ready;
                w_mem_write = i.req_mem_write;
                wb_mem_addr = i.req_mem_addr;
                wb_mem_strob = i.req_mem_strob;
                wb_mem_wdata = i.req_mem_wdata;
            } else if (!(d.req_mem_valid | i.req_mem_valid)) {
                v.state = State_Idle;
            }
        }
        w_data_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_data_resp_mem_data = i_resp_mem_data;
        break;

    case State_IMem:
        w_ctrl_req_ready = i_req_mem_ready;
        w_mem_write = i.req_mem_write;
        wb_mem_addr = i.req_mem_addr;
        wb_mem_strob = i.req_mem_strob;
        wb_mem_wdata = i.req_mem_wdata;
        if (i_resp_mem_data_valid.read()) {
            if (d.req_mem_valid.read()) {
                v.state = State_DMem;
                w_data_req_ready = i_req_mem_ready;
                w_ctrl_req_ready = 0;
                w_mem_write = d.req_mem_write;
                wb_mem_addr = d.req_mem_addr;
                wb_mem_strob = d.req_mem_strob;
                wb_mem_wdata = d.req_mem_wdata;
            } else if (!(d.req_mem_valid | i.req_mem_valid)) {
                v.state = State_Idle;
            }
        }
        w_ctrl_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_ctrl_resp_mem_data = i_resp_mem_data;
        break;
    default:;
    }

    if (!i_nrst.read()) {
        v.state = State_Idle;
    }

    o_req_mem_valid = i.req_mem_valid | d.req_mem_valid;
    o_req_mem_write = w_mem_write;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_strob = wb_mem_strob;
    o_req_mem_data = wb_mem_wdata;
}

void CacheTop::registers() {
    r = v;
}

}  // namespace debugger

