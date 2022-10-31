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

SC_MODULE(L2SerDes) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_out<axi4_l2_in_type> o_l2i;
    sc_in<axi4_l2_out_type> i_l2o;
    sc_in<axi4_master_in_type> i_msti;
    sc_out<axi4_master_out_type> o_msto;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2SerDes);

    L2SerDes(sc_module_name name,
             bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int linew = L2CACHE_LINE_BITS;
    static const int busw = CFG_SYSBUS_DATA_BITS;
    static const int lineb = (linew / 8);
    static const int busb = (busw / 8);
    static const int SERDES_BURST_LEN = (lineb / busb);
    static const uint8_t State_Idle = 0;
    static const uint8_t State_Read = 1;
    static const uint8_t State_Write = 2;

    sc_uint<8> size2len(sc_uint<3> size);
    sc_uint<3> size2size(sc_uint<3> size);

    struct L2SerDes_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<bool> b_wait;
        sc_signal<sc_biguint<linew>> line;
        sc_signal<sc_uint<lineb>> wstrb;
        sc_signal<sc_uint<SERDES_BURST_LEN>> rmux;
    } v, r;

    void L2SerDes_r_reset(L2SerDes_registers &iv) {
        iv.state = State_Idle;
        iv.req_len = 0;
        iv.b_wait = 0;
        iv.line = 0ull;
        iv.wstrb = 0;
        iv.rmux = 0;
    }

};

}  // namespace debugger

