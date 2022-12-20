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

module ddr_tech
#(
    parameter int async_reset = 0
)
(
    input i_sys_nrst,
    input i_sys_clk,
    input i_ddr_nrst,
    input i_ddr_clk,
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
    output logic o_init_calib_done
);

import types_amba_pkg::*;

  // TODO: better ddr functional model
  const mapinfo_type ddr_mapinfo = '{'0, '0};

  axi4_sram #(
    .async_reset(0),
    .abits((10 + $clog2(512*1024)))      // 512MB address
  ) mem0 (
    .clk(i_ddr_clk),
    .nrst(i_ddr_nrst),
    .i_mapinfo(ddr_mapinfo),
    .cfg(),
    .i(i_xslvi),
    .o(o_xslvo)
  );

endmodule
