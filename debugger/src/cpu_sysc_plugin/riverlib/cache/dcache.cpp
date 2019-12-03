/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "dcache.h"

namespace debugger {

DCache::DCache(sc_module_name name_, bool async_reset) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_data_valid("i_req_data_valid"),
    i_req_data_write("i_req_data_write"),
    i_req_data_sz("i_req_data_sz"),
    i_req_data_addr("i_req_data_addr"),
    i_req_data_data("i_req_data_data"),
    o_req_data_ready("o_req_data_ready"),
    o_resp_data_valid("o_resp_data_valid"),
    o_resp_data_addr("o_resp_data_addr"),
    o_resp_data_data("o_resp_data_data"),
    o_resp_data_load_fault("o_resp_data_load_fault"),
    o_resp_data_store_fault("o_resp_data_store_fault"),
    o_resp_data_store_fault_addr("o_resp_data_store_fault_addr"),
    i_resp_data_ready("i_resp_data_ready"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_write("o_req_mem_write"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    o_req_mem_len("o_req_mem_len"),
    o_req_mem_burst("o_req_mem_burst"),
    o_req_mem_last("o_req_mem_last"),
    i_resp_mem_data_valid("i_resp_mem_data_valid"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    i_resp_mem_store_fault_addr("i_resp_mem_store_fault_addr"),
    o_mpu_addr("o_mpu_addr"),
    i_mpu_cachable("i_mpu_cachable"),
    i_mpu_readable("i_mpu_readable"),
    i_mpu_writable("i_mpu_writable"),
    o_dstate("o_dstate") {
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
    sensitive << i_resp_mem_store_fault_addr;
    sensitive << i_req_mem_ready;
    sensitive << i_mpu_cachable;
    sensitive << i_mpu_readable;
    sensitive << i_mpu_writable;
    sensitive << i_resp_data_ready;
    sensitive << r.req_strob;
    sensitive << r.req_wdata;
    sensitive << r.dline_data;
    sensitive << r.dline_addr_req;
    sensitive << r.dline_size_req;
    sensitive << r.dline_load_fault;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void DCache::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_data_valid, i_req_data_valid.name());
        sc_trace(o_vcd, i_req_data_write, i_req_data_write.name());
        sc_trace(o_vcd, i_req_data_sz, i_req_data_sz.name());
        sc_trace(o_vcd, i_req_data_addr, i_req_data_addr.name());
        sc_trace(o_vcd, i_req_data_data, i_req_data_data.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_write, o_req_mem_write.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, o_resp_data_valid, o_resp_data_valid.name());
        sc_trace(o_vcd, o_resp_data_addr, o_resp_data_addr.name());
        sc_trace(o_vcd, o_resp_data_data, o_resp_data_data.name());

        std::string pn(name());
        sc_trace(o_vcd, r.req_strob, pn + ".r_req_strob");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.dline_data, pn + ".r_dline_data");
        sc_trace(o_vcd, r.dline_addr_req, pn + ".r_dline_addr_req");
        sc_trace(o_vcd, r.dline_size_req, pn + ".r_dline_size_req");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, w_wait_response, pn + ".w_wait_response");
    }
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
    }

    wb_o_resp_addr = r.dline_addr_req;
    if (r.state.read() == State_WaitAccept) {
        w_o_resp_valid = 1;
        wb_resp_data_mux = r.dline_data;
        w_o_resp_load_fault = r.dline_load_fault;
    } else {
        w_o_resp_valid = i_resp_mem_data_valid;
        wb_resp_data_mux = i_resp_mem_data;
        w_o_resp_load_fault = i_resp_mem_load_fault;
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
    o_req_mem_len = 0;
    o_req_mem_burst = 0;
    o_req_mem_last = 1;

    o_resp_data_valid = w_o_resp_valid;
    o_resp_data_data = wb_o_resp_data;
    o_resp_data_addr = wb_o_resp_addr;
    o_resp_data_load_fault = w_o_resp_load_fault;
    // AXI B-channel
    o_resp_data_store_fault = i_resp_mem_store_fault;
    o_resp_data_store_fault_addr = i_resp_mem_store_fault_addr;
    o_mpu_addr = 0;
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

