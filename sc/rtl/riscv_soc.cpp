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

riscv_soc::riscv_soc(sc_module_name name)
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
    o_spi_cs("o_spi_cs"),
    o_spi_sclk("o_spi_sclk"),
    o_spi_mosi("o_spi_mosi"),
    i_spi_miso("i_spi_miso"),
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

    apbrdg0 = 0;
    uart1 = 0;
    gpio0 = 0;
    spi0 = 0;
    group0 = 0;

    apbrdg0 = new axi2apb("apbrdg0", async_reset);
    apbrdg0->i_clk(i_sys_clk);
    apbrdg0->i_nrst(i_sys_nrst);
    apbrdg0->i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->o_cfg(dev_pnp[SOC_PNP_PBRIDGE0]);
    apbrdg0->i_xslvi(axisi[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->o_xslvo(axiso[CFG_BUS0_XSLV_PBRIDGE]);
    apbrdg0->i_apbo(apbo);
    apbrdg0->o_apbi(apbi);
    apbrdg0->o_mapinfo(bus1_mapinfo);


    group0 = new Workgroup("group0", async_reset,
                            CFG_CPU_NUM,
                            CFG_ILOG2_NWAYS,
                            CFG_ILOG2_LINES_PER_WAY,
                            CFG_DLOG2_NWAYS,
                            CFG_DLOG2_LINES_PER_WAY,
                            CFG_L2CACHE_ENA,
                            CFG_L2_LOG2_NWAYS,
                            CFG_L2_LOG2_LINES_PER_WAY);
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


    uart1 = new apb_uart<CFG_SOC_UART1_LOG2_FIFOSZ>("uart1", async_reset);
    uart1->i_clk(i_sys_clk);
    uart1->i_nrst(i_sys_nrst);
    uart1->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_UART1]);
    uart1->o_cfg(dev_pnp[SOC_PNP_UART1]);
    uart1->i_apbi(apbi[CFG_BUS1_PSLV_UART1]);
    uart1->o_apbo(apbo[CFG_BUS1_PSLV_UART1]);
    uart1->i_rd(i_uart1_rd);
    uart1->o_td(o_uart1_td);
    uart1->o_irq(w_irq_uart1);


    gpio0 = new apb_gpio<CFG_SOC_GPIO0_WIDTH>("gpio0", async_reset);
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


    spi0 = new apb_spi<CFG_SOC_SPI0_LOG2_FIFOSZ>("spi0", async_reset);
    spi0->i_clk(i_sys_clk);
    spi0->i_nrst(i_sys_nrst);
    spi0->i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_SPI]);
    spi0->o_cfg(dev_pnp[SOC_PNP_SPI]);
    spi0->i_apbi(apbi[CFG_BUS1_PSLV_SPI]);
    spi0->o_apbo(apbo[CFG_BUS1_PSLV_SPI]);
    spi0->o_cs(o_spi_cs);
    spi0->o_sclk(o_spi_sclk);
    spi0->o_mosi(o_spi_mosi);
    spi0->i_miso(i_spi_miso);
    spi0->i_detected(i_sd_detected);
    spi0->i_protect(i_sd_protect);



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
    sensitive << i_spi_miso;
    sensitive << i_sd_detected;
    sensitive << i_sd_protect;
    sensitive << i_prci_apbo;
    sensitive << i_ddr_apbo;
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
    if (apbrdg0) {
        delete apbrdg0;
    }
    if (uart1) {
        delete uart1;
    }
    if (gpio0) {
        delete gpio0;
    }
    if (spi0) {
        delete spi0;
    }
    if (group0) {
        delete group0;
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
        sc_trace(o_vcd, o_spi_cs, o_spi_cs.name());
        sc_trace(o_vcd, o_spi_sclk, o_spi_sclk.name());
        sc_trace(o_vcd, o_spi_mosi, o_spi_mosi.name());
        sc_trace(o_vcd, i_spi_miso, i_spi_miso.name());
        sc_trace(o_vcd, i_sd_detected, i_sd_detected.name());
        sc_trace(o_vcd, i_sd_protect, i_sd_protect.name());
        sc_trace(o_vcd, o_dmreset, o_dmreset.name());
        sc_trace(o_vcd, o_prci_pmapinfo, o_prci_pmapinfo.name());
        sc_trace(o_vcd, i_prci_pdevcfg, i_prci_pdevcfg.name());
        sc_trace(o_vcd, o_prci_apbi, o_prci_apbi.name());
        sc_trace(o_vcd, i_prci_apbo, i_prci_apbo.name());
        sc_trace(o_vcd, o_ddr_pmapinfo, o_ddr_pmapinfo.name());
        sc_trace(o_vcd, i_ddr_pdevcfg, i_ddr_pdevcfg.name());
        sc_trace(o_vcd, o_ddr_apbi, o_ddr_apbi.name());
        sc_trace(o_vcd, i_ddr_apbo, i_ddr_apbo.name());
        sc_trace(o_vcd, o_ddr_xmapinfo, o_ddr_xmapinfo.name());
        sc_trace(o_vcd, i_ddr_xdevcfg, i_ddr_xdevcfg.name());
        sc_trace(o_vcd, o_ddr_xslvi, o_ddr_xslvi.name());
        sc_trace(o_vcd, i_ddr_xslvo, i_ddr_xslvo.name());
    }

    if (apbrdg0) {
        apbrdg0->generateVCD(i_vcd, o_vcd);
    }
    if (uart1) {
        uart1->generateVCD(i_vcd, o_vcd);
    }
    if (gpio0) {
        gpio0->generateVCD(i_vcd, o_vcd);
    }
    if (spi0) {
        spi0->generateVCD(i_vcd, o_vcd);
    }
    if (group0) {
        group0->generateVCD(i_vcd, o_vcd);
    }
}

void riscv_soc::comb() {
    sc_biguint<CFG_PLIC_IRQ_TOTAL> vb_ext_irqs;

    vb_ext_irqs = 0;


    // assign interrupts:
    vb_ext_irqs(22, 0) = 0;
    vb_ext_irqs(((23 + CFG_SOC_GPIO0_WIDTH) - 1), 23) = wb_irq_gpio;// FU740: 16 bits, current 12-bits
    vb_ext_irqs[39] = w_irq_uart1.read();
    vb_ext_irqs(69, 40) = 0;
    vb_ext_irqs[70] = w_irq_pnp.read();
    vb_ext_irqs((CFG_PLIC_IRQ_TOTAL - 1), 71) = 0;
    wb_ext_irqs = vb_ext_irqs;

    o_jtag_vref = 1;

    // Nullify emty AXI-slots:
    aximo[CFG_BUS0_XMST_DMA] = axi4_master_out_none;
    acpo = axi4_master_out_none;
}

}  // namespace debugger

