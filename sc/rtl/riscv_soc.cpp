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

#include "riscv_soc.h"
#include "api_core.h"

namespace debugger {

riscv_soc::riscv_soc(sc_module_name name,
                     bool async_reset,
                     int sim_uart_speedup_rate)
    : sc_module(name),
    i_sys_nrst("i_sys_nrst"),
    i_sys_clk("i_sys_clk"),
    i_dbg_nrst("i_dbg_nrst"),
    i_ddr_nrst("i_ddr_nrst"),
    i_ddr_clk("i_ddr_clk"),
    i_gpio("i_gpio"),
    o_gpio("o_gpio"),
    o_gpio_dir("o_gpio_dir"),
    i_jtag_trst("i_jtag_trst"),
    i_jtag_tck("i_jtag_tck"),
    i_jtag_tms("i_jtag_tms"),
    i_jtag_tdi("i_jtag_tdi"),
    o_jtag_tdo("o_jtag_tdo"),
    o_jtag_vref("o_jtag_vref"),
    i_uart1_rd("i_uart1_rd"),
    o_uart1_td("o_uart1_td"),
    o_sd_sclk("o_sd_sclk"),
    i_sd_cmd("i_sd_cmd"),
    o_sd_cmd("o_sd_cmd"),
    o_sd_cmd_dir("o_sd_cmd_dir"),
    i_sd_dat0("i_sd_dat0"),
    o_sd_dat0("o_sd_dat0"),
    o_sd_dat0_dir("o_sd_dat0_dir"),
    i_sd_dat1("i_sd_dat1"),
    o_sd_dat1("o_sd_dat1"),
    o_sd_dat1_dir("o_sd_dat1_dir"),
    i_sd_dat2("i_sd_dat2"),
    o_sd_dat2("o_sd_dat2"),
    o_sd_dat2_dir("o_sd_dat2_dir"),
    i_sd_cd_dat3("i_sd_cd_dat3"),
    o_sd_cd_dat3("o_sd_cd_dat3"),
    o_sd_cd_dat3_dir("o_sd_cd_dat3_dir"),
    i_sd_detected("i_sd_detected"),
    i_sd_protect("i_sd_protect"),
    o_dmreset("o_dmreset"),
    o_prci_pmapinfo("o_prci_pmapinfo"),
    i_prci_pdevcfg("i_prci_pdevcfg"),
    o_prci_apbi("o_prci_apbi"),
    i_prci_apbo("i_prci_apbo"),
    o_ddr_pmapinfo("o_ddr_pmapinfo"),
    i_ddr_pdevcfg("i_ddr_pdevcfg"),
    o_ddr_apbi("o_ddr_apbi"),
    i_ddr_apbo("i_ddr_apbo"),
    o_ddr_xmapinfo("o_ddr_xmapinfo"),
    i_ddr_xdevcfg("i_ddr_xdevcfg"),
    o_ddr_xslvi("o_ddr_xslvi"),
    i_ddr_xslvo("i_ddr_xslvo"),
    bus0_mapinfo("bus0_mapinfo", CFG_BUS0_XSLV_TOTAL),
    aximi("aximi", CFG_BUS0_XMST_TOTAL),
    aximo("aximo", CFG_BUS0_XMST_TOTAL),
    axisi("axisi", CFG_BUS0_XSLV_TOTAL),
    axiso("axiso", CFG_BUS0_XSLV_TOTAL),
    bus1_mapinfo("bus1_mapinfo", CFG_BUS1_PSLV_TOTAL),
    apbi("apbi", CFG_BUS1_PSLV_TOTAL),
    apbo("apbo", CFG_BUS1_PSLV_TOTAL),
    dev_pnp("dev_pnp", SOC_PNP_TOTAL) {

    async_reset_ = async_reset;
    sim_uart_speedup_rate_ = sim_uart_speedup_rate;
    bus0 = 0;
    bus1 = 0;
    rom0 = 0;
    sram0 = 0;
    clint0 = 0;
    plic0 = 0;
    uart1 = 0;
    gpio0 = 0;
    sdctrl0 = 0;
    pnp0 = 0;
    group0 = 0;
    u_cdc_ddr0 = 0;

    bus0 = new axictrl_bus0("bus0", async_reset);
    bus0->i_clk(i_sys_clk);
    bus0->i_nrst(i_sys_nrst);
    bus0->o_cfg(dev_pnp[SOC_PNP_XCTRL0]);
    bus0->i_xmsto(aximo);
    bus0->o_xmsti(aximi);
    bus0->i_xslvo(axiso);
    bus0->o_xslvi(axisi);
    bus0->o_mapinfo(bus0_mapinfo);

    bus1 = new axi2apb_bus1("bus1", async_reset);
    bus1->i_clk(i_sys_clk);
    bus1->i_nrst(i_sys_nrst);
    bus1->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PBRIDGE]);
    bus1->o_cfg(dev_pnp[SOC_PNP_PBRIDGE0]);
    bus1->i_xslvi(axisi[CFG_BUS0_XSLV_PBRIDGE]);
    bus1->o_xslvo(axiso[CFG_BUS0_XSLV_PBRIDGE]);
    bus1->i_apbo(apbo);
    bus1->o_apbi(apbi);
    bus1->o_mapinfo(bus1_mapinfo);

    group0 = new Workgroup("group0", async_reset,
                            CFG_CPU_NUM,
                            CFG_L2CACHE_ENA);
    group0->i_cores_nrst(i_sys_nrst);
    group0->i_dmi_nrst(i_dbg_nrst);
    group0->i_clk(i_sys_clk);
    group0->i_trst(i_jtag_trst);
    group0->i_tck(i_jtag_tck);
    group0->i_tms(i_jtag_tms);
    group0->i_tdi(i_jtag_tdi);
    group0->o_tdo(o_jtag_tdo);
    group0->i_msip(wb_clint_msip);
    group0->i_mtip(wb_clint_mtip);
    group0->i_meip(wb_plic_meip);
    group0->i_seip(wb_plic_seip);
    group0->i_mtimer(wb_clint_mtimer);
    group0->i_acpo(acpo);
    group0->o_acpi(acpi);
    group0->o_xmst_cfg(dev_pnp[SOC_PNP_GROUP0]);
    group0->i_msti(aximi[CFG_BUS0_XMST_GROUP0]);
    group0->o_msto(aximo[CFG_BUS0_XMST_GROUP0]);
    group0->i_dmi_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_DMI]);
    group0->o_dmi_cfg(dev_pnp[SOC_PNP_DMI]);
    group0->i_dmi_apbi(apbi[CFG_BUS1_PSLV_DMI]);
    group0->o_dmi_apbo(apbo[CFG_BUS1_PSLV_DMI]);
    group0->o_dmreset(o_dmreset);

    rom0 = new axi_rom<CFG_BOOTROM_LOG2_SIZE>("rom0", async_reset,
                                              CFG_BOOTROM_FILE_HEX);
    rom0->i_clk(i_sys_clk);
    rom0->i_nrst(i_sys_nrst);
    rom0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_BOOTROM]);
    rom0->o_cfg(dev_pnp[SOC_PNP_BOOTROM]);
    rom0->i_xslvi(axisi[CFG_BUS0_XSLV_BOOTROM]);
    rom0->o_xslvo(axiso[CFG_BUS0_XSLV_BOOTROM]);

    sram0 = new axi_sram<CFG_SRAM_LOG2_SIZE>("sram0", async_reset);
    sram0->i_clk(i_sys_clk);
    sram0->i_nrst(i_sys_nrst);
    sram0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_SRAM]);
    sram0->o_cfg(dev_pnp[SOC_PNP_SRAM]);
    sram0->i_xslvi(axisi[CFG_BUS0_XSLV_SRAM]);
    sram0->o_xslvo(axiso[CFG_BUS0_XSLV_SRAM]);

    clint0 = new clint<CFG_CPU_MAX>("clint0", async_reset);
    clint0->i_clk(i_sys_clk);
    clint0->i_nrst(i_sys_nrst);
    clint0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_CLINT]);
    clint0->o_cfg(dev_pnp[SOC_PNP_CLINT]);
    clint0->i_xslvi(axisi[CFG_BUS0_XSLV_CLINT]);
    clint0->o_xslvo(axiso[CFG_BUS0_XSLV_CLINT]);
    clint0->o_mtimer(wb_clint_mtimer);
    clint0->o_msip(wb_clint_msip);
    clint0->o_mtip(wb_clint_mtip);

    plic0 = new plic<SOC_PLIC_CONTEXT_TOTAL,
                     SOC_PLIC_IRQ_TOTAL>("plic0", async_reset);
    plic0->i_clk(i_sys_clk);
    plic0->i_nrst(i_sys_nrst);
    plic0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PLIC]);
    plic0->o_cfg(dev_pnp[SOC_PNP_PLIC]);
    plic0->i_xslvi(axisi[CFG_BUS0_XSLV_PLIC]);
    plic0->o_xslvo(axiso[CFG_BUS0_XSLV_PLIC]);
    plic0->i_irq_request(wb_ext_irqs);
    plic0->o_ip(wb_plic_xeip);

    u_cdc_ddr0 = new cdc_axi_sync_tech("u_cdc_ddr0");
    u_cdc_ddr0->i_xslv_clk(i_sys_clk);
    u_cdc_ddr0->i_xslv_nrst(i_sys_nrst);
    u_cdc_ddr0->i_xslvi(axisi[CFG_BUS0_XSLV_DDR]);
    u_cdc_ddr0->o_xslvo(axiso[CFG_BUS0_XSLV_DDR]);
    u_cdc_ddr0->i_xmst_clk(i_ddr_clk);
    u_cdc_ddr0->i_xmst_nrst(i_ddr_nrst);
    u_cdc_ddr0->o_xmsto(o_ddr_xslvi);
    u_cdc_ddr0->i_xmsti(i_ddr_xslvo);

    uart1 = new apb_uart<SOC_UART1_LOG2_FIFOSZ>("uart1", async_reset,
                                                sim_uart_speedup_rate);
    uart1->i_clk(i_sys_clk);
    uart1->i_nrst(i_sys_nrst);
    uart1->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_UART1]);
    uart1->o_cfg(dev_pnp[SOC_PNP_UART1]);
    uart1->i_apbi(apbi[CFG_BUS1_PSLV_UART1]);
    uart1->o_apbo(apbo[CFG_BUS1_PSLV_UART1]);
    uart1->i_rd(i_uart1_rd);
    uart1->o_td(o_uart1_td);
    uart1->o_irq(w_irq_uart1);

    gpio0 = new apb_gpio<SOC_GPIO0_WIDTH>("gpio0", async_reset);
    gpio0->i_clk(i_sys_clk);
    gpio0->i_nrst(i_sys_nrst);
    gpio0->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_GPIO]);
    gpio0->o_cfg(dev_pnp[SOC_PNP_GPIO]);
    gpio0->i_apbi(apbi[CFG_BUS1_PSLV_GPIO]);
    gpio0->o_apbo(apbo[CFG_BUS1_PSLV_GPIO]);
    gpio0->i_gpio(i_gpio);
    gpio0->o_gpio_dir(o_gpio_dir);
    gpio0->o_gpio(o_gpio);
    gpio0->o_irq(wb_irq_gpio);

    sdctrl0 = new sdctrl("sdctrl0", async_reset);
    sdctrl0->i_clk(i_sys_clk);
    sdctrl0->i_nrst(i_sys_nrst);
    sdctrl0->i_xmapinfo(bus0_mapinfo[CFG_BUS0_XSLV_SDCTRL_MEM]);
    sdctrl0->o_xcfg(dev_pnp[SOC_PNP_SDCTRL_MEM]);
    sdctrl0->i_xslvi(axisi[CFG_BUS0_XSLV_SDCTRL_MEM]);
    sdctrl0->o_xslvo(axiso[CFG_BUS0_XSLV_SDCTRL_MEM]);
    sdctrl0->i_pmapinfo(bus1_mapinfo[CFG_BUS1_PSLV_SDCTRL_REG]);
    sdctrl0->o_pcfg(dev_pnp[SOC_PNP_SDCTRL_REG]);
    sdctrl0->i_apbi(apbi[CFG_BUS1_PSLV_SDCTRL_REG]);
    sdctrl0->o_apbo(apbo[CFG_BUS1_PSLV_SDCTRL_REG]);
    sdctrl0->o_sclk(o_sd_sclk);
    sdctrl0->i_cmd(i_sd_cmd);
    sdctrl0->o_cmd(o_sd_cmd);
    sdctrl0->o_cmd_dir(o_sd_cmd_dir);
    sdctrl0->i_dat0(i_sd_dat0);
    sdctrl0->o_dat0(o_sd_dat0);
    sdctrl0->o_dat0_dir(o_sd_dat0_dir);
    sdctrl0->i_dat1(i_sd_dat1);
    sdctrl0->o_dat1(o_sd_dat1);
    sdctrl0->o_dat1_dir(o_sd_dat1_dir);
    sdctrl0->i_dat2(i_sd_dat2);
    sdctrl0->o_dat2(o_sd_dat2);
    sdctrl0->o_dat2_dir(o_sd_dat2_dir);
    sdctrl0->i_cd_dat3(i_sd_cd_dat3);
    sdctrl0->o_cd_dat3(o_sd_cd_dat3);
    sdctrl0->o_cd_dat3_dir(o_sd_cd_dat3_dir);
    sdctrl0->i_detected(i_sd_detected);
    sdctrl0->i_protect(i_sd_protect);

    pnp0 = new apb_pnp<SOC_PNP_TOTAL>("pnp0", async_reset,
                                      SOC_HW_ID,
                                      CFG_CPU_NUM,
                                      CFG_L2CACHE_ENA,
                                      SOC_PLIC_IRQ_TOTAL);
    pnp0->i_clk(i_sys_clk);
    pnp0->i_nrst(i_sys_nrst);
    pnp0->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_PNP]);
    pnp0->i_cfg(dev_pnp);
    pnp0->o_cfg(dev_pnp[SOC_PNP_PNP]);
    pnp0->i_apbi(apbi[CFG_BUS1_PSLV_PNP]);
    pnp0->o_apbo(apbo[CFG_BUS1_PSLV_PNP]);
    pnp0->o_irq(w_irq_pnp);

    SC_METHOD(comb);
    sensitive << i_sys_nrst;
    sensitive << i_sys_clk;
    sensitive << i_dbg_nrst;
    sensitive << i_ddr_nrst;
    sensitive << i_ddr_clk;
    sensitive << i_gpio;
    sensitive << i_jtag_trst;
    sensitive << i_jtag_tck;
    sensitive << i_jtag_tms;
    sensitive << i_jtag_tdi;
    sensitive << i_uart1_rd;
    sensitive << i_sd_cmd;
    sensitive << i_sd_dat0;
    sensitive << i_sd_dat1;
    sensitive << i_sd_dat2;
    sensitive << i_sd_cd_dat3;
    sensitive << i_sd_detected;
    sensitive << i_sd_protect;
    sensitive << i_prci_pdevcfg;
    sensitive << i_prci_apbo;
    sensitive << i_ddr_pdevcfg;
    sensitive << i_ddr_apbo;
    sensitive << i_ddr_xdevcfg;
    sensitive << i_ddr_xslvo;
    sensitive << acpo;
    sensitive << acpi;
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << bus0_mapinfo[i];
    }
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << aximi[i];
    }
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << aximo[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << axisi[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << axiso[i];
    }
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << bus1_mapinfo[i];
    }
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << apbi[i];
    }
    for (int i = 0; i < CFG_BUS1_PSLV_TOTAL; i++) {
        sensitive << apbo[i];
    }
    for (int i = 0; i < SOC_PNP_TOTAL; i++) {
        sensitive << dev_pnp[i];
    }
    sensitive << wb_clint_mtimer;
    sensitive << wb_clint_msip;
    sensitive << wb_clint_mtip;
    sensitive << wb_plic_xeip;
    sensitive << wb_plic_meip;
    sensitive << wb_plic_seip;
    sensitive << w_irq_uart1;
    sensitive << wb_irq_gpio;
    sensitive << w_irq_pnp;
    sensitive << wb_ext_irqs;
}

