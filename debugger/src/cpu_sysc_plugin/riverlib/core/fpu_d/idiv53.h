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

#ifndef __DEBUGGER_RIVERLIB_CORE_FPU_D_IDIV53_H__
#define __DEBUGGER_RIVERLIB_CORE_FPU_D_IDIV53_H__

#include <systemc.h>
#include "../../river_cfg.h"
#include "divstage.h"

namespace debugger {

SC_MODULE(idiv53) {
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_ena;                  // divider enable pulse (1 clock)
    sc_in<sc_uint<53>> i_divident;      // integer value
    sc_in<sc_uint<53>> i_divisor;       // integer value
    sc_out<sc_bv<105>> o_result;        // resulting bits
    sc_out<sc_uint<7>> o_lshift;        // first non-zero bit index
    sc_out<bool> o_rdy;                 // delayed 'enable' signal
    sc_out<bool> o_overflow;            // overflow flag
    sc_out<bool> o_zero_resid;          // reasidual is zero flag

    void comb();
    void registers();

    SC_HAS_PROCESS(idiv53);

    idiv53(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    divstage divstage0;
    struct RegistersType {
        sc_signal<sc_uint<16>> delay;
        sc_signal<sc_uint<16>> lshift;
        sc_signal<bool> lshift_rdy;
        sc_signal<sc_uint<53>> divisor;
        sc_signal<sc_uint<53>> divident;
        sc_signal<sc_bv<105>> bits;
        sc_signal<bool> overflow;
        sc_signal<bool> zero_resid;
    } v, r;

    sc_signal<bool> w_mux_ena_i;
    sc_signal<sc_uint<56>> wb_muxind_i;
    sc_signal<sc_uint<61>> wb_divident_i;
    sc_signal<sc_uint<53>> wb_divisor_i;
    sc_signal<sc_uint<53>> wb_dif_o;
    sc_signal<sc_uint<8>> wb_bits_o;
    sc_signal<sc_uint<7>> wb_muxind_o;
    sc_signal<bool> w_muxind_rdy_o;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CORE_FPU_D_IDIV53_H__
