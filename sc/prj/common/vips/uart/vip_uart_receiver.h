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

SC_MODULE(vip_uart_receiver) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_rx;
    sc_out<bool> o_rdy;
    sc_in<bool> i_rdy_clr;
    sc_out<sc_uint<8>> o_data;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_uart_receiver);

    vip_uart_receiver(sc_module_name name,
                      bool async_reset,
                      int scaler);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int scaler_;
    int scaler_max;
    int scaler_mid;

    static const uint8_t startbit = 0;
    static const uint8_t data = 1;
    static const uint8_t stopbit = 2;
    static const uint8_t dummy = 3;

    struct vip_uart_receiver_registers {
        sc_signal<bool> rx;
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> rdy;
        sc_signal<sc_uint<8>> rdata;
        sc_signal<sc_uint<32>> sample;
        sc_signal<sc_uint<4>> bitpos;
        sc_signal<sc_uint<8>> scratch;
        sc_signal<bool> rx_err;
    } v, r;

    void vip_uart_receiver_r_reset(vip_uart_receiver_registers &iv) {
        iv.rx = 0;
        iv.state = startbit;
        iv.rdy = 0;
        iv.rdata = 0;
        iv.sample = 0;
        iv.bitpos = 0;
        iv.scratch = 0;
        iv.rx_err = 0;
    }

};

}  // namespace debugger

