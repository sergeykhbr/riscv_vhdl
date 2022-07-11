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

namespace debugger {

template<int abits = 6, int dbits = 8>
SC_MODULE(ram) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_adr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<dbits>> i_wdata;
    sc_out<sc_uint<dbits>> o_rdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(ram);

    ram(sc_module_name name);

 private:
    static const int DEPTH = (1 << abits);

    struct ram_registers {
        sc_signal<sc_uint<abits>> adr;
        sc_signal<sc_uint<dbits>> mem[DEPTH];
    } v, r;


};

template<int abits, int dbits>
ram<abits, dbits>::ram(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_adr("i_adr"),
    i_wena("i_wena"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata") {


    SC_METHOD(comb);
    sensitive << i_adr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << r.adr;
    for (int i = 0; i < DEPTH; i++) {
        sensitive << r.mem[i];
    }

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

template<int abits, int dbits>
void ram<abits, dbits>::comb() {
    v.adr = r.adr;
    for (int i = 0; i < DEPTH; i++) {
        v.mem[i] = r.mem[i];
    }

    v.adr = i_adr.read();
    if (i_wena.read() == 1) {
        v.mem[i_adr.read().to_int()] = i_wdata.read();
    }

    o_rdata = r.mem[r.adr.read().to_int()];
}

template<int abits, int dbits>
void ram<abits, dbits>::registers() {
    r.adr = v.adr;
    for (int i = 0; i < DEPTH; i++) {
        r.mem[i] = v.mem[i];
    }
}

}  // namespace debugger

