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

module riscv_soc(
    input logic i_rst,                                      // System reset active HIGH
    input logic i_clk,                                      // CPU clock
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
    output logic o_uart1_td
);

import config_target_pkg::*;
import types_amba_pkg::*;
import types_bus0_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import workgroup_pkg::*;
import riscv_soc_pkg::*;

logic w_sys_nrst;                                           // System reset of whole system
logic w_dbg_nrst;                                           // Reset workgroup debug interface
logic w_dmreset;                                            // Reset request from workgroup debug interface

  axi4_l1_out_vector coreo;
  axi4_l1_in_vector corei;
axi4_master_out_type acpo;
axi4_master_in_type acpi;
apb_in_type apb_dmi_i;
apb_out_type apb_dmi_o;

  //! Arbiter is switching only slaves output signal, data from noc
  //! is connected to all slaves and to the arbiter itself.
bus0_xmst_in_vector aximi;
bus0_xmst_out_vector aximo;
bus0_xslv_in_vector axisi;
bus0_xslv_out_vector axiso;
bus0_xslv_cfg_vector slv_cfg;
bus0_xmst_cfg_vector mst_cfg;
logic [63:0] wb_clint_mtimer;
logic [CFG_CPU_MAX-1:0] wb_clint_msip;
logic [CFG_CPU_MAX-1:0] wb_clint_mtip;
  logic [CFG_PLIC_CONTEXT_TOTAL-1:0] wb_plic_ip;
