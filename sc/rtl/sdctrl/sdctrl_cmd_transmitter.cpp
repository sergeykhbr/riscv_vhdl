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
    o_cmd_cs("o_cmd_cs"),
    i_spi_mode("i_spi_mode"),
    i_err_code("i_err_code"),
    i_wdog_trigger("i_wdog_trigger"),
    i_cmd_set_low("i_cmd_set_low"),
    i_req_valid("i_req_valid"),
    i_req_cmd("i_req_cmd"),
    i_req_arg("i_req_arg"),
    i_req_rn("i_req_rn"),
    o_req_ready("o_req_ready"),
    o_resp_valid("o_resp_valid"),
    o_resp_cmd("o_resp_cmd"),
    o_resp_reg("o_resp_reg"),
    o_resp_crc7_rx("o_resp_crc7_rx"),
    o_resp_crc7_calc("o_resp_crc7_calc"),
    o_resp_spistatus("o_resp_spistatus"),
    i_resp_ready("i_resp_ready"),
    o_wdog_ena("o_wdog_ena"),
    o_err_valid("o_err_valid"),
    o_err_setcode("o_err_setcode") {

    async_reset_ = async_reset;
    crc0 = 0;

    crc0 = new sdctrl_crc7("crc0", async_reset);
    crc0->i_clk(i_clk);
    crc0->i_nrst(i_nrst);
    crc0->i_clear(r.crc7_clear);
    crc0->i_next(w_crc7_next);
    crc0->i_dat(w_crc7_dat);
    crc0->o_crc7(wb_crc7);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sclk_posedge;
    sensitive << i_sclk_negedge;
    sensitive << i_cmd;
    sensitive << i_spi_mode;
    sensitive << i_err_code;
    sensitive << i_wdog_trigger;
    sensitive << i_cmd_set_low;
    sensitive << i_req_valid;
    sensitive << i_req_cmd;
    sensitive << i_req_arg;
    sensitive << i_req_rn;
    sensitive << i_resp_ready;
    sensitive << wb_crc7;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << r.req_cmd;
    sensitive << r.req_rn;
    sensitive << r.resp_valid;
    sensitive << r.resp_cmd;
    sensitive << r.resp_arg;
    sensitive << r.resp_spistatus;
    sensitive << r.cmdshift;
    sensitive << r.cmdmirror;
    sensitive << r.regshift;
    sensitive << r.cidshift;
    sensitive << r.crc_calc;
    sensitive << r.crc_rx;
    sensitive << r.cmdbitcnt;
    sensitive << r.crc7_clear;
    sensitive << r.cmdstate;
    sensitive << r.err_valid;
    sensitive << r.err_setcode;
    sensitive << r.cmd_cs;
    sensitive << r.cmd_dir;
    sensitive << r.wdog_ena;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

sdctrl_cmd_transmitter::~sdctrl_cmd_transmitter() {
    if (crc0) {
        delete crc0;
    }
}

void sdctrl_cmd_transmitter::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_sclk_posedge, i_sclk_posedge.name());
        sc_trace(o_vcd, i_sclk_negedge, i_sclk_negedge.name());
        sc_trace(o_vcd, i_cmd, i_cmd.name());
        sc_trace(o_vcd, o_cmd, o_cmd.name());
        sc_trace(o_vcd, o_cmd_dir, o_cmd_dir.name());
        sc_trace(o_vcd, o_cmd_cs, o_cmd_cs.name());
        sc_trace(o_vcd, i_spi_mode, i_spi_mode.name());
        sc_trace(o_vcd, i_err_code, i_err_code.name());
        sc_trace(o_vcd, i_wdog_trigger, i_wdog_trigger.name());
        sc_trace(o_vcd, i_cmd_set_low, i_cmd_set_low.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_cmd, i_req_cmd.name());
        sc_trace(o_vcd, i_req_arg, i_req_arg.name());
        sc_trace(o_vcd, i_req_rn, i_req_rn.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_cmd, o_resp_cmd.name());
        sc_trace(o_vcd, o_resp_reg, o_resp_reg.name());
        sc_trace(o_vcd, o_resp_crc7_rx, o_resp_crc7_rx.name());
        sc_trace(o_vcd, o_resp_crc7_calc, o_resp_crc7_calc.name());
        sc_trace(o_vcd, o_resp_spistatus, o_resp_spistatus.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, o_wdog_ena, o_wdog_ena.name());
        sc_trace(o_vcd, o_err_valid, o_err_valid.name());
        sc_trace(o_vcd, o_err_setcode, o_err_setcode.name());
        sc_trace(o_vcd, r.req_cmd, pn + ".r_req_cmd");
        sc_trace(o_vcd, r.req_rn, pn + ".r_req_rn");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_cmd, pn + ".r_resp_cmd");
        sc_trace(o_vcd, r.resp_arg, pn + ".r_resp_arg");
        sc_trace(o_vcd, r.resp_spistatus, pn + ".r_resp_spistatus");
        sc_trace(o_vcd, r.cmdshift, pn + ".r_cmdshift");
        sc_trace(o_vcd, r.cmdmirror, pn + ".r_cmdmirror");
        sc_trace(o_vcd, r.regshift, pn + ".r_regshift");
        sc_trace(o_vcd, r.cidshift, pn + ".r_cidshift");
        sc_trace(o_vcd, r.crc_calc, pn + ".r_crc_calc");
        sc_trace(o_vcd, r.crc_rx, pn + ".r_crc_rx");
        sc_trace(o_vcd, r.cmdbitcnt, pn + ".r_cmdbitcnt");
        sc_trace(o_vcd, r.crc7_clear, pn + ".r_crc7_clear");
        sc_trace(o_vcd, r.cmdstate, pn + ".r_cmdstate");
        sc_trace(o_vcd, r.err_valid, pn + ".r_err_valid");
        sc_trace(o_vcd, r.err_setcode, pn + ".r_err_setcode");
        sc_trace(o_vcd, r.cmd_cs, pn + ".r_cmd_cs");
        sc_trace(o_vcd, r.cmd_dir, pn + ".r_cmd_dir");
        sc_trace(o_vcd, r.wdog_ena, pn + ".r_wdog_ena");
    }

    if (crc0) {
        crc0->generateVCD(i_vcd, o_vcd);
    }
}

