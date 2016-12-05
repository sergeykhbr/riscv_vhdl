/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction Cache.
 */

#include "icache.h"

namespace debugger {

ICache::ICache(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_valid;
    sensitive << i_req_ctrl_addr;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_req_mem_ready;
    sensitive << r.ihit;
    sensitive << r.iline_addr;
    sensitive << r.iline_data;
    sensitive << r.iline_addr_req;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_req_ctrl_valid, "/top/cache0/i0/i_req_ctrl_valid");
        //sc_trace(vcd, o_req_ctrl_ready, "/top/cache0/i0/o_req_ctrl_ready");
        sc_trace(vcd, i_req_ctrl_addr, "/top/cache0/i0/i_req_ctrl_addr");
        //sc_trace(vcd, o_req_mem_valid, "/top/cache0/i0/o_req_mem_valid");
        //sc_trace(vcd, o_req_mem_addr, "/top/cache0/i0/o_req_mem_addr");
        //sc_trace(vcd, i_resp_mem_data_valid, "/top/cache0/i0/i_resp_mem_data_valid");
        //sc_trace(vcd, i_resp_mem_data, "/top/cache0/i0/i_resp_mem_data");
        sc_trace(vcd, o_resp_ctrl_valid, "/top/cache0/i0/o_resp_ctrl_valid");
        sc_trace(vcd, o_resp_ctrl_addr, "/top/cache0/i0/o_resp_ctrl_addr");
        sc_trace(vcd, o_resp_ctrl_data, "/top/cache0/i0/o_resp_ctrl_data");
        //sc_trace(vcd, r.ihit, "/top/cache0/i0/r.ihit");
        //sc_trace(vcd, r.ihit_data, "/top/cache0/i0/r.ihit_data");
        //sc_trace(vcd, r.iline_addr, "/top/cache0/i0/r.iline_addr");
        //sc_trace(vcd, wb_req_line, "/top/cache0/i0/wb_req_line");
        //sc_trace(vcd, wb_cached_addr, "/top/cache0/i0/wb_cached_addr");
        //sc_trace(vcd, wb_cached_data, "/top/cache0/i0/wb_cached_data");
    }
};


void ICache::comb() {
    // Instruction cache:
    bool w_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    bool w_valid;
    sc_uint<32> wb_data;
    bool w_o_valid;
    sc_uint<32> wb_o_data;
    sc_uint<BUS_ADDR_WIDTH> wb_o_addr;

    
    v = r;
    w_mem_valid = false;
    wb_mem_addr = 0;
    w_valid = false;
    wb_data = 0;
    w_o_valid = 0;
    wb_o_data = 0;
    wb_o_addr = 0;

    if (i_resp_mem_data_valid.read()) {
        wb_cached_addr = r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3);
        wb_cached_data = i_resp_mem_data;
    } else {
        wb_cached_addr = r.iline_addr;
        wb_cached_data = r.iline_data;
    }

    wb_req_line = i_req_ctrl_addr.read()(BUS_ADDR_WIDTH-1, 3);
    v.ihit = false;

    if (i_req_ctrl_valid.read() && i_req_mem_ready.read()) {
        v.iline_addr_req = i_req_ctrl_addr;
        if (wb_req_line == wb_cached_addr) {
            v.ihit = true;
            if (i_req_ctrl_addr.read()[2] == 0) {
                v.ihit_data = wb_cached_data(31, 0);
            } else {
                v.ihit_data = wb_cached_data(63, 32);
            }
        } else {
            w_mem_valid = true;
            wb_mem_addr = i_req_ctrl_addr.read() & ~0x7;
        }
    }

    if (i_resp_mem_data_valid.read()) {
        w_valid = true;
        v.iline_addr = r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3);
        v.iline_data = i_resp_mem_data;
        if (r.iline_addr_req.read()[2] == 0) {
            wb_data = i_resp_mem_data.read()(31, 0);
        } else {
            wb_data = i_resp_mem_data.read()(63, 32);
        }
        v.hold_ena = !i_resp_ctrl_ready.read();
        v.hold_addr = r.iline_addr_req;
        v.hold_data = wb_data;
    } else if (r.ihit) {
        w_valid = true;
        wb_data = r.ihit_data;
        v.hold_ena = !i_resp_ctrl_ready.read();
        v.hold_addr = r.iline_addr_req;
        v.hold_data = r.ihit_data;
    }

    if (w_valid) {
        w_o_valid = i_resp_ctrl_ready;
        wb_o_data = wb_data;
        wb_o_addr = r.iline_addr_req;
    } else if (r.hold_ena.read()) {
        v.hold_ena = !i_resp_ctrl_ready.read();
        w_o_valid = i_resp_ctrl_ready;
        wb_o_data = r.hold_data;
        wb_o_addr = r.hold_addr;
    }

    if (!i_nrst.read()) {
        v.iline_addr_req = 0;
        v.iline_addr = ~0;
        v.iline_data = 0;
        v.ihit = 0;
        v.ihit_data = 0;
        v.hold_ena = 0;
        v.hold_addr = 0;
        v.hold_data = 0;
    }

    o_req_ctrl_ready = i_req_mem_ready; // cannot accept request when memory is busy (can be improved).

    o_req_mem_valid = w_mem_valid;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_ctrl_valid = w_o_valid;
    o_resp_ctrl_data = wb_o_data;
    o_resp_ctrl_addr = wb_o_addr;
}

void ICache::registers() {
    r = v;
}

}  // namespace debugger

