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

#include "vip_sdcard_ctrl.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_ctrl::vip_sdcard_ctrl(sc_module_name name,
                                 bool async_reset,
                                 int CFG_SDCARD_POWERUP_DONE_DELAY,
                                 bool CFG_SDCARD_HCS,
                                 sc_uint<4> CFG_SDCARD_VHS,
                                 bool CFG_SDCARD_PCIE_1_2V,
                                 bool CFG_SDCARD_PCIE_AVAIL,
                                 sc_uint<24> CFG_SDCARD_VDD_VOLTAGE_WINDOW)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_spi_mode("i_spi_mode"),
    i_cs("i_cs"),
    i_cmd_req_valid("i_cmd_req_valid"),
    i_cmd_req_cmd("i_cmd_req_cmd"),
    i_cmd_req_data("i_cmd_req_data"),
    o_cmd_req_ready("o_cmd_req_ready"),
    o_cmd_resp_valid("o_cmd_resp_valid"),
    o_cmd_resp_data32("o_cmd_resp_data32"),
    i_cmd_resp_ready("i_cmd_resp_ready"),
    o_cmd_resp_r1b("o_cmd_resp_r1b"),
    o_cmd_resp_r2("o_cmd_resp_r2"),
    o_cmd_resp_r3("o_cmd_resp_r3"),
    o_cmd_resp_r7("o_cmd_resp_r7"),
    o_stat_idle_state("o_stat_idle_state"),
    o_stat_illegal_cmd("o_stat_illegal_cmd"),
    o_mem_addr("o_mem_addr"),
    i_mem_rdata("i_mem_rdata"),
    o_crc16_clear("o_crc16_clear"),
    o_crc16_next("o_crc16_next"),
    i_crc16("i_crc16"),
    o_dat_trans("o_dat_trans"),
    o_dat("o_dat"),
    i_cmdio_busy("i_cmdio_busy") {

    async_reset_ = async_reset;
    CFG_SDCARD_POWERUP_DONE_DELAY_ = CFG_SDCARD_POWERUP_DONE_DELAY;
    CFG_SDCARD_HCS_ = CFG_SDCARD_HCS;
    CFG_SDCARD_VHS_ = CFG_SDCARD_VHS;
    CFG_SDCARD_PCIE_1_2V_ = CFG_SDCARD_PCIE_1_2V;
    CFG_SDCARD_PCIE_AVAIL_ = CFG_SDCARD_PCIE_AVAIL;
    CFG_SDCARD_VDD_VOLTAGE_WINDOW_ = CFG_SDCARD_VDD_VOLTAGE_WINDOW;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_spi_mode;
    sensitive << i_cs;
    sensitive << i_cmd_req_valid;
    sensitive << i_cmd_req_cmd;
    sensitive << i_cmd_req_data;
    sensitive << i_cmd_resp_ready;
    sensitive << i_mem_rdata;
    sensitive << i_crc16;
    sensitive << i_cmdio_busy;
    sensitive << r.sdstate;
    sensitive << r.datastate;
    sensitive << r.powerup_cnt;
    sensitive << r.preinit_cnt;
    sensitive << r.delay_cnt;
    sensitive << r.powerup_done;
    sensitive << r.cmd_req_ready;
    sensitive << r.cmd_resp_valid;
    sensitive << r.cmd_resp_valid_delayed;
    sensitive << r.cmd_resp_data32;
    sensitive << r.cmd_resp_r1b;
    sensitive << r.cmd_resp_r2;
    sensitive << r.cmd_resp_r3;
    sensitive << r.cmd_resp_r7;
    sensitive << r.illegal_cmd;
    sensitive << r.ocr_hcs;
    sensitive << r.ocr_vdd_window;
    sensitive << r.req_mem_valid;
    sensitive << r.req_mem_addr;
    sensitive << r.shiftdat;
    sensitive << r.bitcnt;
    sensitive << r.crc16_clear;
    sensitive << r.crc16_next;
    sensitive << r.dat_trans;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_sdcard_ctrl::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_spi_mode, i_spi_mode.name());
        sc_trace(o_vcd, i_cs, i_cs.name());
        sc_trace(o_vcd, i_cmd_req_valid, i_cmd_req_valid.name());
        sc_trace(o_vcd, i_cmd_req_cmd, i_cmd_req_cmd.name());
        sc_trace(o_vcd, i_cmd_req_data, i_cmd_req_data.name());
        sc_trace(o_vcd, o_cmd_req_ready, o_cmd_req_ready.name());
        sc_trace(o_vcd, o_cmd_resp_valid, o_cmd_resp_valid.name());
        sc_trace(o_vcd, o_cmd_resp_data32, o_cmd_resp_data32.name());
        sc_trace(o_vcd, i_cmd_resp_ready, i_cmd_resp_ready.name());
        sc_trace(o_vcd, o_cmd_resp_r1b, o_cmd_resp_r1b.name());
        sc_trace(o_vcd, o_cmd_resp_r2, o_cmd_resp_r2.name());
        sc_trace(o_vcd, o_cmd_resp_r3, o_cmd_resp_r3.name());
        sc_trace(o_vcd, o_cmd_resp_r7, o_cmd_resp_r7.name());
        sc_trace(o_vcd, o_stat_idle_state, o_stat_idle_state.name());
        sc_trace(o_vcd, o_stat_illegal_cmd, o_stat_illegal_cmd.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, i_mem_rdata, i_mem_rdata.name());
        sc_trace(o_vcd, o_crc16_clear, o_crc16_clear.name());
        sc_trace(o_vcd, o_crc16_next, o_crc16_next.name());
        sc_trace(o_vcd, i_crc16, i_crc16.name());
        sc_trace(o_vcd, o_dat_trans, o_dat_trans.name());
        sc_trace(o_vcd, o_dat, o_dat.name());
        sc_trace(o_vcd, i_cmdio_busy, i_cmdio_busy.name());
        sc_trace(o_vcd, r.sdstate, pn + ".r_sdstate");
        sc_trace(o_vcd, r.datastate, pn + ".r_datastate");
        sc_trace(o_vcd, r.powerup_cnt, pn + ".r_powerup_cnt");
        sc_trace(o_vcd, r.preinit_cnt, pn + ".r_preinit_cnt");
        sc_trace(o_vcd, r.delay_cnt, pn + ".r_delay_cnt");
        sc_trace(o_vcd, r.powerup_done, pn + ".r_powerup_done");
        sc_trace(o_vcd, r.cmd_req_ready, pn + ".r_cmd_req_ready");
        sc_trace(o_vcd, r.cmd_resp_valid, pn + ".r_cmd_resp_valid");
        sc_trace(o_vcd, r.cmd_resp_valid_delayed, pn + ".r_cmd_resp_valid_delayed");
        sc_trace(o_vcd, r.cmd_resp_data32, pn + ".r_cmd_resp_data32");
        sc_trace(o_vcd, r.cmd_resp_r1b, pn + ".r_cmd_resp_r1b");
        sc_trace(o_vcd, r.cmd_resp_r2, pn + ".r_cmd_resp_r2");
        sc_trace(o_vcd, r.cmd_resp_r3, pn + ".r_cmd_resp_r3");
        sc_trace(o_vcd, r.cmd_resp_r7, pn + ".r_cmd_resp_r7");
        sc_trace(o_vcd, r.illegal_cmd, pn + ".r_illegal_cmd");
        sc_trace(o_vcd, r.ocr_hcs, pn + ".r_ocr_hcs");
        sc_trace(o_vcd, r.ocr_vdd_window, pn + ".r_ocr_vdd_window");
        sc_trace(o_vcd, r.req_mem_valid, pn + ".r_req_mem_valid");
        sc_trace(o_vcd, r.req_mem_addr, pn + ".r_req_mem_addr");
        sc_trace(o_vcd, r.shiftdat, pn + ".r_shiftdat");
        sc_trace(o_vcd, r.bitcnt, pn + ".r_bitcnt");
        sc_trace(o_vcd, r.crc16_clear, pn + ".r_crc16_clear");
        sc_trace(o_vcd, r.crc16_next, pn + ".r_crc16_next");
        sc_trace(o_vcd, r.dat_trans, pn + ".r_dat_trans");
    }

}

