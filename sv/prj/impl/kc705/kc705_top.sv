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

module kc705_top
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
    output                    o_uart1_td,
    // DDR3 signals:
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
    output o_ddr3_init_calib_complete
);

  logic             ib_rst;
  logic             ib_rstn;
//  logic             ib_clk_tcxo;
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

  // DDR3 signals:
  logic [4:0] wb_soc_ddr_awid;
  logic [47:0] wb_soc_ddr_awaddr;
  logic [7:0] wb_soc_ddr_awlen;
  logic [2:0] wb_soc_ddr_awsize;
  logic [1:0] wb_soc_ddr_awburst;
  logic w_soc_ddr_awlock;
  logic [3:0] wb_soc_ddr_awcache;
  logic [2:0] wb_soc_ddr_awprot;
  logic [3:0] wb_soc_ddr_awregion;
  logic [3:0] wb_soc_ddr_awqos;
  logic [0:0] w_soc_ddr_awuser;
  logic w_ddr_awvalid;
  logic w_soc_ddr_awready;
  logic [63:0] wb_soc_ddr_wdata;
  logic [7:0] wb_soc_ddr_wstrb;
  logic [0:0] w_soc_ddr_wuser;
  logic w_soc_ddr_wlast;
  logic w_soc_ddr_wvalid;
  logic w_ddr_wready;
  logic w_soc_ddr_bready;
  logic [4:0] wb_ddr_bid;
  logic [1:0] wb_ddr_bresp;
  logic [0:0] w_soc_ddr_buser;
  logic w_ddr_bvalid;
  logic [4:0] wb_soc_ddr_arid;
  logic [47:0] wb_soc_ddr_araddr;
  logic [7:0] wb_soc_ddr_arlen;
  logic [2:0] wb_soc_ddr_arsize;
  logic [1:0] wb_soc_ddr_arburst;
  logic w_soc_ddr_arlock;
  logic [3:0] wb_soc_ddr_arcache;
  logic [2:0] wb_soc_ddr_arprot;
  logic [3:0] wb_soc_ddr_arregion;
  logic [3:0] wb_soc_ddr_arqos;
  logic [0:0] w_soc_ddr_aruser;
  logic w_soc_ddr_arvalid;
  logic w_ddr_arready;
  logic w_soc_ddr_rready;
  logic [4:0] wb_ddr_rid;
  logic [63:0] wb_ddr_rdata;
  logic [1:0] wb_ddr_rresp;
  logic [0:0] w_soc_ddr_ruser;
  logic w_ddr_rlast;
  logic w_ddr_rvalid;
  logic w_ddr_ui_clk;
  logic w_ddr_ui_rst;
  logic w_ddr_mmcm_locked;
  logic w_ddr_init_calib_complete;
  logic [11:0] wb_ddr_device_temp;
  logic w_ddr_app_sr_active;
  logic w_ddr_app_ref_ack;
  logic w_ddr_app_zq_ack;


  ibuf_tech irst0(.o(ib_rst),.i(i_rst));
  
