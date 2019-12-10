/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RIVERLIB_CACHE_MEM_RAM_H__
#define __DEBUGGER_RIVERLIB_CACHE_MEM_RAM_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../../river_cfg.h"

namespace debugger {

template <int abits = 6, int dbits = 8>
SC_MODULE(ram) {
    sc_in<bool> i_clk;
    sc_in<sc_uint<abits>> i_adr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<dbits>> i_wdata;
    sc_out<sc_uint<dbits>> o_rdata;
    
    void comb();
    void registers();

    SC_HAS_PROCESS(ram);

    ram(sc_module_name name_) : sc_module(name_) {
        SC_METHOD(comb);
        sensitive << i_adr;
        sensitive << i_wena;
        sensitive << i_wdata;
        sensitive << r.adr;
        sensitive << r.update;
 
        SC_METHOD(registers);
        sensitive << i_clk.pos();
    };


 private:
    struct RegistersType {
        sc_signal<bool> update;  // To generate SystemC delta event only.
        sc_signal<sc_uint<abits>> adr;
        sc_uint<dbits> mem[1 << abits];
    } v, r;
};

template <int abits, int dbits>
void ram<abits, dbits>::comb() {
    v = r;
    v.adr = i_adr.read();

    if (i_wena.read()) {
        v.mem[i_adr.read().to_int()] = i_wdata;
    }
    /** v.mem[] is not a signals, so use update register to trigger process */
    v.update = !r.update.read();

    o_rdata = r.mem[r.adr.read().to_int()];
}

template <int abits, int dbits>
void ram<abits, dbits>::registers() {
    r = v;
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_MEM_RAMBYTE_H__
