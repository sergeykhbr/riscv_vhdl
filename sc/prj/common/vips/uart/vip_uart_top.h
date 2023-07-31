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
#include <string>
#include "../clk/vip_clk.h"
#include "vip_uart_receiver.h"
#include "sv_func.h"

namespace debugger {

SC_MODULE(vip_uart_top) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_rx;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_uart_top);

    vip_uart_top(sc_module_name name,
                 bool async_reset,
                 int instnum,
                 int baudrate,
                 int scaler,
                 std::string logpath);
    virtual ~vip_uart_top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int instnum_;
    int baudrate_;
    int scaler_;
    std::string logpath_;
    double pll_period;

    std::string U8ToString(sc_uint<8> symb);

    sc_signal<bool> w_clk;
    sc_signal<bool> w_rdy;
    sc_signal<bool> w_rdy_clr;
    sc_signal<sc_uint<8>> wb_rdata;
    std::string rdatastr;
    std::string outstr;
    std::string outfilename;                                // formatted string name with instnum
    FILE *fl;

    vip_clk *clk0;
    vip_uart_receiver *rx0;

};

}  // namespace debugger

