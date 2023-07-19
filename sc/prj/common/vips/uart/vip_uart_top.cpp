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
                           double half_period,
                           int scaler)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_rx("i_rx") {

    async_reset_ = async_reset;
    half_period_ = half_period;
    scaler_ = scaler;
    clk0 = 0;
    rx0 = 0;
    strOut_ = "";

    clk0 = new vip_clk("clk0",
                        half_period / scaler);
    clk0->o_clk(w_clk);


    rx0 = new vip_uart_receiver("rx0", async_reset,
                                 scaler);
    rx0->i_nrst(i_nrst);
    rx0->i_clk(w_clk);
    rx0->i_rx(i_rx);
    rx0->o_rdy(w_rdy);
    rx0->i_rdy_clr(w_rdy_clr);
    rx0->o_data(wb_rdata);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_rx;
    sensitive << w_clk;
    sensitive << w_rdy;
    sensitive << w_rdy_clr;
    sensitive << wb_rdata;

    SC_METHOD(registers);
    sensitive << w_clk.posedge_event();
}

vip_uart_top::~vip_uart_top() {
    if (clk0) {
        delete clk0;
    }
    if (rx0) {
        delete rx0;
    }
}

void vip_uart_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_rx, i_rx.name());
    }

    if (clk0) {
        clk0->generateVCD(i_vcd, o_vcd);
    }
    if (rx0) {
        rx0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_uart_top::comb() {
    w_rdy_clr = w_rdy;
}

void vip_uart_top::registers() {
    if (w_rdy.read() == 1) {
        if (wb_rdata.read() == 0x0A) {
            // End of line
            RISCV_printf(0, LOG_INFO, "%s", strOut_.c_str());
            strOut_ = "";
        } else if (wb_rdata.read() == 0x0D) {
            if (strOut_.size()) {
                RISCV_printf(0, LOG_INFO, "%s", strOut_.c_str());
                strOut_ = "";
            }
        } else {
            // Add symbol to string
            char tstr[2] = {0};
            tstr[0] = static_cast<char>(wb_rdata.read().to_uint());
            strOut_ += tstr;
        }
    }
}

}  // namespace debugger

