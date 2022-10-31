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
#include "ram_tech.h"
#include "api_core.h"

namespace debugger {

template<int abits = 6,
         int dbits = 128>
SC_MODULE(ram_cache_bwe_tech) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_in<sc_uint<(dbits / 8)>> i_wena;
    sc_in<sc_biguint<dbits>> i_wdata;
    sc_out<sc_biguint<dbits>> o_rdata;

    void comb();

    SC_HAS_PROCESS(ram_cache_bwe_tech);

    ram_cache_bwe_tech(sc_module_name name);


 private:
    sc_signal<bool> wb_we[(dbits / 8)];
    sc_signal<sc_uint<8>> wb_wdata[(dbits / 8)];
    sc_signal<sc_uint<8>> wb_rdata[(dbits / 8)];

    ram_tech<abits, 8> *rx[(dbits / 8)];

};

template<int abits, int dbits>
ram_cache_bwe_tech<abits, dbits>::ram_cache_bwe_tech(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    i_wena("i_wena"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata") {

    for (int i = 0; i < (dbits / 8); i++) {
        rx[i] = 0;
    }

    for (int i = 0; i < (dbits / 8); i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "rx%d", i);
        rx[i] = new ram_tech<abits,
                             8>(tstr);
        rx[i]->i_clk(i_clk);
        rx[i]->i_addr(i_addr);
        rx[i]->i_wena(wb_we[i]);
        rx[i]->i_wdata(wb_wdata[i]);
        rx[i]->o_rdata(wb_rdata[i]);

    }


    SC_METHOD(comb);
    sensitive << i_addr;
    sensitive << i_wena;
    sensitive << i_wdata;
    for (int i = 0; i < (dbits / 8); i++) {
        sensitive << wb_we[i];
    }
    for (int i = 0; i < (dbits / 8); i++) {
        sensitive << wb_wdata[i];
    }
    for (int i = 0; i < (dbits / 8); i++) {
        sensitive << wb_rdata[i];
    }
}

template<int abits, int dbits>
void ram_cache_bwe_tech<abits, dbits>::comb() {
    sc_biguint<dbits> vb_rdata;

    vb_rdata = 0;

    for (int i = 0; i < (dbits / 8); i++) {
        wb_we[i] = i_wena.read()[i];
        wb_wdata[i] = i_wdata.read()((8 * i) + 8 - 1, (8 * i)).to_uint64();
        vb_rdata((8 * i) + 8- 1, (8 * i)) = wb_rdata[i];
    }
    o_rdata = vb_rdata;
}

}  // namespace debugger

