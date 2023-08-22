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
    regs0->i_cmd_state(wb_cmdstate);
    regs0->i_cmd_err(wb_cmderr);
    regs0->i_cmd_req_valid(r.cmd_req_ena);
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
    cmdtrx0->i_req_valid(r.cmd_req_ena);
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
    cmdtrx0->i_clear_cmderr(w_regs_clear_cmderr);
    cmdtrx0->o_cmdstate(wb_cmdstate);
    cmdtrx0->o_cmderr(wb_cmderr);



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
    sensitive << wb_cmdstate;
    sensitive << wb_cmderr;
    sensitive << w_crc7_clear;
    sensitive << w_crc7_next;
    sensitive << w_crc7_dat;
    sensitive << wb_crc7;
    sensitive << w_crc16_next;
    sensitive << wb_crc16_dat;
    sensitive << wb_crc16;
    sensitive << r.clkcnt;
    sensitive << r.cmd_req_ena;
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
    sensitive << r.initstate_next;

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
        sc_trace(o_vcd, r.cmd_req_ena, pn + ".r_cmd_req_ena");
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
        sc_trace(o_vcd, r.initstate_next, pn + ".r_initstate_next");
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

    v_crc16_next = 0;

    v = r;


    // SD-card global state:
    switch (r.sdstate.read()) {
    case SDSTATE_PRE_INIT:
        // Page 222, Fig.4-96 State Diagram (Pre-Init mode)
        // 1. No commands were sent to the card after POW (except CMD0):
        //     CMD line held High for at least 1 ms, then SDCLK supplied
        //     at least 74 clocks with keeping CMD line High
        if (w_regs_sck_posedge.read() == 1) {
            v.clkcnt = (r.clkcnt.read() + 1);
        }
        if (r.clkcnt.read() >= 75) {
            v.sdstate = SDSTATE_IDLE;
        }
        break;
    case SDSTATE_IDLE:
        switch (r.initstate.read()) {
        case INITSTATE_CMD0:
            v.cmd_req_ena = 1;
            v.cmd_req_cmd = CMD0;
            v.cmd_req_arg = 0;
            v.cmd_req_rn = R1;
            v.initstate = INITSTATE_WAIT_RESP;
            v.initstate_next = INITSTATE_CMD8;
            break;
        case INITSTATE_CMD8:
            // See page 113. 4.3.13 Send Interface Condition Command
            //   [39:22] reserved 00000h
            //   [21]    PCIe 1.2V support 0
            //   [20]    PCIe availability 0
            //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
            //   [15:8]  Check Pattern 55h
            v.cmd_req_ena = 1;
            v.cmd_req_cmd = CMD8;
            v.cmd_req_arg = (0,
                    w_regs_pcie_12V_support,
                    w_regs_pcie_available,
                    wb_regs_voltage_supply,
                    wb_regs_check_pattern);
            v.cmd_req_rn = R1;
            if (wb_cmderr.read() == CMDERR_NONE) {
                // See page 143. 4.10.1 Card Status response on CMD0
                v.initstate = INITSTATE_WAIT_RESP;
                v.initstate_next = INITSTATE_ACMD41;
            } else {
                v.initstate = INITSTATE_ERROR;
            }
            break;
        case INITSTATE_ACMD41:
            break;
        case INITSTATE_CMD11:
            break;
        case INITSTATE_CMD2:
            break;
        case INITSTATE_CMD3:
            break;
        case INITSTATE_WAIT_RESP:
            if (w_cmd_resp_valid.read() == 1) {
                v.cmd_resp_r1 = wb_cmd_resp_cmd;
                v.cmd_resp_reg = wb_cmd_resp_reg;
                v.initstate = r.initstate_next;
            }
            break;
        case INITSTATE_ERROR:
            break;
        case INITSTATE_DONE:
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if ((r.cmd_req_ena.read() == 1) && (w_cmd_req_ready.read() == 1)) {
        v.cmd_req_ena = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(v);
    }

    w_cmd_resp_ready = 1;
    w_crc16_next = v_crc16_next;

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
}

void sdctrl::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

