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
#include "../ambalib/types_amba.h"
#include "river_cfg.h"
#include "types_river.h"

namespace debugger {

SC_MODULE(DummyCpu) {
 public:
    sc_out<axi4_l1_out_type> o_msto;
    sc_out<dport_out_type> o_dport;
    sc_out<bool> o_flush_l2;                                // Flush L2 after D$ has been finished
    sc_out<bool> o_halted;                                  // CPU halted via debug interface
    sc_out<bool> o_available;                               // CPU was instantitated of stubbed

    void comb();

    SC_HAS_PROCESS(DummyCpu);

    DummyCpu(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
};

}  // namespace debugger

