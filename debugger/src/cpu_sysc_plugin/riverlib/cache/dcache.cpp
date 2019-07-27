/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Data Cache.
 */

#include "dcache.h"

namespace debugger {

DCache::DCache(sc_module_name name_, bool async_reset) : sc_module(name_) {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_data_valid;
    sensitive << i_req_data_write;
    sensitive << i_req_data_sz;
    sensitive << i_req_data_addr;
    sensitive << i_req_data_data;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_mem_store_fault;
    sensitive << i_req_mem_ready;
    sensitive << i_resp_data_ready;
    sensitive << r.req_strob;
    sensitive << r.req_wdata;
    sensitive << r.dline_data;
    sensitive << r.dline_addr_req;
    sensitive << r.dline_size_req;
    sensitive << r.dline_load_fault;
    sensitive << r.dline_store_fault;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void DCache::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_data_valid, "/top/cache0/d0/i_req_data_valid");
        sc_trace(o_vcd, i_req_data_write, "/top/cache0/d0/i_req_data_write");
        sc_trace(o_vcd, i_req_data_sz, "/top/cache0/d0/i_req_data_sz");
        sc_trace(o_vcd, i_req_data_addr, "/top/cache0/d0/i_req_data_addr");
        sc_trace(o_vcd, i_req_data_data, "/top/cache0/d0/i_req_data_data");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/d0/o_req_mem_addr");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/d0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_write, "/top/cache0/d0/o_req_mem_write");
        sc_trace(o_vcd, o_req_mem_strob, "/top/cache0/d0/o_req_mem_strob");
        sc_trace(o_vcd, o_req_mem_data, "/top/cache0/d0/o_req_mem_data");
        sc_trace(o_vcd, o_resp_data_valid, "/top/cache0/d0/o_resp_data_valid");
        sc_trace(o_vcd, o_resp_data_addr, "/top/cache0/d0/o_resp_data_addr");
        sc_trace(o_vcd, o_resp_data_data, "/top/cache0/d0/o_resp_data_data");
        sc_trace(o_vcd, r.req_strob, "/top/cache0/d0/r_req_strob");
        sc_trace(o_vcd, r.req_wdata, "/top/cache0/d0/r_req_wdata");
        sc_trace(o_vcd, r.dline_data, "/top/cache0/d0/r_dline_data");
        sc_trace(o_vcd, r.dline_addr_req, "/top/cache0/d0/r_dline_addr_req");
        sc_trace(o_vcd, r.dline_size_req, "/top/cache0/d0/r_dline_size_req");
        sc_trace(o_vcd, r.state, "/top/cache0/d0/r_state");
        sc_trace(o_vcd, w_wait_response, "/top/cache0/d0/w_wait_response");    }
}

