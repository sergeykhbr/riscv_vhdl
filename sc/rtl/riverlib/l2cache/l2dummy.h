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
#include "../../ambalib/types_amba.h"
#include "../river_cfg.h"
#include "../types_river.h"

namespace debugger {

SC_MODULE(L2Dummy) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_vector<sc_in<axi4_l1_out_type>> i_l1o;
    sc_vector<sc_out<axi4_l1_in_type>> o_l1i;
    sc_in<axi4_l2_in_type> i_l2i;
    sc_out<axi4_l2_out_type> o_l2o;
    sc_in<bool> i_flush_valid;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Dummy);

    L2Dummy(sc_module_name name,
            bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t Idle = 0;
    static const uint8_t state_ar = 1;
    static const uint8_t state_r = 2;
    static const uint8_t l1_r_resp = 3;
    static const uint8_t state_aw = 4;
    static const uint8_t state_w = 5;
    static const uint8_t state_b = 6;
    static const uint8_t l1_w_resp = 7;

    struct L2Dummy_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<3>> srcid;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<bool> req_lock;
        sc_signal<sc_uint<CFG_CPU_ID_BITS>> req_id;
        sc_signal<sc_uint<CFG_CPU_USER_BITS>> req_user;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> rdata;
        sc_signal<sc_uint<2>> resp;
    } v, r;

    void L2Dummy_r_reset(L2Dummy_registers &iv) {
        iv.state = Idle;
        iv.srcid = CFG_SLOT_L1_TOTAL;
        iv.req_addr = 0ull;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_lock = 0;
        iv.req_id = 0;
        iv.req_user = 0;
        iv.req_wdata = 0ull;
        iv.req_wstrb = 0;
        iv.rdata = 0ull;
        iv.resp = 0;
    }

};

}  // namespace debugger

