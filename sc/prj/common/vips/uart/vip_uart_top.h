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
#include "../clk/vip_clk.h"
#include "vip_uart_receiver.h"

namespace debugger {

SC_MODULE(vip_uart_top) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_rx;

    void comb();

    SC_HAS_PROCESS(vip_uart_top);

    vip_uart_top(sc_module_name name,
                 bool async_reset,
                 double half_period,
                 int scaler);
    virtual ~vip_uart_top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    double half_period_;
    int scaler_;

    sc_signal<bool> w_clk;
    sc_signal<bool> w_rdy;
    sc_signal<bool> w_rdy_clr;
    sc_signal<sc_uint<8>> wb_rdata;

    vip_clk *clk0;
    vip_uart_receiver *rx0;

};

}  // namespace debugger

