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

#include "vip_uart_transmitter.h"
#include "api_core.h"

namespace debugger {

vip_uart_transmitter::vip_uart_transmitter(sc_module_name name,
                                           bool async_reset,
                                           int scaler)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_we("i_we"),
    i_wdata("i_wdata"),
    o_full("o_full"),
    o_tx("o_tx") {

    async_reset_ = async_reset;
    scaler_ = scaler;
    scaler_max = ((2 * scaler_) - 1);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_we;
    sensitive << i_wdata;
    sensitive << r.state;
    sensitive << r.sample;
    sensitive << r.txdata_rdy;
    sensitive << r.txdata;
    sensitive << r.shiftreg;
    sensitive << r.bitpos;
    sensitive << r.overflow;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_uart_transmitter::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_full, o_full.name());
        sc_trace(o_vcd, o_tx, o_tx.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.sample, pn + ".r_sample");
        sc_trace(o_vcd, r.txdata_rdy, pn + ".r_txdata_rdy");
        sc_trace(o_vcd, r.txdata, pn + ".r_txdata");
        sc_trace(o_vcd, r.shiftreg, pn + ".r_shiftreg");
        sc_trace(o_vcd, r.bitpos, pn + ".r_bitpos");
        sc_trace(o_vcd, r.overflow, pn + ".r_overflow");
    }

}

void vip_uart_transmitter::comb() {
    bool v_next;

    v_next = 0;

    v = r;

    if (i_we.read() == 1) {
        if (r.txdata_rdy.read() == 1) {
            v.overflow = 1;
        } else {
            v.txdata_rdy = 1;
            v.txdata = i_wdata;
        }
    }

    if (r.sample.read() == scaler_max) {
        v.sample = 0;
        v_next = 1;
    } else {
        v.sample = (r.sample.read() + 1);
    }

    if (v_next == 1) {
        switch (r.state.read()) {
        case idle:
            if (r.txdata_rdy.read() == 1) {
                v.txdata_rdy = 0;
                v.overflow = 0;
                v.shiftreg = (r.txdata.read() << 1);
                v.state = startbit;
            }
            break;
        case startbit:
            v.state = data;
            v.bitpos = 0;
            v.shiftreg = (1, r.shiftreg.read()(8, 1));
            break;
        case data:
            if (r.bitpos.read() == 7) {
                v.state = idle;                             // No dummy bit at the end
                v.shiftreg = ~0ull;
            } else {
                v.shiftreg = (1, r.shiftreg.read()(8, 1));
            }
            v.bitpos = (r.bitpos.read() + 1);
            break;
        case stopbit:
            v.state = idle;
            break;
        default:
            break;
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        vip_uart_transmitter_r_reset(v);
    }

    o_full = r.txdata_rdy;
    o_tx = r.shiftreg.read()[0];
}

void vip_uart_transmitter::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_uart_transmitter_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