void DCache::comb() {
    bool w_o_req_data_ready;
    bool w_o_req_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_req_mem_addr;
    sc_uint<BUS_DATA_BYTES> wb_o_req_strob;
    sc_uint<BUS_DATA_WIDTH> wb_o_req_wdata;
    bool w_req_fire;
    bool w_o_resp_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_resp_addr;
    sc_uint<BUS_DATA_WIDTH> wb_resp_data_mux;
    sc_uint<BUS_DATA_WIDTH> wb_o_resp_data;
    bool w_o_resp_load_fault;
    bool w_o_resp_store_fault;
    sc_uint<BUS_DATA_WIDTH> wb_rtmp;

    v = r;

    wb_o_req_strob = 0;
    wb_o_req_wdata = 0;
    wb_o_resp_data = 0;
    wb_rtmp = 0;

    w_wait_response = 0;
    if (r.state.read() == State_WaitResp && i_resp_mem_data_valid.read() == 0) {
        w_wait_response = 1;
    }

    switch (i_req_data_sz.read()) {
    case 0:
        wb_o_req_wdata = (i_req_data_data.read()(7, 0),
            i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
            i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
            i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
            i_req_data_data.read()(7, 0));
        if (i_req_data_addr.read()(2, 0) == 0x0) {
            wb_o_req_strob = 0x01;
        } else if (i_req_data_addr.read()(2, 0) == 0x1) {
            wb_o_req_strob = 0x02;
        } else if (i_req_data_addr.read()(2, 0) == 0x2) {
            wb_o_req_strob = 0x04;
        } else if (i_req_data_addr.read()(2, 0) == 0x3) {
            wb_o_req_strob = 0x08;
        } else if (i_req_data_addr.read()(2, 0) == 0x4) {
            wb_o_req_strob = 0x10;
        } else if (i_req_data_addr.read()(2, 0) == 0x5) {
            wb_o_req_strob = 0x20;
        } else if (i_req_data_addr.read()(2, 0) == 0x6) {
            wb_o_req_strob = 0x40;
        } else if (i_req_data_addr.read()(2, 0) == 0x7) {
            wb_o_req_strob = 0x80;
        }
        break;
    case 1:
        wb_o_req_wdata = (i_req_data_data.read()(15, 0),
            i_req_data_data.read()(15, 0), i_req_data_data.read()(15, 0),
            i_req_data_data.read()(15, 0));
        if (i_req_data_addr.read()(2, 1) == 0) {
            wb_o_req_strob = 0x03;
        } else if (i_req_data_addr.read()(2, 1) == 1) {
            wb_o_req_strob = 0x0C;
        } else if (i_req_data_addr.read()(2, 1) == 2) {
            wb_o_req_strob = 0x30;
        } else {
            wb_o_req_strob = 0xC0;
        }
        break;
    case 2:
        wb_o_req_wdata = (i_req_data_data.read()(31, 0),
                    i_req_data_data.read()(31, 0));
        if (i_req_data_addr.read()[2]) {
            wb_o_req_strob = 0xF0;
        } else {
            wb_o_req_strob = 0x0F;
        }
        break;
    case 3:
        wb_o_req_wdata = i_req_data_data;
        wb_o_req_strob = 0xFF;
        break;
    default:;
    }

    w_o_req_mem_valid = i_req_data_valid.read() && !w_wait_response;
    wb_o_req_mem_addr = i_req_data_addr.read()(BUS_ADDR_WIDTH-1, 3) << 3;
    w_o_req_data_ready = i_req_mem_ready.read();
    w_req_fire = w_o_req_mem_valid && w_o_req_data_ready;
    switch (r.state.read()) {
    case State_Idle:
        if (i_req_data_valid.read()) {
            if (i_req_mem_ready.read()) {
                v.state = State_WaitResp;
            } else {
                v.state = State_WaitGrant;
            }
        }
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            v.state = State_WaitResp;
        }
        break;
    case State_WaitResp:
        if (i_resp_mem_data_valid.read()) {
            if (!i_resp_data_ready.read()) {
                v.state = State_WaitAccept;
            } else if (!i_req_data_valid.read()) {
                v.state = State_Idle;
            } else {
                // New request
                if (i_req_mem_ready.read()) {
                    v.state = State_WaitResp;
                } else {
                    v.state = State_WaitGrant;
                }
            }
        }
        break;
    case State_WaitAccept:
        if (i_resp_data_ready.read()) {
            if (!i_req_data_valid.read()) {
                v.state = State_Idle;
            } else {
                if (i_req_mem_ready.read()) {
                    v.state = State_WaitResp;
                } else {
                    v.state = State_WaitGrant;
                }
            }
        }
        break;
    default:;
    }

    if (w_req_fire) {
        v.dline_addr_req = i_req_data_addr;
        v.dline_size_req = i_req_data_sz;
        v.req_strob = wb_o_req_strob;
        v.req_wdata = wb_o_req_wdata;
    }
    if (i_resp_mem_data_valid.read()) {
        v.dline_data =  i_resp_mem_data;
        v.dline_load_fault = i_resp_mem_load_fault;
        v.dline_store_fault = i_resp_mem_store_fault;
    }

    wb_o_resp_addr = r.dline_addr_req;
    if (r.state.read() == State_WaitAccept) {
        w_o_resp_valid = 1;
        wb_resp_data_mux = r.dline_data;
        w_o_resp_load_fault = r.dline_load_fault;
        w_o_resp_store_fault = r.dline_store_fault;
    } else {
        w_o_resp_valid = i_resp_mem_data_valid;
        wb_resp_data_mux = i_resp_mem_data;
        w_o_resp_load_fault = i_resp_mem_load_fault;
        w_o_resp_store_fault = i_resp_mem_store_fault;
    }

    switch (r.dline_addr_req.read()(2, 0)) {
    case 1:
        wb_rtmp = wb_resp_data_mux(63, 8);
        break;
    case 2:
        wb_rtmp = wb_resp_data_mux(63, 16);
        break;
    case 3:
        wb_rtmp = wb_resp_data_mux(63, 24);
        break;
    case 4:
        wb_rtmp = wb_resp_data_mux(63, 32);
        break;
    case 5:
        wb_rtmp = wb_resp_data_mux(63, 40);
        break;
    case 6:
        wb_rtmp = wb_resp_data_mux(63, 48);
        break;
    case 7:
        wb_rtmp = wb_resp_data_mux(63, 56);
        break;
    default:
        wb_rtmp = wb_resp_data_mux;
    } 

    switch (r.dline_size_req.read()) {
    case 0:
        wb_o_resp_data = wb_rtmp(7, 0);
        break;
    case 1:
        wb_o_resp_data = wb_rtmp(15, 0);
        break;
    case 2:
        wb_o_resp_data = wb_rtmp(31, 0);
        break;
    default:
        wb_o_resp_data = wb_rtmp;
    }
    
    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_req_data_ready = w_o_req_data_ready;

    o_req_mem_valid = w_o_req_mem_valid;
    o_req_mem_addr = wb_o_req_mem_addr;
    o_req_mem_write = i_req_data_write;
    o_req_mem_strob = r.req_strob;
    o_req_mem_data = r.req_wdata;

    o_resp_data_valid = w_o_resp_valid;
    o_resp_data_data = wb_o_resp_data;
    o_resp_data_addr = wb_o_resp_addr;
    o_resp_data_load_fault = w_o_resp_load_fault;
    o_resp_data_store_fault = w_o_resp_store_fault;
    o_dstate = r.state;
}

void DCache::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

