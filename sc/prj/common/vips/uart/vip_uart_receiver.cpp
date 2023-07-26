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

#include "vip_uart_receiver.h"
#include "api_core.h"

namespace debugger {

vip_uart_receiver::vip_uart_receiver(sc_module_name name,
                                     bool async_reset,
                                     int scaler)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_rx("i_rx"),
    o_rdy("o_rdy"),
    i_rdy_clr("i_rdy_clr"),
    o_data("o_data") {

    async_reset_ = async_reset;
    scaler_ = scaler;
    scaler_max = ((2 * scaler) - 1);
    scaler_mid = scaler;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_rx;
    sensitive << i_rdy_clr;
    sensitive << r.state;
    sensitive << r.rdy;
    sensitive << r.rdata;
    sensitive << r.sample;
    sensitive << r.bitpos;
    sensitive << r.scratch;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_uart_receiver::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_rx, i_rx.name());
        sc_trace(o_vcd, o_rdy, o_rdy.name());
        sc_trace(o_vcd, i_rdy_clr, i_rdy_clr.name());
        sc_trace(o_vcd, o_data, o_data.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.rdy, pn + ".r_rdy");
        sc_trace(o_vcd, r.rdata, pn + ".r_rdata");
        sc_trace(o_vcd, r.sample, pn + ".r_sample");
        sc_trace(o_vcd, r.bitpos, pn + ".r_bitpos");
        sc_trace(o_vcd, r.scratch, pn + ".r_scratch");
    }

}

void vip_uart_receiver::comb() {
    v = r;

    if (i_rdy_clr.read() == 1) {
        v.rdy = 0;
    }

    switch (r.state.read()) {
    case startbit:

        // Start counting from the first low sample, once we've
        // sampled a full bit, start collecting data bits.

        if ((i_rx.read() == 0) || (r.sample.read().or_reduce() == 1)) {
            v.sample = (r.sample.read() + 1);
        }

        if (r.sample.read() == scaler_max) {
            v.state = data;
            v.bitpos = 0;
            v.sample = 0;
            v.scratch = 0;
        }
        break;
    case data:
        if (r.sample.read() == scaler_max) {
            v.sample = 0;
        } else {
            v.sample = (r.sample.read() + 1);
        }

        if (r.sample.read() == scaler_mid) {
            v.scratch = (i_rx.read(), r.scratch.read()(7, 1));
            v.bitpos = (r.bitpos.read() + 1);
        }
        if ((r.bitpos.read() == 8) && (r.sample.read() == scaler_mid)) {
            v.state = stopbit;
        }
        break;
    case stopbit:

        // Our baud clock may not be running at exactly the
        // same rate as the transmitter.  If we thing that
        // we're at least half way into the stop bit, allow
        // transition into handling the next start bit.

        if ((r.sample.read() == scaler_max) || ((r.sample.read() >= scaler_mid) && (i_rx.read() == 1))) {
            v.state = startbit;
            v.rdata = r.scratch;
            v.rdy = 1;
            v.sample = 0;
        } else {
            v.sample = (r.sample.read() + 1);
        }
        break;
    default:
        v.state = startbit;
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        vip_uart_receiver_r_reset(v);
    }

    o_rdy = r.rdy;
    o_data = r.rdata;
}

void vip_uart_receiver::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_uart_receiver_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
