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

template<int abits = 16,
         int log2_dbytes = 3>
SC_MODULE(ram_bytes_tech) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<(1 << log2_dbytes)>> i_wstrb;
    sc_in<sc_uint<(8 * (1 << log2_dbytes))>> i_wdata;
    sc_out<sc_uint<(8 * (1 << log2_dbytes))>> o_rdata;

    void comb();

    ram_bytes_tech(sc_module_name name);
    virtual ~ram_bytes_tech();


 private:
    static const int dbytes = (1 << log2_dbytes);
    static const int dbits = (8 * (1 << log2_dbytes));

    sc_signal<sc_uint<(abits - log2_dbytes)>> wb_addr;
    sc_signal<bool> wb_wena[dbytes];
    sc_signal<sc_uint<8>> wb_wdata[dbytes];
    sc_signal<sc_uint<8>> wb_rdata[dbytes];

    ram_tech<(abits - log2_dbytes), 8> *mem[dbytes];

};

template<int abits, int log2_dbytes>
ram_bytes_tech<abits, log2_dbytes>::ram_bytes_tech(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    i_wena("i_wena"),
    i_wstrb("i_wstrb"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata") {

    for (int i = 0; i < dbytes; i++) {
        mem[i] = 0;
    }

    for (int i = 0; i < dbytes; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "mem%d", i);
        mem[i] = new ram_tech<(abits - log2_dbytes),
                              8>(tstr);
        mem[i]->i_clk(i_clk);
        mem[i]->i_addr(wb_addr);
        mem[i]->i_wena(wb_wena[i]);
        mem[i]->i_wdata(wb_wdata[i]);
        mem[i]->o_rdata(wb_rdata[i]);
    }

    SC_METHOD(comb);
    sensitive << i_addr;
    sensitive << i_wena;
    sensitive << i_wstrb;
    sensitive << i_wdata;
    sensitive << wb_addr;
    for (int i = 0; i < dbytes; i++) {
        sensitive << wb_wena[i];
    }
    for (int i = 0; i < dbytes; i++) {
        sensitive << wb_wdata[i];
    }
    for (int i = 0; i < dbytes; i++) {
        sensitive << wb_rdata[i];
    }
}

template<int abits, int log2_dbytes>
ram_bytes_tech<abits, log2_dbytes>::~ram_bytes_tech() {
    for (int i = 0; i < dbytes; i++) {
        if (mem[i]) {
            delete mem[i];
        }
    }
}

template<int abits, int log2_dbytes>
void ram_bytes_tech<abits, log2_dbytes>::comb() {
    sc_uint<dbits> vb_rdata;

    vb_rdata = 0;

    wb_addr = i_addr.read()((abits - 1), log2_dbytes);
    for (int i = 0; i < dbytes; i++) {
        wb_wena[i] = (i_wena.read() && i_wstrb.read()[i]);
        wb_wdata[i] = i_wdata.read()((8 * i) + 8 - 1, (8 * i)).to_uint64();
        vb_rdata((8 * i) + 8 - 1, (8 * i)) = wb_rdata[i].read();
    }

    o_rdata = vb_rdata;
}

}  // namespace debugger

