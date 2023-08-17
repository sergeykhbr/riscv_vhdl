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

#include "vip_sdcard_top.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_top::vip_sdcard_top(sc_module_name name,
                               bool async_reset)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_sclk("i_sclk"),
    io_cmd("io_cmd"),
    io_dat0("io_dat0"),
    io_dat1("io_dat1"),
    io_dat2("io_dat2"),
    io_cd_dat3("io_cd_dat3") {

    async_reset_ = async_reset;
    iobufcmd0 = 0;

    iobufcmd0 = new iobuf_tech("iobufcmd0");
    iobufcmd0->io(io_cmd);
    iobufcmd0->o(w_cmd_in);
    iobufcmd0->i(w_cmd_out);
    iobufcmd0->t(r.cmd_dir);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sclk;
    sensitive << io_cmd;
    sensitive << io_dat0;
    sensitive << io_dat1;
    sensitive << io_dat2;
    sensitive << io_cd_dat3;
    sensitive << w_clk;
    sensitive << wb_rdata;
    sensitive << w_cmd_in;
    sensitive << w_cmd_out;
    sensitive << r.cmd_dir;
    sensitive << r.cmd_rxshift;
    sensitive << r.cmd_txshift;
    sensitive << r.cmd_state;
    sensitive << r.bitcnt;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_sclk.pos();
}

vip_sdcard_top::~vip_sdcard_top() {
    if (iobufcmd0) {
        delete iobufcmd0;
    }
}

void vip_sdcard_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_sclk, i_sclk.name());
        sc_trace(o_vcd, io_cmd, io_cmd.name());
        sc_trace(o_vcd, io_dat0, io_dat0.name());
        sc_trace(o_vcd, io_dat1, io_dat1.name());
        sc_trace(o_vcd, io_dat2, io_dat2.name());
        sc_trace(o_vcd, io_cd_dat3, io_cd_dat3.name());
        sc_trace(o_vcd, r.cmd_dir, pn + ".r_cmd_dir");
        sc_trace(o_vcd, r.cmd_rxshift, pn + ".r_cmd_rxshift");
        sc_trace(o_vcd, r.cmd_txshift, pn + ".r_cmd_txshift");
        sc_trace(o_vcd, r.cmd_state, pn + ".r_cmd_state");
        sc_trace(o_vcd, r.bitcnt, pn + ".r_bitcnt");
    }

    if (iobufcmd0) {
        iobufcmd0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_sdcard_top::comb() {
    sc_uint<48> vb_cmd_txshift;

    vb_cmd_txshift = 0;

    v = r;

    vb_cmd_txshift = ((r.cmd_txshift.read()(46, 0) << 1) | 1);

    switch (r.cmd_state.read()) {
    case CMDSTATE_IDLE:
        v.cmd_dir = 1;
        if (r.cmd_rxshift.read()(7, 6) == 1) {
            v.cmd_state = CMDSTATE_REQ_ARG;
            v.bitcnt = 31;
        }
        break;
    case CMDSTATE_REQ_ARG:
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_REQ_CRC7;
            v.bitcnt = 6;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_REQ_CRC7:
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_REQ_STOPBIT;
            v.bitcnt = 10;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_REQ_STOPBIT:
        v.cmd_state = CMDSTATE_WAIT_RESP;
        v.cmd_dir = 0;
        break;
    case CMDSTATE_WAIT_RESP:
        // Preparing output with some delay (several clocks):
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_RESP;
            v.bitcnt = 47;
            vb_cmd_txshift(47, 46) = 0;
            vb_cmd_txshift(45, 40) = r.cmd_rxshift.read()(45, 40);
            vb_cmd_txshift(39, 8) = 0x55555555;
            vb_cmd_txshift(7, 0) = 0x7D;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_RESP:
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_IDLE;
            v.cmd_dir = 1;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    default:
        break;
    }

    if (r.cmd_state.read() < CMDSTATE_REQ_STOPBIT) {
        // This will includes clock with the stopbit itself
        v.cmd_rxshift = (r.cmd_rxshift.read()(46, 0), w_cmd_in.read());
        v.cmd_txshift = ~0ull;
    } else {
        if ((r.cmd_state.read() == CMDSTATE_RESP) && (r.bitcnt.read().or_reduce() == 0)) {
            v.cmd_rxshift = ~0ull;
        }
        v.cmd_txshift = vb_cmd_txshift;
    }

    w_cmd_out = r.cmd_txshift.read()[47];
}

void vip_sdcard_top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_top_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

