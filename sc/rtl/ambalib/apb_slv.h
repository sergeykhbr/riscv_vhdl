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

namespace debugger {

SC_MODULE(apb_slv) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // Base address information from the interconnect port
    sc_out<dev_config_type> o_cfg;                          // Slave config descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<32>> o_req_addr;
    sc_out<bool> o_req_write;
    sc_out<sc_uint<32>> o_req_wdata;
    sc_in<bool> i_resp_valid;
    sc_in<sc_uint<32>> i_resp_rdata;
    sc_in<bool> i_resp_err;

    void comb();
    void registers();

    SC_HAS_PROCESS(apb_slv);

    apb_slv(sc_module_name name,
            bool async_reset,
            uint32_t vid,
            uint32_t did);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t vid_;
    uint32_t did_;

    static const uint8_t State_Idle = 0;
    static const uint8_t State_Request = 1;
    static const uint8_t State_WaitResp = 2;
    static const uint8_t State_Resp = 3;

    struct apb_slv_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> req_valid;
        sc_signal<sc_uint<32>> req_addr;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<32>> req_wdata;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void apb_slv_r_reset(apb_slv_registers &iv) {
        iv.state = State_Idle;
        iv.req_valid = 0;
        iv.req_addr = 0;
        iv.req_write = 0;
        iv.req_wdata = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

};

}  // namespace debugger