logic [CFG_CPU_MAX-1:0] wb_plic_meip;
logic [CFG_CPU_MAX-1:0] wb_plic_seip;

  logic [CFG_BUS0_XMST_TOTAL-1:0] wb_bus_util_w;
  logic [CFG_BUS0_XMST_TOTAL-1:0] wb_bus_util_r;

  logic [15:0] wb_irq_gpio;
  logic w_irq_uart0;
  logic w_irq_pnp;
  logic [CFG_PLIC_IRQ_TOTAL-1:0] irq_pins;

  // Nullify emty AXI-slots:
  assign mst_cfg[CFG_BUS0_XMST_DMA] = axi4_master_config_none;
  assign aximo[CFG_BUS0_XMST_DMA] = axi4_master_out_none;
  assign acpo = axi4_master_out_none;

  assign o_jtag_vref = 1'b1;


  // assign interrupts:
  assign irq_pins[22:0] = '0;
  assign irq_pins[38:23] = wb_irq_gpio;  // FU740: 16 bits, current 12-bits
  assign irq_pins[39] = w_irq_uart0;
  assign irq_pins[69:40] = '0;
  assign irq_pins[70] = w_irq_pnp;
  assign irq_pins[CFG_PLIC_IRQ_TOTAL-1:71] = '0;


  ////////////////////////////////////
  //! @brief System Reset device instance.
  reset_global rst0 (
    .i_clk,
    .i_pwrreset(i_rst),   // external button reset
    .i_dmreset(w_dmreset),    // reset from DMI (debug) interface.
    .o_sys_nrst(w_sys_nrst),   // reset whole system
    .o_dbg_nrst(w_dbg_nrst)   // reset dmi interface
  );

  //! @brief AXI4 controller.
  `ifndef WF_AXI_IC
  axictrl_bus0 #(
    .async_reset(CFG_ASYNC_RESET)
  )
  `else //WF_AXI_IC
  wf_axi_ic
  `endif //WF_AXI_IC
   ctrl0 (
    .i_clk(i_clk),
    .i_nrst(w_sys_nrst),
    .i_slvcfg(slv_cfg),
    .i_slvo(axiso),
    .i_msto(aximo),
    .o_slvi(axisi),
    .o_msti(aximi),
    .o_bus_util_w(wb_bus_util_w), // Bus write access utilization per master statistic
    .o_bus_util_r(wb_bus_util_r)  // Bus read access utilization per master statistic
  );

Workgroup #(
    .async_reset(async_reset),
    .cpu_num(CFG_CPU_NUM),
    .l2cache_ena(CFG_L2CACHE_ENA)
) group0 (
    .i_cores_nrst(w_sys_nrst),
    .i_dmi_nrst(w_dbg_nrst),
    .i_clk(i_clk),
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
    .o_xcfg(mst_cfg[CFG_BUS0_XMST_CPU0]),
    .i_acpo(acpo),
    .o_acpi(acpi),
    .i_msti(aximi[CFG_BUS0_XMST_CPU0]),
    .o_msto(aximo[CFG_BUS0_XMST_CPU0]),
    .i_dmi_apbi(apb_dmi_i),
    .o_dmi_apbo(apb_dmi_o),
    .o_dmreset(w_dmreset)
);
  assign apb_dmi_i = apb_in_none;

  ////////////////////////////////////
  //! @brief BOOT ROM module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_00010000..0x00000000_0001ffff (64 KB total upto 0x0100_0000 on FU740)
  axi4_rom #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h0000000000010),
    .xmask(64'hffffffffffff0),
    .sim_hexfile(CFG_SIM_BOOTROM_HEX)
  ) boot0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .cfg(slv_cfg[CFG_BUS0_XSLV_BOOTROM]),
    .i(axisi[CFG_BUS0_XSLV_BOOTROM]),
    .o(axiso[CFG_BUS0_XSLV_BOOTROM])
  );

  ////////////////////////////////////
  //! Internal SRAM module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_08000000..0x00000000_081fffff (2MB on FU740)
  axi4_sram #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h0000000008000),
//    .xmask(64'hffffffffffe00),            // 2MB KB mask
//    .abits((10 + $clog2(2048))),        // 2 MB address
    .xmask(64'hfffffffffffc0),            // 256KB KB mask
    .abits((10 + $clog2(256))),        // 256KB address
    .init_file(CFG_SIM_FWIMAGE_HEX)     // Initialization will work only in RTL simulation
  ) sram0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .cfg(slv_cfg[CFG_BUS0_XSLV_SRAM]),
    .i(axisi[CFG_BUS0_XSLV_SRAM]),
    .o(axiso[CFG_BUS0_XSLV_SRAM])
  );


  ////////////////////////////////////
  //! External DDR module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_80000000..0x00000000_7fffffff (2GB on FU740)
  //axi4_sram #(
  //  .async_reset(CFG_ASYNC_RESET),
  //  .xaddr(64'h0000000080000),
  //  .xmask(64'hffffffffe0000),           // 512MB mask
  //  .abits((10 + $clog2(512*1024))),      // 512MB address
  //  .init_file(CFG_SIM_DDR_INIT_HEX)     // Initialization will work only in RTL simulation
  //) ddr0 (
  //  .clk(i_clk),
  //  .nrst(w_sys_nrst),
  //  .cfg(slv_cfg[CFG_BUS0_XSLV_DDR]),
  //  .i(axisi[CFG_BUS0_XSLV_DDR]),
  //  .o(axiso[CFG_BUS0_XSLV_DDR])
  //);
  assign slv_cfg[CFG_BUS0_XSLV_DDR] = axi4_slave_config_none;
  assign axiso[CFG_BUS0_XSLV_DDR] = axi4_slave_out_none;

  ////////////////////////////////////
  //! @brief Controller of the LEDs, DIPs and GPIO with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_10060000..0x00000000_10060fff (4 KB total)
  axi4_gpio  #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h0000000010060),
    .xmask(64'hfffffffffffff),
    .width(12)
  ) gpio0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .cfg(slv_cfg[CFG_BUS0_XSLV_GPIO]),
    .i(axisi[CFG_BUS0_XSLV_GPIO]),
    .o(axiso[CFG_BUS0_XSLV_GPIO]),
    .i_gpio(i_gpio),
    .o_gpio(o_gpio),
    .o_gpio_dir(o_gpio_dir),
    .o_irq(wb_irq_gpio[11:0])
  );
  assign wb_irq_gpio[15:12] = '0;

  //! @brief UART Controller with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_10010000..0x00000000_10010fff (4 KB total)
  axi4_uart #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h0000000010010),
    .xmask(64'hFFFFFFFFFFFFF),
    .fifosz(CFG_UART0_FIFO_SZ)
  ) uart0 (
    .nrst(w_sys_nrst),
    .clk(i_clk),
    .i_rd(i_uart1_rd),
    .o_td(o_uart1_td),
    .cfg(slv_cfg[CFG_BUS0_XSLV_UART0]),
    .i_axi(axisi[CFG_BUS0_XSLV_UART0]),
    .o_axi(axiso[CFG_BUS0_XSLV_UART0]),
    .o_irq(w_irq_uart0)
  );

  ////////////////////////////////////
  //! @brief Core local interrupt controller (CLINT).
  //! @details Map address:
  //!          0x00000000_02000000..0x00000000_02000fff (64 KB total)
  clint #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h0000000002000),
    .xmask(64'hFFFFFFFFFFFF0)
  ) clint0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .o_cfg(slv_cfg[CFG_BUS0_XSLV_CLINT]),
    .i_axi(axisi[CFG_BUS0_XSLV_CLINT]),
    .o_axi(axiso[CFG_BUS0_XSLV_CLINT]),
    .o_mtimer(wb_clint_mtimer),
    .o_msip(wb_clint_msip),
    .o_mtip(wb_clint_mtip)
  );


  ////////////////////////////////////
  //! @brief External interrupt controller (PLIC).
  //! @details Map address:
  //!          0x00000000_0C000000..0x00000000_0fffffff (64 MB total)
  plic #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h000000000C000),
    .xmask(64'hFFFFFFFFFC000),
    .ctxmax(CFG_PLIC_CONTEXT_TOTAL),
    .irqmax(CFG_PLIC_IRQ_TOTAL)
  ) plic0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .o_cfg(slv_cfg[CFG_BUS0_XSLV_PLIC]),
    .i_axi(axisi[CFG_BUS0_XSLV_PLIC]),
    .o_axi(axiso[CFG_BUS0_XSLV_PLIC]),
    .i_irq_request(irq_pins),  // [0] must be tight to GND
    .o_ip(wb_plic_ip)
  );
  // FU740 implements 5 cores (we implement only 4):
  //   Hart0 - M-mode only (S7 Core RV64IMAC)
  //   Hart1..4 - M+S modes (U74 Cores RV64GC)
  // Hart4 ignored
  assign wb_plic_meip = {wb_plic_ip[5], wb_plic_ip[3], wb_plic_ip[1], wb_plic_ip[0]};
  assign wb_plic_seip = {wb_plic_ip[6], wb_plic_ip[4], wb_plic_ip[2], 1'b0};

  //! @brief Plug'n'Play controller of the current configuration with the
  //!        AXI4 interface.
  //! @details Map address:
  //!          0x00000000_100ff000..0x00000000_100fffff (4 KB total)
  axi4_pnp #(
    .async_reset(CFG_ASYNC_RESET),
    .xaddr(64'h00000000100ff),
    .xmask(64'hfffffffffffff),
    .hw_id(CFG_HW_ID),
    .cpu_max(CFG_CPU_MAX[3:0]),
    .l2cache_ena(CFG_L2CACHE_ENA),
    .plic_irq_max(CFG_PLIC_IRQ_TOTAL[7:0])
  ) pnp0 (
    .sys_clk(i_clk),
    .nrst(w_sys_nrst),
    .mstcfg(mst_cfg),
    .slvcfg(slv_cfg),
    .cfg(slv_cfg[CFG_BUS0_XSLV_PNP]),
    .i(axisi[CFG_BUS0_XSLV_PNP]),
    .o(axiso[CFG_BUS0_XSLV_PNP]),
    .o_irq(w_irq_pnp)
  );

endmodule: riscv_soc
