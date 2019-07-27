/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Cache Top level.
 */

#include "cache_top.h"

namespace debugger {

CacheTop::CacheTop(sc_module_name name_, bool async_reset, int icfg) :
    sc_module(name_) {
    async_reset_ = async_reset;
    icache_cfg_ = icfg;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_ready;
    sensitive << i.req_mem_valid;
    sensitive << i.req_mem_write;
    sensitive << i.req_mem_addr;
    sensitive << i.req_mem_strob;
    sensitive << i.req_mem_wdata;
    sensitive << i.req_mem_len;
    sensitive << i.req_mem_burst;
    sensitive << i.req_mem_burst_last;
    sensitive << d.req_mem_valid;
    sensitive << d.req_mem_write;
    sensitive << d.req_mem_addr;
    sensitive << d.req_mem_strob;
    sensitive << d.req_mem_wdata;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_mem_store_fault;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (icache_cfg_ == 0) {
        i0 = new ICacheStub("i0", async_reset);
        i0->i_clk(i_clk);
        i0->i_nrst(i_nrst);
        i0->i_req_ctrl_valid(i_req_ctrl_valid);
        i0->i_req_ctrl_addr(i_req_ctrl_addr);
        i0->o_req_ctrl_ready(o_req_ctrl_ready);
        i0->o_resp_ctrl_valid(o_resp_ctrl_valid);
        i0->o_resp_ctrl_addr(o_resp_ctrl_addr);
        i0->o_resp_ctrl_data(o_resp_ctrl_data);
        i0->o_resp_ctrl_load_fault(o_resp_ctrl_load_fault);
        i0->i_resp_ctrl_ready(i_resp_ctrl_ready);
        i0->i_req_mem_ready(w_ctrl_req_ready);
        i0->o_req_mem_valid(i.req_mem_valid);
        i0->o_req_mem_write(i.req_mem_write);
        i0->o_req_mem_addr(i.req_mem_addr);
        i0->o_req_mem_strob(i.req_mem_strob);
        i0->o_req_mem_data(i.req_mem_wdata);
        i0->o_req_mem_len(i.req_mem_len);
        i0->o_req_mem_burst(i.req_mem_burst);
        i0->o_req_mem_burst_last(i.req_mem_burst_last);
        i0->i_resp_mem_data_valid(w_ctrl_resp_mem_data_valid);
        i0->i_resp_mem_data(wb_ctrl_resp_mem_data);
        i0->i_resp_mem_load_fault(w_ctrl_resp_mem_load_fault);
        i0->i_flush_address(i_flush_address);
        i0->i_flush_valid(i_flush_valid);
        i0->o_istate(o_istate);
    } else {
        i1 = new ICacheLru("i1", async_reset, CFG_IINDEX_WIDTH);
        i1->i_clk(i_clk);
        i1->i_nrst(i_nrst);
        i1->i_req_ctrl_valid(i_req_ctrl_valid);
        i1->i_req_ctrl_addr(i_req_ctrl_addr);
        i1->o_req_ctrl_ready(o_req_ctrl_ready);
        i1->o_resp_ctrl_valid(o_resp_ctrl_valid);
        i1->o_resp_ctrl_addr(o_resp_ctrl_addr);
        i1->o_resp_ctrl_data(o_resp_ctrl_data);
        i1->o_resp_ctrl_load_fault(o_resp_ctrl_load_fault);
        i1->i_resp_ctrl_ready(i_resp_ctrl_ready);
        i1->i_req_mem_ready(w_ctrl_req_ready);
        i1->o_req_mem_valid(i.req_mem_valid);
        i1->o_req_mem_write(i.req_mem_write);
        i1->o_req_mem_addr(i.req_mem_addr);
        i1->o_req_mem_strob(i.req_mem_strob);
        i1->o_req_mem_data(i.req_mem_wdata);
        i1->o_req_mem_len(i.req_mem_len);
        i1->o_req_mem_burst(i.req_mem_burst);
        i1->o_req_mem_burst_last(i.req_mem_burst_last);
        i1->i_resp_mem_data_valid(w_ctrl_resp_mem_data_valid);
        i1->i_resp_mem_data(wb_ctrl_resp_mem_data);
        i1->i_resp_mem_load_fault(w_ctrl_resp_mem_load_fault);
        i1->i_flush_address(i_flush_address);
        i1->i_flush_valid(i_flush_valid);
        i1->o_istate(o_istate);
    }

    d0 = new DCache("d0", async_reset);
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
    d0->o_resp_data_load_fault(o_resp_data_load_fault);
    d0->o_resp_data_store_fault(o_resp_data_store_fault);
    d0->i_resp_data_ready(i_resp_data_ready);
    d0->i_req_mem_ready(w_data_req_ready);
    d0->o_req_mem_valid(d.req_mem_valid);
    d0->o_req_mem_write(d.req_mem_write);
    d0->o_req_mem_addr(d.req_mem_addr);
    d0->o_req_mem_strob(d.req_mem_strob);
    d0->o_req_mem_data(d.req_mem_wdata);
    d0->i_resp_mem_data_valid(w_data_resp_mem_data_valid);
    d0->i_resp_mem_data(wb_data_resp_mem_data);
    d0->i_resp_mem_load_fault(w_data_resp_mem_load_fault);
    d0->i_resp_mem_store_fault(w_data_resp_mem_store_fault);
    d0->o_dstate(o_dstate);

#ifdef DBG_ICACHE_TB
    i0_tb = new ICache_tb("ictb0");
#endif
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
        sc_trace(o_vcd, w_ctrl_req_ready, "/top/cache0/w_ctrl_req_ready");
        
        sc_trace(o_vcd, i_req_mem_ready, "/top/cache0/i_req_mem_ready");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_write, "/top/cache0/o_req_mem_write");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/o_req_mem_addr");
        sc_trace(o_vcd, o_req_mem_strob, "/top/cache0/o_req_mem_strob");
        sc_trace(o_vcd, o_req_mem_data, "/top/cache0/o_req_mem_data");
        sc_trace(o_vcd, i_resp_mem_data_valid, "/top/cache0/i_resp_mem_data_valid");
        sc_trace(o_vcd, i_resp_mem_data, "/top/cache0/i_resp_mem_data");
        sc_trace(o_vcd, o_istate, "/top/cache0/o_istate");
        sc_trace(o_vcd, o_dstate, "/top/cache0/o_dstate");
        sc_trace(o_vcd, o_cstate, "/top/cache0/o_cstate");
        sc_trace(o_vcd, r.state, "/top/cache0/r_state");
    }
    if (icache_cfg_ == 0) {
        i0->generateVCD(i_vcd, o_vcd);
    } else {
        i1->generateVCD(i_vcd, o_vcd);
    }
    d0->generateVCD(i_vcd, o_vcd);
}

