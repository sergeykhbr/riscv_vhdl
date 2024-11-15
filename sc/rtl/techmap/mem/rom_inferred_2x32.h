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
#include "api_core.h"
#include "sv_func.h"

namespace debugger {

template<int abits = 6>
SC_MODULE(rom_inferred_2x32) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_out<sc_uint<64>> o_rdata;

    void registers();

    SC_HAS_PROCESS(rom_inferred_2x32);

    rom_inferred_2x32(sc_module_name name,
                      std::string filename);


 private:
    std::string filename_;

    static const int DEPTH = (1 << abits);

    std::string hexname0;
    std::string hexname1;
    sc_uint<32> wb_rdata0;
    sc_uint<32> wb_rdata1;
    sc_uint<32> mem0[DEPTH];
    sc_uint<32> mem1[DEPTH];

};

template<int abits>
rom_inferred_2x32<abits>::rom_inferred_2x32(sc_module_name name,
                                            std::string filename)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    o_rdata("o_rdata") {

    filename_ = filename;
    // initial
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_lo.hex", filename_.c_str());
    hexname0 = std::string(tstr);
    SV_readmemh(hexname0.c_str(), mem0);

    RISCV_sprintf(tstr, sizeof(tstr), "%s_hi.hex", filename_.c_str());
    hexname1 = std::string(tstr);
    SV_readmemh(hexname1.c_str(), mem1);
    // end initial

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

template<int abits>
void rom_inferred_2x32<abits>::registers() {
    wb_rdata0 = mem0[i_addr.read().to_int()];
    wb_rdata1 = mem1[i_addr.read().to_int()];

    o_rdata = (wb_rdata1, wb_rdata0);
}

}  // namespace debugger