riscv_soc::~riscv_soc() {
    if (bus0) {
        delete bus0;
    }
    if (bus1) {
        delete bus1;
    }
    if (rom0) {
        delete rom0;
    }
    if (sram0) {
        delete sram0;
    }
    if (clint0) {
        delete clint0;
    }
    if (plic0) {
        delete plic0;
    }
    if (uart1) {
        delete uart1;
    }
    if (gpio0) {
        delete gpio0;
    }
    if (sdctrl0) {
        delete sdctrl0;
    }
    if (pnp0) {
        delete pnp0;
    }
    if (group0) {
        delete group0;
    }
    if (u_cdc_ddr0) {
        delete u_cdc_ddr0;
    }
}

void riscv_soc::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_sys_nrst, i_sys_nrst.name());
        sc_trace(o_vcd, i_sys_clk, i_sys_clk.name());
        sc_trace(o_vcd, i_dbg_nrst, i_dbg_nrst.name());
        sc_trace(o_vcd, i_ddr_nrst, i_ddr_nrst.name());
        sc_trace(o_vcd, i_ddr_clk, i_ddr_clk.name());
        sc_trace(o_vcd, i_gpio, i_gpio.name());
        sc_trace(o_vcd, o_gpio, o_gpio.name());
        sc_trace(o_vcd, o_gpio_dir, o_gpio_dir.name());
        sc_trace(o_vcd, i_jtag_trst, i_jtag_trst.name());
        sc_trace(o_vcd, i_jtag_tck, i_jtag_tck.name());
        sc_trace(o_vcd, i_jtag_tms, i_jtag_tms.name());
        sc_trace(o_vcd, i_jtag_tdi, i_jtag_tdi.name());
        sc_trace(o_vcd, o_jtag_tdo, o_jtag_tdo.name());
        sc_trace(o_vcd, o_jtag_vref, o_jtag_vref.name());
        sc_trace(o_vcd, i_uart1_rd, i_uart1_rd.name());
        sc_trace(o_vcd, o_uart1_td, o_uart1_td.name());
        sc_trace(o_vcd, o_sd_sclk, o_sd_sclk.name());
        sc_trace(o_vcd, i_sd_cmd, i_sd_cmd.name());
        sc_trace(o_vcd, o_sd_cmd, o_sd_cmd.name());
        sc_trace(o_vcd, o_sd_cmd_dir, o_sd_cmd_dir.name());
        sc_trace(o_vcd, i_sd_dat0, i_sd_dat0.name());
        sc_trace(o_vcd, o_sd_dat0, o_sd_dat0.name());
        sc_trace(o_vcd, o_sd_dat0_dir, o_sd_dat0_dir.name());
        sc_trace(o_vcd, i_sd_dat1, i_sd_dat1.name());
        sc_trace(o_vcd, o_sd_dat1, o_sd_dat1.name());
        sc_trace(o_vcd, o_sd_dat1_dir, o_sd_dat1_dir.name());
        sc_trace(o_vcd, i_sd_dat2, i_sd_dat2.name());
        sc_trace(o_vcd, o_sd_dat2, o_sd_dat2.name());
        sc_trace(o_vcd, o_sd_dat2_dir, o_sd_dat2_dir.name());
        sc_trace(o_vcd, i_sd_cd_dat3, i_sd_cd_dat3.name());
        sc_trace(o_vcd, o_sd_cd_dat3, o_sd_cd_dat3.name());
        sc_trace(o_vcd, o_sd_cd_dat3_dir, o_sd_cd_dat3_dir.name());
        sc_trace(o_vcd, i_sd_detected, i_sd_detected.name());
        sc_trace(o_vcd, i_sd_protect, i_sd_protect.name());
        sc_trace(o_vcd, o_dmreset, o_dmreset.name());
        sc_trace(o_vcd, o_prci_apbi, o_prci_apbi.name());
        sc_trace(o_vcd, i_prci_apbo, i_prci_apbo.name());
        sc_trace(o_vcd, o_ddr_apbi, o_ddr_apbi.name());
        sc_trace(o_vcd, i_ddr_apbo, i_ddr_apbo.name());
        sc_trace(o_vcd, o_ddr_xslvi, o_ddr_xslvi.name());
        sc_trace(o_vcd, i_ddr_xslvo, i_ddr_xslvo.name());
        sc_trace(o_vcd, acpo, acpo.name());
        sc_trace(o_vcd, acpi, acpi.name());
    }

    if (bus0) {
        bus0->generateVCD(i_vcd, o_vcd);
    }
    if (bus1) {
        bus1->generateVCD(i_vcd, o_vcd);
    }
    if (rom0) {
        rom0->generateVCD(i_vcd, o_vcd);
    }
    if (sram0) {
        sram0->generateVCD(i_vcd, o_vcd);
    }
    if (clint0) {
        clint0->generateVCD(i_vcd, o_vcd);
    }
    if (plic0) {
        plic0->generateVCD(i_vcd, o_vcd);
    }
    if (uart1) {
        uart1->generateVCD(i_vcd, o_vcd);
    }
    if (gpio0) {
        gpio0->generateVCD(i_vcd, o_vcd);
    }
    if (sdctrl0) {
        sdctrl0->generateVCD(i_vcd, o_vcd);
    }
    if (pnp0) {
        pnp0->generateVCD(i_vcd, o_vcd);
    }
    if (group0) {
        group0->generateVCD(i_vcd, o_vcd);
    }
    if (u_cdc_ddr0) {
        u_cdc_ddr0->generateVCD(i_vcd, o_vcd);
    }
}

