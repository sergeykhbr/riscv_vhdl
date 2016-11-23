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
        sc_trace(vcd, i_req_data_valid, "/top/cache0/d0/i_req_data_valid");
        sc_trace(vcd, i_req_data_write, "/top/cache0/d0/i_req_data_write");
        sc_trace(vcd, i_req_data_sz, "/top/cache0/d0/i_req_data_sz");
        sc_trace(vcd, i_req_data_addr, "/top/cache0/d0/i_req_data_addr");
        sc_trace(vcd, i_req_data_data, "/top/cache0/d0/i_req_data_data");
        sc_trace(vcd, o_req_mem_addr, "/top/cache0/d0/o_req_mem_addr");
        sc_trace(vcd, o_req_mem_strob, "/top/cache0/d0/o_req_mem_strob");
        sc_trace(vcd, o_req_mem_data, "/top/cache0/d0/o_req_mem_data");
        sc_trace(vcd, o_resp_data_valid, "/top/cache0/d0/o_resp_data_valid");
        sc_trace(vcd, o_resp_data_addr, "/top/cache0/d0/o_resp_data_addr");
        sc_trace(vcd, o_resp_data_data, "/top/cache0/d0/o_resp_data_data");
    }
};


void DCache::comb() {
    v = r;

    wb_req_addr = 0;
    wb_req_strob = 0;
    wb_rdata = 0;
    wb_wdata = 0;
    wb_rtmp = 0;

    wb_req_addr(BUS_ADDR_WIDTH-1, 3) 
        = i_req_data_addr.read()(BUS_ADDR_WIDTH-1, 3);

    v.rena = !i_req_data_write.read() & i_req_data_valid.read();
    if (i_req_data_write.read()) {
        switch (i_req_data_sz.read()) {
        case 0:
            wb_wdata = (i_req_data_data.read()(7, 0),
                i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
                i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
                i_req_data_data.read()(7, 0), i_req_data_data.read()(7, 0),
                i_req_data_data.read()(7, 0));
            if (i_req_data_addr.read()(2, 0) == 0x0) {
                wb_req_strob = 0x01;
            } else if (i_req_data_addr.read()(2, 0) == 0x1) {
                wb_req_strob = 0x02;
            } else if (i_req_data_addr.read()(2, 0) == 0x2) {
                wb_req_strob = 0x04;
            } else if (i_req_data_addr.read()(2, 0) == 0x3) {
                wb_req_strob = 0x08;
            } else if (i_req_data_addr.read()(2, 0) == 0x4) {
                wb_req_strob = 0x10;
            } else if (i_req_data_addr.read()(2, 0) == 0x5) {
                wb_req_strob = 0x20;
            } else if (i_req_data_addr.read()(2, 0) == 0x6) {
                wb_req_strob = 0x40;
            } else if (i_req_data_addr.read()(2, 0) == 0x7) {
                wb_req_strob = 0x80;
            }
            break;
        case 1:
            wb_wdata = (i_req_data_data.read()(15, 0),
                i_req_data_data.read()(15, 0), i_req_data_data.read()(15, 0),
                i_req_data_data.read()(15, 0));
            if (i_req_data_addr.read()(2, 1) == 0) {
                wb_req_strob = 0x03;
            } else if (i_req_data_addr.read()(2, 1) == 1) {
                wb_req_strob = 0x0C;
            } else if (i_req_data_addr.read()(2, 1) == 2) {
                wb_req_strob = 0x30;
            } else {
                wb_req_strob = 0xC0;
            }
            break;
        case 2:
            wb_wdata = (i_req_data_data.read()(31, 0),
                        i_req_data_data.read()(31, 0));
            if (i_req_data_addr.read()[2]) {
                wb_req_strob = 0xF0;
            } else {
                wb_req_strob = 0x0F;
            }
            break;
        case 3:
            wb_wdata = i_req_data_data;
            wb_req_strob = 0xFF;
            break;
        default:;
        }
    }

    switch (r.req_addr.read()(2, 0)) {
    case 1:
        wb_rtmp = i_resp_mem_data.read()(63, 8);
        break;
    case 2:
        wb_rtmp = i_resp_mem_data.read()(63, 16);
        break;
    case 3:
        wb_rtmp = i_resp_mem_data.read()(63, 24);
        break;
    case 4:
        wb_rtmp = i_resp_mem_data.read()(63, 32);
        break;
    case 5:
        wb_rtmp = i_resp_mem_data.read()(63, 40);
        break;
    case 6:
        wb_rtmp = i_resp_mem_data.read()(63, 48);
        break;
    case 7:
        wb_rtmp = i_resp_mem_data.read()(63, 56);
        break;
    default:
        wb_rtmp = i_resp_mem_data;
    } 

    switch (r.req_size.read()) {
    case 0:
        wb_rdata = wb_rtmp(7, 0);
        break;
    case 1:
        wb_rdata = wb_rtmp(15, 0);
        break;
    case 2:
        wb_rdata = wb_rtmp(31, 0);
        break;
    default:
        wb_rdata = wb_rtmp;
    }
    
    if (i_req_data_valid.read()) {
        v.req_addr = i_req_data_addr;
        v.req_size = i_req_data_sz;
    }

    if (!i_nrst.read()) {
        v.req_addr = 0;
        v.req_size = 0;
        v.rena = 0;
    }

    o_req_mem_valid = i_req_data_valid;
    o_req_mem_write = i_req_data_write;
    o_req_mem_addr = wb_req_addr;
    o_req_mem_strob = wb_req_strob;
    o_req_mem_data = wb_wdata;

    o_resp_data_valid = i_resp_mem_data_valid;
    o_resp_data_data = wb_rdata;
    o_resp_data_addr = r.req_addr;
}

void DCache::registers() {
    r = v;
}

}  // namespace debugger

