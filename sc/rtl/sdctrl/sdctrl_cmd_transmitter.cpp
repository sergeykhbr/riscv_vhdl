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

#include "sdctrl_cmd_transmitter.h"
#include "api_core.h"

namespace debugger {

sdctrl_cmd_transmitter::sdctrl_cmd_transmitter(sc_module_name name,
                                               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_sclk_posedge("i_sclk_posedge"),
    i_sclk_negedge("i_sclk_negedge"),
    i_cmd("i_cmd"),
    o_cmd("o_cmd"),
    o_cmd_dir("o_cmd_dir"),
    i_watchdog("i_watchdog"),
    i_cmd_set_low("i_cmd_set_low"),
    i_req_valid("i_req_valid"),
    i_req_cmd("i_req_cmd"),
    i_req_arg("i_req_arg"),
    i_req_rn("i_req_rn"),
    o_req_ready("o_req_ready"),
    i_crc7("i_crc7"),
    o_crc7_clear("o_crc7_clear"),
    o_crc7_next("o_crc7_next"),
    o_crc7_dat("o_crc7_dat"),
    o_resp_valid("o_resp_valid"),
    o_resp_cmd("o_resp_cmd"),
    o_resp_reg("o_resp_reg"),
    o_resp_crc7_rx("o_resp_crc7_rx"),
    o_resp_crc7_calc("o_resp_crc7_calc"),
    i_resp_ready("i_resp_ready"),
    i_clear_cmderr("i_clear_cmderr"),
    o_cmdstate("o_cmdstate"),
    o_cmderr("o_cmderr") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sclk_posedge;
    sensitive << i_sclk_negedge;
    sensitive << i_cmd;
    sensitive << i_watchdog;
    sensitive << i_cmd_set_low;
    sensitive << i_req_valid;
    sensitive << i_req_cmd;
    sensitive << i_req_arg;
    sensitive << i_req_rn;
    sensitive << i_crc7;
    sensitive << i_resp_ready;
    sensitive << i_clear_cmderr;
    sensitive << r.req_cmd;
    sensitive << r.req_rn;
    sensitive << r.resp_valid;
    sensitive << r.resp_cmd;
    sensitive << r.resp_arg;
    sensitive << r.cmdshift;
    sensitive << r.cmdmirror;
    sensitive << r.regshift;
    sensitive << r.cidshift;
    sensitive << r.crc_calc;
    sensitive << r.crc_rx;
    sensitive << r.cmdbitcnt;
    sensitive << r.crc7_clear;
    sensitive << r.cmdstate;
    sensitive << r.cmderr;
    sensitive << r.watchdog;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_cmd_transmitter::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_sclk_posedge, i_sclk_posedge.name());
        sc_trace(o_vcd, i_sclk_negedge, i_sclk_negedge.name());
        sc_trace(o_vcd, i_cmd, i_cmd.name());
        sc_trace(o_vcd, o_cmd, o_cmd.name());
        sc_trace(o_vcd, o_cmd_dir, o_cmd_dir.name());
        sc_trace(o_vcd, i_watchdog, i_watchdog.name());
        sc_trace(o_vcd, i_cmd_set_low, i_cmd_set_low.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_cmd, i_req_cmd.name());
        sc_trace(o_vcd, i_req_arg, i_req_arg.name());
        sc_trace(o_vcd, i_req_rn, i_req_rn.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, i_crc7, i_crc7.name());
        sc_trace(o_vcd, o_crc7_clear, o_crc7_clear.name());
        sc_trace(o_vcd, o_crc7_next, o_crc7_next.name());
        sc_trace(o_vcd, o_crc7_dat, o_crc7_dat.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_cmd, o_resp_cmd.name());
        sc_trace(o_vcd, o_resp_reg, o_resp_reg.name());
        sc_trace(o_vcd, o_resp_crc7_rx, o_resp_crc7_rx.name());
        sc_trace(o_vcd, o_resp_crc7_calc, o_resp_crc7_calc.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, i_clear_cmderr, i_clear_cmderr.name());
        sc_trace(o_vcd, o_cmdstate, o_cmdstate.name());
        sc_trace(o_vcd, o_cmderr, o_cmderr.name());
        sc_trace(o_vcd, r.req_cmd, pn + ".r_req_cmd");
        sc_trace(o_vcd, r.req_rn, pn + ".r_req_rn");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_cmd, pn + ".r_resp_cmd");
        sc_trace(o_vcd, r.resp_arg, pn + ".r_resp_arg");
        sc_trace(o_vcd, r.cmdshift, pn + ".r_cmdshift");
        sc_trace(o_vcd, r.cmdmirror, pn + ".r_cmdmirror");
        sc_trace(o_vcd, r.regshift, pn + ".r_regshift");
        sc_trace(o_vcd, r.cidshift, pn + ".r_cidshift");
        sc_trace(o_vcd, r.crc_calc, pn + ".r_crc_calc");
        sc_trace(o_vcd, r.crc_rx, pn + ".r_crc_rx");
        sc_trace(o_vcd, r.cmdbitcnt, pn + ".r_cmdbitcnt");
        sc_trace(o_vcd, r.crc7_clear, pn + ".r_crc7_clear");
        sc_trace(o_vcd, r.cmdstate, pn + ".r_cmdstate");
        sc_trace(o_vcd, r.cmderr, pn + ".r_cmderr");
        sc_trace(o_vcd, r.watchdog, pn + ".r_watchdog");
    }

}

