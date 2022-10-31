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
#include "../ambalib/types_amba.h"
#include "river_cfg.h"
#include "types_river.h"

namespace debugger {

SC_MODULE(ic_axi4_to_l1) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // AXI4 port
    sc_in<axi4_master_out_type> i_xmsto;
    sc_out<axi4_master_in_type> o_xmsti;
    // L1 port
    sc_in<axi4_l1_in_type> i_l1i;
    sc_out<axi4_l1_out_type> o_l1o;

    void comb();
    void registers();

    SC_HAS_PROCESS(ic_axi4_to_l1);

    ic_axi4_to_l1(sc_module_name name,
                  bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t Idle = 0;
    static const uint8_t ReadLineRequest = 1;
    static const uint8_t WaitReadLineResponse = 2;
    static const uint8_t WriteDataAccept = 3;
    static const uint8_t WriteLineRequest = 4;
    static const uint8_t WaitWriteConfirmResponse = 5;
    static const uint8_t WaitWriteAccept = 6;
    static const uint8_t WaitReadAccept = 7;
    static const uint8_t CheckBurst = 8;

    struct ic_axi4_to_l1_registers {
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_SYSBUS_ID_BITS>> req_id;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> req_user;
        sc_signal<sc_uint<8>> req_wstrb;
        sc_signal<sc_uint<64>> req_wdata;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<bool> writing;
        sc_signal<bool> read_modify_write;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> line_data;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> line_wstrb;
        sc_signal<sc_uint<64>> resp_data;
    } v, r;

    void ic_axi4_to_l1_r_reset(ic_axi4_to_l1_registers &iv) {
        iv.state = Idle;
        iv.req_addr = 0ull;
        iv.req_id = 0;
        iv.req_user = 0;
        iv.req_wstrb = 0;
        iv.req_wdata = 0ull;
        iv.req_len = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.writing = 0;
        iv.read_modify_write = 0;
        iv.line_data = 0ull;
        iv.line_wstrb = 0;
        iv.resp_data = 0ull;
    }

};

}  // namespace debugger

