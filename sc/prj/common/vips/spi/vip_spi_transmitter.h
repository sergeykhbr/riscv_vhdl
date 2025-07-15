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

SC_MODULE(vip_spi_transmitter) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_csn;
    sc_in<bool> i_sclk;
    sc_in<bool> i_mosi;
    sc_out<bool> o_miso;
    sc_out<bool> o_req_valid;
    sc_out<bool> o_req_write;
    sc_out<sc_uint<32>> o_req_addr;
    sc_out<sc_uint<32>> o_req_wdata;
    sc_in<bool> i_req_ready;
    sc_in<bool> i_resp_valid;
    sc_in<sc_uint<32>> i_resp_rdata;
    sc_out<bool> o_resp_ready;

    void comb();
    void registers();

    vip_spi_transmitter(sc_module_name name,
                        bool async_reset,
                        int scaler);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int scaler_;
    int scaler_max;
    int scaler_mid;

    static const uint8_t state_cmd = 0;
    static const uint8_t state_addr = 1;
    static const uint8_t state_data = 2;

    struct vip_spi_transmitter_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> sclk;
        sc_signal<sc_uint<32>> rxshift;
        sc_signal<sc_uint<32>> txshift;
        sc_signal<sc_uint<4>> bitcnt;
        sc_signal<sc_uint<3>> bytecnt;
        sc_signal<bool> byterdy;
        sc_signal<bool> req_valid;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<32>> req_addr;
        sc_signal<sc_uint<32>> req_wdata;
    };

    void vip_spi_transmitter_r_reset(vip_spi_transmitter_registers& iv) {
        iv.state = state_cmd;
        iv.sclk = 0;
        iv.rxshift = ~0ull;
        iv.txshift = ~0ull;
        iv.bitcnt = 0;
        iv.bytecnt = 0;
        iv.byterdy = 0;
        iv.req_valid = 0;
        iv.req_write = 0;
        iv.req_addr = 0;
        iv.req_wdata = 0;
    }

    vip_spi_transmitter_registers v;
    vip_spi_transmitter_registers r;

};

}  // namespace debugger

