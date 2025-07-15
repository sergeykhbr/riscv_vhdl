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

#include "SysPLL_tech.h"
#include "api_core.h"

namespace debugger {

SysPLL_tech::SysPLL_tech(sc_module_name name)
    : sc_module(name),
    i_reset("i_reset"),
    i_clk_tcxo("i_clk_tcxo"),
    o_clk_sys("o_clk_sys"),
    o_clk_ddr("o_clk_ddr"),
    o_clk_pcie("o_clk_pcie"),
    o_locked("o_locked") {


    SC_METHOD(comb);
    sensitive << i_reset;
    sensitive << i_clk_tcxo;
}

void SysPLL_tech::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_reset, i_reset.name());
        sc_trace(o_vcd, i_clk_tcxo, i_clk_tcxo.name());
        sc_trace(o_vcd, o_clk_sys, o_clk_sys.name());
        sc_trace(o_vcd, o_clk_ddr, o_clk_ddr.name());
        sc_trace(o_vcd, o_clk_pcie, o_clk_pcie.name());
        sc_trace(o_vcd, o_locked, o_locked.name());
    }

}

void SysPLL_tech::comb() {
    o_clk_sys = i_clk_tcxo.read();
    o_clk_ddr = i_clk_tcxo.read();
    o_clk_pcie = i_clk_tcxo.read();
    o_locked = 1;
}

}  // namespace debugger