//  idsbuf_tech iclk0(.clk_p(i_sclk_p), .clk_n(i_sclk_n), .o_clk(ib_clk_tcxo));
  
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
    .i_clk_tcxo(w_ddr_ui_clk),//ib_clk_tcxo),
    .o_clk_bus(w_clk_bus),
    .o_locked(w_pll_lock)
  );  

  assign ib_rstn = ~ib_rst;
  assign w_ext_reset = ib_rst | ~w_pll_lock;
  assign o_ddr3_init_calib_complete = w_ddr_init_calib_complete;
  
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
    // DDR signal:
    .o_ddr_awid(wb_soc_ddr_awid),
    .o_ddr_awaddr(wb_soc_ddr_awaddr),
    .o_ddr_awlen(wb_soc_ddr_awlen),
    .o_ddr_awsize(wb_soc_ddr_awsize),
    .o_ddr_awburst(wb_soc_ddr_awburst),
    .o_ddr_awlock(w_soc_ddr_awlock),
    .o_ddr_awcache(wb_soc_ddr_awcache),
    .o_ddr_awprot(wb_soc_ddr_awprot),
    .o_ddr_awregion(wb_soc_ddr_awregion),
    .o_ddr_awqos(wb_soc_ddr_awqos),
    .o_ddr_awuser(w_soc_ddr_awuser),
    .o_ddr_awvalid(w_ddr_awvalid),
    .i_ddr_awready(w_soc_ddr_awready),
    .o_ddr_wdata(wb_soc_ddr_wdata),
    .o_ddr_wstrb(wb_soc_ddr_wstrb),
    .o_ddr_wuser(w_soc_ddr_wuser),
    .o_ddr_wlast(w_soc_ddr_wlast),
    .o_ddr_wvalid(w_soc_ddr_wvalid),
    .i_ddr_wready(w_ddr_wready),
    .o_ddr_bready(w_soc_ddr_bready),
    .i_ddr_bid(wb_ddr_bid),
    .i_ddr_bresp(wb_ddr_bresp),
    .i_ddr_buser(w_ddr_buser),
    .i_ddr_bvalid(w_ddr_bvalid),
    .o_ddr_arid(wb_soc_ddr_arid),
    .o_ddr_araddr(wb_soc_ddr_araddr),
    .o_ddr_arlen(wb_soc_ddr_arlen),
    .o_ddr_arsize(wb_soc_ddr_arsize),
    .o_ddr_arburst(wb_soc_ddr_arburst),
    .o_ddr_arlock(w_soc_ddr_arlock),
    .o_ddr_arcache(wb_soc_ddr_arcache),
    .o_ddr_arprot(wb_soc_ddr_arprot),
    .o_ddr_arregion(wb_soc_ddr_arregion),
    .o_ddr_arqos(wb_soc_ddr_arqos),
    .o_ddr_aruser(w_soc_ddr_aruser),
    .o_ddr_arvalid(w_soc_ddr_arvalid),
    .i_ddr_arready(w_ddr_arready),
    .o_ddr_rready(w_soc_ddr_rready),
    .i_ddr_rid(wb_ddr_rid),
    .i_ddr_rdata(wb_ddr_rdata),
    .i_ddr_rresp(wb_ddr_rresp),
    .i_ddr_ruser(w_ddr_ruser),
    .i_ddr_rlast(w_ddr_rlast),
    .i_ddr_rvalid(w_ddr_rvalid),
    .i_ddr_ui_clk(w_ddr_ui_clk),
    .i_ddr_ui_rst(w_ddr_ui_rst),
    .i_ddr_mmcm_locked(w_ddr_mmcm_locked),
    .i_ddr_init_calib_complete(w_ddr_init_calib_complete),
    .i_ddr_device_temp(wb_ddr_device_temp),
    .i_ddr_app_sr_active(w_ddr_app_sr_active),
    .i_ddr_app_ref_ack(w_ddr_app_ref_ack),
    .i_ddr_app_zq_ack(w_ddr_app_zq_ack)
  );
  assign w_ddr_ruser = '0;
  assign w_ddr_buser = '0;

   mig_ddr3 mig0 (
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
    .sys_clk_p(i_sclk_p),
    .sys_clk_n(i_sclk_n),
    .ui_clk(w_ddr_ui_clk),
    .ui_clk_sync_rst(w_ddr_ui_rst),
    .mmcm_locked(w_ddr_mmcm_locked),
    .aresetn(ib_rstn),
    .app_sr_req(1'b0),
    .app_ref_req(1'b0),
    .app_zq_req(1'b0),
    .app_sr_active(w_ddr_app_sr_active),
    .app_ref_ack(w_ddr_app_ref_ack),
    .app_zq_ack(w_ddr_app_zq_ack),
    .s_axi_awid(wb_soc_ddr_awid),
    .s_axi_awaddr(wb_soc_ddr_awaddr[29:0]),
    .s_axi_awlen(wb_soc_ddr_awlen),
    .s_axi_awsize(wb_soc_ddr_awsize),
    .s_axi_awburst(wb_soc_ddr_awburst),
    .s_axi_awlock(w_soc_ddr_awlock),
    .s_axi_awcache(wb_soc_ddr_awcache),
    .s_axi_awprot(wb_soc_ddr_awprot),
    .s_axi_awqos(wb_soc_ddr_awqos),
    .s_axi_awvalid(w_ddr_awvalid),
    .s_axi_awready(w_soc_ddr_awready),
    .s_axi_wdata(wb_soc_ddr_wdata),
    .s_axi_wstrb(wb_soc_ddr_wstrb),
    .s_axi_wlast(w_soc_ddr_wlast),
    .s_axi_wvalid(w_soc_ddr_wvalid),
    .s_axi_wready(w_ddr_wready),
    .s_axi_bready(w_soc_ddr_bready),
    .s_axi_bid(wb_ddr_bid),
    .s_axi_bresp(wb_ddr_bresp),
    .s_axi_bvalid(w_ddr_bvalid),
    .s_axi_arid(wb_soc_ddr_arid),
    .s_axi_araddr(wb_soc_ddr_araddr[29:0]),
    .s_axi_arlen(wb_soc_ddr_arlen),
    .s_axi_arsize(wb_soc_ddr_arsize),
    .s_axi_arburst(wb_soc_ddr_arburst),
    .s_axi_arlock(w_soc_ddr_arlock),
    .s_axi_arcache(wb_soc_ddr_arcache),
    .s_axi_arprot(wb_soc_ddr_arprot),
    .s_axi_arqos(wb_soc_ddr_arqos),
    .s_axi_arvalid(w_soc_ddr_arvalid),
    .s_axi_arready(w_ddr_arready),
    .s_axi_rready(w_soc_ddr_rready),
    .s_axi_rid(wb_ddr_rid),
    .s_axi_rdata(wb_ddr_rdata),
    .s_axi_rresp(wb_ddr_rresp),
    .s_axi_rlast(w_ddr_rlast),
    .s_axi_rvalid(w_ddr_rvalid),
    .init_calib_complete(w_ddr_init_calib_complete),
    .device_temp(wb_ddr_device_temp),
    .sys_rst(ib_rstn)  // active LOW
  );

  
endmodule