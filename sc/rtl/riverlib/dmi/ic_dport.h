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
#include "../types_river.h"

namespace debugger {

SC_MODULE(ic_dport) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // DMI connection
    sc_in<sc_uint<CFG_LOG2_CPU_MAX>> i_hartsel;             // Selected hart index
    sc_in<bool> i_haltreq;
    sc_in<bool> i_resumereq;
    sc_in<bool> i_resethaltreq;                             // Halt core after reset request
    sc_in<bool> i_hartreset;                                // Reset currently selected hart
    sc_in<bool> i_dport_req_valid;                          // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_req_type;        // Debug access types
    sc_in<sc_uint<RISCV_ARCH>> i_dport_addr;                // Register index
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;               // Write value
    sc_in<sc_uint<3>> i_dport_size;                         // 0=1B;1=2B;2=4B;3=8B;4=128B
    sc_out<bool> o_dport_req_ready;                         // Response is ready
    sc_in<bool> i_dport_resp_ready;                         // ready to accept response
    sc_out<bool> o_dport_resp_valid;                        // Response is valid
    sc_out<bool> o_dport_resp_error;                        // Something goes wrong
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;              // Response value or error code
    // To Cores cluster
    sc_vector<sc_out<dport_in_type>> o_dporti;
    sc_vector<sc_in<dport_out_type>> i_dporto;

    void comb();
    void registers();

    SC_HAS_PROCESS(ic_dport);

    ic_dport(sc_module_name name,
             bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t ALL_CPU_MASK = ~0ul;

    struct ic_dport_registers {
        sc_signal<sc_uint<CFG_LOG2_CPU_MAX>> hartsel;
    } v, r;

    void ic_dport_r_reset(ic_dport_registers &iv) {
        iv.hartsel = 0;
    }

};

}  // namespace debugger

