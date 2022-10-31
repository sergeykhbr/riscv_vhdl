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
#include "../river_cfg.h"
#include "../../ambalib/types_amba.h"
#include "../types_river.h"

namespace debugger {

SC_MODULE(L2Destination) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_resp_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_rdata;
    sc_in<sc_uint<2>> i_resp_status;
    sc_vector<sc_in<axi4_l1_out_type>> i_l1o;
    sc_vector<sc_out<axi4_l1_in_type>> o_l1i;
    // cache interface
    sc_in<bool> i_req_ready;
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<L2_REQ_TYPE_BITS>> o_req_type;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_addr;
    sc_out<sc_uint<3>> o_req_size;
    sc_out<sc_uint<3>> o_req_prot;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_wdata;
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_wstrb;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Destination);

    L2Destination(sc_module_name name,
                  bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t Idle = 0;
    static const uint8_t CacheReadReq = 1;
    static const uint8_t CacheWriteReq = 2;
    static const uint8_t ReadMem = 3;
    static const uint8_t WriteMem = 4;
    static const uint8_t snoop_ac = 5;
    static const uint8_t snoop_cr = 6;
    static const uint8_t snoop_cd = 7;

    struct L2Destination_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<3>> srcid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<5>> req_src;
        sc_signal<sc_uint<L2_REQ_TYPE_BITS>> req_type;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_uint<(CFG_SLOT_L1_TOTAL + 1)>> ac_valid;
        sc_signal<sc_uint<(CFG_SLOT_L1_TOTAL + 1)>> cr_ready;
        sc_signal<sc_uint<(CFG_SLOT_L1_TOTAL + 1)>> cd_ready;
    } v, r;

    void L2Destination_r_reset(L2Destination_registers &iv) {
        iv.state = Idle;
        iv.srcid = CFG_SLOT_L1_TOTAL;
        iv.req_addr = 0ull;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_src = 0;
        iv.req_type = 0;
        iv.req_wdata = 0ull;
        iv.req_wstrb = 0;
        iv.ac_valid = 0;
        iv.cr_ready = 0;
        iv.cd_ready = 0;
    }

};

}  // namespace debugger

