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

#include "vip_i2c_s.h"
#include "api_core.h"

namespace debugger {

vip_i2c_s::vip_i2c_s(sc_module_name name,
                     bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_scl("i_scl"),
    i_sda("i_sda"),
    o_sda("o_sda"),
    o_sda_dir("o_sda_dir") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_scl;
    sensitive << i_sda;
    sensitive << r.sda;
    sensitive << r.scl;
    sensitive << r.state;
    sensitive << r.control_start;
    sensitive << r.control_stop;
    sensitive << r.addr;
    sensitive << r.read;
    sensitive << r.rdata;
    sensitive << r.sda_dir;
    sensitive << r.txbyte;
    sensitive << r.rxbyte;
    sensitive << r.bit_cnt;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_i2c_s::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_scl, i_scl.name());
        sc_trace(o_vcd, i_sda, i_sda.name());
        sc_trace(o_vcd, o_sda, o_sda.name());
        sc_trace(o_vcd, o_sda_dir, o_sda_dir.name());
        sc_trace(o_vcd, r.sda, pn + ".r.sda");
        sc_trace(o_vcd, r.scl, pn + ".r.scl");
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.control_start, pn + ".r.control_start");
        sc_trace(o_vcd, r.control_stop, pn + ".r.control_stop");
        sc_trace(o_vcd, r.addr, pn + ".r.addr");
        sc_trace(o_vcd, r.read, pn + ".r.read");
        sc_trace(o_vcd, r.rdata, pn + ".r.rdata");
        sc_trace(o_vcd, r.sda_dir, pn + ".r.sda_dir");
        sc_trace(o_vcd, r.txbyte, pn + ".r.txbyte");
        sc_trace(o_vcd, r.rxbyte, pn + ".r.rxbyte");
        sc_trace(o_vcd, r.bit_cnt, pn + ".r.bit_cnt");
    }

}

void vip_i2c_s::comb() {
    bool v_latch_data;

    v = r;
    v_latch_data = 0;

    v.sda = i_sda.read();
    v.scl = i_scl.read();

    v.control_start = (r.sda.read() & (!i_sda.read()) & i_scl.read());
    v.control_stop = ((!r.sda.read()) & i_sda.read() & i_scl.read());
    v_latch_data = ((!r.scl.read()) & i_scl.read());
    if (v_latch_data == 1) {
        v.rxbyte = (r.rxbyte.read()(6, 0), i_sda.read());
    }

    // Transmitter's state machine:
    switch (r.state.read()) {
    case STATE_IDLE:
        v.txbyte = 0;
        v.addr = 0;
        v.read = 0;
        v.rdata = 0;
        v.sda_dir = PIN_DIR_INPUT;
        if (r.control_start.read() == 1) {
            // Start condition SDA goes LOW while SCL is HIGH
            v.bit_cnt = 7;
            v.state = STATE_HEADER;
        }
        break;

    case STATE_HEADER:
        if (v_latch_data == 1) {
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.sda_dir = PIN_DIR_OUTPUT;
                v.state = STATE_ACK_HEADER;
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_ACK_HEADER:
        if (v_latch_data == 1) {
            v.addr = r.rxbyte.read()(7, 1);
            v.read = r.rxbyte.read()[0];
            v.bit_cnt = 7;
            if (r.rxbyte.read()[0] == 0) {
                // 0=write
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_RX_DATA;
            } else {
                // 1=read
                v.state = STATE_TX_DATA;
                v.txbyte = 0x081;                           // Some random value to transmit as 1-st byte
            }
        }
        break;

    case STATE_RX_DATA:
        if (v_latch_data == 1) {
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.sda_dir = PIN_DIR_OUTPUT;
                v.state = STATE_ACK_DATA;
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_ACK_DATA:
        if (v_latch_data == 1) {
            v.rdata = r.rxbyte.read();
            if (i_sda.read() == 1) {
                // Not acked (last byte)
                v.state = STATE_IDLE;
            } else {
                v.bit_cnt = 7;
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_RX_DATA;
            }
        }
        break;

    case STATE_TX_DATA:
        if (v_latch_data == 1) {
            v.txbyte = (r.txbyte.read()(7, 0) << 1);
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_WAIT_ACK_DATA;
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_WAIT_ACK_DATA:
        if (v_latch_data == 1) {
            v.sda_dir = PIN_DIR_OUTPUT;
            if (i_sda.read() == 1) {
                v.state = STATE_IDLE;
            } else {
                v.bit_cnt = 7;
                v.txbyte = 0x085;                           // Some random value to transmit
                v.state = STATE_TX_DATA;
            }
        }
        break;

    default:
        v.state = STATE_IDLE;
        break;
    }

    if ((r.state.read() != STATE_IDLE) && (r.control_stop.read() == 1)) {
        v.state = 0;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        vip_i2c_s_r_reset(v);
    }

    o_sda = r.txbyte.read()[8];
    o_sda_dir = r.sda_dir.read();
}

void vip_i2c_s::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        vip_i2c_s_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

