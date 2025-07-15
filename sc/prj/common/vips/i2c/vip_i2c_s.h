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

SC_MODULE(vip_i2c_s) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_scl;                                      // Clock upto 400 kHz (default 100 kHz)
    sc_in<bool> i_sda;                                      // Tri-state buffer output
    sc_out<bool> o_sda;                                     // Data output (tri-state buffer input)
    sc_out<bool> o_sda_dir;                                 // Data to control tri-state buffer

    void comb();
    void registers();

    vip_i2c_s(sc_module_name name,
              bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // SPI states
    static const uint8_t STATE_IDLE = 0x00;
    static const uint8_t STATE_HEADER = 0x02;
    static const uint8_t STATE_ACK_HEADER = 0x04;
    static const uint8_t STATE_RX_DATA = 0x08;
    static const uint8_t STATE_ACK_DATA = 0x10;
    static const uint8_t STATE_TX_DATA = 0x20;
    static const uint8_t STATE_WAIT_ACK_DATA = 0x40;

    static const bool PIN_DIR_INPUT = 1;
    static const bool PIN_DIR_OUTPUT = 0;

    struct vip_i2c_s_registers {
        sc_signal<bool> sda;
        sc_signal<bool> scl;
        sc_signal<sc_uint<8>> state;
        sc_signal<bool> control_start;
        sc_signal<bool> control_stop;
        sc_signal<sc_uint<7>> addr;
        sc_signal<bool> read;
        sc_signal<sc_uint<8>> rdata;
        sc_signal<bool> sda_dir;
        sc_signal<sc_uint<9>> txbyte;                       // ack + data
        sc_signal<sc_uint<8>> rxbyte;
        sc_signal<sc_uint<3>> bit_cnt;
    };

    void vip_i2c_s_r_reset(vip_i2c_s_registers& iv) {
        iv.sda = 1;
        iv.scl = 1;
        iv.state = STATE_IDLE;
        iv.control_start = 0;
        iv.control_stop = 0;
        iv.addr = 0;
        iv.read = 0;
        iv.rdata = 0;
        iv.sda_dir = PIN_DIR_OUTPUT;
        iv.txbyte = ~0ull;
        iv.rxbyte = 0;
        iv.bit_cnt = 0;
    }

    vip_i2c_s_registers v;
    vip_i2c_s_registers r;

};

}  // namespace debugger

