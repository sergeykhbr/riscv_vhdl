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
    
    v = r;
    w_mem_valid = false;
    w_valid = false;
    wb_data = 0;

    if (i_resp_mem_data_valid.read()) {
        wb_cached_addr = r.iline_addr_req.range(BUS_ADDR_WIDTH-1, 3);
        wb_cached_data = i_resp_mem_data;
    } else {
        wb_cached_addr = r.iline_addr;
        wb_cached_data = r.iline_data;
    }

    wb_req_line = i_req_ctrl_addr.read().range(BUS_ADDR_WIDTH-1, 3);
    v.ihit = false;

    if (i_req_ctrl_valid.read()) {
        v.iline_addr_req = i_req_ctrl_addr;
        if (wb_req_line == wb_cached_addr) {
            v.ihit = true;
            if (i_req_ctrl_addr.read()[2] == 0) {
                v.ihit_data = wb_cached_data.range(31, 0);
            } else {
                v.ihit_data = wb_cached_data.range(63, 32);
            }
        } else {
            w_mem_valid = true;
            wb_mem_addr = i_req_ctrl_addr.read() & ~0x7;
        }
    }

    if (i_resp_mem_data_valid.read()) {
        w_valid = true;
        v.iline_addr = r.iline_addr_req.range(BUS_ADDR_WIDTH-1, 3);
        v.iline_data = i_resp_mem_data;
        if (r.iline_addr_req[2] == 0) {
            wb_data = i_resp_mem_data.read().range(31, 0);
        } else {
            wb_data = i_resp_mem_data.read().range(63, 32);
        }
    } else if (r.ihit) {
        w_valid = true;
        wb_data = r.ihit_data;
    }

    if (!i_nrst.read()) {
        v.iline_addr = ~0;
        v.iline_data = 0;
        v.ihit = 0;
    }

    o_req_mem_valid = w_mem_valid;
    o_req_mem_addr = wb_mem_addr;
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_ctrl_valid = w_valid;
    o_resp_ctrl_data = wb_data;
    o_resp_ctrl_addr = r.iline_addr_req;
}

void ICache::registers() {
    r = v;
}

}  // namespace debugger

