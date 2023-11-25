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

`timescale 1ns/10ps

module riscv_soc #(
    parameter bit async_reset = 1'b0,
    parameter int sim_uart_speedup_rate = 0                 // simulation UART speed-up: 0=no speed up, 1=2x, 2=4x, etc
)
(
    input logic i_sys_nrst,                                 // Power-on system reset active LOW
    input logic i_sys_clk,                                  // System/Bus clock
    input logic i_dbg_nrst,                                 // Reset from Debug interface (DMI). Reset everything except DMI
    input logic i_ddr_nrst,                                 // DDR related logic reset (AXI clock transformator)
    input logic i_ddr_clk,                                  // DDR memoru clock
    // GPIO signals:
    input logic [11:0] i_gpio,
    output logic [11:0] o_gpio,
    output logic [11:0] o_gpio_dir,
    // JTAG signals:
    input logic i_jtag_trst,
    input logic i_jtag_tck,
    input logic i_jtag_tms,
    input logic i_jtag_tdi,
    output logic o_jtag_tdo,
    output logic o_jtag_vref,
    // UART1 signals
    input logic i_uart1_rd,
    output logic o_uart1_td,
    // SD-card signals:
    output logic o_sd_sclk,                                 // Clock up to 50 MHz
    input logic i_sd_cmd,                                   // Command response;
    output logic o_sd_cmd,                                  // Command request; DO in SPI mode
    output logic o_sd_cmd_dir,                              // Direction bit: 1=input; 0=output
    input logic i_sd_dat0,                                  // Data Line[0] input; DI in SPI mode
    output logic o_sd_dat0,                                 // Data Line[0] output
    output logic o_sd_dat0_dir,                             // Direction bit: 1=input; 0=output
    input logic i_sd_dat1,                                  // Data Line[1] input
    output logic o_sd_dat1,                                 // Data Line[1] output
    output logic o_sd_dat1_dir,                             // Direction bit: 1=input; 0=output
    input logic i_sd_dat2,                                  // Data Line[2] input
    output logic o_sd_dat2,                                 // Data Line[2] output
    output logic o_sd_dat2_dir,                             // Direction bit: 1=input; 0=output
    input logic i_sd_cd_dat3,                               // Card Detect / Data Line[3] input
    output logic o_sd_cd_dat3,                              // Card Detect / Data Line[3] output; CS output in SPI mode
    output logic o_sd_cd_dat3_dir,                          // Direction bit: 1=input; 0=output
    input logic i_sd_detected,                              // SD-card detected
    input logic i_sd_protect,                               // SD-card write protect
    // PLL and Reset interfaces:
    output logic o_dmreset,                                 // Debug reset request. Everything except DMI.
    output types_amba_pkg::mapinfo_type o_prci_pmapinfo,    // PRCI mapping information
    input types_pnp_pkg::dev_config_type i_prci_pdevcfg,    // PRCI device descriptor
    output types_amba_pkg::apb_in_type o_prci_apbi,         // APB: PLL and Reset configuration interface
    input types_amba_pkg::apb_out_type i_prci_apbo,         // APB: PLL and Reset configuration interface
    // DDR interfaces:
    output types_amba_pkg::mapinfo_type o_ddr_pmapinfo,     // DDR configuration mapping information
    input types_pnp_pkg::dev_config_type i_ddr_pdevcfg,     // DDR configuration device descriptor
    output types_amba_pkg::apb_in_type o_ddr_apbi,          // APB: DDR configuration interface
    input types_amba_pkg::apb_out_type i_ddr_apbo,          // APB: DDR configuration interface
    output types_amba_pkg::mapinfo_type o_ddr_xmapinfo,     // DDR memory bank mapping information
    input types_pnp_pkg::dev_config_type i_ddr_xdevcfg,     // DDR memory bank descriptor
    output types_amba_pkg::axi4_slave_in_type o_ddr_xslvi,  // AXI DDR memory interface
    input types_amba_pkg::axi4_slave_out_type i_ddr_xslvo   // AXI DDR memory interface
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
import types_bus0_pkg::*;
import types_bus1_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import sdctrl_cfg_pkg::*;
import target_cfg_pkg::*;
import riscv_soc_pkg::*;

axi4_master_out_type acpo;
axi4_master_in_type acpi;
bus0_mapinfo_vector bus0_mapinfo;
bus0_xmst_in_vector aximi;
bus0_xmst_out_vector aximo;
bus0_xslv_in_vector axisi;
bus0_xslv_out_vector axiso;
bus1_mapinfo_vector bus1_mapinfo;
bus1_apb_in_vector apbi;
bus1_apb_out_vector apbo;
soc_pnp_vector dev_pnp;
logic [63:0] wb_clint_mtimer;
logic [CFG_CPU_MAX-1:0] wb_clint_msip;
logic [CFG_CPU_MAX-1:0] wb_clint_mtip;
logic [SOC_PLIC_CONTEXT_TOTAL-1:0] wb_plic_xeip;
logic [CFG_CPU_MAX-1:0] wb_plic_meip;
logic [CFG_CPU_MAX-1:0] wb_plic_seip;
logic w_irq_uart1;
logic [SOC_GPIO0_WIDTH-1:0] wb_irq_gpio;
logic w_irq_pnp;
logic [SOC_PLIC_IRQ_TOTAL-1:0] wb_ext_irqs;

axictrl_bus0 #(
    .async_reset(async_reset)
) bus0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .o_cfg(dev_pnp[SOC_PNP_XCTRL0]),
    .i_xmsto(aximo),
    .o_xmsti(aximi),
    .i_xslvo(axiso),
    .o_xslvi(axisi),
    .o_mapinfo(bus0_mapinfo)
);


axi2apb_bus1 #(
    .async_reset(async_reset)
) bus1 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PBRIDGE]),
    .o_cfg(dev_pnp[SOC_PNP_PBRIDGE0]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_PBRIDGE]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_PBRIDGE]),
    .i_apbo(apbo),
    .o_apbi(apbi),
    .o_mapinfo(bus1_mapinfo)
);


Workgroup #(
    .async_reset(async_reset),
    .cpu_num(CFG_CPU_NUM),
    .l2cache_ena(CFG_L2CACHE_ENA)
) group0 (
    .i_cores_nrst(i_sys_nrst),
    .i_dmi_nrst(i_dbg_nrst),
    .i_clk(i_sys_clk),
    .i_trst(i_jtag_trst),
    .i_tck(i_jtag_tck),
    .i_tms(i_jtag_tms),
    .i_tdi(i_jtag_tdi),
    .o_tdo(o_jtag_tdo),
    .i_msip(wb_clint_msip),
    .i_mtip(wb_clint_mtip),
    .i_meip(wb_plic_meip),
    .i_seip(wb_plic_seip),
    .i_mtimer(wb_clint_mtimer),
    .i_acpo(acpo),
    .o_acpi(acpi),
    .o_xmst_cfg(dev_pnp[SOC_PNP_GROUP0]),
    .i_msti(aximi[CFG_BUS0_XMST_GROUP0]),
    .o_msto(aximo[CFG_BUS0_XMST_GROUP0]),
    .i_dmi_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_DMI]),
    .o_dmi_cfg(dev_pnp[SOC_PNP_DMI]),
    .i_dmi_apbi(apbi[CFG_BUS1_PSLV_DMI]),
    .o_dmi_apbo(apbo[CFG_BUS1_PSLV_DMI]),
    .o_dmreset(o_dmreset)
);


axi_rom #(
    .async_reset(async_reset),
    .abits(CFG_BOOTROM_LOG2_SIZE),
    .filename(CFG_BOOTROM_FILE_HEX)
) rom0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_BOOTROM]),
    .o_cfg(dev_pnp[SOC_PNP_BOOTROM]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_BOOTROM]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_BOOTROM])
);


axi_sram #(
    .async_reset(async_reset),
    .abits(CFG_SRAM_LOG2_SIZE)
) sram0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_SRAM]),
    .o_cfg(dev_pnp[SOC_PNP_SRAM]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_SRAM]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_SRAM])
);


clint #(
    .async_reset(async_reset),
    .cpu_total(CFG_CPU_MAX)
) clint0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_CLINT]),
    .o_cfg(dev_pnp[SOC_PNP_CLINT]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_CLINT]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_CLINT]),
    .o_mtimer(wb_clint_mtimer),
    .o_msip(wb_clint_msip),
    .o_mtip(wb_clint_mtip)
);


plic #(
    .async_reset(async_reset),
    .ctxmax(SOC_PLIC_CONTEXT_TOTAL),
    .irqmax(SOC_PLIC_IRQ_TOTAL)
) plic0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PLIC]),
    .o_cfg(dev_pnp[SOC_PNP_PLIC]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_PLIC]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_PLIC]),
    .i_irq_request(wb_ext_irqs),
    .o_ip(wb_plic_xeip)
);


cdc_axi_sync_tech u_cdc_ddr0 (
    .i_xslv_clk(i_sys_clk),
    .i_xslv_nrst(i_sys_nrst),
    .i_xslvi(axisi[CFG_BUS0_XSLV_DDR]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_DDR]),
    .i_xmst_clk(i_ddr_clk),
    .i_xmst_nrst(i_ddr_nrst),
    .o_xmsto(o_ddr_xslvi),
    .i_xmsti(i_ddr_xslvo)
);


apb_uart #(
    .async_reset(async_reset),
    .log2_fifosz(SOC_UART1_LOG2_FIFOSZ),
    .sim_speedup_rate(sim_uart_speedup_rate)
) uart1 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_UART1]),
    .o_cfg(dev_pnp[SOC_PNP_UART1]),
    .i_apbi(apbi[CFG_BUS1_PSLV_UART1]),
    .o_apbo(apbo[CFG_BUS1_PSLV_UART1]),
    .i_rd(i_uart1_rd),
    .o_td(o_uart1_td),
    .o_irq(w_irq_uart1)
);


apb_gpio #(
    .async_reset(async_reset),
    .width(SOC_GPIO0_WIDTH)
) gpio0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_GPIO]),
    .o_cfg(dev_pnp[SOC_PNP_GPIO]),
    .i_apbi(apbi[CFG_BUS1_PSLV_GPIO]),
    .o_apbo(apbo[CFG_BUS1_PSLV_GPIO]),
    .i_gpio(i_gpio),
    .o_gpio_dir(o_gpio_dir),
    .o_gpio(o_gpio),
    .o_irq(wb_irq_gpio)
);


sdctrl #(
    .async_reset(async_reset)
) sdctrl0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_xmapinfo(bus0_mapinfo[CFG_BUS0_XSLV_SDCTRL_MEM]),
    .o_xcfg(dev_pnp[SOC_PNP_SDCTRL_MEM]),
    .i_xslvi(axisi[CFG_BUS0_XSLV_SDCTRL_MEM]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_SDCTRL_MEM]),
    .i_pmapinfo(bus1_mapinfo[CFG_BUS1_PSLV_SDCTRL_REG]),
    .o_pcfg(dev_pnp[SOC_PNP_SDCTRL_REG]),
    .i_apbi(apbi[CFG_BUS1_PSLV_SDCTRL_REG]),
    .o_apbo(apbo[CFG_BUS1_PSLV_SDCTRL_REG]),
    .o_sclk(o_sd_sclk),
    .i_cmd(i_sd_cmd),
    .o_cmd(o_sd_cmd),
    .o_cmd_dir(o_sd_cmd_dir),
    .i_dat0(i_sd_dat0),
    .o_dat0(o_sd_dat0),
    .o_dat0_dir(o_sd_dat0_dir),
    .i_dat1(i_sd_dat1),
    .o_dat1(o_sd_dat1),
    .o_dat1_dir(o_sd_dat1_dir),
    .i_dat2(i_sd_dat2),
    .o_dat2(o_sd_dat2),
    .o_dat2_dir(o_sd_dat2_dir),
    .i_cd_dat3(i_sd_cd_dat3),
    .o_cd_dat3(o_sd_cd_dat3),
    .o_cd_dat3_dir(o_sd_cd_dat3_dir),
    .i_detected(i_sd_detected),
    .i_protect(i_sd_protect)
);


apb_pnp #(
    .async_reset(async_reset),
    .cfg_slots(SOC_PNP_TOTAL),
    .hwid(SOC_HW_ID),
    .cpu_max(CFG_CPU_NUM),
    .l2cache_ena(CFG_L2CACHE_ENA),
    .plic_irq_max(SOC_PLIC_IRQ_TOTAL)
) pnp0 (
    .i_clk(i_sys_clk),
    .i_nrst(i_sys_nrst),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_PNP]),
    .i_cfg(dev_pnp),
    .o_cfg(dev_pnp[SOC_PNP_PNP]),
    .i_apbi(apbi[CFG_BUS1_PSLV_PNP]),
    .o_apbo(apbo[CFG_BUS1_PSLV_PNP]),
    .o_irq(w_irq_pnp)
);


always_comb
begin: comb_proc
    logic v_gnd1;                                           // 1
    logic [SOC_PLIC_IRQ_TOTAL-1:0] vb_ext_irqs;

    v_gnd1 = 1'b0;
    vb_ext_irqs = '0;


    // assign interrupts:
    vb_ext_irqs[22: 0] = '0;
    vb_ext_irqs[((23 + SOC_GPIO0_WIDTH) - 1): 23] = wb_irq_gpio;// FU740: 16 bits, current 12-bits
    vb_ext_irqs[39] = w_irq_uart1;
    vb_ext_irqs[69: 40] = '0;
    vb_ext_irqs[70] = w_irq_pnp;
    vb_ext_irqs[(SOC_PLIC_IRQ_TOTAL - 1): 71] = '0;
    wb_ext_irqs = vb_ext_irqs;

    // FU740 implements 5 cores (we implement only 4):
    //     Hart0 - M-mode only (S7 Core RV64IMAC)
    //     Hart1..4 - M+S modes (U74 Cores RV64GC)
    // Hart4 ignored
    wb_plic_meip = {wb_plic_xeip[5],
            wb_plic_xeip[3],
            wb_plic_xeip[1],
            wb_plic_xeip[0]};
    wb_plic_seip = {wb_plic_xeip[6],
            wb_plic_xeip[4],
            wb_plic_xeip[2],
            v_gnd1};

    o_jtag_vref = 1'b1;

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
end: comb_proc

endmodule: riscv_soc
