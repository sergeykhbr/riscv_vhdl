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

#include "vip_spi_top.h"
#include "api_core.h"

namespace debugger {

vip_spi_top::vip_spi_top(sc_module_name name,
                         bool async_reset,
                         int instnum,
                         int baudrate,
                         int scaler)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_csn("i_csn"),
    i_sclk("i_sclk"),
    i_mosi("i_mosi"),
    o_miso("o_miso"),
    o_vip_uart_loopback_ena("o_vip_uart_loopback_ena"),
    io_vip_gpio("io_vip_gpio") {

    async_reset_ = async_reset;
    instnum_ = instnum;
    baudrate_ = baudrate;
    scaler_ = scaler;
    pll_period = (1.0 / ((2 * scaler_) * baudrate_));
    clk0 = 0;
    tx0 = 0;

    clk0 = new vip_clk("clk0",
                        pll_period);
    clk0->o_clk(w_clk);

    tx0 = new vip_spi_transmitter("tx0", async_reset,
                                   scaler);
    tx0->i_nrst(i_nrst);
    tx0->i_clk(w_clk);
    tx0->i_csn(i_csn);
    tx0->i_sclk(i_sclk);
    tx0->i_mosi(i_mosi);
    tx0->o_miso(o_miso);
    tx0->o_req_valid(w_req_valid);
    tx0->o_req_write(w_req_write);
    tx0->o_req_addr(wb_req_addr);
    tx0->o_req_wdata(wb_req_wdata);
    tx0->i_req_ready(w_req_ready);
    tx0->i_resp_valid(w_resp_valid);
    tx0->i_resp_rdata(wb_resp_rdata);
    tx0->o_resp_ready(w_resp_ready);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_csn;
    sensitive << i_sclk;
    sensitive << i_mosi;
    sensitive << io_vip_gpio;
    sensitive << w_clk;
    sensitive << w_req_valid;
    sensitive << w_req_write;
    sensitive << wb_req_addr;
    sensitive << wb_req_wdata;
    sensitive << w_req_ready;
    sensitive << w_resp_valid;
    sensitive << wb_resp_rdata;
    sensitive << w_resp_ready;
    sensitive << wb_gpio_in;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.scratch0;
    sensitive << r.scratch1;
    sensitive << r.scratch2;
    sensitive << r.uart_loopback;
    sensitive << r.gpio_out;
    sensitive << r.gpio_dir;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << w_clk.posedge_event();
}

vip_spi_top::~vip_spi_top() {
    if (clk0) {
        delete clk0;
    }
    if (tx0) {
        delete tx0;
    }
}

void vip_spi_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_csn, i_csn.name());
        sc_trace(o_vcd, i_sclk, i_sclk.name());
        sc_trace(o_vcd, i_mosi, i_mosi.name());
        sc_trace(o_vcd, o_miso, o_miso.name());
        sc_trace(o_vcd, o_vip_uart_loopback_ena, o_vip_uart_loopback_ena.name());
        sc_trace(o_vcd, io_vip_gpio, io_vip_gpio.name());
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.scratch0, pn + ".r_scratch0");
        sc_trace(o_vcd, r.scratch1, pn + ".r_scratch1");
        sc_trace(o_vcd, r.scratch2, pn + ".r_scratch2");
        sc_trace(o_vcd, r.uart_loopback, pn + ".r_uart_loopback");
        sc_trace(o_vcd, r.gpio_out, pn + ".r_gpio_out");
        sc_trace(o_vcd, r.gpio_dir, pn + ".r_gpio_dir");
    }

    if (clk0) {
        clk0->generateVCD(i_vcd, o_vcd);
    }
    if (tx0) {
        tx0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_spi_top::comb() {
    sc_uint<32> rdata;
    bool vb_gpio_in;

    rdata = 0;
    vb_gpio_in = 0;

    v = r;

    rdata = r.resp_rdata;

    if ((r.resp_valid.read() == 1) && (w_resp_ready.read() == 1)) {
        v.resp_valid = 0;
    } else if (w_req_valid.read() == 1) {
        v.resp_valid = 1;
    }

    switch (wb_req_addr.read()(7, 2)) {
    case 0x00:                                              // [0x00] hwid
        rdata = 0xCAFECAFE;
        break;
    case 0x01:                                              // [0x04] scratch0
        rdata = r.scratch0;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scratch0 = wb_req_wdata;
        }
        break;
    case 0x02:                                              // [0x08] scratch1
        rdata = r.scratch1;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scratch1 = wb_req_wdata;
        }
        break;
    case 0x03:                                              // [0x0C] scratch2
        rdata = r.scratch2;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scratch2 = wb_req_wdata;
        }
        break;
    case 0x04:                                              // [0x10] uart control
        rdata[0] = r.uart_loopback;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.uart_loopback = wb_req_wdata.read()[0];
        }
        break;
    case 0x05:                                              // [0x14] gpio in
        rdata(15, 0) = wb_gpio_in;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.gpio_out = wb_req_wdata.read()(15, 0);
        }
        break;
    case 0x06:                                              // [0x18] gpio direction
        rdata(15, 0) = r.gpio_dir;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.gpio_dir = wb_req_wdata.read()(15, 0);
        }
        break;
    default:
        break;
    }
    v.resp_rdata = rdata;

    if (!async_reset_ && i_nrst.read() == 0) {
        vip_spi_top_r_reset(v);
    }

    w_req_ready = 1;
    w_resp_valid = r.resp_valid;
    wb_resp_rdata = r.resp_rdata;
    o_vip_uart_loopback_ena = r.uart_loopback;
}

void vip_spi_top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_spi_top_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

