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

SC_MODULE(vip_clk) {
 public:
    sc_out<bool> o_clk;

    void comb();

    SC_HAS_PROCESS(vip_clk);

    vip_clk(sc_module_name name,
            double period);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    double period_;

    sc_clock pll;

};

}  // namespace debugger