void sdctrl_cmd_transmitter::comb() {
    bool v_req_ready;
    sc_uint<48> vb_cmdshift;
    sc_uint<15> vb_resp_spistatus;
    bool v_crc7_dat;
    bool v_crc7_next;

    v_req_ready = 0;
    vb_cmdshift = 0;
    vb_resp_spistatus = 0;
    v_crc7_dat = 0;
    v_crc7_next = 0;

    v = r;

    vb_cmdshift = r.cmdshift;
    vb_resp_spistatus = r.resp_spistatus;
    v.err_valid = 0;
    v.err_setcode = 0;
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
            v.wdog_ena = 0;
            v.cmd_cs = 1;
            v.cmd_dir = DIR_OUTPUT;
            v.crc7_clear = 1;
            v_req_ready = 1;
            if (i_err_code.read() != CMDERR_NONE) {
                v_req_ready = 0;
            } else if (i_req_valid.read() == 1) {
                v.cmd_cs = 0;
                v.req_cmd = i_req_cmd;
                v.req_rn = i_req_rn;
                vb_cmdshift = (0x1, i_req_cmd.read(), i_req_arg.read());
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
                v.crc_calc = wb_crc7;
                vb_cmdshift(39, 32) = ((wb_crc7.read() << 1) | 1);
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
            v.cmd_dir = DIR_INPUT;
            v.wdog_ena = 1;
            v.crc7_clear = 0;
        } else if (r.cmdstate.read() == CMDSTATE_PAUSE) {
            v.crc7_clear = 1;
            v.cmd_cs = 1;
            v.cmd_dir = DIR_OUTPUT;
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_IDLE;
            }
        }
    } else if (i_sclk_posedge.read() == 1) {
        // CMD Response (see page 140. '4.9 Responses'):

        if (r.cmdstate.read() == CMDSTATE_RESP_WAIT) {
            // [47] start bit; [135] for R2
            if (i_cmd.read() == 0) {
                if (i_spi_mode.read() == 0) {
                    v_crc7_next = 1;
                    v.cmdstate = CMDSTATE_RESP_TRANSBIT;
                } else {
                    // Response in SPI mode:
                    v.cmdstate = CMDSTATE_RESP_SPI_R1;
                    v.cmdbitcnt = 6;
                    v.cmdmirror = r.req_cmd;
                    v.resp_spistatus = 0;
                    v.regshift = 0;
                }
            } else if (i_wdog_trigger.read() == 1) {
                v.wdog_ena = 0;
                v.err_valid = 1;
                v.err_setcode = CMDERR_NO_RESPONSE;
                v.cmdstate = CMDSTATE_IDLE;
                v.resp_valid = 1;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_TRANSBIT) {
            // [46](134) transmission bit (R2);
            v_crc7_next = 1;
            if (i_cmd.read() == 0) {
                v.cmdstate = CMDSTATE_RESP_CMD_MIRROR;
                v.cmdmirror = 0;
                v.cmdbitcnt = 5;
            } else {
                v.err_valid = 1;
                v.err_setcode = CMDERR_WRONG_RESP_STARTBIT;
                v.cmdstate = CMDSTATE_IDLE;
                v.resp_valid = 1;
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
                v.crc_calc = wb_crc7;
                v.cmdbitcnt = 6;
                v.cmdstate = CMDSTATE_RESP_CRC7;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_CID_CSD) {
            // [127:8] CID or CSD register incl. internal CRC7 R2 response on CMD2 and CMD10 (CID) or CMD9 (CSD)
            v.cidshift = (r.cidshift.read()(118, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.crc_calc = wb_crc7;
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
                v.err_valid = 1;
                v.err_setcode = CMDERR_WRONG_RESP_STOPBIT;
            }
            v.cmdstate = CMDSTATE_PAUSE;
            v.cmdbitcnt = 2;
            v.resp_valid = 1;
        } else if (r.cmdstate.read() == CMDSTATE_RESP_SPI_R1) {
            vb_resp_spistatus(14, 8) = (r.resp_spistatus.read()(13, 8), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                if (r.req_rn.read() == R2) {
                    v.cmdbitcnt = 7;
                    v.cmdstate = CMDSTATE_RESP_SPI_R2;
                } else if ((r.req_rn.read() == R3) || (r.req_rn.read() == R7)) {
                    v.cmdbitcnt = 31;
                    v.cmdstate = CMDSTATE_RESP_SPI_DATA;
                } else {
                    v.cmdstate = CMDSTATE_PAUSE;
                    v.cmdbitcnt = 2;
                    v.resp_valid = 1;
                }
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_SPI_R2) {
            vb_resp_spistatus(7, 0) = (r.resp_spistatus.read()(6, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_PAUSE;
                v.cmdbitcnt = 2;
                v.resp_valid = 1;
            }
        } else if (r.cmdstate.read() == CMDSTATE_RESP_SPI_DATA) {
            v.regshift = (r.regshift.read()(30, 0), i_cmd.read());
            if (r.cmdbitcnt.read().or_reduce() == 1) {
                v.cmdbitcnt = (r.cmdbitcnt.read() - 1);
            } else {
                v.cmdstate = CMDSTATE_PAUSE;
                v.cmdbitcnt = 2;
                v.resp_valid = 1;
            }
        }
    }
    v.cmdshift = vb_cmdshift;
    v.resp_spistatus = vb_resp_spistatus;

    if ((r.cmdstate.read() < CMDSTATE_RESP_WAIT)
            || (r.cmdstate.read() == CMDSTATE_PAUSE)) {
        v_crc7_dat = r.cmdshift.read()[39];
    } else {
        v_crc7_dat = i_cmd;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_cmd_transmitter_r_reset(v);
    }

    w_crc7_next = v_crc7_next;
    w_crc7_dat = v_crc7_dat;
    o_cmd = r.cmdshift.read()[39];
    o_cmd_dir = r.cmd_dir;
    o_cmd_cs = r.cmd_cs;
    o_req_ready = v_req_ready;
    o_resp_valid = r.resp_valid;
    o_resp_cmd = r.cmdmirror;
    o_resp_reg = r.regshift;
    o_resp_crc7_rx = r.crc_rx;
    o_resp_crc7_calc = r.crc_calc;
    o_resp_spistatus = r.resp_spistatus;
    o_wdog_ena = r.wdog_ena;
    o_err_valid = r.err_valid;
    o_err_setcode = r.err_setcode;
}

void sdctrl_cmd_transmitter::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_cmd_transmitter_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

