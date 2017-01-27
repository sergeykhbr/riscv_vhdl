/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction Cache.
 */

#include "icache.h"

namespace debugger {

ICache::ICache(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_valid;
    sensitive << i_req_ctrl_addr;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_req_mem_ready;
    sensitive << r.iline_addr;
    sensitive << r.iline_data;
    sensitive << r.iline_addr_hit;
    sensitive << r.iline_data_hit;
    sensitive << r.iline_addr_req;
    sensitive << r.hit_line;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void ICache::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, "/top/cache0/i0/i_nrst");
        sc_trace(o_vcd, i_req_ctrl_valid, "/top/cache0/i0/i_req_ctrl_valid");
        sc_trace(o_vcd, i_req_ctrl_addr, "/top/cache0/i0/i_req_ctrl_addr");
        sc_trace(o_vcd, o_req_ctrl_ready, "/top/cache0/i0/o_req_ctrl_ready");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/i0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/i0/o_req_mem_addr");
        sc_trace(o_vcd, i_req_mem_ready, "/top/cache0/i0/i_req_mem_ready");
        sc_trace(o_vcd, i_resp_mem_data_valid, "/top/cache0/i0/i_resp_mem_data_valid");
        sc_trace(o_vcd, i_resp_mem_data, "/top/cache0/i0/i_resp_mem_data");
        sc_trace(o_vcd, o_resp_ctrl_valid, "/top/cache0/i0/o_resp_ctrl_valid");
        sc_trace(o_vcd, i_resp_ctrl_ready, "/top/cache0/i0/i_resp_ctrl_ready");
        sc_trace(o_vcd, o_resp_ctrl_addr, "/top/cache0/i0/o_resp_ctrl_addr");
        sc_trace(o_vcd, o_resp_ctrl_data, "/top/cache0/i0/o_resp_ctrl_data");
        sc_trace(o_vcd, r.iline_addr, "/top/cache0/i0/r_iline_addr");
        sc_trace(o_vcd, r.iline_data, "/top/cache0/i0/r_iline_data");
        sc_trace(o_vcd, r.iline_addr_hit, "/top/cache0/i0/r_iline_addr_hit");
        sc_trace(o_vcd, r.iline_data_hit, "/top/cache0/i0/r_iline_data_hit");
        sc_trace(o_vcd, r.iline_addr_req, "/top/cache0/i0/r_iline_addr_req");
        sc_trace(o_vcd, r.hit_line, "/top/cache0/i0/r_hit_line");
        sc_trace(o_vcd, r.state, "/top/cache0/i0/r_state");
        //sc_trace(o_vcd, wb_req_line, "/top/cache0/i0/wb_req_line");
        //sc_trace(o_vcd, wb_cached_addr, "/top/cache0/i0/wb_cached_addr");
        //sc_trace(o_vcd, wb_cached_data, "/top/cache0/i0/wb_cached_data");
    }
}

void ICache::comb() {
    // Instruction cache:
    bool w_o_req_ctrl_ready;
    bool w_o_req_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_req_mem_addr;
    bool w_req_fire;
    bool w_o_resp_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_resp_addr;
    sc_uint<32> wb_o_resp_data;
    bool w_hit_req;
    bool w_hit_line;
    bool w_hit;
    sc_uint<BUS_ADDR_WIDTH - 3> wb_req_line;

    
    v = r;
    w_hit_req = 0;
    w_hit_line = 0;
    w_hit = 0;

    wb_req_line = i_req_ctrl_addr.read()(BUS_ADDR_WIDTH-1, 3);
    if (i_resp_mem_data_valid.read() 
      && (wb_req_line == r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3))) {
        w_hit_req = 1;
    }
    if (wb_req_line == r.iline_addr.read()) {
        w_hit_line = 1;
    }
    w_hit = w_hit_req || w_hit_line;

    w_o_req_mem_valid = !w_hit & i_req_ctrl_valid.read();
    wb_o_req_mem_addr = i_req_ctrl_addr.read()(BUS_ADDR_WIDTH-1, 3) << 3;
    w_o_req_ctrl_ready = w_hit | i_req_mem_ready.read();
    w_req_fire = i_req_ctrl_valid.read() && w_o_req_ctrl_ready;
    switch (r.state.read()) {
    case State_Idle:
        if (i_req_ctrl_valid.read()) {
            if (w_hit_line) {
                v.state = State_WaitAccept;
            } else if (i_req_mem_ready.read()) {
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
            if (!i_resp_ctrl_ready.read()) {
                v.state = State_WaitAccept;
            } else if (!i_req_ctrl_valid.read()) {
                v.state = State_Idle;
            } else {
                // New request
                if (w_hit) {
                    v.state = State_WaitAccept;
                } else if (i_req_mem_ready.read()) {
                    v.state = State_WaitResp;
                } else {
                    v.state = State_WaitGrant;
                }
            }
        }
        break;
    case State_WaitAccept:
        if (i_resp_ctrl_ready.read()) {
            if (!i_req_ctrl_valid.read()) {
                v.state = State_Idle;
            } else {
                if (w_hit) {
                    v.state = State_WaitAccept;
                } else if (i_req_mem_ready.read()) {
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
        v.iline_addr_req = i_req_ctrl_addr;
        v.hit_line = 0;
        if (w_hit_line) {
            v.hit_line = 1;
            v.iline_addr_hit = i_req_ctrl_addr;
            if (i_req_ctrl_addr.read()[2] == 0) {
                v.iline_data_hit =  r.iline_data.read()(31, 0);
            } else {
                v.iline_data_hit =  r.iline_data.read()(63, 32);
            }
        }
    }
    if (i_resp_mem_data_valid.read()) {
        v.iline_addr = r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3);
        v.iline_data =  i_resp_mem_data;
    }

    wb_o_resp_addr = r.iline_addr_req;
    if (r.state.read() == State_WaitAccept) {
        w_o_resp_valid = 1;
        if (r.hit_line) {
            wb_o_resp_addr = r.iline_addr_hit;
            wb_o_resp_data = r.iline_data_hit;
        } else {
            if (r.iline_addr_req.read()[2] == 0) {
                wb_o_resp_data = r.iline_data.read()(31, 0);
            } else {
                wb_o_resp_data = r.iline_data.read()(63, 32);
            }
        }
    } else {
        w_o_resp_valid = i_resp_mem_data_valid;
        if (r.iline_addr_req.read()[2] == 0) {
            wb_o_resp_data = i_resp_mem_data.read()(31, 0);
        } else {
            wb_o_resp_data = i_resp_mem_data.read()(63, 32);
        }
    }


    if (!i_nrst.read()) {
        v.iline_addr_req = 0;
        v.iline_addr = ~0;
        v.iline_data = 0;
        v.iline_addr_hit = ~0;
        v.iline_data_hit = 0;
        v.hit_line = 0;
        v.state = State_Idle;
    }

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = w_o_req_mem_valid;
    o_req_mem_addr = wb_o_req_mem_addr;
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_ctrl_valid = w_o_resp_valid;
    o_resp_ctrl_data = wb_o_resp_data;
    o_resp_ctrl_addr = wb_o_resp_addr;
}

void ICache::registers() {
    r = v;
}

}  // namespace debugger

