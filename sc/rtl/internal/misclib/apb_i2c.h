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
#include "../ambalib/types_amba.h"
#include "../ambalib/types_pnp.h"
#include "../ambalib/apb_slv.h"

namespace debugger {

SC_MODULE(apb_i2c) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_scl;                                     // Clock output upto 400 kHz (default 100 kHz)
    sc_out<bool> o_sda;                                     // Data output (tri-state buffer input)
    sc_out<bool> o_sda_dir;                                 // Data to control tri-state buffer
    sc_in<bool> i_sda;                                      // Tri-state buffer output
    sc_out<bool> o_irq;                                     // Interrupt request
    sc_out<bool> o_nreset;                                  // I2C slave reset. PCA9548 I2C mux must be de-asserted.

    void comb();
    void registers();

    apb_i2c(sc_module_name name,
            bool async_reset);
    virtual ~apb_i2c();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // SPI states
    static const uint8_t STATE_IDLE = 0x00;
    static const uint8_t STATE_START = 0x01;
    static const uint8_t STATE_HEADER = 0x02;
    static const uint8_t STATE_ACK_HEADER = 0x04;
    static const uint8_t STATE_RX_DATA = 0x08;
    static const uint8_t STATE_ACK_DATA = 0x10;
    static const uint8_t STATE_TX_DATA = 0x20;
    static const uint8_t STATE_WAIT_ACK_DATA = 0x40;
    static const uint8_t STATE_STOP = 0x80;

    static const bool PIN_DIR_INPUT = 1;
    static const bool PIN_DIR_OUTPUT = 0;

    struct apb_i2c_registers {
        sc_signal<sc_uint<16>> scaler;
        sc_signal<sc_uint<16>> scaler_cnt;
        sc_signal<sc_uint<16>> setup_time;                  // Interval after negedge of the clock pulsse
        sc_signal<bool> level;
        sc_signal<sc_uint<7>> addr;                         // I2C multiplexer
        sc_signal<bool> R_nW;                               // 0=Write; 1=read
        sc_signal<sc_uint<32>> payload;
        sc_signal<sc_uint<8>> state;
        sc_signal<bool> start;
        sc_signal<bool> sda_dir;
        sc_signal<sc_uint<19>> shiftreg;                    // 1start+7adr+1rw+1ack+8data+ack
        sc_signal<sc_uint<8>> rxbyte;
        sc_signal<sc_uint<3>> bit_cnt;
        sc_signal<sc_uint<4>> byte_cnt;
        sc_signal<bool> ack;
        sc_signal<bool> err_ack_header;
        sc_signal<bool> err_ack_data;
        sc_signal<bool> irq;
        sc_signal<bool> ie;
        sc_signal<bool> nreset;                             // Active LOW (by default), could be any
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    };

    void apb_i2c_r_reset(apb_i2c_registers& iv) {
        iv.scaler = 0;
        iv.scaler_cnt = 0;
        iv.setup_time = 0x0001;
        iv.level = 1;
        iv.addr = 0x74;
        iv.R_nW = 0;
        iv.payload = 0;
        iv.state = STATE_IDLE;
        iv.start = 0;
        iv.sda_dir = PIN_DIR_OUTPUT;
        iv.shiftreg = ~0ull;
        iv.rxbyte = 0;
        iv.bit_cnt = 0;
        iv.byte_cnt = 0;
        iv.ack = 0;
        iv.err_ack_header = 0;
        iv.err_ack_data = 0;
        iv.irq = 0;
        iv.ie = 0;
        iv.nreset = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;
    apb_i2c_registers v;
    apb_i2c_registers r;

    apb_slv *pslv0;

};

}  // namespace debugger

