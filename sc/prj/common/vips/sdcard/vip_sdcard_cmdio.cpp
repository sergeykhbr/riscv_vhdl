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
    i_cs("i_cs"),
    o_spi_mode("o_spi_mode"),
    i_cmd("i_cmd"),
    o_cmd("o_cmd"),
    o_cmd_dir("o_cmd_dir"),
    o_cmd_req_valid("o_cmd_req_valid"),
    o_cmd_req_cmd("o_cmd_req_cmd"),
    o_cmd_req_data("o_cmd_req_data"),
    i_cmd_req_ready("i_cmd_req_ready"),
    i_cmd_resp_valid("i_cmd_resp_valid"),
    i_cmd_resp_data32("i_cmd_resp_data32"),
    o_cmd_resp_ready("o_cmd_resp_ready"),
    i_cmd_resp_r1b("i_cmd_resp_r1b"),
    i_cmd_resp_r2("i_cmd_resp_r2"),
    i_cmd_resp_r3("i_cmd_resp_r3"),
    i_cmd_resp_r7("i_cmd_resp_r7"),
    i_stat_idle_state("i_stat_idle_state"),
    i_stat_erase_reset("i_stat_erase_reset"),
    i_stat_illegal_cmd("i_stat_illegal_cmd"),
    i_stat_err_erase_sequence("i_stat_err_erase_sequence"),
    i_stat_err_address("i_stat_err_address"),
    i_stat_err_parameter("i_stat_err_parameter"),
    i_stat_locked("i_stat_locked"),
    i_stat_wp_erase_skip("i_stat_wp_erase_skip"),
    i_stat_err("i_stat_err"),
    i_stat_err_cc("i_stat_err_cc"),
    i_stat_ecc_failed("i_stat_ecc_failed"),
    i_stat_wp_violation("i_stat_wp_violation"),
    i_stat_erase_param("i_stat_erase_param"),
    i_stat_out_of_range("i_stat_out_of_range"),
    o_busy("o_busy") {

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
    sensitive << i_cs;
    sensitive << i_cmd;
    sensitive << i_cmd_req_ready;
    sensitive << i_cmd_resp_valid;
    sensitive << i_cmd_resp_data32;
    sensitive << i_cmd_resp_r1b;
    sensitive << i_cmd_resp_r2;
    sensitive << i_cmd_resp_r3;
    sensitive << i_cmd_resp_r7;
    sensitive << i_stat_idle_state;
    sensitive << i_stat_erase_reset;
    sensitive << i_stat_illegal_cmd;
    sensitive << i_stat_err_erase_sequence;
    sensitive << i_stat_err_address;
    sensitive << i_stat_err_parameter;
    sensitive << i_stat_locked;
    sensitive << i_stat_wp_erase_skip;
    sensitive << i_stat_err;
    sensitive << i_stat_err_cc;
    sensitive << i_stat_ecc_failed;
    sensitive << i_stat_wp_violation;
    sensitive << i_stat_erase_param;
    sensitive << i_stat_out_of_range;
    sensitive << w_cmd_out;
    sensitive << w_crc7_clear;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << wb_crc7;
    sensitive << r.clkcnt;
    sensitive << r.cs;
    sensitive << r.spi_mode;
    sensitive << r.cmdz;
    sensitive << r.cmd_dir;
    sensitive << r.cmd_rxshift;
    sensitive << r.cmd_txshift;
    sensitive << r.cmd_state;
    sensitive << r.cmd_req_crc_err;
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
        sc_trace(o_vcd, i_cs, i_cs.name());
        sc_trace(o_vcd, o_spi_mode, o_spi_mode.name());
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
        sc_trace(o_vcd, i_cmd_resp_r1b, i_cmd_resp_r1b.name());
        sc_trace(o_vcd, i_cmd_resp_r2, i_cmd_resp_r2.name());
        sc_trace(o_vcd, i_cmd_resp_r3, i_cmd_resp_r3.name());
        sc_trace(o_vcd, i_cmd_resp_r7, i_cmd_resp_r7.name());
        sc_trace(o_vcd, i_stat_idle_state, i_stat_idle_state.name());
        sc_trace(o_vcd, i_stat_erase_reset, i_stat_erase_reset.name());
        sc_trace(o_vcd, i_stat_illegal_cmd, i_stat_illegal_cmd.name());
        sc_trace(o_vcd, i_stat_err_erase_sequence, i_stat_err_erase_sequence.name());
        sc_trace(o_vcd, i_stat_err_address, i_stat_err_address.name());
        sc_trace(o_vcd, i_stat_err_parameter, i_stat_err_parameter.name());
        sc_trace(o_vcd, i_stat_locked, i_stat_locked.name());
        sc_trace(o_vcd, i_stat_wp_erase_skip, i_stat_wp_erase_skip.name());
        sc_trace(o_vcd, i_stat_err, i_stat_err.name());
        sc_trace(o_vcd, i_stat_err_cc, i_stat_err_cc.name());
        sc_trace(o_vcd, i_stat_ecc_failed, i_stat_ecc_failed.name());
        sc_trace(o_vcd, i_stat_wp_violation, i_stat_wp_violation.name());
        sc_trace(o_vcd, i_stat_erase_param, i_stat_erase_param.name());
        sc_trace(o_vcd, i_stat_out_of_range, i_stat_out_of_range.name());
        sc_trace(o_vcd, o_busy, o_busy.name());
        sc_trace(o_vcd, r.clkcnt, pn + ".r_clkcnt");
        sc_trace(o_vcd, r.cs, pn + ".r_cs");
        sc_trace(o_vcd, r.spi_mode, pn + ".r_spi_mode");
        sc_trace(o_vcd, r.cmdz, pn + ".r_cmdz");
        sc_trace(o_vcd, r.cmd_dir, pn + ".r_cmd_dir");
        sc_trace(o_vcd, r.cmd_rxshift, pn + ".r_cmd_rxshift");
        sc_trace(o_vcd, r.cmd_txshift, pn + ".r_cmd_txshift");
        sc_trace(o_vcd, r.cmd_state, pn + ".r_cmd_state");
        sc_trace(o_vcd, r.cmd_req_crc_err, pn + ".r_cmd_req_crc_err");
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
    bool v_busy;

    vb_cmd_txshift = 0;
    v_crc7_clear = 0;
    v_crc7_next = 0;
    v_crc7_in = 0;
    v_busy = 1;

    v = r;

    vb_cmd_txshift = ((r.cmd_txshift.read()(46, 0) << 1) | 1);
    v_crc7_in = i_cmd;

    if (i_cmd_req_ready.read() == 1) {
        v.cmd_req_valid = 0;
    }
    v.clkcnt = (r.clkcnt.read() + 1);

    switch (r.cmd_state.read()) {
    case CMDSTATE_INIT:
        v.spi_mode = 0;
        v.cmd_dir = 1;
        v_crc7_clear = 1;
        v.cmd_req_crc_err = 0;
        // Wait several (72) clocks to switch into idle state
        if (r.clkcnt.read() == 70) {
            v.cmd_state = CMDSTATE_REQ_STARTBIT;
        }
        break;
    case CMDSTATE_REQ_STARTBIT:
        v_busy = 0;
        v_crc7_clear = 1;
        if ((r.spi_mode.read() && i_cs.read()) == 1) {
            // Do nothing
        } else if ((r.cmdz.read() == 1) && (i_cmd.read() == 0)) {
            v.cs = i_cs;
            v.cmd_state = CMDSTATE_REQ_TXBIT;
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
        if (r.crc_calc.read() != r.crc_rx.read()) {
            v.cmd_req_crc_err = 1;
        }
        if (r.txbit.read() == 1) {
            v.cmd_state = CMDSTATE_WAIT_RESP;
            v.cmd_req_valid = 1;
            if ((r.cmd_rxshift.read()(45, 40).or_reduce() == 0) && (r.cs.read() == 0)) {
                // CMD0 with CS = 0 (CD_DAT3)
                v.spi_mode = 1;
            }
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
            if (r.spi_mode.read() == 0) {
                v.bitcnt = 39;
                vb_cmd_txshift = 0;
                vb_cmd_txshift(45, 40) = r.cmd_rxshift.read()(45, 40);
                vb_cmd_txshift(39, 8) = i_cmd_resp_data32;
                vb_cmd_txshift(7, 0) = 0xFF;
            } else {
                // Default R1 response in SPI mode:
                v.bitcnt = 7;
                vb_cmd_txshift = ~0ull;
                vb_cmd_txshift[47] = 0;
                vb_cmd_txshift[46] = i_stat_err_parameter;
                vb_cmd_txshift[45] = i_stat_err_address;
                vb_cmd_txshift[44] = i_stat_err_erase_sequence;
                vb_cmd_txshift[43] = r.cmd_req_crc_err;
                vb_cmd_txshift[42] = i_stat_illegal_cmd;
                vb_cmd_txshift[41] = i_stat_erase_reset;
                vb_cmd_txshift[40] = i_stat_idle_state;
                if (i_cmd_resp_r2.read() == 1) {
                    v.bitcnt = 15;
                    vb_cmd_txshift[39] = i_stat_out_of_range;
                    vb_cmd_txshift[38] = i_stat_erase_param;
                    vb_cmd_txshift[37] = i_stat_wp_violation;
                    vb_cmd_txshift[36] = i_stat_ecc_failed;
                    vb_cmd_txshift[35] = i_stat_err_cc;
                    vb_cmd_txshift[34] = i_stat_err;
                    vb_cmd_txshift[33] = i_stat_wp_erase_skip;
                    vb_cmd_txshift[32] = i_stat_locked;
                } else if ((i_cmd_resp_r3.read() == 1) || (i_cmd_resp_r7.read() == 1)) {
                    v.bitcnt = 39;
                    vb_cmd_txshift(39, 8) = i_cmd_resp_data32;
                }
            }
        }
        break;
    case CMDSTATE_RESP:
        v_crc7_in = r.cmd_txshift.read()[47];
        if (r.bitcnt.read().or_reduce() == 0) {
            if (r.spi_mode.read() == 0) {
                v.bitcnt = 6;
                v.cmd_state = CMDSTATE_RESP_CRC7;
                vb_cmd_txshift(47, 40) = ((wb_crc7.read() << 1) | 1);
                v.crc_calc = wb_crc7;
            } else {
                v.cmd_state = CMDSTATE_RESP_STOPBIT;
            }
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
        v_crc7_clear = 1;
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
    o_spi_mode = r.spi_mode;
    o_busy = v_busy;
}

void vip_sdcard_cmdio::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_cmdio_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

