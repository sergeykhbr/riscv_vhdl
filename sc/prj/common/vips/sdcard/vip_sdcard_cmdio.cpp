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

#include "vip_sdcard_cmdio.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_cmdio::vip_sdcard_cmdio(sc_module_name name,
                                   bool async_reset)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_cmd("i_cmd"),
    o_cmd("o_cmd"),
    o_cmd_dir("o_cmd_dir"),
    o_cmd_req_valid("o_cmd_req_valid"),
    o_cmd_req_cmd("o_cmd_req_cmd"),
    o_cmd_req_data("o_cmd_req_data"),
    i_cmd_req_ready("i_cmd_req_ready"),
    i_cmd_resp_valid("i_cmd_resp_valid"),
    i_cmd_resp_data32("i_cmd_resp_data32"),
    o_cmd_resp_ready("o_cmd_resp_ready") {

    async_reset_ = async_reset;
    crccmd0 = 0;

    crccmd0 = new vip_sdcard_crc7("crccmd0", async_reset);
    crccmd0->i_clk(i_clk);
    crccmd0->i_nrst(i_nrst);
    crccmd0->i_clear(w_crc7_clear);
    crccmd0->i_next(w_crc7_next);
    crccmd0->i_dat(w_crc7_dat);
    crccmd0->o_crc7(wb_crc7);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_cmd;
    sensitive << i_cmd_req_ready;
    sensitive << i_cmd_resp_valid;
    sensitive << i_cmd_resp_data32;
    sensitive << w_cmd_out;
    sensitive << w_crc7_clear;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << wb_crc7;
    sensitive << r.clkcnt;
    sensitive << r.cmdz;
    sensitive << r.cmd_dir;
    sensitive << r.cmd_rxshift;
    sensitive << r.cmd_txshift;
    sensitive << r.cmd_state;
    sensitive << r.bitcnt;
    sensitive << r.txbit;
    sensitive << r.crc_calc;
    sensitive << r.crc_rx;
    sensitive << r.cmd_req_valid;
    sensitive << r.cmd_req_cmd;
    sensitive << r.cmd_req_data;
    sensitive << r.cmd_resp_ready;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

vip_sdcard_cmdio::~vip_sdcard_cmdio() {
    if (crccmd0) {
        delete crccmd0;
    }
}

void vip_sdcard_cmdio::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_cmd, i_cmd.name());
        sc_trace(o_vcd, o_cmd, o_cmd.name());
        sc_trace(o_vcd, o_cmd_dir, o_cmd_dir.name());
        sc_trace(o_vcd, o_cmd_req_valid, o_cmd_req_valid.name());
        sc_trace(o_vcd, o_cmd_req_cmd, o_cmd_req_cmd.name());
        sc_trace(o_vcd, o_cmd_req_data, o_cmd_req_data.name());
        sc_trace(o_vcd, i_cmd_req_ready, i_cmd_req_ready.name());
        sc_trace(o_vcd, i_cmd_resp_valid, i_cmd_resp_valid.name());
        sc_trace(o_vcd, i_cmd_resp_data32, i_cmd_resp_data32.name());
        sc_trace(o_vcd, o_cmd_resp_ready, o_cmd_resp_ready.name());
        sc_trace(o_vcd, r.clkcnt, pn + ".r_clkcnt");
        sc_trace(o_vcd, r.cmdz, pn + ".r_cmdz");
        sc_trace(o_vcd, r.cmd_dir, pn + ".r_cmd_dir");
        sc_trace(o_vcd, r.cmd_rxshift, pn + ".r_cmd_rxshift");
        sc_trace(o_vcd, r.cmd_txshift, pn + ".r_cmd_txshift");
        sc_trace(o_vcd, r.cmd_state, pn + ".r_cmd_state");
        sc_trace(o_vcd, r.bitcnt, pn + ".r_bitcnt");
        sc_trace(o_vcd, r.txbit, pn + ".r_txbit");
        sc_trace(o_vcd, r.crc_calc, pn + ".r_crc_calc");
        sc_trace(o_vcd, r.crc_rx, pn + ".r_crc_rx");
        sc_trace(o_vcd, r.cmd_req_valid, pn + ".r_cmd_req_valid");
        sc_trace(o_vcd, r.cmd_req_cmd, pn + ".r_cmd_req_cmd");
        sc_trace(o_vcd, r.cmd_req_data, pn + ".r_cmd_req_data");
        sc_trace(o_vcd, r.cmd_resp_ready, pn + ".r_cmd_resp_ready");
    }

    if (crccmd0) {
        crccmd0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_sdcard_cmdio::comb() {
    sc_uint<48> vb_cmd_txshift;
    bool v_crc7_clear;
    bool v_crc7_next;
    bool v_crc7_in;

    vb_cmd_txshift = 0;
    v_crc7_clear = 0;
    v_crc7_next = 0;
    v_crc7_in = 0;

    v = r;

    vb_cmd_txshift = ((r.cmd_txshift.read()(46, 0) << 1) | 1);
    v_crc7_in = i_cmd;

    if (i_cmd_req_ready.read() == 1) {
        v.cmd_req_valid = 0;
    }
    v.clkcnt = (r.clkcnt.read() + 1);

    switch (r.cmd_state.read()) {
    case CMDSTATE_INIT:
        v.cmd_dir = 1;
        v_crc7_clear = 1;
        // Wait several (72) clocks to switch into idle state
        if (r.clkcnt.read() == 70) {
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
        }
        break;
    case CMDSTATE_REQ_STARTBIT:
        if ((r.cmdz.read() == 1) && (i_cmd.read() == 0)) {
            v_crc7_next = 1;
            v.cmd_state = CMDSTATE_REQ_TXBIT;
        } else {
            v_crc7_clear = 1;
        }
        break;
    case CMDSTATE_REQ_TXBIT:
        v.cmd_state = CMDSTATE_REQ_CMD;
        v.bitcnt = 5;
        v_crc7_next = 1;
        v.txbit = i_cmd;
        break;
    case CMDSTATE_REQ_CMD:
        v_crc7_next = 1;
        if (r.bitcnt.read().or_reduce() == 0) {
            v.bitcnt = 31;
            v.cmd_state = CMDSTATE_REQ_ARG;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_REQ_ARG:
        v_crc7_next = 1;
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_REQ_CRC7;
            v.bitcnt = 6;
            v.crc_calc = wb_crc7;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_REQ_CRC7:
        v.crc_rx = (r.crc_rx.read()(5, 0), i_cmd.read());
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_REQ_STOPBIT;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_REQ_STOPBIT:
        v.cmd_state = CMDSTATE_REQ_VALID;
        v.cmd_dir = 0;
        v_crc7_clear = 1;
        break;
    case CMDSTATE_REQ_VALID:
        if ((r.txbit.read() == 1)
                && (r.crc_calc.read() == r.crc_rx.read())) {
            v.cmd_state = CMDSTATE_WAIT_RESP;
            v.cmd_req_valid = 1;
        } else {
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
            v.cmd_dir = 1;
        }
        v.cmd_req_cmd = r.cmd_rxshift.read()(45, 40);
        v.cmd_req_data = r.cmd_rxshift.read()(39, 8);
        break;
    case CMDSTATE_WAIT_RESP:
        v.cmd_resp_ready = 1;
        if (i_cmd_resp_valid.read() == 1) {
            v.cmd_resp_ready = 0;
            v.cmd_state = CMDSTATE_RESP;
            v.bitcnt = 39;
            vb_cmd_txshift = 0;
            vb_cmd_txshift(45, 40) = r.cmd_rxshift.read()(45, 40);
            vb_cmd_txshift(39, 8) = i_cmd_resp_data32;
            vb_cmd_txshift(7, 0) = 0xFF;
        }
        break;
    case CMDSTATE_RESP:
        v_crc7_in = r.cmd_txshift.read()[47];
        if (r.bitcnt.read().or_reduce() == 0) {
            v.bitcnt = 6;
            v.cmd_state = CMDSTATE_RESP_CRC7;
            vb_cmd_txshift(47, 40) = ((wb_crc7.read() << 1) | 1);
            v.crc_calc = wb_crc7;
        } else {
            v_crc7_next = 1;
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_RESP_CRC7:
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_RESP_STOPBIT;
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_RESP_STOPBIT:
        v.cmd_state = CMDSTATE_REQ_STARTBIT;
        v.cmd_dir = 1;
        break;
    default:
        break;
    }

    if (r.cmd_state.read() < CMDSTATE_REQ_VALID) {
        v.cmd_rxshift = (r.cmd_rxshift.read()(46, 0), i_cmd.read());
        v.cmd_txshift = ~0ull;
    } else {
        if ((r.cmd_state.read() == CMDSTATE_RESP_STOPBIT) && (r.bitcnt.read().or_reduce() == 0)) {
            v.cmd_rxshift = ~0ull;
        }
        v.cmd_txshift = vb_cmd_txshift;
    }

    if (r.cmd_dir.read() == 0) {
        // Output:
        v.cmdz = r.cmd_txshift.read()[47];
    } else {
        // Input:
        v.cmdz = i_cmd;
    }

    w_crc7_clear = v_crc7_clear;
    w_crc7_next = v_crc7_next;
    w_crc7_dat = v_crc7_in;
    o_cmd = r.cmd_txshift.read()[47];
    o_cmd_dir = r.cmd_dir;
    o_cmd_req_valid = r.cmd_req_valid;
    o_cmd_req_cmd = r.cmd_req_cmd;
    o_cmd_req_data = r.cmd_req_data;
    o_cmd_resp_ready = r.cmd_resp_ready;
}

void vip_sdcard_cmdio::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_cmdio_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

