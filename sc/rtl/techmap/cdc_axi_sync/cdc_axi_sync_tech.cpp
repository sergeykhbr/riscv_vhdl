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

#include "cdc_axi_sync_tech.h"
#include "api_core.h"

namespace debugger {

cdc_axi_sync_tech::cdc_axi_sync_tech(sc_module_name name)
    : sc_module(name),
    i_xslv_clk("i_xslv_clk"),
    i_xslv_nrst("i_xslv_nrst"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_xmst_clk("i_xmst_clk"),
    i_xmst_nrst("i_xmst_nrst"),
    o_xmsto("o_xmsto"),
    i_xmsti("i_xmsti") {


    SC_METHOD(comb);
    sensitive << i_xslv_clk;
    sensitive << i_xslv_nrst;
    sensitive << i_xslvi;
    sensitive << i_xmst_clk;
    sensitive << i_xmst_nrst;
    sensitive << i_xmsti;
}

void cdc_axi_sync_tech::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_xslv_clk, i_xslv_clk.name());
        sc_trace(o_vcd, i_xslv_nrst, i_xslv_nrst.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_xmst_clk, i_xmst_clk.name());
        sc_trace(o_vcd, i_xmst_nrst, i_xmst_nrst.name());
        sc_trace(o_vcd, o_xmsto, o_xmsto.name());
        sc_trace(o_vcd, i_xmsti, i_xmsti.name());
    }

}

void cdc_axi_sync_tech::comb() {
    o_xmsto = i_xslvi;
    o_xslvo = i_xmsti;
}

}  // namespace debugger

