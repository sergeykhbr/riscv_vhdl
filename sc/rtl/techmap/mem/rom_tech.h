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
#include "rom_inferred_2x32.h"
#include "api_core.h"
#include "sv_func.h"

namespace debugger {

template<int abits = 6,
         int log2_dbytes = 3>
SC_MODULE(rom_tech) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<abits>> i_addr;
    sc_out<sc_uint<(8 * (1 << log2_dbytes))>> o_rdata;

    void comb();

    SC_HAS_PROCESS(rom_tech);

    rom_tech(sc_module_name name,
             std::string filename);
    virtual ~rom_tech();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    std::string filename_;

    static const int dbits = (8 * (1 << log2_dbytes));

    sc_signal<sc_uint<(abits - log2_dbytes)>> wb_addr;

    rom_inferred_2x32<(abits - log2_dbytes)> *inf0;

};

template<int abits, int log2_dbytes>
rom_tech<abits, log2_dbytes>::rom_tech(sc_module_name name,
                                       std::string filename)
    : sc_module(name),
    i_clk("i_clk"),
    i_addr("i_addr"),
    o_rdata("o_rdata") {

    filename_ = filename;
    inf0 = 0;

    // if (dbits != 64) begin
    //     TODO: GENERATE assert
    // else
    inf0 = new rom_inferred_2x32<(abits - log2_dbytes)>("inf0",
                                                        filename);
    inf0->i_clk(i_clk);
    inf0->i_addr(wb_addr);
    inf0->o_rdata(o_rdata);
    // endif

    SC_METHOD(comb);
    sensitive << i_addr;
    sensitive << wb_addr;
}

template<int abits, int log2_dbytes>
rom_tech<abits, log2_dbytes>::~rom_tech() {
    if (inf0) {
        delete inf0;
    }
}

template<int abits, int log2_dbytes>
void rom_tech<abits, log2_dbytes>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
    }

}

template<int abits, int log2_dbytes>
void rom_tech<abits, log2_dbytes>::comb() {
    wb_addr = i_addr.read()((abits - 1), log2_dbytes);
}

}  // namespace debugger

