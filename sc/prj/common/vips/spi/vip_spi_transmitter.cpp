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

#include "vip_spi_transmitter.h"
#include "api_core.h"

namespace debugger {

vip_spi_transmitter::vip_spi_transmitter(sc_module_name name,
                                         bool async_reset,
                                         int scaler)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_csn("i_csn"),
    i_sclk("i_sclk"),
    i_mosi("i_mosi"),
    o_miso("o_miso"),
    o_req_valid("o_req_valid"),
    o_req_write("o_req_write"),
    o_req_addr("o_req_addr"),
    o_req_wdata("o_req_wdata"),
    i_req_ready("i_req_ready"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    o_resp_ready("o_resp_ready") {

    async_reset_ = async_reset;
    scaler_ = scaler;
    scaler_max = ((2 * scaler_) - 1);
    scaler_mid = scaler;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_csn;
    sensitive << i_sclk;
    sensitive << i_mosi;
    sensitive << i_req_ready;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << r.state;
    sensitive << r.sclk;
    sensitive << r.rxshift;
    sensitive << r.txshift;
    sensitive << r.bitcnt;
    sensitive << r.bytecnt;
    sensitive << r.byterdy;
    sensitive << r.req_valid;
    sensitive << r.req_write;
    sensitive << r.req_addr;
    sensitive << r.req_wdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_spi_transmitter::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_csn, i_csn.name());
        sc_trace(o_vcd, i_sclk, i_sclk.name());
        sc_trace(o_vcd, i_mosi, i_mosi.name());
        sc_trace(o_vcd, o_miso, o_miso.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_write, o_req_write.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, i_req_ready, i_req_ready.name());
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, o_resp_ready, o_resp_ready.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.sclk, pn + ".r_sclk");
        sc_trace(o_vcd, r.rxshift, pn + ".r_rxshift");
        sc_trace(o_vcd, r.txshift, pn + ".r_txshift");
        sc_trace(o_vcd, r.bitcnt, pn + ".r_bitcnt");
        sc_trace(o_vcd, r.bytecnt, pn + ".r_bytecnt");
        sc_trace(o_vcd, r.byterdy, pn + ".r_byterdy");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
    }

}

void vip_spi_transmitter::comb() {
    bool v_pos;
    bool v_neg;
    bool v_resp_ready;

    v_pos = 0;
    v_neg = 0;
    v_resp_ready = 0;

    v = r;

    v.byterdy = 0;
    if ((r.req_valid.read() == 1) && (i_req_ready.read() == 1)) {
        v.req_valid = 0;
    }

    v.sclk = i_sclk;
    v_pos = ((!r.sclk.read()) && i_sclk.read());
    v_neg = (r.sclk.read() && (!i_sclk.read()));

    if ((i_csn.read() == 0) && (v_pos == 1)) {
        v.rxshift = (r.rxshift.read()(31, 0), i_mosi.read());
        v.bitcnt = (r.bitcnt.read() + 1);
        if (r.bitcnt.read() == 0x7) {
            v.byterdy = 1;
        }
    } else if (i_csn.read() == 1) {
        v.bitcnt = 0;
    }

    if ((i_csn.read() == 0) && (v_neg == 1)) {
        v_resp_ready = 1;
        if (i_resp_valid.read() == 1) {
            // There's one negedge before CSn goes high:
            v.txshift = i_resp_rdata;
        } else {
            v.txshift = ((r.txshift.read()(31, 0) << 1) | 1);
        }
    }

    if (r.byterdy.read() == 1) {
        switch (r.state.read()) {
        case state_cmd:
            v.bytecnt = 0;
            if (r.rxshift.read()(7, 0) == 0x41) {
                v.state = state_addr;
                v.req_write = 0;                            // Read request
            } else if (r.rxshift.read()(7, 0) == 0x42) {
                v.state = state_addr;
                v.req_write = 1;                            // Write request
            }
            break;
        case state_addr:
            v.bytecnt = (r.bytecnt.read() + 1);
            if (r.bytecnt.read() == 3) {
                v.bytecnt = 0;
                v.state = state_data;
                v.req_addr = r.rxshift;
                v.req_valid = (!r.req_write.read());
            }
            break;
        case state_data:
            v.bytecnt = (r.bytecnt.read() + 1);
            if (r.bytecnt.read() == 3) {
                v.bytecnt = 0;
                v.state = state_cmd;
                v.req_wdata = r.rxshift;
                v.req_valid = r.req_write;
            }
            break;
        default:
            v.state = state_cmd;
            break;
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        vip_spi_transmitter_r_reset(v);
    }

    o_req_valid = r.req_valid;
    o_req_write = r.req_write;
    o_req_addr = r.req_addr;
    o_req_wdata = r.req_wdata;
    o_resp_ready = v_resp_ready;
    o_miso = r.txshift.read()[31];
}

void vip_spi_transmitter::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_spi_transmitter_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