void sdctrl_cmd_transmitter::comb() {
    bool v_req_ready;
    sc_uint<48> vb_cmdshift;
    bool v_cmd_dir;
    bool v_crc7_dat;
    bool v_crc7_next;

    v_req_ready = 0;
    vb_cmdshift = 0;
    v_cmd_dir = 0;
    v_crc7_dat = 0;
    v_crc7_next = 0;

    v = r;

    vb_cmdshift = r.cmdshift;
    if (i_clear_cmderr.read() == 1) {
        v.cmderr = 0;
    }
    if (i_resp_ready.read() == 1) {
        v.resp_valid = 0;
    }

    // command state:
    if (i_sclk_negedge.read() == 1) {
        // CMD Request:
        if (r.cmdstate.read() == CMDSTATE_IDLE) {
            if (i_cmd_set_low.read() == 1) {
                // Used during p-init state (power-up)
                vb_cmdshift = 0;
            } else {
                vb_cmdshift = ~0ull;
            }
            v.crc7_clear = 1;
            v_req_ready = 1;
            if (r.cmderr.read() != CMDERR_NONE) {
                v_req_ready = 0;
            } else if (i_req_valid.read() == 1) {
                v.req_cmd = i_req_cmd;
                v.req_rn = i_req_rn;
                vb_cmdshift = (0x1, i_req_cmd, i_req_arg);
                v.cmdbitcnt = 39;
                v.crc7_clear = 0;
                v.cmdstate = CMDSTATE_REQ_CONTENT;
            }
        } else if (r.cmdstate.read() == CMDSTATE_REQ_CONTENT) {
            v_crc7_next = 1;
            vb_cmdshift = ((r.cmdshift.read()(38, 0) << 1) | 1);
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_REQ_CRC7;
                vb_cmdshift(39, 32) = ((i_crc7.read() << 1) | 1);
                v.cmdbitcnt = 6;
                v.crc7_clear = 1;
            }
        } else if (r.cmdstate.read() == CMDSTATE_REQ_CRC7) {
            vb_cmdshift = ((r.cmdshift.read()(38, 0) << 1) | 1);
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_REQ_STOPBIT;
            }
        } else if (r.cmdstate.read() == CMDSTATE_REQ_STOPBIT) {
            v.cmdstate = CMDSTATE_RESP_WAIT;
            v.watchdog = i_watchdog;
            v.crc7_clear = 0;
        }
    } else if (i_sclk_posedge.read() == 1) {
        // CMD Response (see page 140. '4.9 Responses'):

        if (r.cmdstate.read() == CMDSTATE_RESP_WAIT) {
            // [47] start bit; [135] for R2
            v.watchdog = (r.watchdog.read() - 1);
            if (i_cmd.read() == 0) {
                v_crc7_next = 1;
                v.cmdstate = CMDSTATE_RESP_TRANSBIT;
            } else if (r.watchdog.read().or_reduce() == 0) {
                v.cmderr = CMDERR_NO_RESPONSE;
                v.cmdstate = CMDSTATE_IDLE;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_TRANSBIT) {
            // [46](134) transmission bit (R2);
            v_crc7_next = 1;
            if (i_cmd.read() == 0) {
                v.cmdstate = CMDSTATE_RESP_CMD_MIRROR;
                v.cmdmirror = 0;
                v.cmdbitcnt = 5;
            } else {
                v.cmderr = CMDERR_WRONG_RESP_STARTBIT;
                v.cmdstate = CMDSTATE_IDLE;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_CMD_MIRROR) {
            // [45:40] [133:128] command index mirrored: 111111 for R2 and R3 (OCR)
            v_crc7_next = 1;
            v.cmdmirror = (r.cmdmirror.read()(4, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                if (r.req_rn.read() == R2) {
                    v.cmdbitcnt = 119;
                    v.cmdstate = CMDSTATE_RESP_CID_CSD;
                } else {
                    v.cmdbitcnt = 31;
                    v.cmdstate = CMDSTATE_RESP_REG;
                }
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_REG) {
            // [39:8] Card status (R1), OCR (R3) or RCA (R6) register
            v_crc7_next = 1;
            v.regshift = (r.regshift.read()(30, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.crc_calc = i_crc7;
                v.cmdbitcnt = 6;
                v.cmdstate = CMDSTATE_RESP_CRC7;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_CID_CSD) {
            // [127:8] CID or CSD register incl. internal CRC7 R2 response on CMD2 and CMD10 (CID) or CMD9 (CSD)
            v.cidshift = (r.cidshift.read()(118, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.crc_calc = i_crc7;
                v.cmdbitcnt = 6;
                v.cmdstate = CMDSTATE_RESP_CRC7;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_CRC7) {
            // [7:1] CRC7: 1111111 for R3 (OCR) no proteection
            v.crc_rx = (r.crc_rx.read()(5, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_RESP_STOPBIT;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_STOPBIT) {
            // [7:1] End bit
            if (i_cmd.read() == 0) {
                v.cmderr = CMDERR_WRONG_RESP_STOPBIT;
            }
            v.cmdstate = CMDSTATE_PAUSE;
            v.cmdbitcnt = 2;
            v.resp_valid = 1;
        } else if (r.cmdstate.read() == CMDSTATE_PAUSE) {
            v.crc7_clear = 1;
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_IDLE;
            }
        }
    }
    v.cmdshift = vb_cmdshift;

    if ((r.cmdstate.read() < CMDSTATE_RESP_WAIT)
            || (r.cmdstate.read() == CMDSTATE_PAUSE)) {
        v_cmd_dir = DIR_OUTPUT;
        v_crc7_dat = r.cmdshift.read()[39];
    } else {
        v_cmd_dir = DIR_INPUT;
        v_crc7_dat = i_cmd;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_cmd_transmitter_r_reset(v);
    }

    o_cmd = r.cmdshift.read()[39];
    o_cmd_dir = v_cmd_dir;
    o_req_ready = v_req_ready;
    o_crc7_clear = r.crc7_clear;
    o_crc7_next = v_crc7_next;
    o_crc7_dat = v_crc7_dat;
    o_resp_valid = r.resp_valid;
    o_resp_cmd = r.cmdmirror;
    o_resp_reg = r.regshift;
    o_resp_crc7_rx = r.crc_rx;
    o_resp_crc7_calc = r.crc_calc;
    o_cmdstate = r.cmdstate;
    o_cmderr = r.cmderr;
}

void sdctrl_cmd_transmitter::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_cmd_transmitter_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