CacheTop::~CacheTop() {
    if (icache_cfg_ == 0) {
        delete i0;
    } else {
        delete i1;
    }
    delete d0;
}


void CacheTop::comb() {
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<8> wb_mem_len;
    sc_uint<2> wb_mem_burst;

    v = r;

    wb_mem_addr = 0;
    wb_mem_len = 0;
    wb_mem_burst = 0;

    w_data_req_ready = 0;
    w_data_resp_mem_data_valid = 0;
    wb_data_resp_mem_data = 0;
    w_data_resp_mem_load_fault = 0;
    w_data_resp_mem_store_fault = 0;
    w_ctrl_req_ready = 0;
    w_ctrl_resp_mem_data_valid = 0;
    wb_ctrl_resp_mem_data = 0;
    w_ctrl_resp_mem_load_fault = 0;

    switch (r.state.read()) {
    case State_Idle:
        if (i_req_mem_ready.read() == 1) {
            if (d.req_mem_valid.read()) {
                w_data_req_ready = 1;
                wb_mem_addr = d.req_mem_addr;
                v.state = State_DMem;
            } else if (i.req_mem_valid.read()) {
                w_ctrl_req_ready = 1;
                wb_mem_addr = i.req_mem_addr;
                wb_mem_len = i.req_mem_len;
                wb_mem_burst = i.req_mem_burst;
                v.state = State_IMem;
            }
        }
        break;

    case State_DMem:
        if (i_resp_mem_data_valid.read()) {
            if (i_req_mem_ready.read() == 1) {
                if (d.req_mem_valid.read()) {
                    w_data_req_ready = 1;
                    wb_mem_addr = d.req_mem_addr;
                    v.state = State_DMem;
                } else if (i.req_mem_valid.read() == 1) {
                    w_ctrl_req_ready = 1;
                    wb_mem_addr = i.req_mem_addr;
                    wb_mem_len = i.req_mem_len;
                    wb_mem_burst = i.req_mem_burst;
                    v.state = State_IMem;
                } else {
                    v.state = State_Idle;
                }
            } else {
                v.state = State_Idle;
            }
        }
        w_data_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_data_resp_mem_data = i_resp_mem_data;
        w_data_resp_mem_load_fault = i_resp_mem_load_fault;
        w_data_resp_mem_store_fault = i_resp_mem_store_fault;
        break;

    case State_IMem:
        if (i_resp_mem_data_valid.read() && i.req_mem_burst_last.read()) {
            if (i_req_mem_ready.read() == 1) {
                if (d.req_mem_valid.read()) {
                    w_data_req_ready = 1;
                    wb_mem_addr = d.req_mem_addr;
                    v.state = State_DMem;
                } else if (i.req_mem_valid.read() == 1) {
                    w_ctrl_req_ready = 1;
                    wb_mem_addr = i.req_mem_addr;
                    wb_mem_len = i.req_mem_len;
                    wb_mem_burst = i.req_mem_burst;
                    v.state = State_IMem;
                } else {
                    v.state = State_Idle;
                }
            } else {
                v.state = State_Idle;
            }
        }
        w_ctrl_resp_mem_data_valid = i_resp_mem_data_valid;
        wb_ctrl_resp_mem_data = i_resp_mem_data;
        w_ctrl_resp_mem_load_fault = i_resp_mem_load_fault;
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        v.state = State_Idle;
    }

    o_req_mem_valid = i.req_mem_valid | d.req_mem_valid;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_write = d.req_mem_write;
    o_req_mem_strob = d.req_mem_strob;
    o_req_mem_data = d.req_mem_wdata;
    o_req_mem_len = wb_mem_len;
    o_req_mem_burst = wb_mem_burst;
    o_cstate = r.state;
}

void CacheTop::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.state = State_Idle;
    } else {
        r = v;
    }
}

}  // namespace debugger

