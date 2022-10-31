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
#include "divstage53.h"

namespace debugger {

SC_MODULE(idiv53) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;                                      // divider enable pulse (1 clock)
    sc_in<sc_uint<53>> i_divident;                          // integer value
    sc_in<sc_uint<53>> i_divisor;                           // integer value
    sc_out<sc_biguint<105>> o_result;                       // resulting bits
    sc_out<sc_uint<7>> o_lshift;                            // first non-zero bit index
    sc_out<bool> o_rdy;                                     // delayed 'enable' signal
    sc_out<bool> o_overflow;                                // overflow flag
    sc_out<bool> o_zero_resid;                              // reasidual is zero flag

    void comb();
    void registers();

    SC_HAS_PROCESS(idiv53);

    idiv53(sc_module_name name,
           bool async_reset);
    virtual ~idiv53();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct idiv53_registers {
        sc_signal<sc_uint<15>> delay;
        sc_signal<sc_uint<7>> lshift;
        sc_signal<bool> lshift_rdy;
        sc_signal<sc_uint<53>> divisor;
        sc_signal<sc_uint<61>> divident;
        sc_signal<sc_biguint<105>> bits;
        sc_signal<bool> overflow;
        sc_signal<bool> zero_resid;
    } v, r;

    void idiv53_r_reset(idiv53_registers &iv) {
        iv.delay = 0;
        iv.lshift = 0;
        iv.lshift_rdy = 0;
        iv.divisor = 0ull;
        iv.divident = 0ull;
        iv.bits = 0ull;
        iv.overflow = 0;
        iv.zero_resid = 0;
    }

    sc_signal<bool> w_mux_ena_i;
    sc_signal<sc_uint<56>> wb_muxind_i;
    sc_signal<sc_uint<61>> wb_divident_i;
    sc_signal<sc_uint<53>> wb_divisor_i;
    sc_signal<sc_uint<53>> wb_dif_o;
    sc_signal<sc_uint<8>> wb_bits_o;
    sc_signal<sc_uint<7>> wb_muxind_o;
    sc_signal<bool> w_muxind_rdy_o;

    divstage53 *divstage0;

};

}  // namespace debugger

