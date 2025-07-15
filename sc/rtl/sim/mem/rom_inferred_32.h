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
SC_MODULE(rom_inferred_32) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_out<sc_uint<32>> o_rdata;

    void init();
    void registers();

    rom_inferred_32(sc_module_name name,
                    std::string filename);


 private:
    std::string filename_;

    static const int DEPTH = (1 << abits);

    sc_uint<32> mem[DEPTH];

};

template<int abits>
rom_inferred_32<abits>::rom_inferred_32(sc_module_name name,
                                        std::string filename)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    o_rdata("o_rdata") {

    filename_ = filename;

    SC_THREAD(init);

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

template<int abits>
void rom_inferred_32<abits>::init() {
    SV_readmemh(filename_.c_str(), mem);
}

template<int abits>
void rom_inferred_32<abits>::registers() {
    o_rdata = mem[i_addr.read().to_int()];
}

}  // namespace debugger

