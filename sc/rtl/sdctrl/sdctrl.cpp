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

#include "sdctrl.h"
#include "api_core.h"

namespace debugger {

sdctrl::sdctrl(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_xmapinfo("i_xmapinfo"),
    o_xcfg("o_xcfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_pmapinfo("i_pmapinfo"),
    o_pcfg("o_pcfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_sclk("o_sclk"),
    i_cmd("i_cmd"),
    o_cmd("o_cmd"),
    o_cmd_dir("o_cmd_dir"),
    i_dat0("i_dat0"),
    o_dat0("o_dat0"),
    o_dat0_dir("o_dat0_dir"),
    i_dat1("i_dat1"),
    o_dat1("o_dat1"),
    o_dat1_dir("o_dat1_dir"),
    i_dat2("i_dat2"),
    o_dat2("o_dat2"),
    o_dat2_dir("o_dat2_dir"),
    i_cd_dat3("i_cd_dat3"),
    o_cd_dat3("o_cd_dat3"),
    o_cd_dat3_dir("o_cd_dat3_dir"),
    i_detected("i_detected"),
    i_protect("i_protect") {

    async_reset_ = async_reset;
    xslv0 = 0;
    regs0 = 0;
    crccmd0 = 0;
    crcdat0 = 0;
    cmdtrx0 = 0;

    xslv0 = new axi_slv("xslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_SDCTRL_MEM);
    xslv0->i_clk(i_clk);
    xslv0->i_nrst(i_nrst);
    xslv0->i_mapinfo(i_xmapinfo);
    xslv0->o_cfg(o_xcfg);
    xslv0->i_xslvi(i_xslvi);
    xslv0->o_xslvo(o_xslvo);
    xslv0->o_req_valid(w_mem_req_valid);
    xslv0->o_req_addr(wb_mem_req_addr);
    xslv0->o_req_size(wb_mem_req_size);
    xslv0->o_req_write(w_mem_req_write);
    xslv0->o_req_wdata(wb_mem_req_wdata);
    xslv0->o_req_wstrb(wb_mem_req_wstrb);
    xslv0->o_req_last(w_mem_req_last);
    xslv0->i_req_ready(w_mem_req_ready);
    xslv0->i_resp_valid(w_mem_resp_valid);
    xslv0->i_resp_rdata(wb_mem_resp_rdata);
    xslv0->i_resp_err(wb_mem_resp_err);


    regs0 = new sdctrl_regs("regs0", async_reset);
    regs0->i_clk(i_clk);
    regs0->i_nrst(i_nrst);
    regs0->i_pmapinfo(i_pmapinfo);
    regs0->o_pcfg(o_pcfg);
    regs0->i_apbi(i_apbi);
    regs0->o_apbo(o_apbo);
    regs0->o_sck(o_sclk);
    regs0->o_sck_posedge(w_regs_sck_posedge);
    regs0->o_sck_negedge(w_regs_sck);
    regs0->o_watchdog(wb_regs_watchdog);
    regs0->o_clear_cmderr(w_regs_clear_cmderr);
    regs0->o_pcie_12V_support(w_regs_pcie_12V_support);
    regs0->o_pcie_available(w_regs_pcie_available);
    regs0->o_voltage_supply(wb_regs_voltage_supply);
    regs0->o_check_pattern(wb_regs_check_pattern);
    regs0->i_400khz_ena(w_400kHz_ena);
    regs0->i_sdtype(r.sdtype);
    regs0->i_sdstate(r.sdstate);
    regs0->i_cmd_state(wb_trx_cmdstate);
    regs0->i_cmd_err(wb_trx_cmderr);
    regs0->i_cmd_req_valid(r.cmd_req_valid);
    regs0->i_cmd_req_cmd(r.cmd_req_cmd);
    regs0->i_cmd_resp_valid(w_cmd_resp_valid);
    regs0->i_cmd_resp_cmd(wb_cmd_resp_cmd);
    regs0->i_cmd_resp_reg(wb_cmd_resp_reg);
    regs0->i_cmd_resp_crc7_rx(wb_cmd_resp_crc7_rx);
    regs0->i_cmd_resp_crc7_calc(wb_cmd_resp_crc7_calc);


    crccmd0 = new sdctrl_crc7("crccmd0", async_reset);
    crccmd0->i_clk(i_clk);
    crccmd0->i_nrst(i_nrst);
    crccmd0->i_clear(w_crc7_clear);
    crccmd0->i_next(w_crc7_next);
    crccmd0->i_dat(w_crc7_dat);
    crccmd0->o_crc7(wb_crc7);


    crcdat0 = new sdctrl_crc16("crcdat0", async_reset);
    crcdat0->i_clk(i_clk);
    crcdat0->i_nrst(i_nrst);
    crcdat0->i_clear(r.crc16_clear);
    crcdat0->i_next(w_crc16_next);
    crcdat0->i_dat(wb_crc16_dat);
    crcdat0->o_crc16(wb_crc16);


    cmdtrx0 = new sdctrl_cmd_transmitter("cmdtrx0", async_reset);
    cmdtrx0->i_clk(i_clk);
    cmdtrx0->i_nrst(i_nrst);
    cmdtrx0->i_sclk_posedge(w_regs_sck_posedge);
    cmdtrx0->i_sclk_negedge(w_regs_sck);
    cmdtrx0->i_cmd(i_cmd);
    cmdtrx0->o_cmd(o_cmd);
    cmdtrx0->o_cmd_dir(o_cmd_dir);
    cmdtrx0->i_watchdog(wb_regs_watchdog);
    cmdtrx0->i_cmd_set_low(r.cmd_set_low);
    cmdtrx0->i_req_valid(r.cmd_req_valid);
    cmdtrx0->i_req_cmd(r.cmd_req_cmd);
    cmdtrx0->i_req_arg(r.cmd_req_arg);
    cmdtrx0->i_req_rn(r.cmd_req_rn);
    cmdtrx0->o_req_ready(w_cmd_req_ready);
    cmdtrx0->i_crc7(wb_crc7);
    cmdtrx0->o_crc7_clear(w_crc7_clear);
    cmdtrx0->o_crc7_next(w_crc7_next);
    cmdtrx0->o_crc7_dat(w_crc7_dat);
    cmdtrx0->o_resp_valid(w_cmd_resp_valid);
    cmdtrx0->o_resp_cmd(wb_cmd_resp_cmd);
    cmdtrx0->o_resp_reg(wb_cmd_resp_reg);
    cmdtrx0->o_resp_crc7_rx(wb_cmd_resp_crc7_rx);
    cmdtrx0->o_resp_crc7_calc(wb_cmd_resp_crc7_calc);
    cmdtrx0->i_resp_ready(w_cmd_resp_ready);
    cmdtrx0->i_clear_cmderr(w_clear_cmderr);
    cmdtrx0->o_cmdstate(wb_trx_cmdstate);
    cmdtrx0->o_cmderr(wb_trx_cmderr);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xmapinfo;
    sensitive << i_xslvi;
    sensitive << i_pmapinfo;
    sensitive << i_apbi;
    sensitive << i_cmd;
    sensitive << i_dat0;
    sensitive << i_dat1;
    sensitive << i_dat2;
    sensitive << i_cd_dat3;
    sensitive << i_detected;
    sensitive << i_protect;
    sensitive << w_regs_sck_posedge;
    sensitive << w_regs_sck;
    sensitive << w_regs_clear_cmderr;
    sensitive << wb_regs_watchdog;
    sensitive << w_regs_pcie_12V_support;
    sensitive << w_regs_pcie_available;
    sensitive << wb_regs_voltage_supply;
    sensitive << wb_regs_check_pattern;
    sensitive << w_mem_req_valid;
    sensitive << wb_mem_req_addr;
    sensitive << wb_mem_req_size;
    sensitive << w_mem_req_write;
    sensitive << wb_mem_req_wdata;
    sensitive << wb_mem_req_wstrb;
    sensitive << w_mem_req_last;
    sensitive << w_mem_req_ready;
    sensitive << w_mem_resp_valid;
    sensitive << wb_mem_resp_rdata;
    sensitive << wb_mem_resp_err;
    sensitive << w_cmd_req_ready;
    sensitive << w_cmd_resp_valid;
    sensitive << wb_cmd_resp_cmd;
    sensitive << wb_cmd_resp_reg;
    sensitive << wb_cmd_resp_crc7_rx;
    sensitive << wb_cmd_resp_crc7_calc;
    sensitive << w_cmd_resp_ready;
    sensitive << wb_trx_cmdstate;
    sensitive << wb_trx_cmderr;
    sensitive << w_clear_cmderr;
    sensitive << w_400kHz_ena;
    sensitive << w_crc7_clear;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << wb_crc7;
    sensitive << w_crc16_next;
    sensitive << wb_crc16_dat;
    sensitive << wb_crc16;
    sensitive << r.clkcnt;
    sensitive << r.cmd_set_low;
    sensitive << r.cmd_req_valid;
    sensitive << r.cmd_req_cmd;
    sensitive << r.cmd_req_arg;
    sensitive << r.cmd_req_rn;
    sensitive << r.cmd_resp_r1;
    sensitive << r.cmd_resp_reg;
    sensitive << r.crc16_clear;
    sensitive << r.dat;
    sensitive << r.dat_dir;
    sensitive << r.sdstate;
    sensitive << r.initstate;
    sensitive << r.readystate;
    sensitive << r.identstate;
    sensitive << r.wait_cmd_resp;
    sensitive << r.sdtype;
    sensitive << r.HCS;
    sensitive << r.S18;
    sensitive << r.RCA;
    sensitive << r.OCR_VoltageWindow;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

sdctrl::~sdctrl() {
    if (xslv0) {
        delete xslv0;
    }
    if (regs0) {
        delete regs0;
    }
    if (crccmd0) {
        delete crccmd0;
    }
    if (crcdat0) {
        delete crcdat0;
    }
    if (cmdtrx0) {
        delete cmdtrx0;
    }
}

void sdctrl::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xmapinfo, i_xmapinfo.name());
        sc_trace(o_vcd, o_xcfg, o_xcfg.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_pmapinfo, i_pmapinfo.name());
        sc_trace(o_vcd, o_pcfg, o_pcfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_sclk, o_sclk.name());
        sc_trace(o_vcd, i_cmd, i_cmd.name());
        sc_trace(o_vcd, o_cmd, o_cmd.name());
        sc_trace(o_vcd, o_cmd_dir, o_cmd_dir.name());
        sc_trace(o_vcd, i_dat0, i_dat0.name());
        sc_trace(o_vcd, o_dat0, o_dat0.name());
        sc_trace(o_vcd, o_dat0_dir, o_dat0_dir.name());
        sc_trace(o_vcd, i_dat1, i_dat1.name());
        sc_trace(o_vcd, o_dat1, o_dat1.name());
        sc_trace(o_vcd, o_dat1_dir, o_dat1_dir.name());
        sc_trace(o_vcd, i_dat2, i_dat2.name());
        sc_trace(o_vcd, o_dat2, o_dat2.name());
        sc_trace(o_vcd, o_dat2_dir, o_dat2_dir.name());
        sc_trace(o_vcd, i_cd_dat3, i_cd_dat3.name());
        sc_trace(o_vcd, o_cd_dat3, o_cd_dat3.name());
        sc_trace(o_vcd, o_cd_dat3_dir, o_cd_dat3_dir.name());
        sc_trace(o_vcd, i_detected, i_detected.name());
        sc_trace(o_vcd, i_protect, i_protect.name());
        sc_trace(o_vcd, r.clkcnt, pn + ".r_clkcnt");
        sc_trace(o_vcd, r.cmd_set_low, pn + ".r_cmd_set_low");
        sc_trace(o_vcd, r.cmd_req_valid, pn + ".r_cmd_req_valid");
        sc_trace(o_vcd, r.cmd_req_cmd, pn + ".r_cmd_req_cmd");
        sc_trace(o_vcd, r.cmd_req_arg, pn + ".r_cmd_req_arg");
        sc_trace(o_vcd, r.cmd_req_rn, pn + ".r_cmd_req_rn");
        sc_trace(o_vcd, r.cmd_resp_r1, pn + ".r_cmd_resp_r1");
        sc_trace(o_vcd, r.cmd_resp_reg, pn + ".r_cmd_resp_reg");
        sc_trace(o_vcd, r.crc16_clear, pn + ".r_crc16_clear");
        sc_trace(o_vcd, r.dat, pn + ".r_dat");
        sc_trace(o_vcd, r.dat_dir, pn + ".r_dat_dir");
        sc_trace(o_vcd, r.sdstate, pn + ".r_sdstate");
        sc_trace(o_vcd, r.initstate, pn + ".r_initstate");
        sc_trace(o_vcd, r.readystate, pn + ".r_readystate");
        sc_trace(o_vcd, r.identstate, pn + ".r_identstate");
        sc_trace(o_vcd, r.wait_cmd_resp, pn + ".r_wait_cmd_resp");
        sc_trace(o_vcd, r.sdtype, pn + ".r_sdtype");
        sc_trace(o_vcd, r.HCS, pn + ".r_HCS");
        sc_trace(o_vcd, r.S18, pn + ".r_S18");
        sc_trace(o_vcd, r.RCA, pn + ".r_RCA");
        sc_trace(o_vcd, r.OCR_VoltageWindow, pn + ".r_OCR_VoltageWindow");
    }

    if (xslv0) {
        xslv0->generateVCD(i_vcd, o_vcd);
    }
    if (regs0) {
        regs0->generateVCD(i_vcd, o_vcd);
    }
    if (crccmd0) {
        crccmd0->generateVCD(i_vcd, o_vcd);
    }
    if (crcdat0) {
        crcdat0->generateVCD(i_vcd, o_vcd);
    }
    if (cmdtrx0) {
        cmdtrx0->generateVCD(i_vcd, o_vcd);
    }
}

void sdctrl::comb() {
    bool v_crc16_next;
    sc_uint<32> vb_cmd_req_arg;
    bool v_cmd_resp_ready;
    bool v_clear_cmderr;

    v_crc16_next = 0;
    vb_cmd_req_arg = 0;
    v_cmd_resp_ready = 0;
    v_clear_cmderr = 0;

    v = r;

    vb_cmd_req_arg = r.cmd_req_arg;

    if (r.wait_cmd_resp.read() == 1) {
        v_cmd_resp_ready = 1;
        if (w_cmd_resp_valid.read() == 1) {
            v.wait_cmd_resp = 0;
            v.cmd_resp_r1 = wb_cmd_resp_cmd;
            v.cmd_resp_reg = wb_cmd_resp_reg;

            if ((r.cmd_req_cmd.read() == CMD8)
                    && (wb_trx_cmderr.read() == CMDERR_NO_RESPONSE)) {
                v.sdtype = SDCARD_VER1X;
                v.HCS = 0;                                  // Standard Capacity only
                v.initstate = IDLESTATE_CMD55;
                v_clear_cmderr = 1;
            } else if (wb_trx_cmderr.read() != CMDERR_NONE) {
                if (r.cmd_req_cmd.read() == CMD0) {
                    // Re-send CMD0
                    v.initstate = IDLESTATE_CMD0;
                    v_clear_cmderr = 1;
                } else {
                    v.sdstate = SDSTATE_INA;
                    v.sdtype = SDCARD_UNUSABLE;
                }
            } else {
                // Parse Rx response:
                switch (r.cmd_req_rn.read()) {
                case R1:
                    break;
                case R3:
                    // Table 5-1: OCR Register definition, page 246
                    //     [23:0]  Voltage window can be requested by CMD58
                    //     [24]    Switching to 1.8V accepted (S18A)
                    //     [27]    Over 2TB support status (CO2T)
                    //     [29]    UHS-II Card status
                    //     [30]    Card Capacity Status (CCS)
                    //     [31]    Card power-up status (busy is LOW if the card not finished the power-up routine)
                    if (wb_cmd_resp_reg.read()[31] == 1) {
                        v.OCR_VoltageWindow = wb_cmd_resp_reg.read()(23, 0);
                        v.HCS = wb_cmd_resp_reg.read()[30];
                        v.S18 = wb_cmd_resp_reg.read()[24];
                    }
                    break;
                case R6:
                    v.RCA = (wb_cmd_resp_reg.read()(31, 16) << 16);
                    break;
                default:
                    break;
                }
            }
        }
    } else if (r.cmd_req_valid.read() == 1) {
        // Do nothing wait to accept
    } else {
        // SD-card global state:
        switch (r.sdstate.read()) {
        case SDSTATE_PRE_INIT:
            // Page 222, Fig.4-96 State Diagram (Pre-Init mode)
            // 1. No commands were sent to the card after POW (except CMD0):
            //     CMD line held High for at least 1 ms (by SW), then SDCLK supplied
            //     at least 74 clocks with keeping CMD line High
            // 2. CMD High to Low transition && CMD=Low < 74 clocks then go idle,
            //     if Low >= 74 clocks then Fast boot in CV-mode
            if (w_regs_sck_posedge.read() == 1) {
                v.clkcnt = (r.clkcnt.read() + 1);
            }
            if (r.clkcnt.read() >= 73) {
                v.sdstate = SDSTATE_IDLE;
            }
            if (r.clkcnt.read() <= 63) {
            } else {
                v.cmd_set_low = 0;
            }
            break;
        case SDSTATE_IDLE:
            switch (r.initstate.read()) {
            case IDLESTATE_CMD0:
                v.sdtype = SDCARD_UNKNOWN;
                v.HCS = 1;
                v.S18 = 0;
                v.RCA = 0;
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD0;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = 0;
                v.initstate = IDLESTATE_CMD8;
                break;
            case IDLESTATE_CMD8:
                // See page 113. 4.3.13 Send Interface Condition Command
                //   [39:22] reserved 00000h
                //   [21]    PCIe 1.2V support 0
                //   [20]    PCIe availability 0
                //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
                //   [15:8]  Check Pattern 55h
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD8;
                v.cmd_req_rn = R7;
                vb_cmd_req_arg = 0;
                vb_cmd_req_arg[13] = w_regs_pcie_12V_support.read();
                vb_cmd_req_arg[12] = w_regs_pcie_available.read();
                vb_cmd_req_arg(11, 8) = wb_regs_voltage_supply;
                vb_cmd_req_arg(7, 0) = wb_regs_check_pattern;
                v.initstate = IDLESTATE_CMD55;
                break;
            case IDLESTATE_CMD55:
                // Page 64: APP_CMD (CMD55) shall always precede ACMD41.
                //   [31:16] RCA (Relative Adrress should be set 0)
                //   [15:0] stuff bits
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD55;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = 0;
                v.initstate = IDLESTATE_ACMD41;
                break;
            case IDLESTATE_ACMD41:
                // Page 131: SD_SEND_OP_COND. 
                //   [31] reserved bit
                //   [30] HCS (high capacity support)
                //   [29] reserved for eSD
                //   [28] XPC (maximum power in default speed)
                //   [27:25] reserved bits
                //   [24] S18R Send request to switch to 1.8V
                //   [23:0] VDD voltage window (OCR[23:0])
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = ACMD41;
                v.cmd_req_rn = R3;
                vb_cmd_req_arg = 0;
                vb_cmd_req_arg[30] = r.HCS.read();
                vb_cmd_req_arg[24] = r.S18.read();
                vb_cmd_req_arg(23, 0) = r.OCR_VoltageWindow;
                v.initstate = IDLESTATE_CARD_IDENTIFICATION;
                break;
            case IDLESTATE_CARD_IDENTIFICATION:
                if (r.cmd_resp_reg.read()[31] == 0) {
                    // LOW if the card has not finished power-up routine
                    v.initstate = IDLESTATE_CMD55;
                } else {
                    if (r.HCS.read() == 1) {
                        v.sdtype = SDCARD_VER2X_HC;
                    } else if (r.sdtype.read() == SDCARD_UNKNOWN) {
                        v.sdtype = SDCARD_VER2X_SC;
                    }
                    if (r.S18.read() == 1) {
                        // Voltage switch command to change 3.3V to 1.8V
                        v.readystate = READYSTATE_CMD11;
                    } else {
                        v.readystate = READYSTATE_CMD2;
                    }
                    v.sdstate = SDSTATE_READY;
                }
                break;
            default:
                v.initstate = IDLESTATE_CMD0;
                break;
            }
            break;
        case SDSTATE_READY:
            switch (r.readystate.read()) {
            case READYSTATE_CMD11:
                // CMD11: VOLTAGE_SWITCH siwtch to 1.8V bus signaling.
                //   [31:0] reserved all zeros
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD11;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = 0;
                v.readystate = READYSTATE_CMD2;
                break;
            case READYSTATE_CMD2:
                // CMD2: ALL_SEND_CID ask to send CID number.
                //   [31:0] stuff bits
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD2;
                v.cmd_req_rn = R2;
                vb_cmd_req_arg = 0;
                v.readystate = READYSTATE_CHECK_CID;
                break;
            case READYSTATE_CHECK_CID:
                v.sdstate = SDSTATE_IDENT;
                v.identstate = IDENTSTATE_CMD3;
                break;
            default:
                break;
            }
            break;
        case SDSTATE_IDENT:
            switch (r.identstate.read()) {
            case IDENTSTATE_CMD3:
                // CMD3: SEND_RELATIVE_ADDR ask card to publish a new relative address (RCA).
                //   [31:0] stuff bits
                v.cmd_req_valid = 1;
                v.cmd_req_cmd = CMD3;
                v.cmd_req_rn = R6;
                vb_cmd_req_arg = 0;
                v.identstate = IDENTSTATE_CHECK_RCA;
                break;
            case IDENTSTATE_CHECK_RCA:
                v.sdstate = SDSTATE_STBY;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    v.cmd_req_arg = vb_cmd_req_arg;

    if ((r.cmd_req_valid.read() == 1) && (w_cmd_req_ready.read() == 1)) {
        v.cmd_req_valid = 0;
        v.wait_cmd_resp = 1;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(v);
    }

    w_cmd_resp_ready = v_cmd_resp_ready;
    w_crc16_next = v_crc16_next;
    // Page 222, Table 4-81 Overview of Card States vs Operation Modes table
    if ((r.sdstate.read() <= SDSTATE_IDENT)
            || (r.sdstate.read() == SDSTATE_INA)
            || (r.sdstate.read() == SDSTATE_PRE_INIT)) {
        w_400kHz_ena = 1;
    } else {
        // data transfer mode:
        // Stand-By, Transfer, Sending, Receive, Programming, Disconnect states
        w_400kHz_ena = 0;
    }

    o_cd_dat3 = r.dat.read()[3];
    o_dat2 = r.dat.read()[2];
    o_dat1 = r.dat.read()[1];
    o_dat0 = r.dat.read()[0];
    // Direction bits:
    o_dat0_dir = r.dat_dir;
    o_dat1_dir = r.dat_dir;
    o_dat2_dir = r.dat_dir;
    o_cd_dat3_dir = r.dat_dir;
    // Memory request:
    w_mem_req_ready = 1;
    w_mem_resp_valid = 1;
    wb_mem_resp_rdata = ~0ull;
    wb_mem_resp_err = 0;
    w_clear_cmderr = (w_regs_clear_cmderr || v_clear_cmderr);
}

void sdctrl::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

