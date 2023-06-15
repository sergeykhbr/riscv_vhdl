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

#include "vip_sdcard_top.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_top::vip_sdcard_top(sc_module_name name,
                               int half_period)
    : sc_module(name),
    i_rstn("i_rstn"),
    i_rx("i_rx") {

    half_period_ = half_period;

    SC_METHOD(comb);
    sensitive << i_rstn;
    sensitive << i_rx;
    sensitive << w_clk;
    sensitive << wb_rdata;
}

void vip_sdcard_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_rstn, i_rstn.name());
        sc_trace(o_vcd, i_rx, i_rx.name());
    }

}

void vip_sdcard_top::comb() {
}

}  // namespace debugger

