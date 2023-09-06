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
#include "vip_spi_transmitter.h"

namespace debugger {

SC_MODULE(vip_spi_top) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_csn;
    sc_in<bool> i_sclk;
    sc_in<bool> i_mosi;
    sc_out<bool> o_miso;
    sc_out<bool> o_vip_uart_loopback_ena;
    sc_inout<sc_uint<16>> io_vip_gpio;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_spi_top);

    vip_spi_top(sc_module_name name,
                bool async_reset,
                int instnum,
                int baudrate,
                int scaler);
    virtual ~vip_spi_top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int instnum_;
    int baudrate_;
    int scaler_;
    double pll_period;

    struct vip_spi_top_registers {
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<sc_uint<32>> scratch0;
        sc_signal<sc_uint<32>> scratch1;
        sc_signal<sc_uint<32>> scratch2;
        sc_signal<bool> uart_loopback;
        sc_signal<sc_uint<16>> gpio_out;
        sc_signal<sc_uint<16>> gpio_dir;
    } v, r;

    void vip_spi_top_r_reset(vip_spi_top_registers &iv) {
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.scratch0 = 0;
        iv.scratch1 = 0;
        iv.scratch2 = 0;
        iv.uart_loopback = 0;
        iv.gpio_out = 0;
        iv.gpio_dir = 0;
    }

    sc_signal<bool> w_clk;
    sc_signal<bool> w_req_valid;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<sc_uint<32>> wb_req_wdata;
    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<32>> wb_resp_rdata;
    sc_signal<bool> w_resp_ready;
    sc_signal<sc_uint<16>> wb_gpio_in;

    vip_clk *clk0;
    vip_spi_transmitter *tx0;

};

}  // namespace debugger

