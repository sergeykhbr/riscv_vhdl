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

#include "asic_top.h"
#include "api_core.h"

namespace debugger {

asic_top::asic_top(sc_module_name name,
                   int sim_uart_speedup_rate)
    : sc_module(name),
    i_rst("i_rst"),
    i_sclk_p("i_sclk_p"),
    i_sclk_n("i_sclk_n"),
    io_gpio("io_gpio"),
    i_jtag_trst("i_jtag_trst"),
    i_jtag_tck("i_jtag_tck"),
    i_jtag_tms("i_jtag_tms"),
    i_jtag_tdi("i_jtag_tdi"),
    o_jtag_tdo("o_jtag_tdo"),
    o_jtag_vref("o_jtag_vref"),
    i_uart1_rd("i_uart1_rd"),
    o_uart1_td("o_uart1_td"),
    o_sd_sclk("o_sd_sclk"),
    io_sd_cmd("io_sd_cmd"),
    io_sd_dat0("io_sd_dat0"),
    io_sd_dat1("io_sd_dat1"),
    io_sd_dat2("io_sd_dat2"),
    io_sd_cd_dat3("io_sd_cd_dat3"),
    i_sd_detected("i_sd_detected"),
    i_sd_protect("i_sd_protect") {

    sim_uart_speedup_rate_ = sim_uart_speedup_rate;
    iclk0 = 0;
    iosdcmd0 = 0;
    iosddat0 = 0;
    iosddat1 = 0;
    iosddat2 = 0;
    iosddat3 = 0;
    pll0 = 0;
    prci0 = 0;
    soc0 = 0;

    iclk0 = new ids_tech("iclk0");
    iclk0->i_clk_p(i_sclk_p);
    iclk0->i_clk_n(i_sclk_n);
    iclk0->o_clk(ib_clk_tcxo);

    iosdcmd0 = new iobuf_tech("iosdcmd0");
    iosdcmd0->io(io_sd_cmd);
    iosdcmd0->o(ib_sd_cmd);
    iosdcmd0->i(ob_sd_cmd);
    iosdcmd0->t(ob_sd_cmd_direction);

    iosddat0 = new iobuf_tech("iosddat0");
    iosddat0->io(io_sd_dat0);
    iosddat0->o(ib_sd_dat0);
    iosddat0->i(ob_sd_dat0);
    iosddat0->t(ob_sd_dat0_direction);

    iosddat1 = new iobuf_tech("iosddat1");
    iosddat1->io(io_sd_dat1);
    iosddat1->o(ib_sd_dat1);
    iosddat1->i(ob_sd_dat1);
    iosddat1->t(ob_sd_dat1_direction);

    iosddat2 = new iobuf_tech("iosddat2");
    iosddat2->io(io_sd_dat2);
    iosddat2->o(ib_sd_dat2);
    iosddat2->i(ob_sd_dat2);
    iosddat2->t(ob_sd_dat2_direction);

    iosddat3 = new iobuf_tech("iosddat3");
    iosddat3->io(io_sd_cd_dat3);
    iosddat3->o(ib_sd_cd_dat3);
    iosddat3->i(ob_sd_cd_dat3);
    iosddat3->t(ob_sd_cd_dat3_direction);

    pll0 = new SysPLL_tech("pll0");
    pll0->i_reset(i_rst);
    pll0->i_clk_tcxo(ib_clk_tcxo);
    pll0->o_clk_sys(w_sys_clk);
    pll0->o_clk_ddr(w_ddr_clk);
    pll0->o_locked(w_pll_lock);

    prci0 = new apb_prci("prci0", async_reset);
    prci0->i_clk(ib_clk_tcxo);
    prci0->i_pwrreset(i_rst);
    prci0->i_dmireset(w_dmreset);
    prci0->i_sys_locked(w_pll_lock);
    prci0->i_ddr_locked(w_ddr3_init_calib_complete);
    prci0->o_sys_rst(w_sys_rst);
    prci0->o_sys_nrst(w_sys_nrst);
    prci0->o_dbg_nrst(w_dbg_nrst);
    prci0->i_mapinfo(prci_pmapinfo);
    prci0->o_cfg(prci_dev_cfg);
    prci0->i_apbi(prci_apbi);
    prci0->o_apbo(prci_apbo);

    soc0 = new riscv_soc("soc0", async_reset,
                          sim_uart_speedup_rate);
    soc0->i_sys_nrst(w_sys_nrst);
    soc0->i_sys_clk(w_sys_clk);
    soc0->i_dbg_nrst(w_dbg_nrst);
    soc0->i_ddr_nrst(w_ddr_ui_nrst);
    soc0->i_ddr_clk(w_ddr_ui_clk);
    soc0->i_gpio(ib_gpio_ipins);
    soc0->o_gpio(ob_gpio_opins);
    soc0->o_gpio_dir(ob_gpio_direction);
    soc0->i_jtag_trst(i_jtag_trst);
    soc0->i_jtag_tck(i_jtag_tck);
    soc0->i_jtag_tms(i_jtag_tms);
    soc0->i_jtag_tdi(i_jtag_tdi);
    soc0->o_jtag_tdo(o_jtag_tdo);
    soc0->o_jtag_vref(o_jtag_vref);
    soc0->i_uart1_rd(i_uart1_rd);
    soc0->o_uart1_td(o_uart1_td);
    soc0->o_sd_sclk(o_sd_sclk);
    soc0->i_sd_cmd(ib_sd_cmd);
    soc0->o_sd_cmd(ob_sd_cmd);
    soc0->o_sd_cmd_dir(ob_sd_cmd_direction);
    soc0->i_sd_dat0(ib_sd_dat0);
    soc0->o_sd_dat0(ob_sd_dat0);
    soc0->o_sd_dat0_dir(ob_sd_dat0_direction);
    soc0->i_sd_dat1(ib_sd_dat1);
    soc0->o_sd_dat1(ob_sd_dat1);
    soc0->o_sd_dat1_dir(ob_sd_dat1_direction);
    soc0->i_sd_dat2(ib_sd_dat2);
    soc0->o_sd_dat2(ob_sd_dat2);
    soc0->o_sd_dat2_dir(ob_sd_dat2_direction);
    soc0->i_sd_cd_dat3(ib_sd_cd_dat3);
    soc0->o_sd_cd_dat3(ob_sd_cd_dat3);
    soc0->o_sd_cd_dat3_dir(ob_sd_cd_dat3_direction);
    soc0->i_sd_detected(i_sd_detected);
    soc0->i_sd_protect(i_sd_protect);
    soc0->o_dmreset(w_dmreset);
    soc0->o_prci_pmapinfo(prci_pmapinfo);
    soc0->i_prci_pdevcfg(prci_dev_cfg);
    soc0->o_prci_apbi(prci_apbi);
    soc0->i_prci_apbo(prci_apbo);
    soc0->o_ddr_pmapinfo(ddr_pmapinfo);
    soc0->i_ddr_pdevcfg(ddr_pdev_cfg);
    soc0->o_ddr_apbi(ddr_apbi);
    soc0->i_ddr_apbo(ddr_apbo);
    soc0->o_ddr_xmapinfo(ddr_xmapinfo);
    soc0->i_ddr_xdevcfg(ddr_xdev_cfg);
    soc0->o_ddr_xslvi(ddr_xslvi);
    soc0->i_ddr_xslvo(ddr_xslvo);
}

asic_top::~asic_top() {
    if (iclk0) {
        delete iclk0;
    }
    if (iosdcmd0) {
        delete iosdcmd0;
    }
    if (iosddat0) {
        delete iosddat0;
    }
    if (iosddat1) {
        delete iosddat1;
    }
    if (iosddat2) {
        delete iosddat2;
    }
    if (iosddat3) {
        delete iosddat3;
    }
    if (pll0) {
        delete pll0;
    }
    if (prci0) {
        delete prci0;
    }
    if (soc0) {
        delete soc0;
    }
}

void asic_top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_rst, i_rst.name());
        sc_trace(o_vcd, i_sclk_p, i_sclk_p.name());
        sc_trace(o_vcd, i_sclk_n, i_sclk_n.name());
        sc_trace(o_vcd, io_gpio, io_gpio.name());
        sc_trace(o_vcd, i_jtag_trst, i_jtag_trst.name());
        sc_trace(o_vcd, i_jtag_tck, i_jtag_tck.name());
        sc_trace(o_vcd, i_jtag_tms, i_jtag_tms.name());
        sc_trace(o_vcd, i_jtag_tdi, i_jtag_tdi.name());
        sc_trace(o_vcd, o_jtag_tdo, o_jtag_tdo.name());
        sc_trace(o_vcd, o_jtag_vref, o_jtag_vref.name());
        sc_trace(o_vcd, i_uart1_rd, i_uart1_rd.name());
        sc_trace(o_vcd, o_uart1_td, o_uart1_td.name());
        sc_trace(o_vcd, o_sd_sclk, o_sd_sclk.name());
        sc_trace(o_vcd, io_sd_cmd, io_sd_cmd.name());
        sc_trace(o_vcd, io_sd_dat0, io_sd_dat0.name());
        sc_trace(o_vcd, io_sd_dat1, io_sd_dat1.name());
        sc_trace(o_vcd, io_sd_dat2, io_sd_dat2.name());
        sc_trace(o_vcd, io_sd_cd_dat3, io_sd_cd_dat3.name());
        sc_trace(o_vcd, i_sd_detected, i_sd_detected.name());
        sc_trace(o_vcd, i_sd_protect, i_sd_protect.name());
        sc_trace(o_vcd, ddr_xslvo, ddr_xslvo.name());
        sc_trace(o_vcd, ddr_xslvi, ddr_xslvi.name());
        sc_trace(o_vcd, ddr_apbi, ddr_apbi.name());
        sc_trace(o_vcd, ddr_apbo, ddr_apbo.name());
        sc_trace(o_vcd, prci_apbi, prci_apbi.name());
        sc_trace(o_vcd, prci_apbo, prci_apbo.name());
    }

    if (iclk0) {
        iclk0->generateVCD(i_vcd, o_vcd);
    }
    if (iosdcmd0) {
        iosdcmd0->generateVCD(i_vcd, o_vcd);
    }
    if (iosddat0) {
        iosddat0->generateVCD(i_vcd, o_vcd);
    }
    if (iosddat1) {
        iosddat1->generateVCD(i_vcd, o_vcd);
    }
    if (iosddat2) {
        iosddat2->generateVCD(i_vcd, o_vcd);
    }
    if (iosddat3) {
        iosddat3->generateVCD(i_vcd, o_vcd);
    }
    if (pll0) {
        pll0->generateVCD(i_vcd, o_vcd);
    }
    if (prci0) {
        prci0->generateVCD(i_vcd, o_vcd);
    }
    if (soc0) {
        soc0->generateVCD(i_vcd, o_vcd);
    }
}

}  // namespace debugger

