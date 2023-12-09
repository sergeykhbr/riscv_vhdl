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
    o_tx("o_tx"),
    i_loopback_ena("i_loopback_ena") {

    async_reset_ = async_reset;
    instnum_ = instnum;
    baudrate_ = baudrate;
    scaler_ = scaler;
    logpath_ = logpath;
    pll_period = (1.0 / ((2 * scaler_) * baudrate_));
    clk0 = 0;
    rx0 = 0;
    tx0 = 0;

    // initial
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_%1d.log",
            logpath_.c_str(),
            instnum_);
    outfilename = std::string(tstr);
    fl = fopen(outfilename.c_str(), "wb");

    // Output string with each new symbol ended
    RISCV_sprintf(tstr, sizeof(tstr), "%s_%1d.log.tmp",
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
    tx0->i_we(w_tx_we);
    tx0->i_wdata(wb_rdata);
    tx0->o_full(w_tx_full);
    tx0->o_tx(o_tx);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_rx;
    sensitive << i_loopback_ena;
    sensitive << w_clk;
    sensitive << w_rx_rdy;
    sensitive << w_rx_rdy_clr;
    sensitive << w_tx_we;
    sensitive << w_tx_full;
    sensitive << wb_rdata;
    sensitive << wb_rdataz;
    sensitive << r.initdone;

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
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_rx, i_rx.name());
        sc_trace(o_vcd, o_tx, o_tx.name());
        sc_trace(o_vcd, i_loopback_ena, i_loopback_ena.name());
        sc_trace(o_vcd, r.initdone, pn + ".r_initdone");
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

std::string vip_uart_top::U8ToString(
        std::string istr,
        sc_uint<8> symb) {
    char tstr[256];
    std::string ostr;

    int tsz = RISCV_sprintf(tstr, sizeof(tstr), "%s", istr.c_str());
    tstr[tsz++] = static_cast<char>(symb.to_uint());
    tstr[tsz] = 0;
    ostr = tstr;
    return ostr;
}

void vip_uart_top::comb() {
    v = r;

    w_tx_we = (w_rx_rdy.read() & i_loopback_ena.read());
    w_rx_rdy_clr = (!w_tx_full.read());
    v.initdone = ((r.initdone.read()[0] << 1) | 1);

    if (!async_reset_ && i_nrst.read() == 0) {
        vip_uart_top_r_reset(v);
    }
}

void vip_uart_top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_uart_top_r_reset(r);
    } else {
        r = v;
    }

    if (r.initdone.read()[1] == 0) {
        outstrtmp = "";
        outstrtmp = U8ToString(outstrtmp,
                EOF_0x0D);
    }

    if (w_rx_rdy.read() == 1) {
        if ((wb_rdata.read() == 0x0A) && (wb_rdataz.read() != 0x0D)) {
            // Create CR LF (0xd 0xa) instead of 0x0a:
            outstr = U8ToString(outstr,
                    EOF_0x0D);
        }
        // Add symbol to string:
        outstr = U8ToString(outstr,
                wb_rdata);
        outstrtmp = U8ToString(outstrtmp,
                wb_rdata);

        if (wb_rdata.read() == 0x0A) {
            // Output simple string:
            SV_display(outstr.c_str());
            fwrite(outstr.c_str(), 1, outstr.size(), fl);
            fflush(fl);
        }

        // Output string with the line ending symbol 0x0D first:
        fwrite(outstrtmp.c_str(), 1, outstrtmp.size(), fl_tmp);
        fflush(fl_tmp);

        // End-of-line
        if (wb_rdata.read() == 0x0A) {
            outstr = "";
            outstrtmp = "";
            outstrtmp = U8ToString(outstrtmp,
                    EOF_0x0D);
        }
        wb_rdataz = wb_rdata;
    }
}

}  // namespace debugger

