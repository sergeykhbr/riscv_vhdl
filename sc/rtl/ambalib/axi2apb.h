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

SC_MODULE(axi2apb) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI4 Interconnect Bridge interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI4 Bridge to Interconnect interface
    sc_out<apb_in_type> o_apbi;                             // APB Bridge to Slave interface
    sc_in<apb_out_type> i_apbo;                             // APB  Slave to Bridge interface

    void comb();
    void registers();

    SC_HAS_PROCESS(axi2apb);

    axi2apb(sc_module_name name,
            bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t State_Idle = 0;
    static const uint8_t State_w = 1;
    static const uint8_t State_b = 2;
    static const uint8_t State_r = 3;
    static const uint8_t State_setup = 4;
    static const uint8_t State_access = 5;
    static const uint8_t State_err = 6;

    struct axi2apb_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<32>> paddr;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> pdata;
        sc_signal<bool> pwrite;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> pstrb;
        sc_signal<sc_uint<3>> pprot;
        sc_signal<bool> pselx;
        sc_signal<bool> penable;
        sc_signal<bool> pslverr;
        sc_signal<sc_uint<8>> xsize;
        sc_signal<sc_uint<CFG_SYSBUS_ID_BITS>> req_id;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> req_user;
    } v, r;

    void axi2apb_r_reset(axi2apb_registers &iv) {
        iv.state = State_Idle;
        iv.paddr = 0;
        iv.pdata = 0ull;
        iv.pwrite = 0;
        iv.pstrb = 0;
        iv.pprot = 0;
        iv.pselx = 0;
        iv.penable = 0;
        iv.pslverr = 0;
        iv.xsize = 0;
        iv.req_id = 0;
        iv.req_user = 0;
    }

};

}  // namespace debugger

