//!
//! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

module asic_top
( 
    //! Input reset. Active HIGH.
    input                     i_rst,
    //! Differential clock (LVDS) positive/negaive signal.
    input                     i_sclk_p,
    input                     i_sclk_n,
    //! GPIO: [11:4] LEDs; [3:0] DIP switch
    inout [11:0]              io_gpio,
    //! JTAG signals:
    input                     i_jtag_tck,
    input                     i_jtag_trst,
    input                     i_jtag_tms,
    input                     i_jtag_tdi,
    output                    o_jtag_tdo,
    output                    o_jtag_vref,
    //! UART1 signals:
    input                     i_uart1_rd,
    output                    o_uart1_td
);

  import types_amba_pkg::*;

  logic             ib_rst;
  logic             ib_rstn;
  logic             ib_clk_tcxo;
  logic             ib_sclk_n;  

  logic [11:0]      ob_gpio_direction;
  logic [11:0]      ob_gpio_opins;
  logic [11:0]      ib_gpio_ipins;  
  logic             ib_uart1_rd;  
  logic             ob_uart1_td;  
  //! JTAG signals:  
  logic             ib_jtag_tck;  
  logic             ib_jtag_trst; 
  logic             ib_jtag_tms; 
  logic             ib_jtag_tdi; 
  logic             ob_jtag_tdo; 
  logic             ob_jtag_vref;   

  logic             w_ext_reset;
  logic             w_glob_rst;
  logic             w_glob_nrst;
  logic             w_soft_rst;
  logic             w_bus_nrst;
  logic             w_clk_bus;
  logic             w_pll_lock;

  axi4_slave_in_type ddr_xslvi;
  axi4_slave_out_type ddr_xslvo;
  

  ibuf_tech irst0(.o(ib_rst),.i(i_rst));
  assign ib_rstn = ib_rst;
  
  idsbuf_tech iclk0(.clk_p(i_sclk_p), .clk_n(i_sclk_n), .o_clk(ib_clk_tcxo));
  
  ibuf_tech ird1(.o(ib_uart1_rd),.i(i_uart1_rd));
  obuf_tech otd1(.o(o_uart1_td),.i(ob_uart1_td));

  genvar i;
  generate 
    for(i=0; i<=11; i++) begin: gpiox  
      iobuf_tech iob0(.o(ib_gpio_ipins[i]), .io(io_gpio[i]), .i(ob_gpio_opins[i]), .t(ob_gpio_direction[i])); 
    end
  endgenerate
  
  ibuf_tech ijtck0(.o(ib_jtag_tck),.i(i_jtag_tck));  
  ibuf_tech ijtrst0(.o(ib_jtag_trst),.i(i_jtag_trst)); 
  ibuf_tech ijtms0(.o(ib_jtag_tms),.i(i_jtag_tms));   
  ibuf_tech ijtdi0(.o(ib_jtag_tdi),.i(i_jtag_tdi)); 
  obuf_tech ojtdo0(.o(o_jtag_tdo),.i(ob_jtag_tdo));   
  obuf_tech ojvrf0(.o(o_jtag_vref),.i(ob_jtag_vref)); 
  

  SysPLL_tech pll0(
    .i_reset(ib_rst),
    .i_clk_tcxo(ib_clk_tcxo),
    .o_clk_bus(w_clk_bus),
    .o_locked(w_pll_lock)
  );  

  assign w_ext_reset = ib_rst | ~w_pll_lock;
  
  riscv_soc soc0(
    .i_rst (w_ext_reset),
    .i_clk (w_clk_bus),
    //! GPIO.
    .i_gpio (ib_gpio_ipins),
    .o_gpio (ob_gpio_opins),
    .o_gpio_dir(ob_gpio_direction),
    //! JTAG signals:
    .i_jtag_tck(ib_jtag_tck),
    .i_jtag_trst(ib_jtag_trst),
    .i_jtag_tms(ib_jtag_tms),
    .i_jtag_tdi(ib_jtag_tdi),
    .o_jtag_tdo(ob_jtag_tdo),
    .o_jtag_vref(ob_jtag_vref),
    //! UART1 signals:
    .i_uart1_rd(ib_uart1_rd),
    .o_uart1_td(ob_uart1_td),
    // DDR:
  .o_ddr_awid(ddr_xslvi.aw_id),
  .o_ddr_awaddr(ddr_xslvi.aw_bits.addr),
  .o_ddr_awlen(ddr_xslvi.aw_bits.len),
  .o_ddr_awsize(ddr_xslvi.aw_bits.size),
  .o_ddr_awburst(ddr_xslvi.aw_bits.burst),
  .o_ddr_awlock(ddr_xslvi.aw_bits.lock),
  .o_ddr_awcache(ddr_xslvi.aw_bits.cache),
  .o_ddr_awprot(ddr_xslvi.aw_bits.prot),
  .o_ddr_awregion(ddr_xslvi.aw_bits.region),
  .o_ddr_awqos(ddr_xslvi.aw_bits.qos),
  .o_ddr_awuser(ddr_xslvi.aw_user),
  .o_ddr_awvalid(ddr_xslvi.aw_valid),
  .i_ddr_awready(ddr_xslvo.aw_ready),
  .o_ddr_wdata(ddr_xslvi.w_data),
  .o_ddr_wstrb(ddr_xslvi.w_strb),
  .o_ddr_wlast(ddr_xslvi.w_last),
  .o_ddr_wuser(ddr_xslvi.w_user),
  .o_ddr_wvalid(ddr_xslvi.w_valid),
  .i_ddr_wready(ddr_xslvo.w_ready),
  .i_ddr_bid(ddr_xslvo.b_id),
  .i_ddr_bresp(ddr_xslvo.b_resp),
  .i_ddr_buser(ddr_xslvo.b_user),
  .i_ddr_bvalid(ddr_xslvo.b_valid),
  .o_ddr_bready(ddr_xslvi.b_ready),
  .o_ddr_arid(ddr_xslvi.ar_id),
  .o_ddr_araddr(ddr_xslvi.ar_bits.addr),
  .o_ddr_arlen(ddr_xslvi.ar_bits.len),
  .o_ddr_arsize(ddr_xslvi.ar_bits.size),
  .o_ddr_arburst(ddr_xslvi.ar_bits.burst),
  .o_ddr_arlock(ddr_xslvi.ar_bits.lock),
  .o_ddr_arcache(ddr_xslvi.ar_bits.cache),
  .o_ddr_arprot(ddr_xslvi.ar_bits.prot),
  .o_ddr_arregion(ddr_xslvi.ar_bits.region),
  .o_ddr_arqos(ddr_xslvi.ar_bits.qos),
  .o_ddr_aruser(ddr_xslvi.ar_user),
  .o_ddr_arvalid(ddr_xslvi.ar_valid),
  .i_ddr_arready(ddr_xslvo.ar_ready),
  .i_ddr_rid(ddr_xslvo.r_id),
  .i_ddr_rdata(ddr_xslvo.r_data),
  .i_ddr_rresp(ddr_xslvo.r_resp),
  .i_ddr_rlast(ddr_xslvo.r_last),
  .i_ddr_ruser(ddr_xslvo.r_user),
  .i_ddr_rvalid(ddr_xslvo.r_valid),
  .o_ddr_rready(ddr_xslvi.r_ready),
  .i_ddr_ui_clk(ib_clk_tcxo),  // 200 MHz DDR clock (unused in inferred)
  .i_ddr_ui_rst(ib_rstn),  // active LOW (unused in inferred actually)
  .i_ddr_mmcm_locked(1'b1),
  .i_ddr_init_calib_complete(1'b1),
  .i_ddr_device_temp('0),
  .i_ddr_app_sr_active(1'b0),
  .i_ddr_app_ref_ack(1'b0),
  .i_ddr_app_zq_ack(1'b0)
  );



  // TODO: better ddr functional model
  const mapinfo_type ddr_mapinfo = '{'0, '0};

  axi4_sram #(
    .async_reset(0),
    .abits((10 + $clog2(512*1024)))      // 512MB address
  ) ddr0 (
    .clk(ib_clk_tcxo),
    .nrst(ib_rstn),
    .i_mapinfo(ddr_mapinfo),
    .cfg(),
    .i(ddr_xslvi),
    .o(ddr_xslvo)
  );


  
endmodule