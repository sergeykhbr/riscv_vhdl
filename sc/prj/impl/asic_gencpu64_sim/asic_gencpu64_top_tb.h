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
#include "../../../rtl/sim/pll/pll_generic.h"
#include "../../common/vips/uart/vip_uart_top.h"
#include "../asic_gencpu64/asic_gencpu64_top.h"
#include "sv_func.h"

namespace debugger {

SC_MODULE(asic_gencpu64_top_tb) {
 public:

    void test();

    asic_gencpu64_top_tb(sc_module_name name);
    virtual ~asic_gencpu64_top_tb();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int sim_uart_speedup_rate = 3;             // 0=no speed-up, 1=2x speed, 2=4x speed, 3=8x speed, 4=16x speed, .. etc
    static const int sim_uart_baudrate = (115200 * (1 << sim_uart_speedup_rate));

    sc_signal<bool> w_rst;                                  // Power-on system reset active HIGH
    sc_signal<bool> w_nrst;
    sc_signal<bool> w_sclk_p;
    sc_signal<bool> w_sclk_n;
    sc_signal<sc_uint<12>> wb_gpio;
    sc_signal<bool> w_jtag_trst;
    sc_signal<bool> w_jtag_tck;
    sc_signal<bool> w_jtag_tms;
    sc_signal<bool> w_jtag_tdi;
    sc_signal<bool> w_jtag_tdo;
    sc_signal<bool> w_jtag_vref;
    sc_signal<bool> w_uart1_rd;
    sc_signal<bool> w_uart1_td;
    sc_signal<bool> w_uart1_loopback_ena;
    sc_signal<bool> w_sd_sclk;
    sc_signal<bool, SC_UNCHECKED_WRITERS> w_sd_cmd;
    sc_signal<bool, SC_UNCHECKED_WRITERS> w_sd_dat0;
    sc_signal<bool, SC_UNCHECKED_WRITERS> w_sd_dat1;
    sc_signal<bool, SC_UNCHECKED_WRITERS> w_sd_dat2;
    sc_signal<bool, SC_UNCHECKED_WRITERS> w_sd_cd_dat3;
    sc_signal<bool> w_sd_detected;
    sc_signal<bool> w_sd_protect;
    sc_uint<32> wb_clk_cnt;

    pll_generic *clk0;
    vip_uart_top *uart1;
    asic_gencpu64_top *tt;

};

}  // namespace debugger

