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

#include "vip_uart_top.h"
#include "api_core.h"

namespace debugger {

vip_uart_top::vip_uart_top(sc_module_name name,
                           bool async_reset,
                           int instnum,
                           int baudrate,
                           int scaler,
                           std::string logpath)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_rx("i_rx"),
    o_tx("o_tx") {

    async_reset_ = async_reset;
    instnum_ = instnum;
    baudrate_ = baudrate;
    scaler_ = scaler;
    logpath_ = logpath;
    pll_period = (1.0 / ((2 * scaler) * baudrate));
    clk0 = 0;
    rx0 = 0;
    tx0 = 0;

    // initial
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_%d.log",
            logpath_.c_str(),
            instnum_);
    outfilename = std::string(tstr);
    fl = fopen(outfilename.c_str(), "wb");

    // Output string with each new symbol ended
    RISCV_sprintf(tstr, sizeof(tstr), "%s_%d.log.tmp",
            logpath_.c_str(),
            instnum_);
    outfilename = std::string(tstr);
    fl_tmp = fopen(outfilename.c_str(), "wb");

    // end initial

    clk0 = new vip_clk("clk0",
                        pll_period);
    clk0->o_clk(w_clk);


    rx0 = new vip_uart_receiver("rx0", async_reset,
                                 scaler);
    rx0->i_nrst(i_nrst);
    rx0->i_clk(w_clk);
    rx0->i_rx(i_rx);
    rx0->o_rdy(w_rx_rdy);
    rx0->i_rdy_clr(w_rx_rdy_clr);
    rx0->o_data(wb_rdata);


    tx0 = new vip_uart_transmitter("tx0", async_reset,
                                    scaler);
    tx0->i_nrst(i_nrst);
    tx0->i_clk(w_clk);
    tx0->i_we(w_rx_rdy);
    tx0->i_wdata(wb_rdata);
    tx0->o_full(w_tx_full);
    tx0->o_tx(o_tx);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_rx;
    sensitive << w_clk;
    sensitive << w_rx_rdy;
    sensitive << w_rx_rdy_clr;
    sensitive << w_tx_full;
    sensitive << wb_rdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << w_clk.posedge_event();
}

vip_uart_top::~vip_uart_top() {
    if (clk0) {
        delete clk0;
    }
    if (rx0) {
        delete rx0;
    }
    if (tx0) {
        delete tx0;
    }
}

void vip_uart_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_rx, i_rx.name());
        sc_trace(o_vcd, o_tx, o_tx.name());
    }

    if (clk0) {
        clk0->generateVCD(i_vcd, o_vcd);
    }
    if (rx0) {
        rx0->generateVCD(i_vcd, o_vcd);
    }
    if (tx0) {
        tx0->generateVCD(i_vcd, o_vcd);
    }
}

std::string vip_uart_top::U8ToString(sc_uint<8> symb) {
    char tstr[256];
    std::string ostr;

    tstr[0] = static_cast<char>(symb.to_uint());
    tstr[1] = 0;
    ostr = tstr;
    return ostr;
}

void vip_uart_top::comb() {
    w_rx_rdy_clr = w_rx_rdy;
}

void vip_uart_top::registers() {

    if (w_rx_rdy.read() == 1) {
        if (wb_rdata.read() == 0x0A) {
            // Use 0x0d as a new symbol in fl_tmp file:
            rdatastr = U8ToString(EOF_0x0D);
            fwrite(rdatastr.c_str(), 1, rdatastr.size(), fl_tmp);
            fwrite(outstr.c_str(), 1, outstr.size(), fl_tmp);
            fflush(fl_tmp);

            SV_display(outstr.c_str());
            fwrite(outstr.c_str(), 1, outstr.size(), fl);
            fflush(fl);
            outstr = "";
        } else if (wb_rdata.read() == 0x0D) {
            if (outstr != "") {
                SV_display(outstr.c_str());
                fwrite(outstr.c_str(), 1, outstr.size(), fl);
                fflush(fl);
                outstr = "";
            }
        } else {
            // Add symbol to string
            rdatastr = U8ToString(wb_rdata);
            outstr += rdatastr;

            // Output string with the line ending symbol 0x0D first:
            rdatastr = U8ToString(EOF_0x0D);
            fwrite(rdatastr.c_str(), 1, rdatastr.size(), fl_tmp);
            fwrite(outstr.c_str(), 1, outstr.size(), fl_tmp);
            fflush(fl_tmp);
        }
    }
}

}  // namespace debugger

