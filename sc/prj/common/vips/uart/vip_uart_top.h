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
#include "../../../../rtl/sim/pll/pll_generic.h"
#include "vip_uart_receiver.h"
#include "vip_uart_transmitter.h"
#include "sv_func.h"

namespace debugger {

SC_MODULE(vip_uart_top) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_rx;
    sc_out<bool> o_tx;
    sc_in<bool> i_loopback_ena;                             // redirect Rx bytes into Tx

    void init();
    void comb();
    void fileout();
    void registers();

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

    static const uint8_t EOF_0x0D = 0x0D;

    struct vip_uart_top_registers {
        sc_signal<sc_uint<2>> initdone;
    };

    void vip_uart_top_r_reset(vip_uart_top_registers& iv) {
        iv.initdone = 0;
    }

    std::string U8ToString(std::string istr, sc_uint<8> symb);

    sc_signal<bool> w_clk;
    sc_signal<bool> w_rx_rdy;
    sc_signal<bool> w_rx_rdy_clr;
    sc_signal<bool> w_tx_we;
    sc_signal<bool> w_tx_full;
    sc_signal<sc_uint<8>> wb_rdata;
    sc_signal<sc_uint<8>> wb_rdataz;
    std::string outstr;
    std::string outstrtmp;
    std::string outfilename;                                // formatted string name with instnum
    FILE* fl;
    FILE* fl_tmp;
    vip_uart_top_registers v;
    vip_uart_top_registers r;

    pll_generic *clk0;
    vip_uart_receiver *rx0;
    vip_uart_transmitter *tx0;

};

}  // namespace debugger

