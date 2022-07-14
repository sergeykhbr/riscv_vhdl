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

#include "mmu_tlb.h"
#include "api_core.h"

namespace debugger {

MmuTlb::MmuTlb(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_adr("i_adr"),
    i_wena("i_wena"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata") {


    for (int i = 0; i < CFG_MMU_PTE_DBYTES; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "mem%d", i);
        mem[i] = new ram<CFG_MMU_TLB_AWIDTH,
                         8>(tstr);
        mem[i]->i_clk(i_clk);
        mem[i]->i_adr(i_adr);
        mem[i]->i_wena(i_wena);
        mem[i]->i_wdata(wb_mem_data[i].wdata);
        mem[i]->o_rdata(wb_mem_data[i].rdata);

    }

    SC_METHOD(comb);
    sensitive << i_adr;
    sensitive << i_wena;
    sensitive << i_wdata;
    for (int i = 0; i < CFG_MMU_PTE_DBYTES; i++) {
        sensitive << wb_mem_data[i].rdata;
        sensitive << wb_mem_data[i].wdata;
    }
}

void MmuTlb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_adr, i_adr.name());
        sc_trace(o_vcd, i_wena, i_wena.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
    }

}

void MmuTlb::comb() {
    sc_biguint<CFG_MMU_PTE_DWIDTH> vb_rdata;

    for (int i = 0; i < CFG_MMU_PTE_DBYTES; i++) {
        wb_mem_data[i].wdata = i_wdata.read()(((8 * (i + 1)) - 1), (8 * i)).to_uint64();
        vb_rdata(((8 * (i + 1)) - 1), (8 * i)) = wb_mem_data[i].rdata;
    }
    o_rdata = vb_rdata;
}

}  // namespace debugger

