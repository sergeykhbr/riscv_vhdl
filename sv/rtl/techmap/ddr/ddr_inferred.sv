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

module ddr_inferred
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
    // to debug PIN:
    output o_ui_nrst,  // xilinx generte ddr clock inside ddr controller
    output o_ui_clk,  // xilinx generte ddr clock inside ddr controller
    output logic o_init_calib_done
);

import types_amba_pkg::*;

localparam SIM_DDRINIT_FILE_HEX = "../../../../examples/bbl-q/bbl-q-noprintf.hex";


  apb_ddr #(
    .async_reset(async_reset)
  ) apb0 (
    .i_clk(i_apb_clk),
    .i_nrst(i_apb_nrst),
    .i_mapinfo(i_pmapinfo),
    .o_cfg(o_pcfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .i_pll_locked(1'b1),
    .i_init_calib_done(1'b1),
    .i_device_temp(12'h456),
    .i_sr_active(1'b0),
    .i_ref_ack(1'b0),
    .i_zq_ack(1'b0)
  );

  // TODO: better ddr functional model
  axi4_sram #(
    .async_reset(async_reset),
    .abits((10 + $clog2(512*1024))),      // 512MB address
    .init_file(SIM_DDRINIT_FILE_HEX)
  ) mem0 (
    .clk(i_xslv_clk),
    .nrst(i_xslv_nrst),
    .i_mapinfo(i_xmapinfo),
    .cfg(),
    .i(i_xslvi),
    .o(o_xslvo)
  );

  assign o_ui_nrst = i_xslv_nrst;
  assign o_ui_clk = i_xslv_clk;
  assign o_init_calib_done = 1'b1;


endmodule
