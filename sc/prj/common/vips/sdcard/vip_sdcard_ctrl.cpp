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
                                 sc_uint<4> CFG_SDCARD_VHS,
                                 bool CFG_SDCARD_PCIE_1_2V,
                                 bool CFG_SDCARD_PCIE_AVAIL,
                                 sc_uint<24> CFG_SDCARD_VDD_VOLTAGE_WINDOW)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_clk("i_clk"),
    i_cmd_req_valid("i_cmd_req_valid"),
    i_cmd_req_cmd("i_cmd_req_cmd"),
    i_cmd_req_data("i_cmd_req_data"),
    o_cmd_req_ready("o_cmd_req_ready"),
    o_cmd_resp_valid("o_cmd_resp_valid"),
    o_cmd_resp_data32("o_cmd_resp_data32"),
    i_cmd_resp_ready("i_cmd_resp_ready") {

    async_reset_ = async_reset;
    CFG_SDCARD_POWERUP_DONE_DELAY_ = CFG_SDCARD_POWERUP_DONE_DELAY;
    CFG_SDCARD_VHS_ = CFG_SDCARD_VHS;
    CFG_SDCARD_PCIE_1_2V_ = CFG_SDCARD_PCIE_1_2V;
    CFG_SDCARD_PCIE_AVAIL_ = CFG_SDCARD_PCIE_AVAIL;
    CFG_SDCARD_VDD_VOLTAGE_WINDOW_ = CFG_SDCARD_VDD_VOLTAGE_WINDOW;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_cmd_req_valid;
    sensitive << i_cmd_req_cmd;
    sensitive << i_cmd_req_data;
    sensitive << i_cmd_resp_ready;
    sensitive << r.sdstate;
    sensitive << r.powerup_cnt;
    sensitive << r.preinit_cnt;
    sensitive << r.delay_cnt;
    sensitive << r.powerup_done;
    sensitive << r.cmd_req_ready;
    sensitive << r.cmd_resp_valid;
    sensitive << r.cmd_resp_valid_delayed;
    sensitive << r.cmd_resp_data32;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_sdcard_ctrl::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_cmd_req_valid, i_cmd_req_valid.name());
        sc_trace(o_vcd, i_cmd_req_cmd, i_cmd_req_cmd.name());
        sc_trace(o_vcd, i_cmd_req_data, i_cmd_req_data.name());
        sc_trace(o_vcd, o_cmd_req_ready, o_cmd_req_ready.name());
        sc_trace(o_vcd, o_cmd_resp_valid, o_cmd_resp_valid.name());
        sc_trace(o_vcd, o_cmd_resp_data32, o_cmd_resp_data32.name());
        sc_trace(o_vcd, i_cmd_resp_ready, i_cmd_resp_ready.name());
        sc_trace(o_vcd, r.sdstate, pn + ".r_sdstate");
        sc_trace(o_vcd, r.powerup_cnt, pn + ".r_powerup_cnt");
        sc_trace(o_vcd, r.preinit_cnt, pn + ".r_preinit_cnt");
        sc_trace(o_vcd, r.delay_cnt, pn + ".r_delay_cnt");
        sc_trace(o_vcd, r.powerup_done, pn + ".r_powerup_done");
        sc_trace(o_vcd, r.cmd_req_ready, pn + ".r_cmd_req_ready");
        sc_trace(o_vcd, r.cmd_resp_valid, pn + ".r_cmd_resp_valid");
        sc_trace(o_vcd, r.cmd_resp_valid_delayed, pn + ".r_cmd_resp_valid_delayed");
        sc_trace(o_vcd, r.cmd_resp_data32, pn + ".r_cmd_resp_data32");
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
                v.cmd_resp_valid = 1;
                v.delay_cnt = 20;
                vb_resp_data32[31] = r.powerup_done.read();
                vb_resp_data32(23, 0) = (i_cmd_req_data.read()(23, 0) & CFG_SDCARD_VDD_VOLTAGE_WINDOW_);
                if ((i_cmd_req_data.read()(23, 0) & CFG_SDCARD_VDD_VOLTAGE_WINDOW_) == 0) {
                    // OCR check failed:
                    v.sdstate = SDSTATE_INA;
                } else if (r.powerup_done.read() == 1) {
                    v.sdstate = SDSTATE_READY;
                }
                break;
            default:
                // Illegal commands in 'idle' state:
                v.cmd_resp_valid = 1;
                vb_resp_data32 = ~0ull;
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
}

void vip_sdcard_ctrl::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        vip_sdcard_ctrl_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

