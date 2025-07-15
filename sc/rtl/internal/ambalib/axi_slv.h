// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "types_amba.h"
#include "types_pnp.h"

namespace debugger {

SC_MODULE(axi_slv) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // Base address information from the interconnect port
    sc_out<dev_config_type> o_cfg;                          // Slave config descriptor
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI Slave input interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI Slave output interface
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<CFG_SYSBUS_ADDR_BITS>> o_req_addr;
    sc_out<sc_uint<8>> o_req_size;
    sc_out<bool> o_req_write;
    sc_out<sc_uint<CFG_SYSBUS_DATA_BITS>> o_req_wdata;
    sc_out<sc_uint<CFG_SYSBUS_DATA_BYTES>> o_req_wstrb;
    sc_out<bool> o_req_last;
    sc_in<bool> i_req_ready;
    sc_in<bool> i_resp_valid;
    sc_in<sc_uint<CFG_SYSBUS_DATA_BITS>> i_resp_rdata;
    sc_in<bool> i_resp_err;

    void comb();
    void registers();

    axi_slv(sc_module_name name,
            bool async_reset,
            uint32_t vid,
            uint32_t did);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t vid_;
    uint32_t did_;

    static const uint8_t State_r_idle = 0;
    static const uint8_t State_r_addr = 0x01;
    static const uint8_t State_r_pipe = 0x02;
    static const uint8_t State_r_resp_last = 0x04;
    static const uint8_t State_r_wait_accept = 0x08;
    static const uint8_t State_r_buf = 0x10;
    static const uint8_t State_r_wait_writing = 0x20;
    static const uint8_t State_w_idle = 0;
    static const uint8_t State_w_wait_reading = 0x01;
    static const uint8_t State_w_req = 0x02;
    static const uint8_t State_w_pipe = 0x04;
    static const uint8_t State_w_buf = 0x08;
    static const uint8_t State_w_resp = 0x10;
    static const uint8_t State_b = 0x20;

    struct axi_slv_registers {
        sc_signal<sc_uint<6>> rstate;
        sc_signal<sc_uint<6>> wstate;
        sc_signal<bool> ar_ready;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> ar_addr;
        sc_signal<sc_uint<8>> ar_len;
        sc_signal<sc_uint<XSIZE_TOTAL>> ar_bytes;
        sc_signal<sc_uint<2>> ar_burst;
        sc_signal<sc_uint<CFG_SYSBUS_ID_BITS>> ar_id;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> ar_user;
        sc_signal<bool> ar_last;
        sc_signal<bool> aw_ready;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> aw_addr;
        sc_signal<sc_uint<XSIZE_TOTAL>> aw_bytes;
        sc_signal<sc_uint<2>> aw_burst;
        sc_signal<sc_uint<CFG_SYSBUS_ID_BITS>> aw_id;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> aw_user;
        sc_signal<bool> w_last;
        sc_signal<bool> w_ready;
        sc_signal<bool> r_valid;
        sc_signal<bool> r_last;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> r_data;
        sc_signal<bool> r_err;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> r_data_buf;
        sc_signal<bool> r_err_buf;
        sc_signal<bool> r_last_buf;
        sc_signal<bool> b_err;
        sc_signal<bool> b_valid;
        sc_signal<bool> req_valid;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<bool> req_last;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> req_wdata;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> req_wstrb;
        sc_signal<sc_uint<8>> req_bytes;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr_buf;
        sc_signal<bool> req_last_buf;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> req_wdata_buf;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> req_wstrb_buf;
        sc_signal<bool> requested;
        sc_signal<bool> resp_last;
    };

    void axi_slv_r_reset(axi_slv_registers& iv) {
        iv.rstate = State_r_idle;
        iv.wstate = State_w_idle;
        iv.ar_ready = 0;
        iv.ar_addr = 0;
        iv.ar_len = 0;
        iv.ar_bytes = 0;
        iv.ar_burst = 0;
        iv.ar_id = 0;
        iv.ar_user = 0;
        iv.ar_last = 0;
        iv.aw_ready = 0;
        iv.aw_addr = 0;
        iv.aw_bytes = 0;
        iv.aw_burst = 0;
        iv.aw_id = 0;
        iv.aw_user = 0;
        iv.w_last = 0;
        iv.w_ready = 0;
        iv.r_valid = 0;
        iv.r_last = 0;
        iv.r_data = 0;
        iv.r_err = 0;
        iv.r_data_buf = 0;
        iv.r_err_buf = 0;
        iv.r_last_buf = 0;
        iv.b_err = 0;
        iv.b_valid = 0;
        iv.req_valid = 0;
        iv.req_addr = 0;
        iv.req_last = 0;
        iv.req_write = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_bytes = 0;
        iv.req_addr_buf = 0;
        iv.req_last_buf = 0;
        iv.req_wdata_buf = 0;
        iv.req_wstrb_buf = 0;
        iv.requested = 0;
        iv.resp_last = 0;
    }

    axi_slv_registers v;
    axi_slv_registers r;

};

}  // namespace debugger

