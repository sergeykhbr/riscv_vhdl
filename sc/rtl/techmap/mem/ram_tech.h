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

namespace debugger {

template<int abits = 6,
         int dbits = 8>
SC_MODULE(ram_tech) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<dbits>> i_wdata;
    sc_out<sc_uint<dbits>> o_rdata;

    void registers();

    SC_HAS_PROCESS(ram_tech);

    ram_tech(sc_module_name name);


 private:
    static const int DEPTH = (1 << abits);

    sc_uint<dbits> rdata;
    sc_uint<dbits> mem[DEPTH];


};

template<int abits, int dbits>
ram_tech<abits, dbits>::ram_tech(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    i_wena("i_wena"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata") {


    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

template<int abits, int dbits>
void ram_tech<abits, dbits>::registers() {

    if (i_wena.read() == 1) {
        mem[i_addr.read().to_int()] = i_wdata;
    }
    rdata = mem[i_addr.read().to_int()];

    o_rdata = rdata;
}

}  // namespace debugger

