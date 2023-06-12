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
#include "../../ambalib/types_amba.h"

namespace debugger {

SC_MODULE(cdc_axi_sync_tech) {
 public:
    sc_in<bool> i_xslv_clk;                                 // system clock
    sc_in<bool> i_xslv_nrst;                                // system reset
    sc_in<axi4_slave_in_type> i_xslvi;                      // system clock
    sc_out<axi4_slave_out_type> o_xslvo;                    // system clock
    sc_in<bool> i_xmst_clk;                                 // ddr clock
    sc_in<bool> i_xmst_nrst;                                // ddr reset
    sc_out<axi4_slave_in_type> o_xmsto;                     // ddr clock
    sc_in<axi4_slave_out_type> i_xmsti;                     // ddr clock

    void comb();

    SC_HAS_PROCESS(cdc_axi_sync_tech);

    cdc_axi_sync_tech(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
};

}  // namespace debugger

