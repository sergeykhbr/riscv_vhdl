//!
//! Copyright 2021 
//! Autor: Sergey Khabarov, sergeykhbr@gmail.com
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!

//! @brief   SOC Top-level entity declaration.
//! @details This module implements full SOC functionality and all IO signals
//!          are available on FPGA/ASIC IO pins.
module riscv_soc (
  input             i_rst,
  input             i_clk,
  //! GPIO.
  input [11:0]      i_gpio,
  output [11:0]     o_gpio,
  output [11:0]     o_gpio_dir,
  //! JTAG signals:
  input             i_jtag_tck,
  input             i_jtag_trst,
  input             i_jtag_tms,
  input             i_jtag_tdi,
  output            o_jtag_tdo,
  output            o_jtag_vref,
  //! UART1 signals:
  input             i_uart1_rd,
  output            o_uart1_td,
  // DDR signal:
  output [4:0] o_ddr_awid,
  output [47:0] o_ddr_awaddr,
  output [7:0] o_ddr_awlen,
  output [2:0] o_ddr_awsize,
  output [1:0] o_ddr_awburst,
  output o_ddr_awlock,
  output [3:0] o_ddr_awcache,
  output [2:0] o_ddr_awprot,
  output [3:0] o_ddr_awregion,
  output [3:0] o_ddr_awqos,
  output [0:0] o_ddr_awuser,
  output o_ddr_awvalid,
  input i_ddr_awready,
  output [63:0] o_ddr_wdata,
  output [7:0] o_ddr_wstrb,
  output [0:0] o_ddr_wuser,
  output o_ddr_wlast,
  output o_ddr_wvalid,
  input i_ddr_wready,
  output o_ddr_bready,
  input [4:0] i_ddr_bid,
  input [1:0] i_ddr_bresp,
  input [0:0] i_ddr_buser,
  input i_ddr_bvalid,
  output [4:0] o_ddr_arid,
  output [47:0] o_ddr_araddr,
  output [7:0] o_ddr_arlen,
  output [2:0] o_ddr_arsize,
  output [1:0] o_ddr_arburst,
  output o_ddr_arlock,
  output [3:0] o_ddr_arcache,
  output [2:0] o_ddr_arprot,
  output [3:0] o_ddr_arregion,
  output [3:0] o_ddr_arqos,
  output [0:0] o_ddr_aruser,
  output o_ddr_arvalid,
  input i_ddr_arready,
  output o_ddr_rready,
  input [4:0] i_ddr_rid,
  input [63:0] i_ddr_rdata,
  input [1:0] i_ddr_rresp,
  input i_ddr_rlast,
  input [0:0] i_ddr_ruser,
  input i_ddr_rvalid,
  input i_ddr_ui_clk,
  input i_ddr_ui_rst,
  input i_ddr_mmcm_locked,
  input i_ddr_init_calib_complete,
  input [11:0] i_ddr_device_temp,
  input i_ddr_app_sr_active,
  input i_ddr_app_ref_ack,
  input i_ddr_app_zq_ack
);

import config_target_pkg::*;
import types_bus0_pkg::*;
import types_bus1_pkg::*;
import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import workgroup_pkg::*;
import riscv_soc_pkg::*;


logic w_sys_nrst;                                           // System reset of whole system
logic w_dbg_nrst;                                           // Reset workgroup debug interface
logic w_dmreset;                                            // Reset request from workgroup debug interface
logic w_ddr_ui_nrst;
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
axi4_master_out_type ddr_xmsto;
axi4_master_in_type ddr_xmsti;
logic [63:0] wb_clint_mtimer;
logic [CFG_CPU_MAX-1:0] wb_clint_msip;
logic [CFG_CPU_MAX-1:0] wb_clint_mtip;
logic [CFG_PLIC_CONTEXT_TOTAL-1:0] wb_plic_xeip;
logic [CFG_CPU_MAX-1:0] wb_plic_meip;
logic [CFG_CPU_MAX-1:0] wb_plic_seip;
logic w_irq_uart1;
logic [15:0] wb_irq_gpio;
logic w_irq_pnp;
logic [CFG_PLIC_IRQ_TOTAL-1:0] wb_ext_irqs;


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
  axictrl_bus0 #(
    .async_reset(CFG_ASYNC_RESET)
  )
   ctrl0 (
    .i_clk(i_clk),
    .i_nrst(w_sys_nrst),
    .o_cfg(dev_pnp[SOC_PNP_XCTRL0]),
    .i_slvo(axiso),
    .i_msto(aximo),
    .o_slvi(axisi),
    .o_msti(aximi),
    .o_mapinfo(bus0_mapinfo)
  );

