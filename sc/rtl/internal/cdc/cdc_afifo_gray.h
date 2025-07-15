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

template<int abits = 3>
SC_MODULE(cdc_afifo_gray) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_ena;
    sc_in<sc_uint<(abits + 1)>> i_q2_gray;
    sc_out<sc_uint<abits>> o_addr;
    sc_out<sc_uint<(abits + 1)>> o_gray;
    sc_out<bool> o_empty;
    sc_out<bool> o_full;

    void proc_comb();
    void proc_ff();

    cdc_afifo_gray(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<sc_uint<(abits + 1)>> wb_bin_next;
    sc_signal<sc_uint<(abits + 1)>> wb_gray_next;
    sc_signal<sc_uint<(abits + 1)>> bin;
    sc_signal<sc_uint<(abits + 1)>> gray;
    sc_signal<bool> empty;
    sc_signal<bool> full;

};

template<int abits>
cdc_afifo_gray<abits>::cdc_afifo_gray(sc_module_name name)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_ena("i_ena"),
    i_q2_gray("i_q2_gray"),
    o_addr("o_addr"),
    o_gray("o_gray"),
    o_empty("o_empty"),
    o_full("o_full") {


    SC_METHOD(proc_comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_q2_gray;
    sensitive << wb_bin_next;
    sensitive << wb_gray_next;
    sensitive << bin;
    sensitive << gray;
    sensitive << empty;
    sensitive << full;

    SC_METHOD(proc_ff);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abits>
void cdc_afifo_gray<abits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, i_ena.name());
        sc_trace(o_vcd, i_q2_gray, i_q2_gray.name());
        sc_trace(o_vcd, o_addr, o_addr.name());
        sc_trace(o_vcd, o_gray, o_gray.name());
        sc_trace(o_vcd, o_empty, o_empty.name());
        sc_trace(o_vcd, o_full, o_full.name());
    }

}

template<int abits>
void cdc_afifo_gray<abits>::proc_comb() {
    wb_bin_next = (bin.read() + (0, i_ena.read()));
    wb_gray_next = ((wb_bin_next.read() >> 1) ^ wb_bin_next.read());
    o_addr = bin.read()((abits - 1), 0);
    o_gray = gray.read();
    o_empty = empty.read();
    o_full = full.read();
}

template<int abits>
void cdc_afifo_gray<abits>::proc_ff() {
    if (i_nrst.read() == 0) {
        bin = 0;
        gray = 0;
        empty = 1;
        full = 0;
    } else {
        bin = wb_bin_next.read();
        gray = wb_gray_next.read();
        empty = (wb_gray_next.read() == i_q2_gray.read());
        // Optimized version of 3 conditions:
        //     wb_gray_next[abits] != i_q2_ptr[abits]
        //     wb_gray_next[abits-1] != i_q2_ptr[abits-1]
        //     wb_gray_next[abits-2:0] == i_q2_ptr[abits-2:0]
        full = ((wb_gray_next.read()[abits] ^ i_q2_gray.read()[abits])
                & (wb_gray_next.read()[(abits - 1)] ^ i_q2_gray.read()[(abits - 1)])
                & (wb_gray_next.read()((abits - 2), 0) == i_q2_gray.read()((abits - 2), 0)));
    }
}

}  // namespace debugger

