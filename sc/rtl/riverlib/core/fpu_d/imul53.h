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
#include "zeroenc.h"

namespace debugger {

SC_MODULE(imul53) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;                                      // enable pulse (1 clock)
    sc_in<sc_uint<53>> i_a;                                 // integer value
    sc_in<sc_uint<53>> i_b;                                 // integer value
    sc_out<sc_biguint<106>> o_result;                       // resulting bits
    sc_out<sc_uint<7>> o_shift;                             // first non-zero bit index
    sc_out<bool> o_rdy;                                     // delayed 'enable' signal
    sc_out<bool> o_overflow;                                // overflow flag

    void comb();
    void registers();

    SC_HAS_PROCESS(imul53);

    imul53(sc_module_name name,
           bool async_reset);
    virtual ~imul53();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct imul53_registers {
        sc_signal<sc_uint<16>> delay;
        sc_signal<sc_uint<7>> shift;
        sc_signal<bool> accum_ena;
        sc_signal<sc_uint<56>> b;
        sc_signal<sc_biguint<106>> sum;
        sc_signal<bool> overflow;
    } v, r;

    void imul53_r_reset(imul53_registers &iv) {
        iv.delay = 0;
        iv.shift = 0;
        iv.accum_ena = 0;
        iv.b = 0ull;
        iv.sum = 0ull;
        iv.overflow = 0;
    }

    sc_signal<sc_biguint<105>> wb_sumInv;
    sc_signal<sc_uint<7>> wb_lshift;

    zeroenc<105, 7> *enc0;

};

}  // namespace debugger