assign w_ddr_ui_nrst = ~i_ddr_ui_rst;


/// AXI to APB bridge
axi2apb #(
    .async_reset(async_reset)
) apbrdg0 (
    .i_clk(i_clk),
    .i_nrst(w_sys_nrst),
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
    .ilog2_nways(CFG_ILOG2_NWAYS),
    .ilog2_lines_per_way(CFG_ILOG2_LINES_PER_WAY),
    .dlog2_nways(CFG_DLOG2_NWAYS),
    .dlog2_lines_per_way(CFG_DLOG2_LINES_PER_WAY),
    .l2cache_ena(CFG_L2CACHE_ENA),
    .l2log2_nways(CFG_L2_LOG2_NWAYS),
    .l2log2_lines_per_way(CFG_L2_LOG2_LINES_PER_WAY)
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
    .i_acpo(acpo),
    .o_acpi(acpi),
    .o_xmst_cfg(dev_pnp[SOC_PNP_GROUP0]),
    .i_msti(aximi[CFG_BUS0_XMST_GROUP0]),
    .o_msto(aximo[CFG_BUS0_XMST_GROUP0]),
    .i_dmi_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_DMI]),
    .o_dmi_cfg(dev_pnp[SOC_PNP_DMI]),
    .i_dmi_apbi(apbi[CFG_BUS1_PSLV_DMI]),
    .o_dmi_apbo(apbo[CFG_BUS1_PSLV_DMI]),
    .o_dmreset(w_dmreset)
);

  ////////////////////////////////////
  //! @brief BOOT ROM module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_00010000..0x00000000_0001ffff (64 KB total upto 0x0100_0000 on FU740)
  axi4_rom #(
    .abits(CFG_BOOTROM_LOG2_SIZE),
    .async_reset(CFG_ASYNC_RESET),
    .filename(CFG_BOOTROM_FILE)
  ) boot0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_BOOTROM]),
    .cfg(dev_pnp[SOC_PNP_BOOTROM]),
    .i(axisi[CFG_BUS0_XSLV_BOOTROM]),
    .o(axiso[CFG_BUS0_XSLV_BOOTROM])
  );


  ////////////////////////////////////
  //! Internal SRAM module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_08000000..0x00000000_081fffff (2MB on FU740)
  axi4_sram #(
    .async_reset(CFG_ASYNC_RESET),
    .abits(CFG_SRAM_LOG2_SIZE)
  ) sram0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_SRAM]),
    .cfg(dev_pnp[SOC_PNP_SRAM]),
    .i(axisi[CFG_BUS0_XSLV_SRAM]),
    .o(axiso[CFG_BUS0_XSLV_SRAM])
  );

  ////////////////////////////////////
  //! @brief Core local interrupt controller (CLINT).
  //! @details Map address:
  //!          0x00000000_02000000..0x00000000_02000fff (64 KB total)
  clint #(
    .async_reset(CFG_ASYNC_RESET)
  ) clint0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_CLINT]),
    .o_cfg(dev_pnp[SOC_PNP_CLINT]),
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
    .ctxmax(CFG_PLIC_CONTEXT_TOTAL),
    .irqmax(CFG_PLIC_IRQ_TOTAL)
  ) plic0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .i_mapinfo(bus0_mapinfo[CFG_BUS0_XSLV_PLIC]),
    .o_cfg(dev_pnp[SOC_PNP_PLIC]),
    .i_axi(axisi[CFG_BUS0_XSLV_PLIC]),
    .o_axi(axiso[CFG_BUS0_XSLV_PLIC]),
    .i_irq_request(wb_ext_irqs),  // [0] must be tight to GND
    .o_ip(wb_plic_xeip)
  );
  // FU740 implements 5 cores (we implement only 4):
  //   Hart0 - M-mode only (S7 Core RV64IMAC)
  //   Hart1..4 - M+S modes (U74 Cores RV64GC)
  // Hart4 ignored
  assign wb_plic_meip = {wb_plic_xeip[5], wb_plic_xeip[3], wb_plic_xeip[1], wb_plic_xeip[0]};
  assign wb_plic_seip = {wb_plic_xeip[6], wb_plic_xeip[4], wb_plic_xeip[2], 1'b0};


  ////////////////////////////////////
  //! External DDR module instance with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_80000000..0x00000000_7fffffff (2GB on FU740)
assign dev_pnp[SOC_PNP_DDR] = dev_config_none;

// TODO: move into interconnect
  cdc_axi_sync_tech u_cdc_ddr0 (
    .i_xslv_clk(i_clk),
    .i_xslv_nrst(w_sys_nrst),
    .i_xslvi(axisi[CFG_BUS0_XSLV_DDR]),
    .o_xslvo(axiso[CFG_BUS0_XSLV_DDR]),
    .i_xmst_clk(i_ddr_ui_clk),
    .i_xmst_nrst(w_ddr_ui_nrst),
    .o_xmsto(ddr_xmsto),
    .i_xmsti(ddr_xmsti)
  );

  assign o_ddr_awid = ddr_xmsto.aw_id;
  assign o_ddr_awaddr = ddr_xmsto.aw_bits.addr - bus0_mapinfo[CFG_BUS0_XSLV_DDR].addr_start;
  assign o_ddr_awlen = ddr_xmsto.aw_bits.len;
  assign o_ddr_awsize = ddr_xmsto.aw_bits.size;
  assign o_ddr_awburst = ddr_xmsto.aw_bits.burst;
  assign o_ddr_awlock = ddr_xmsto.aw_bits.lock;
  assign o_ddr_awcache = ddr_xmsto.aw_bits.cache;
  assign o_ddr_awprot = ddr_xmsto.aw_bits.prot;
  assign o_ddr_awregion = ddr_xmsto.aw_bits.region;
  assign o_ddr_awqos = ddr_xmsto.aw_bits.qos;
  assign o_ddr_awuser = ddr_xmsto.aw_user;
  assign o_ddr_awvalid = ddr_xmsto.aw_valid;
  assign ddr_xmsti.aw_ready = i_ddr_awready;
  assign o_ddr_wdata = ddr_xmsto.w_data;
  assign o_ddr_wstrb = ddr_xmsto.w_strb;
  assign o_ddr_wlast = ddr_xmsto.w_last;
  assign o_ddr_wuser = ddr_xmsto.w_user;
  assign o_ddr_wvalid = ddr_xmsto.w_valid;
  assign ddr_xmsti.w_ready = i_ddr_wready;
  assign ddr_xmsti.b_id = i_ddr_bid;
  assign ddr_xmsti.b_resp = i_ddr_bresp;
  assign ddr_xmsti.b_user = i_ddr_buser;
  assign ddr_xmsti.b_valid = i_ddr_bvalid;
  assign o_ddr_bready = ddr_xmsto.b_ready;
  assign o_ddr_arid = ddr_xmsto.ar_id;
  assign o_ddr_araddr = ddr_xmsto.ar_bits.addr - bus0_mapinfo[CFG_BUS0_XSLV_DDR].addr_start;
  assign o_ddr_arlen = ddr_xmsto.ar_bits.len;
  assign o_ddr_arsize = ddr_xmsto.ar_bits.size;
  assign o_ddr_arburst = ddr_xmsto.ar_bits.burst;
  assign o_ddr_arlock = ddr_xmsto.ar_bits.lock;
  assign o_ddr_arcache = ddr_xmsto.ar_bits.cache;
  assign o_ddr_arprot = ddr_xmsto.ar_bits.prot;
  assign o_ddr_arregion = ddr_xmsto.ar_bits.region;
  assign o_ddr_arqos = ddr_xmsto.ar_bits.qos;
  assign o_ddr_aruser = ddr_xmsto.ar_user;
  assign o_ddr_arvalid = ddr_xmsto.ar_valid;
  assign ddr_xmsti.ar_ready = i_ddr_arready;
  assign ddr_xmsti.r_id = i_ddr_rid;
  assign ddr_xmsti.r_data = i_ddr_rdata;
  assign ddr_xmsti.r_resp = i_ddr_rresp;
  assign ddr_xmsti.r_last = i_ddr_rlast;
  assign ddr_xmsti.r_user = i_ddr_ruser;
  assign ddr_xmsti.r_valid = i_ddr_rvalid;
  assign o_ddr_rready = ddr_xmsto.r_ready;

  ////////////////////////////////////
  //! @brief Controller of the LEDs, DIPs and GPIO with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_10060000..0x00000000_10060fff (4 KB total)
assign axiso[CFG_BUS0_XSLV_GPIO] = axi4_slave_out_none;

apb_uart #(
    .async_reset(async_reset),
    .log2_fifosz(CFG_SOC_UART1_LOG2_FIFOSZ)
) uart1 (
    .i_clk(i_clk),
    .i_nrst(w_sys_nrst),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_UART1]),
    .o_cfg(dev_pnp[SOC_PNP_UART1]),
    .i_apbi(apbi[CFG_BUS1_PSLV_UART1]),
    .o_apbo(apbo[CFG_BUS1_PSLV_UART1]),
    .i_rd(i_uart1_rd),
    .o_td(o_uart1_td),
    .o_irq(w_irq_uart1)
);

  ////////////////////////////////////
  //! @brief Controller of the LEDs, DIPs and GPIO with the AXI4 interface.
  //! @details Map address:
  //!          0x00000000_10060000..0x00000000_10060fff (4 KB total)
  apb_gpio  #(
    .async_reset(CFG_ASYNC_RESET),
    .width(12)
  ) gpio0 (
    .clk(i_clk),
    .nrst(w_sys_nrst),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_GPIO]),
    .cfg(dev_pnp[SOC_PNP_GPIO]),
    .i(apbi[CFG_BUS1_PSLV_GPIO]),
    .o(apbo[CFG_BUS1_PSLV_GPIO]),
    .i_gpio(i_gpio),
    .o_gpio(o_gpio),
    .o_gpio_dir(o_gpio_dir),
    .o_irq(wb_irq_gpio[11:0])
  );
  assign wb_irq_gpio[15:12] = '0;


  //! @brief Plug'n'Play controller of the current configuration with the
  //!        AXI4 interface.
  //! @details Map address:
  //!          0x00000000_100ff000..0x00000000_100fffff (4 KB total)
