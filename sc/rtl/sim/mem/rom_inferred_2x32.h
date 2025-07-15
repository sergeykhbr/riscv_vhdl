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
#include "rom_inferred_32.h"
#include "api_core.h"
#include "sv_func.h"

namespace debugger {

template<int abits = 6>
SC_MODULE(rom_inferred_2x32) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_out<sc_uint<64>> o_rdata;

    void comb();

    rom_inferred_2x32(sc_module_name name,
                      std::string filename);
    virtual ~rom_inferred_2x32();


 private:
    std::string filename_;

    static const int DEPTH = (1 << abits);

    sc_signal<sc_uint<32>> wb_rdata0;
    sc_signal<sc_uint<32>> wb_rdata1;

    rom_inferred_32<abits> *rom0;
    rom_inferred_32<abits> *rom1;

};

template<int abits>
rom_inferred_2x32<abits>::rom_inferred_2x32(sc_module_name name,
                                            std::string filename)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    o_rdata("o_rdata") {

    filename_ = filename;
    rom0 = 0;
    rom1 = 0;

    rom0 = new rom_inferred_32<abits>("rom0",
                                      std::string(filename) + std::string("_lo.hex"));
    rom0->i_clk(i_clk);
    rom0->i_addr(i_addr);
    rom0->o_rdata(wb_rdata0);

    rom1 = new rom_inferred_32<abits>("rom1",
                                      std::string(filename) + std::string("_hi.hex"));
    rom1->i_clk(i_clk);
    rom1->i_addr(i_addr);
    rom1->o_rdata(wb_rdata1);

    SC_METHOD(comb);
    sensitive << i_addr;
    sensitive << wb_rdata0;
    sensitive << wb_rdata1;
}

template<int abits>
rom_inferred_2x32<abits>::~rom_inferred_2x32() {
    if (rom0) {
        delete rom0;
    }
    if (rom1) {
        delete rom1;
    }
}

template<int abits>
void rom_inferred_2x32<abits>::comb() {
    o_rdata = (wb_rdata1.read(), wb_rdata0.read());
}

}  // namespace debugger

