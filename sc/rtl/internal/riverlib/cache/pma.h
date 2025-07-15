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

namespace debugger {

SC_MODULE(PMA) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_iaddr;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_daddr;
    sc_out<bool> o_icached;                                 // Hardcoded cached memory range for I$
    sc_out<bool> o_dcached;                                 // Hardcoded cached memory range for D$

    void comb();

    PMA(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const uint64_t CLINT_BAR = 0x000002000000;
    static const uint64_t CLINT_MASK = 0x00000000FFFF;      // 64 KB
    static const uint64_t PLIC_BAR = 0x00000C000000;
    static const uint64_t PLIC_MASK = 0x000003FFFFFF;       // 64 MB
    static const uint64_t IO1_BAR = 0x000010000000;
    static const uint64_t IO1_MASK = 0x0000000FFFFF;        // 1 MB

};

}  // namespace debugger

