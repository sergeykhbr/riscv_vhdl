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

#ifndef __DEBUGGER_RIVERLIB_CORE_FPU_D_DIVSTAGE64_H__
#define __DEBUGGER_RIVERLIB_CORE_FPU_D_DIVSTAGE64_H__

#include <systemc.h>
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(divstage64) {
    sc_in<sc_uint<64>> i_divident;      // integer value
    sc_in<sc_biguint<128>> i_divisor;   // integer value
    sc_out<sc_uint<64>> o_resid;        // residual
    sc_out<sc_uint<4>> o_bits;          // resulting bits

    void comb();

    SC_HAS_PROCESS(divstage64);

    divstage64(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_biguint<129> wb_thresh[16];
    sc_biguint<128> wb_dif;
    sc_biguint<129> wb_divx3;
    sc_biguint<129> wb_divx2;
    sc_biguint<129> wb_divident;
    sc_biguint<129> wb_divisor;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CORE_FPU_D_DIVSTAGE64_H__
