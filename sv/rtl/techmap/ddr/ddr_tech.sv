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

axi4_slave_in_type wb_xslvi;  // remapped

`ifdef TARGET_INFERRED
  ddr_inferred #(
    .async_reset(async_reset),
    .SYSCLK_TYPE(SYSCLK_TYPE), // "NO_BUFFER,"DIFFERENTIAL"
    .SIM_BYPASS_INIT_CAL(SIM_BYPASS_INIT_CAL),  // "FAST"-for simulation true; "OFF"
    .SIMULATION(SIMULATION)
  ) inf0 (
    .i_apb_nrst(i_apb_nrst),
    .i_apb_clk(i_apb_clk),
    .i_xslv_nrst(i_xslv_nrst),
    .i_xslv_clk(i_xslv_clk),
    // AXI memory access (ddr clock)
    .i_xmapinfo(i_xmapinfo),
    .o_xcfg(o_xcfg),
    .i_xslvi(wb_xslvi),
    .o_xslvo(o_xslvo),
    // APB control interface (sys clock):
    .i_pmapinfo(i_pmapinfo),
    .o_pcfg(o_pcfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_ui_nrst(o_ui_nrst),
    .o_ui_clk(o_ui_clk),
    .o_init_calib_done(o_init_calib_done)
  );
`elsif TARGET_KC705
  ddr_kc705 #(
    .async_reset(async_reset),
    .SYSCLK_TYPE(SYSCLK_TYPE), // "NO_BUFFER,"DIFFERENTIAL"
    .SIM_BYPASS_INIT_CAL(SIM_BYPASS_INIT_CAL),  // "FAST"-for simulation true; "OFF"
    .SIMULATION(SIMULATION)
  ) kc705 (
    .i_apb_nrst(i_apb_nrst),
    .i_apb_clk(i_apb_clk),
    .i_xslv_nrst(i_xslv_nrst),
    .i_xslv_clk(i_xslv_clk),
    // AXI memory access (ddr clock)
    .i_xmapinfo(i_xmapinfo),
    .o_xcfg(o_xcfg),
    .i_xslvi(wb_xslvi),
    .o_xslvo(o_xslvo),
    // APB control interface (sys clock):
    .i_pmapinfo(i_pmapinfo),
    .o_pcfg(o_pcfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_ui_nrst(o_ui_nrst),
    .o_ui_clk(o_ui_clk),
    // DDR signals:
    .o_ddr3_reset_n(o_ddr3_reset_n),
    .o_ddr3_ck_n(o_ddr3_ck_n),
    .o_ddr3_ck_p(o_ddr3_ck_p),
    .o_ddr3_cke(o_ddr3_cke),
    .o_ddr3_cs_n(o_ddr3_cs_n),
    .o_ddr3_ras_n(o_ddr3_ras_n),
    .o_ddr3_cas_n(o_ddr3_cas_n),
    .o_ddr3_we_n(o_ddr3_we_n),
    .o_ddr3_dm(o_ddr3_dm),
    .o_ddr3_ba(o_ddr3_ba),
    .o_ddr3_addr(o_ddr3_addr),
    .io_ddr3_dq(io_ddr3_dq),
    .io_ddr3_dqs_n(io_ddr3_dqs_n),
    .io_ddr3_dqs_p(io_ddr3_dqs_p),
    .o_ddr3_odt(o_ddr3_odt),
    .o_init_calib_done(o_init_calib_done)
  );
`else
    initial $error("INSTANCE macro is undefined, check technology-dependent memories.");
`endif


always_comb
begin: comb_proc
    axi4_slave_in_type vxslvi;

    vxslvi = i_xslvi;
    vxslvi.ar_bits.addr = i_xslvi.ar_bits.addr - i_xmapinfo.addr_start;
    vxslvi.aw_bits.addr = i_xslvi.aw_bits.addr - i_xmapinfo.addr_start;

    wb_xslvi = vxslvi;
end: comb_proc

endmodule