void vip_sdcard_ctrl::comb() {
    bool v_resp_valid;
    sc_uint<32> vb_resp_data32;

    v_resp_valid = 0;
    vb_resp_data32 = 0;

    v = r;

    vb_resp_data32 = r.cmd_resp_data32;

    if ((r.cmd_resp_valid_delayed.read() == 1) && (i_cmd_resp_ready.read() == 1)) {
        v.cmd_resp_valid_delayed = 0;
        v.cmd_resp_r1b = 0;
        v.cmd_resp_r2 = 0;
        v.cmd_resp_r3 = 0;
        v.cmd_resp_r7 = 0;
        v.illegal_cmd = 0;
    }
    // Power-up counter emulates 'busy' bit in ACMD41 response:
    if ((r.powerup_done.read() == 0) && (r.powerup_cnt.read() < CFG_SDCARD_POWERUP_DONE_DELAY_)) {
        v.powerup_cnt = (r.powerup_cnt.read() + 1);
    } else {
        v.powerup_done = 1;
    }

    if (i_cmd_req_valid.read() == 1) {
        switch (r.sdstate.read()) {
        case SDSTATE_IDLE:
            switch (i_cmd_req_cmd.read()) {
            case 0:                                         // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1;
                vb_resp_data32 = 0;
                v.delay_cnt = 20;
                break;
            case 8:                                         // CMD8: SEND_IF_COND.
                // Send memory Card interface condition:
                // [21] PCIe 1.2V support
                // [20] PCIe availability
                // [19:16] Voltage supply
                // [15:8] check pattern
                v.cmd_resp_valid = 1;
                v.cmd_resp_r7 = 1;
                v.delay_cnt = 20;
                vb_resp_data32[13] = (i_cmd_req_data.read()[13] & CFG_SDCARD_PCIE_1_2V_);
                vb_resp_data32[12] = (i_cmd_req_data.read()[12] & CFG_SDCARD_PCIE_AVAIL_);
                vb_resp_data32(11, 8) = (i_cmd_req_data.read()(11, 8) & CFG_SDCARD_VHS_);
                vb_resp_data32(7, 0) = i_cmd_req_data.read()(7, 0);
                break;
            case 55:                                        // CMD55: APP_CMD.
                v.cmd_resp_valid = 1;
                vb_resp_data32 = 0;
                break;
            case 41:                                        // ACMD41: SD_SEND_OP_COND.
                // Send host capacity info:
                // [39] BUSY, active LOW
                // [38] HCS (OCR[30]) Host Capacity
                // [36] XPC
                // [32] S18R
                // [31:8] VDD Voltage Window (OCR[23:0])
                v.ocr_hcs = (i_cmd_req_data.read()[30] & CFG_SDCARD_HCS_);
                v.ocr_vdd_window = (i_cmd_req_data.read()(23, 0) & CFG_SDCARD_VDD_VOLTAGE_WINDOW_);
                v.cmd_resp_valid = 1;
                v.delay_cnt = 20;
                vb_resp_data32[31] = r.powerup_done;
                vb_resp_data32[30] = (i_cmd_req_data.read()[30] & CFG_SDCARD_HCS_);
                vb_resp_data32(23, 0) = (i_cmd_req_data.read()(23, 0) & CFG_SDCARD_VDD_VOLTAGE_WINDOW_);
                if ((i_cmd_req_data.read()(23, 0) & CFG_SDCARD_VDD_VOLTAGE_WINDOW_) == 0) {
                    // OCR check failed:
                    v.sdstate = SDSTATE_INA;
                } else if ((i_spi_mode.read() == 0) && (r.powerup_done.read() == 1)) {
                    // SD mode only
                    v.sdstate = SDSTATE_READY;
                }
                break;
            case 58:                                        // CMD58: READ_OCR.
                v.cmd_resp_valid = 1;
                v.cmd_resp_r7 = 1;
                v.delay_cnt = 20;
                if (i_spi_mode.read() == 1) {
                    vb_resp_data32 = 0;
                    vb_resp_data32[31] = r.powerup_done;
                    vb_resp_data32[30] = r.ocr_hcs;
                    vb_resp_data32(23, 0) = r.ocr_vdd_window;
                } else {
                    v.illegal_cmd = 1;
                }
                break;
            case 17:                                        // CMD17: READ_SINGLE_BLOCK.
                v.cmd_resp_valid = 1;
                v.delay_cnt = 20;
                if (i_spi_mode.read() == 1) {
                    v.req_mem_valid = 1;
                    v.req_mem_addr = (i_cmd_req_data.read() << 9);
                    vb_resp_data32 = 0;
                } else {
                    v.illegal_cmd = 1;
                }
                break;
            default:
                // Illegal commands in 'idle' state:
                v.cmd_resp_valid = 1;
                vb_resp_data32 = ~0ull;
                v.illegal_cmd = 1;
                break;
            }
            break;
        case SDSTATE_READY:
            switch (i_cmd_req_cmd.read()) {
            case 0:                                         // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1;
                vb_resp_data32 = 0;
                v.delay_cnt = 2;
                v.sdstate = SDSTATE_IDLE;
                break;
            case 2:                                         // CMD2: .
                v.cmd_resp_valid = 1;
                v.delay_cnt = 1;
                v.sdstate = SDSTATE_IDENT;
                break;
            case 11:                                        // CMD11: .
                v.cmd_resp_valid = 1;
                v.delay_cnt = 1;
                break;
            default:
                // Illegal commands in 'ready' state:
                v.cmd_resp_valid = 1;
                vb_resp_data32 = ~0ull;
                v.illegal_cmd = 1;
                break;
            }
            break;
        case SDSTATE_IDENT:
            switch (i_cmd_req_cmd.read()) {
            case 0:                                         // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1;
                vb_resp_data32 = 0;
                v.delay_cnt = 2;
                v.sdstate = SDSTATE_IDLE;
                break;
            case 3:                                         // CMD3: .
                v.cmd_resp_valid = 1;
                v.delay_cnt = 1;
                v.sdstate = SDSTATE_STBY;
                break;
            default:
                // Illegal commands in 'stby' state:
                v.cmd_resp_valid = 1;
                vb_resp_data32 = ~0ull;
                v.illegal_cmd = 1;
                break;
            }
            break;
        case SDSTATE_STBY:
            break;
        case SDSTATE_TRAN:
            break;
        case SDSTATE_DATA:
            break;
        case SDSTATE_RCV:
            break;
        case SDSTATE_PRG:
            break;
        case SDSTATE_DIS:
            break;
        case SDSTATE_INA:
            break;
        default:
            break;
        }
    }

    v.shiftdat = ((r.shiftdat.read()(14, 0) << 1) | 1);
    switch (r.datastate.read()) {
    case DATASTATE_IDLE:
        v.crc16_clear = 1;
        v.crc16_next = 0;
        v.dat_trans = 0;
        if ((r.req_mem_valid.read() == 1) && (i_cmdio_busy.read() == 0) && (i_cs.read() == 0)) {
            v.req_mem_valid = 0;
            v.datastate = DATASTATE_START;
            v.shiftdat = 0xFE00;
            v.bitcnt = 0;
            v.dat_trans = 1;
        }
        break;
    case DATASTATE_START:
        v.bitcnt = (r.bitcnt.read() + 1);
        if (r.bitcnt.read()(2, 0).and_reduce() == 1) {
            v.crc16_clear = 0;
            v.crc16_next = 1;
            if (r.bitcnt.read()(12, 3) == 512) {
                v.datastate = DATASTATE_CRC15;
                v.shiftdat = i_crc16;
                v.bitcnt = 0;
                v.crc16_next = 0;
            } else {
                // Read memory byte:
                v.shiftdat = (i_mem_rdata.read(), r.shiftdat.read()(7, 0));
                v.req_mem_addr = (r.req_mem_addr.read() + 1);
            }
        }
        break;
    case DATASTATE_CRC15:
        v.bitcnt = (r.bitcnt.read() + 1);
        if (r.bitcnt.read()(3, 0).and_reduce() == 1) {
            v.datastate = DATASTATE_IDLE;
            v.dat_trans = 0;
        }
        break;
    default:
        break;
    }

    v.cmd_resp_data32 = vb_resp_data32;
    v.cmd_req_ready = (!r.delay_cnt.read().or_reduce());
    if (r.cmd_resp_valid.read() == 1) {
        if (r.delay_cnt.read().or_reduce() == 0) {
            v.cmd_resp_valid_delayed = r.cmd_resp_valid;
            v.cmd_resp_valid = 0;
        } else {
            v.delay_cnt = (r.delay_cnt.read() - 1);
        }
    }

    o_cmd_req_ready = r.cmd_req_ready;
    o_cmd_resp_valid = r.cmd_resp_valid_delayed;
    o_cmd_resp_data32 = r.cmd_resp_data32;
    o_cmd_resp_r1b = r.cmd_resp_r1b;
    o_cmd_resp_r2 = r.cmd_resp_r2;
    o_cmd_resp_r3 = r.cmd_resp_r3;
    o_cmd_resp_r7 = r.cmd_resp_r7;
    o_stat_illegal_cmd = r.illegal_cmd;
    o_stat_idle_state = r.powerup_done;
    o_mem_addr = r.req_mem_addr;
    o_crc16_clear = r.crc16_clear;
    o_crc16_next = r.crc16_next;
    o_dat_trans = r.dat_trans;
    o_dat = r.shiftdat.read()(15, 12);
}

void vip_sdcard_ctrl::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_ctrl_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

