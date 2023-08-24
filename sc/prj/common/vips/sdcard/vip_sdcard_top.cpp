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
    crccmd0 = 0;

    iobufcmd0 = new iobuf_tech("iobufcmd0");
    iobufcmd0->io(io_cmd);
    iobufcmd0->o(w_cmd_in);
    iobufcmd0->i(w_cmd_out);
    iobufcmd0->t(r.cmd_dir);


    crccmd0 = new vip_sdcard_crc7("crccmd0", async_reset);
    crccmd0->i_clk(i_sclk);
    crccmd0->i_nrst(i_nrst);
    crccmd0->i_clear(w_crc7_clear);
    crccmd0->i_next(w_crc7_next);
    crccmd0->i_dat(w_crc7_dat);
    crccmd0->o_crc7(wb_crc7);



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
    sensitive << w_crc7_clear;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << wb_crc7;
    sensitive << r.cmd_dir;
    sensitive << r.cmd_rxshift;
    sensitive << r.cmd_txshift;
    sensitive << r.cmd_state;
    sensitive << r.bitcnt;
    sensitive << r.powerup_cnt;
    sensitive << r.powerup_done;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_sclk.pos();
}

vip_sdcard_top::~vip_sdcard_top() {
    if (iobufcmd0) {
        delete iobufcmd0;
    }
    if (crccmd0) {
        delete crccmd0;
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
        sc_trace(o_vcd, r.powerup_cnt, pn + ".r_powerup_cnt");
        sc_trace(o_vcd, r.powerup_done, pn + ".r_powerup_done");
    }

    if (iobufcmd0) {
        iobufcmd0->generateVCD(i_vcd, o_vcd);
    }
    if (crccmd0) {
        crccmd0->generateVCD(i_vcd, o_vcd);
    }
}

void vip_sdcard_top::comb() {
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
    v_crc7_in = w_cmd_in;
    if ((r.powerup_done.read() == 0) && (r.powerup_cnt.read() < CFG_SDCARD_POWERUP_DONE_DELAY)) {
        v.powerup_cnt = (r.powerup_cnt.read() + 1);
    } else {
        v.powerup_done = 1;
    }

    switch (r.cmd_state.read()) {
    case CMDSTATE_IDLE:
        v.cmd_dir = 1;
        if (w_cmd_in.read() == 0) {
            v_crc7_next = 1;
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
        } else {
            v_crc7_clear = 1;
        }
        break;
    case CMDSTATE_REQ_STARTBIT:
        if (w_cmd_in.read() == 1) {
            v_crc7_next = 1;
            v.cmd_state = CMDSTATE_REQ_CMD;
            v.bitcnt = 5;
        } else {
            v_crc7_clear = 1;
            v.cmd_state = CMDSTATE_IDLE;
        }
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
        v_crc7_clear = 1;
        break;
    case CMDSTATE_WAIT_RESP:
        // Preparing output with some delay (several clocks):
        if (r.bitcnt.read().or_reduce() == 0) {
            v.cmd_state = CMDSTATE_RESP;
            v.bitcnt = 39;
            vb_cmd_txshift = 0;
            vb_cmd_txshift(45, 40) = r.cmd_rxshift.read()(45, 40);
            vb_cmd_txshift(7, 0) = 0xFF;

            // Commands response arguments:
            switch (r.cmd_rxshift.read()(45, 40)) {
            case 8:                                         // CMD8: SEND_IF_COND. Send memory Card interface condition
                // [21] PCIe 1.2V support
                // [20] PCIe availability
                // [19:16] Voltage supply
                // [15:8] check pattern
                vb_cmd_txshift[21] = (r.cmd_rxshift.read()[21] & CFG_SDCARD_PCIE_1_2V);
                vb_cmd_txshift[20] = (r.cmd_rxshift.read()[20] & CFG_SDCARD_PCIE_AVAIL);
                vb_cmd_txshift(19, 16) = (r.cmd_rxshift.read()(19, 16) & CFG_SDCARD_VHS);
                vb_cmd_txshift(15, 8) = r.cmd_rxshift.read()(15, 8);
                break;
            case 55:                                        // CMD55: APP_CMD. 
                vb_cmd_txshift(39, 8) = 0;
                break;
            case 41:                                        // ACMD41: SD_SEND_OP_COND. Send host capacity info
                // [39] BUSY, active LOW
                // [38] HCS (OCR[30]) Host Capacity
                // [36] XPC
                // [32] S18R
                // [31:8] VDD Voltage Window (OCR[23:0])
                vb_cmd_txshift(45, 40) = ~0ull;
                vb_cmd_txshift[39] = r.powerup_done.read();
                vb_cmd_txshift(31, 8) = (r.cmd_rxshift.read()(31, 8) & CFG_SDCARD_VDD_VOLTAGE_WINDOW);
                break;
            default:
                vb_cmd_txshift(39, 8) = 0;
                break;
            }
        } else {
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_RESP:
        v_crc7_in = r.cmd_txshift.read()[47];
        if (r.bitcnt.read().or_reduce() == 0) {
            v.bitcnt = 7;
            v.cmd_state = CMDSTATE_RESP_CRC7;
            vb_cmd_txshift(47, 40) = ((wb_crc7.read() << 1) | 1);
        } else {
            v_crc7_next = 1;
            v.bitcnt = (r.bitcnt.read() - 1);
        }
        break;
    case CMDSTATE_RESP_CRC7:
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

    if (r.cmd_state.read() <= CMDSTATE_REQ_STOPBIT) {
        // This will includes clock with the stopbit itself
        v.cmd_rxshift = (r.cmd_rxshift.read()(46, 0), w_cmd_in.read());
        v.cmd_txshift = ~0ull;
    } else {
        if ((r.cmd_state.read() == CMDSTATE_RESP_CRC7) && (r.bitcnt.read().or_reduce() == 0)) {
            v.cmd_rxshift = ~0ull;
        }
        v.cmd_txshift = vb_cmd_txshift;
    }

    w_cmd_out = r.cmd_txshift.read()[47];
    w_crc7_clear = v_crc7_clear;
    w_crc7_next = v_crc7_next;
    w_crc7_dat = v_crc7_in;
}

void vip_sdcard_top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_top_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