assign axiso[CFG_BUS0_XSLV_PNP] = axi4_slave_out_none;

  apb_pnp #(
    .async_reset(CFG_ASYNC_RESET),
    .cfg_slots(SOC_PNP_TOTAL),
    .hw_id(CFG_HW_ID),
    .cpu_max(CFG_CPU_NUM[3:0]), //CFG_CPU_MAX[3:0]),
    .l2cache_ena(CFG_L2CACHE_ENA),
    .plic_irq_max(CFG_PLIC_IRQ_TOTAL[7:0])
  ) pnp0 (
    .sys_clk(i_clk),
    .nrst(w_sys_nrst),
    .ddr_init_done(i_ddr_init_calib_complete),
    .i_mapinfo(bus1_mapinfo[CFG_BUS1_PSLV_PNP]),
    .i_cfg(dev_pnp),
    .o_cfg(dev_pnp[SOC_PNP_PNP]),
    .i(apbi[CFG_BUS1_PSLV_PNP]),
    .o(apbo[CFG_BUS1_PSLV_PNP]),
    .o_irq(w_irq_pnp)
  );


always_comb
begin: comb_proc
    logic [CFG_PLIC_IRQ_TOTAL-1:0] vb_ext_irqs;

    vb_ext_irqs = '0;


    // assign interrupts:
    vb_ext_irqs[22: 0] = '0;
    vb_ext_irqs[38: 23] = wb_irq_gpio;                      // FU740: 16 bits, current 12-bits
    vb_ext_irqs[39] = w_irq_uart1;
    vb_ext_irqs[69: 40] = '0;
    vb_ext_irqs[70] = w_irq_pnp;
    vb_ext_irqs[(CFG_PLIC_IRQ_TOTAL - 1): 71] = '0;
    wb_ext_irqs = vb_ext_irqs;

//    o_jtag_vref = 1'b1;

    // Nullify emty AXI-slots:
    aximo[CFG_BUS0_XMST_DMA] = axi4_master_out_none;
    acpo = axi4_master_out_none;
end: comb_proc
assign o_jtag_vref = 1'b1;


endmodule
