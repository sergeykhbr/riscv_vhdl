//!
//! Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
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

module ddr_kc705
#(
    parameter int async_reset = 0,
    parameter SYSCLK_TYPE           = "DIFFERENTIAL",
    parameter SIM_BYPASS_INIT_CAL   = "OFF",
    parameter SIMULATION            = "FALSE"
)
(
    input i_apb_nrst,
    input i_apb_clk,
    input i_xslv_nrst,
    input i_xslv_clk,
    // AXI memory access (ddr clock)
    input types_amba_pkg::mapinfo_type i_xmapinfo,
    output types_amba_pkg::dev_config_type o_xcfg,
    input types_amba_pkg::axi4_slave_in_type i_xslvi,
    output types_amba_pkg::axi4_slave_out_type o_xslvo,
    // APB control interface (sys clock):
    input types_amba_pkg::mapinfo_type i_pmapinfo,
    output types_amba_pkg::dev_config_type o_pcfg,
    input types_amba_pkg::apb_in_type i_apbi,
    output types_amba_pkg::apb_out_type o_apbo,
    // to SOC:
    output o_ui_nrst,  // xilinx generte ddr clock inside ddr controller
    output o_ui_clk,  // xilinx generte ddr clock inside ddr controller
    // DDR signals:
    output o_ddr3_reset_n,
    output [0:0] o_ddr3_ck_n,
    output [0:0] o_ddr3_ck_p,
    output [0:0] o_ddr3_cke,
    output [0:0] o_ddr3_cs_n,
    output o_ddr3_ras_n,
    output o_ddr3_cas_n,
    output o_ddr3_we_n,
    output [7:0] o_ddr3_dm,
    output [2:0] o_ddr3_ba,
    output [13:0] o_ddr3_addr,
    inout [63:0] io_ddr3_dq,
    inout [7:0] io_ddr3_dqs_n,
    inout [7:0] io_ddr3_dqs_p,
    output [0:0] o_ddr3_odt,
    output logic o_init_calib_done
);

import types_amba_pkg::*;

bit w_ddr_mmcm_locked;
bit w_ddr_app_sr_active;
bit w_ddr_app_ref_ack;
bit w_ddr_app_zq_ack;
bit w_ddr_init_calib_complete;
bit [11:0] wb_ddr_device_temp;
bit w_ui_rst;

  apb_ddr #(
    .async_reset(async_reset)
  ) apb0 (
    .i_clk(i_apb_clk),
    .i_nrst(i_apb_nrst),
    .i_mapinfo(i_pmapinfo),
    .o_cfg(o_pcfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .i_pll_locked(w_ddr_mmcm_locked),
    .i_init_calib_done(w_ddr_init_calib_complete),
    .i_device_temp(wb_ddr_device_temp),
    .i_sr_active(w_ddr_app_sr_active),
    .i_ref_ack(w_ddr_app_ref_ack),
    .i_zq_ack(w_ddr_app_zq_ack)
  );

  assign o_ui_nrst = ~w_ui_rst;
  assign o_init_calib_done = w_ddr_init_calib_complete;


  mig_ddr3 #(
    .SYSCLK_TYPE(SYSCLK_TYPE), // "NO_BUFFER,"DIFFERENTIAL"
    .SIM_BYPASS_INIT_CAL(SIM_BYPASS_INIT_CAL),  // "FAST"-for simulation true; "OFF"
    .SIMULATION(SIMULATION)
  ) mig0 (
    .ddr3_dq(io_ddr3_dq),
    .ddr3_dqs_n(io_ddr3_dqs_n),
    .ddr3_dqs_p(io_ddr3_dqs_p),
    .ddr3_addr(o_ddr3_addr),
    .ddr3_ba(o_ddr3_ba),
    .ddr3_ras_n(o_ddr3_ras_n),
    .ddr3_cas_n(o_ddr3_cas_n),
    .ddr3_we_n(o_ddr3_we_n),
    .ddr3_reset_n(o_ddr3_reset_n),
    .ddr3_ck_p(o_ddr3_ck_p),
    .ddr3_ck_n(o_ddr3_ck_n),
    .ddr3_cke(o_ddr3_cke),
    .ddr3_cs_n(o_ddr3_cs_n),
    .ddr3_dm(o_ddr3_dm),
    .ddr3_odt(o_ddr3_odt),
    .sys_clk_p(1'b0),
    .sys_clk_n(1'b0),
    .sys_clk_i(i_xslv_clk),
    .ui_clk(o_ui_clk),
    .ui_clk_sync_rst(w_ui_rst),
    .mmcm_locked(w_ddr_mmcm_locked),
    .aresetn(i_xslv_nrst),
    .app_sr_req(1'b0),
    .app_ref_req(1'b0),
    .app_zq_req(1'b0),
    .app_sr_active(w_ddr_app_sr_active),
    .app_ref_ack(w_ddr_app_ref_ack),
    .app_zq_ack(w_ddr_app_zq_ack),
    .s_axi_awid(i_xslvi.aw_id),
    .s_axi_awaddr(i_xslvi.aw_bits.addr[29:0]),
    .s_axi_awlen(i_xslvi.aw_bits.len),
    .s_axi_awsize(i_xslvi.aw_bits.size),
    .s_axi_awburst(i_xslvi.aw_bits.burst),
    .s_axi_awlock(i_xslvi.aw_bits.lock),
    .s_axi_awcache(i_xslvi.aw_bits.cache),
    .s_axi_awprot(i_xslvi.aw_bits.prot),
    .s_axi_awqos(i_xslvi.aw_bits.qos),
    .s_axi_awvalid(i_xslvi.aw_valid),
    .s_axi_awready(o_xslvo.aw_ready),
    .s_axi_wdata(i_xslvi.w_data),
    .s_axi_wstrb(i_xslvi.w_strb),
    .s_axi_wlast(i_xslvi.w_last),
    .s_axi_wvalid(i_xslvi.w_valid),
    .s_axi_wready(o_xslvo.w_ready),
    .s_axi_bready(i_xslvi.b_ready),
    .s_axi_bid(o_xslvo.b_id),
    .s_axi_bresp(o_xslvo.b_resp),
    .s_axi_bvalid(o_xslvo.b_valid),
    .s_axi_arid(i_xslvi.ar_id),
    .s_axi_araddr(i_xslvi.ar_bits.addr[29:0]),
    .s_axi_arlen(i_xslvi.ar_bits.len),
    .s_axi_arsize(i_xslvi.ar_bits.size),
    .s_axi_arburst(i_xslvi.ar_bits.burst),
    .s_axi_arlock(i_xslvi.ar_bits.lock),
    .s_axi_arcache(i_xslvi.ar_bits.cache),
    .s_axi_arprot(i_xslvi.ar_bits.prot),
    .s_axi_arqos(i_xslvi.ar_bits.qos),
    .s_axi_arvalid(i_xslvi.ar_valid),
    .s_axi_arready(o_xslvo.ar_ready),
    .s_axi_rready(i_xslvi.r_ready),
    .s_axi_rid(o_xslvo.r_id),
    .s_axi_rdata(o_xslvo.r_data),
    .s_axi_rresp(o_xslvo.r_resp),
    .s_axi_rlast(o_xslvo.r_last),
    .s_axi_rvalid(o_xslvo.r_valid),
    .init_calib_complete(w_ddr_init_calib_complete),
    .device_temp(wb_ddr_device_temp),
    .sys_rst(i_xslv_nrst)  // active LOW. Connected to IODELAY
  );

  // TODO: fix me with registers
  assign o_xslvo.r_user = '0;
  assign o_xslvo.b_user = '0;

endmodule
