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

SC_MODULE(vip_uart_transmitter) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_we;
    sc_in<sc_uint<8>> i_wdata;
    sc_out<bool> o_full;
    sc_out<bool> o_tx;

    void comb();
    void registers();

    vip_uart_transmitter(sc_module_name name,
                         bool async_reset,
                         int scaler);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int scaler_;
    int scaler_max;

    static const uint8_t idle = 0x0;
    static const uint8_t startbit = 0x1;
    static const uint8_t data = 0x2;
    static const uint8_t stopbit = 0x3;

    struct vip_uart_transmitter_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<32>> sample;
        sc_signal<bool> txdata_rdy;
        sc_signal<sc_uint<8>> txdata;
        sc_signal<sc_uint<9>> shiftreg;
        sc_signal<sc_uint<4>> bitpos;
        sc_signal<bool> overflow;
    };

    void vip_uart_transmitter_r_reset(vip_uart_transmitter_registers& iv) {
        iv.state = idle;
        iv.sample = 0;
        iv.txdata_rdy = 0;
        iv.txdata = 0;
        iv.shiftreg = ~0ull;
        iv.bitpos = 0;
        iv.overflow = 0;
    }

    vip_uart_transmitter_registers v;
    vip_uart_transmitter_registers r;

};

}  // namespace debugger

