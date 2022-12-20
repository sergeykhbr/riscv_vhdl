//!
//! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

module cdc_axi_sync_tech
(
    input i_xslv_clk,
    input i_xslv_nrst,
    input types_amba_pkg::axi4_slave_in_type i_xslvi,    // system clock
    output types_amba_pkg::axi4_slave_out_type o_xslvo,  // system clock
    input i_xmst_clk,
    input i_xmst_nrst,
    output types_amba_pkg::axi4_slave_in_type o_xmsto,  // ddr clock
    input types_amba_pkg::axi4_slave_out_type i_xmsti   // ddr clock
);

import types_amba_pkg::*;

// TODO: ifdef KC705 or other

cdc_ddr u_cdc_ddr0(
  .s_axi_aclk(i_xslv_clk),
  .s_axi_aresetn(i_xslv_nrst),
  .s_axi_awid(i_xslvi.aw_id),
  .s_axi_awaddr(i_xslvi.aw_bits.addr),
  .s_axi_awlen(i_xslvi.aw_bits.len),
  .s_axi_awsize(i_xslvi.aw_bits.size),
  .s_axi_awburst(i_xslvi.aw_bits.burst),
  .s_axi_awlock(i_xslvi.aw_bits.lock),
  .s_axi_awcache(i_xslvi.aw_bits.cache),
  .s_axi_awprot(i_xslvi.aw_bits.prot),
  .s_axi_awregion(i_xslvi.aw_bits.region),
  .s_axi_awqos(i_xslvi.aw_bits.qos),
  .s_axi_awuser(i_xslvi.aw_user),
  .s_axi_awvalid(i_xslvi.aw_valid),
  .s_axi_awready(o_xslvo.aw_ready),
  .s_axi_wdata(i_xslvi.w_data),
  .s_axi_wstrb(i_xslvi.w_strb),
  .s_axi_wlast(i_xslvi.w_last),
  .s_axi_wuser(i_xslvi.w_user),
  .s_axi_wvalid(i_xslvi.w_valid),
  .s_axi_wready(o_xslvo.w_ready),
  .s_axi_bid(o_xslvo.b_id),
  .s_axi_bresp(o_xslvo.b_resp),
  .s_axi_buser(o_xslvo.b_user),
  .s_axi_bvalid(o_xslvo.b_valid),
  .s_axi_bready(i_xslvi.b_ready),
  .s_axi_arid(i_xslvi.ar_id),
  .s_axi_araddr(i_xslvi.ar_bits.addr),
  .s_axi_arlen(i_xslvi.ar_bits.len),
  .s_axi_arsize(i_xslvi.ar_bits.size),
  .s_axi_arburst(i_xslvi.ar_bits.burst),
  .s_axi_arlock(i_xslvi.ar_bits.lock),
  .s_axi_arcache(i_xslvi.ar_bits.cache),
  .s_axi_arprot(i_xslvi.ar_bits.prot),
  .s_axi_arregion(i_xslvi.ar_bits.region),
  .s_axi_arqos(i_xslvi.ar_bits.qos),
  .s_axi_aruser(i_xslvi.ar_user),
  .s_axi_arvalid(i_xslvi.ar_valid),
  .s_axi_arready(o_xslvo.ar_ready),
  .s_axi_rid(o_xslvo.r_id),
  .s_axi_rdata(o_xslvo.r_data),
  .s_axi_rresp(o_xslvo.r_resp),
  .s_axi_rlast(o_xslvo.r_last),
  .s_axi_ruser(o_xslvo.r_user),
  .s_axi_rvalid(o_xslvo.r_valid),
  .s_axi_rready(i_xslvi.r_ready),
  .m_axi_aclk(i_xmst_clk),
  .m_axi_aresetn(i_xmst_nrst),
  .m_axi_awid(o_xmsto.aw_id),
  .m_axi_awaddr(o_xmsto.aw_bits.addr),
  .m_axi_awlen(o_xmsto.aw_bits.len),
  .m_axi_awsize(o_xmsto.aw_bits.size),
  .m_axi_awburst(o_xmsto.aw_bits.burst),
  .m_axi_awlock(o_xmsto.aw_bits.lock),
  .m_axi_awcache(o_xmsto.aw_bits.cache),
  .m_axi_awprot(o_xmsto.aw_bits.prot),
  .m_axi_awregion(o_xmsto.aw_bits.region),
  .m_axi_awqos(o_xmsto.aw_bits.qos),
  .m_axi_awuser(o_xmsto.aw_user),
  .m_axi_awvalid(o_xmsto.aw_valid),
  .m_axi_awready(i_xmsti.aw_ready),
  .m_axi_wdata(o_xmsto.w_data),
  .m_axi_wstrb(o_xmsto.w_strb),
  .m_axi_wlast(o_xmsto.w_last),
  .m_axi_wuser(o_xmsto.w_user),
  .m_axi_wvalid(o_xmsto.w_valid),
  .m_axi_wready(i_xmsti.w_ready),
  .m_axi_bid(i_xmsti.b_id),
  .m_axi_bresp(i_xmsti.b_resp),
  .m_axi_buser(i_xmsti.b_user),
  .m_axi_bvalid(i_xmsti.b_valid),
  .m_axi_bready(o_xmsto.b_ready),
  .m_axi_arid(o_xmsto.ar_id),
  .m_axi_araddr(o_xmsto.ar_bits.addr),
  .m_axi_arlen(o_xmsto.ar_bits.len),
  .m_axi_arsize(o_xmsto.ar_bits.size),
  .m_axi_arburst(o_xmsto.ar_bits.burst),
  .m_axi_arlock(o_xmsto.ar_bits.lock),
  .m_axi_arcache(o_xmsto.ar_bits.cache),
  .m_axi_arprot(o_xmsto.ar_bits.prot),
  .m_axi_arregion(o_xmsto.ar_bits.region),
  .m_axi_arqos(o_xmsto.ar_bits.qos),
  .m_axi_aruser(o_xmsto.ar_user),
  .m_axi_arvalid(o_xmsto.ar_valid),
  .m_axi_arready(i_xmsti.ar_ready),
  .m_axi_rid(i_xmsti.r_id),
  .m_axi_rdata(i_xmsti.r_data),
  .m_axi_rresp(i_xmsti.r_resp),
  .m_axi_rlast(i_xmsti.r_last),
  .m_axi_ruser(i_xmsti.r_user),
  .m_axi_rvalid(i_xmsti.r_valid),
  .m_axi_rready(o_xmsto.r_ready)
);


endmodule
