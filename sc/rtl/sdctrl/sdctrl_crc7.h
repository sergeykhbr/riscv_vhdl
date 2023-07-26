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

SC_MODULE(sdctrl_crc7) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_clear;                                    // Clear CRC register;
    sc_in<bool> i_next;                                     // Shift enable strob
    sc_in<bool> i_dat;                                      // Input bit
    sc_out<sc_uint<7>> o_crc7;                              // Computed value

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_crc7);

    sdctrl_crc7(sc_module_name name,
                bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct sdctrl_crc7_registers {
        sc_signal<sc_uint<7>> crc7;
    } v, r;

    void sdctrl_crc7_r_reset(sdctrl_crc7_registers &iv) {
        iv.crc7 = 0;
    }

};

}  // namespace debugger