void riscv_soc::comb() {
    sc_uint<1> v_gnd1;
    sc_biguint<SOC_PLIC_IRQ_TOTAL> vb_ext_irqs;

    v_gnd1 = 0;
    vb_ext_irqs = 0;


    // assign interrupts:
    vb_ext_irqs(22, 0) = 0;
    vb_ext_irqs(((23 + SOC_GPIO0_WIDTH) - 1), 23) = wb_irq_gpio;// FU740: 16 bits, current 12-bits
    vb_ext_irqs[39] = w_irq_uart1;
    vb_ext_irqs(69, 40) = 0;
    vb_ext_irqs[70] = w_irq_pnp;
    vb_ext_irqs((SOC_PLIC_IRQ_TOTAL - 1), 71) = 0;
    wb_ext_irqs = vb_ext_irqs;

    // FU740 implements 5 cores (we implement only 4):
    //     Hart0 - M-mode only (S7 Core RV64IMAC)
    //     Hart1..4 - M+S modes (U74 Cores RV64GC)
    // Hart4 ignored
    wb_plic_meip = (wb_plic_xeip.read()[5],
            wb_plic_xeip.read()[3],
            wb_plic_xeip.read()[1],
            wb_plic_xeip.read()[0]);
    wb_plic_seip = (wb_plic_xeip.read()[6],
            wb_plic_xeip.read()[4],
            wb_plic_xeip.read()[2],
            v_gnd1);

    o_jtag_vref = 1;

    // Nullify emty AXI-slots:
    aximo[CFG_BUS0_XMST_DMA] = axi4_master_out_none;
    acpo = axi4_master_out_none;

    // PRCI:
    o_prci_apbi = apbi[CFG_BUS1_PSLV_PRCI];
    apbo[CFG_BUS1_PSLV_PRCI] = i_prci_apbo;
    dev_pnp[SOC_PNP_PRCI] = i_prci_pdevcfg;

    // DDR:
    o_ddr_xmapinfo = bus0_mapinfo[CFG_BUS0_XSLV_DDR];
    dev_pnp[SOC_PNP_DDR_AXI] = i_ddr_xdevcfg;
    o_ddr_pmapinfo = bus1_mapinfo[CFG_BUS1_PSLV_DDR];
    dev_pnp[SOC_PNP_DDR_APB] = i_ddr_pdevcfg;
    o_ddr_apbi = apbi[CFG_BUS1_PSLV_DDR];
    apbo[CFG_BUS1_PSLV_DDR] = i_ddr_apbo;
}

}  // namespace debugger

