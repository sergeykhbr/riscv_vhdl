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
#include "../cache/mem/ram.h"

namespace debugger {

SC_MODULE(MmuTlb) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<CFG_MMU_TLB_AWIDTH>> i_adr;
    sc_in<bool> i_wena;
    sc_in<sc_biguint<CFG_MMU_PTE_DWIDTH>> i_wdata;
    sc_out<sc_biguint<CFG_MMU_PTE_DWIDTH>> o_rdata;

    void comb();

    SC_HAS_PROCESS(MmuTlb);

    MmuTlb(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct MemType {
        sc_signal<sc_uint<8>> rdata;
        sc_signal<sc_uint<8>> wdata;
    };


    MemType wb_mem_data[CFG_MMU_PTE_DBYTES];

    ram<CFG_MMU_TLB_AWIDTH, 8> *mem[CFG_MMU_PTE_DWIDTH];

};

}  // namespace debugger

