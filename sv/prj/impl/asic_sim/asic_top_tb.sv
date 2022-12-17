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

`ifndef SCLK_PERIOD
  `define SCLK_PERIOD 25
`endif


`define WF_AXI_MONITOR_MST_CONN(device_path, ic_port_nmb) \
.aclk       (device_path.i_clk),\
.aresetn    (device_path.i_nrst),\
\
.awid       (device_path.i_msto[ic_port_nmb].aw_id),\
.awaddr     (device_path.i_msto[ic_port_nmb].aw_bits.addr),\
.awlen      (device_path.i_msto[ic_port_nmb].aw_bits.len),\
.awsize     (device_path.i_msto[ic_port_nmb].aw_bits.size),\
.awburst    (device_path.i_msto[ic_port_nmb].aw_bits.burst),\
.awlock     (device_path.i_msto[ic_port_nmb].aw_bits.lock),\
.awprot     (device_path.i_msto[ic_port_nmb].aw_bits.prot),\
.awqos      (device_path.i_msto[ic_port_nmb].aw_bits.qos),\
.awregion   (device_path.i_msto[ic_port_nmb].aw_bits.region),\
.awcache    (device_path.i_msto[ic_port_nmb].aw_bits.cache),\
.awvalid    (device_path.i_msto[ic_port_nmb].aw_valid),\
.awready    (device_path.o_msti[ic_port_nmb].aw_ready),\
.awuser     (device_path.i_msto[ic_port_nmb].aw_user),\
\
.wdata      (device_path.i_msto[ic_port_nmb].w_data),\
.wstrb      (device_path.i_msto[ic_port_nmb].w_strb),\
.wlast      (device_path.i_msto[ic_port_nmb].w_last),\
.wvalid     (device_path.i_msto[ic_port_nmb].w_valid),\
.wready     (device_path.o_msti[ic_port_nmb].w_ready),\
.wuser      (device_path.i_msto[ic_port_nmb].w_user),\
\
.bid        (device_path.o_msti[ic_port_nmb].b_id),\
.bresp      (device_path.o_msti[ic_port_nmb].b_resp),\
.bvalid     (device_path.o_msti[ic_port_nmb].b_valid),\
.bready     (device_path.i_msto[ic_port_nmb].b_ready),\
.buser      (device_path.o_msti[ic_port_nmb].b_user),\
\
.arid       (device_path.i_msto[ic_port_nmb].ar_id),\
.araddr     (device_path.i_msto[ic_port_nmb].ar_bits.addr),\
.arlen      (device_path.i_msto[ic_port_nmb].ar_bits.len),\
.arsize     (device_path.i_msto[ic_port_nmb].ar_bits.size),\
.arburst    (device_path.i_msto[ic_port_nmb].ar_bits.burst),\
.arlock     (device_path.i_msto[ic_port_nmb].ar_bits.lock),\
.arprot     (device_path.i_msto[ic_port_nmb].ar_bits.prot),\
.arqos      (device_path.i_msto[ic_port_nmb].ar_bits.qos),\
.arregion   (device_path.i_msto[ic_port_nmb].ar_bits.region),\
.arcache    (device_path.i_msto[ic_port_nmb].ar_bits.cache),\
.arvalid    (device_path.i_msto[ic_port_nmb].ar_valid),\
.arready    (device_path.o_msti[ic_port_nmb].ar_ready),\
.aruser     (device_path.i_msto[ic_port_nmb].ar_user),\
\
.rid        (device_path.o_msti[ic_port_nmb].r_id),\
.rdata      (device_path.o_msti[ic_port_nmb].r_data),\
.rlast      (device_path.o_msti[ic_port_nmb].r_last),\
.rvalid     (device_path.o_msti[ic_port_nmb].r_valid),\
.rready     (device_path.i_msto[ic_port_nmb].r_ready),\
.rresp      (device_path.o_msti[ic_port_nmb].r_resp),\
.ruser      (device_path.o_msti[ic_port_nmb].r_user),\
\
.csysreq(1'b1),\
.csysack(0),\
.cactive(0)








`define WF_AXI_MONITOR_SLV_CONN(device_path, ic_port_nmb) \
.aclk       (device_path.i_clk),\
.aresetn    (device_path.i_nrst),\
\
.awid       (device_path.o_slvi[ic_port_nmb].aw_id),\
.awaddr     (device_path.o_slvi[ic_port_nmb].aw_bits.addr),\
.awlen      (device_path.o_slvi[ic_port_nmb].aw_bits.len),\
.awsize     (device_path.o_slvi[ic_port_nmb].aw_bits.size),\
.awburst    (device_path.o_slvi[ic_port_nmb].aw_bits.burst),\
.awlock     (device_path.o_slvi[ic_port_nmb].aw_bits.lock),\
.awprot     (device_path.o_slvi[ic_port_nmb].aw_bits.prot),\
.awqos      (device_path.o_slvi[ic_port_nmb].aw_bits.qos),\
.awregion   (device_path.o_slvi[ic_port_nmb].aw_bits.region),\
.awcache    (device_path.o_slvi[ic_port_nmb].aw_bits.cache),\
.awvalid    (device_path.o_slvi[ic_port_nmb].aw_valid),\
.awready    (device_path.i_slvo[ic_port_nmb].aw_ready),\
.awuser     (device_path.o_slvi[ic_port_nmb].aw_user),\
\
.wdata      (device_path.o_slvi[ic_port_nmb].w_data),\
.wstrb      (device_path.o_slvi[ic_port_nmb].w_strb),\
.wlast      (device_path.o_slvi[ic_port_nmb].w_last),\
.wvalid     (device_path.o_slvi[ic_port_nmb].w_valid),\
.wready     (device_path.i_slvo[ic_port_nmb].w_ready),\
.wuser      (device_path.o_slvi[ic_port_nmb].w_user),\
\
.bid        (device_path.i_slvo[ic_port_nmb].b_id),\
.bresp      (device_path.i_slvo[ic_port_nmb].b_resp),\
.bvalid     (device_path.i_slvo[ic_port_nmb].b_valid),\
.bready     (device_path.o_slvi[ic_port_nmb].b_ready),\
.buser      (device_path.i_slvo[ic_port_nmb].b_user),\
\
.arid       (device_path.o_slvi[ic_port_nmb].ar_id),\
.araddr     (device_path.o_slvi[ic_port_nmb].ar_bits.addr),\
.arlen      (device_path.o_slvi[ic_port_nmb].ar_bits.len),\
.arsize     (device_path.o_slvi[ic_port_nmb].ar_bits.size),\
.arburst    (device_path.o_slvi[ic_port_nmb].ar_bits.burst),\
.arlock     (device_path.o_slvi[ic_port_nmb].ar_bits.lock),\
.arprot     (device_path.o_slvi[ic_port_nmb].ar_bits.prot),\
.arqos      (device_path.o_slvi[ic_port_nmb].ar_bits.qos),\
.arregion   (device_path.o_slvi[ic_port_nmb].ar_bits.region),\
.arcache    (device_path.o_slvi[ic_port_nmb].ar_bits.cache),\
.arvalid    (device_path.o_slvi[ic_port_nmb].ar_valid),\
.arready    (device_path.i_slvo[ic_port_nmb].ar_ready),\
.aruser     (device_path.o_slvi[ic_port_nmb].ar_user),\
\
.rid        (device_path.i_slvo[ic_port_nmb].r_id),\
.rdata      (device_path.i_slvo[ic_port_nmb].r_data),\
.rlast      (device_path.i_slvo[ic_port_nmb].r_last),\
.rvalid     (device_path.i_slvo[ic_port_nmb].r_valid),\
.rready     (device_path.o_slvi[ic_port_nmb].r_ready),\
.rresp      (device_path.i_slvo[ic_port_nmb].r_resp),\
.ruser      (device_path.i_slvo[ic_port_nmb].r_user),\
\
.csysreq(1'b1),\
.csysack(0),\
.cactive(0)




`define WF_AXI_MONITOR_SLV_CONN_SINGLE(device_path, clk_name, rstn_name, slv_inp_struct, slv_out_struct) \
.aclk       (device_path.clk_name),\
.aresetn    (device_path.rstn_name),\
\
.awid       (device_path.slv_inp_struct.aw_id),\
.awaddr     (device_path.slv_inp_struct.aw_bits.addr),\
.awlen      (device_path.slv_inp_struct.aw_bits.len),\
.awsize     (device_path.slv_inp_struct.aw_bits.size),\
.awburst    (device_path.slv_inp_struct.aw_bits.burst),\
.awlock     (device_path.slv_inp_struct.aw_bits.lock),\
.awprot     (device_path.slv_inp_struct.aw_bits.prot),\
.awqos      (device_path.slv_inp_struct.aw_bits.qos),\
.awregion   (device_path.slv_inp_struct.aw_bits.region),\
.awcache    (device_path.slv_inp_struct.aw_bits.cache),\
.awvalid    (device_path.slv_inp_struct.aw_valid),\
.awready    (device_path.slv_out_struct.aw_ready),\
.awuser     (device_path.slv_inp_struct.aw_user),\
\
.wdata      (device_path.slv_inp_struct.w_data),\
.wstrb      (device_path.slv_inp_struct.w_strb),\
.wlast      (device_path.slv_inp_struct.w_last),\
.wvalid     (device_path.slv_inp_struct.w_valid),\
.wready     (device_path.slv_out_struct.w_ready),\
.wuser      (device_path.slv_inp_struct.w_user),\
\
.bid        (device_path.slv_out_struct.b_id),\
.bresp      (device_path.slv_out_struct.b_resp),\
.bvalid     (device_path.slv_out_struct.b_valid),\
.bready     (device_path.slv_inp_struct.b_ready),\
.buser      (device_path.slv_out_struct.b_user),\
\
.arid       (device_path.slv_inp_struct.ar_id),\
.araddr     (device_path.slv_inp_struct.ar_bits.addr),\
.arlen      (device_path.slv_inp_struct.ar_bits.len),\
.arsize     (device_path.slv_inp_struct.ar_bits.size),\
.arburst    (device_path.slv_inp_struct.ar_bits.burst),\
.arlock     (device_path.slv_inp_struct.ar_bits.lock),\
.arprot     (device_path.slv_inp_struct.ar_bits.prot),\
.arqos      (device_path.slv_inp_struct.ar_bits.qos),\
.arregion   (device_path.slv_inp_struct.ar_bits.region),\
.arcache    (device_path.slv_inp_struct.ar_bits.cache),\
.arvalid    (device_path.slv_inp_struct.ar_valid),\
.arready    (device_path.slv_out_struct.ar_ready),\
.aruser     (device_path.slv_inp_struct.ar_user),\
\
.rid        (device_path.slv_out_struct.r_id),\
.rdata      (device_path.slv_out_struct.r_data),\
.rlast      (device_path.slv_out_struct.r_last),\
.rvalid     (device_path.slv_out_struct.r_valid),\
.rready     (device_path.slv_inp_struct.r_ready),\
.rresp      (device_path.slv_out_struct.r_resp),\
.ruser      (device_path.slv_out_struct.r_user),\
\
.csysreq(1'b1),\
.csysack(0),\
.cactive(0)




`define WF_AXI_LITE_MONITOR_SLV_CONN(device_path) \
.aclk       (device_path.s_axi_aclk),\
.aresetn    (device_path.s_axi_aresetn),\
\
.awid       (p_AXI_LITE_MONITOR_TRANS_ID),\
.awaddr     (device_path.aw_bits.addr),\
.awlen      ('0),\
.awsize     (p_AXI_LITE_size),\
.awburst    ('0),\
.awlock     ('0),\
.awprot     (device_path.aw_bits.prot),\
.awqos      ('0),\
.awregion   ('0),\
.awcache    ('0),\
.awvalid    (device_path.aw_valid),\
.awready    (device_path.aw_ready),\
.awuser     ('0),\
\
.wdata      (device_path.w_data),\
.wstrb      (device_path.w_strb),\
.wlast      (1'b1),\
.wvalid     (device_path.w_valid),\
.wready     (device_path.w_ready),\
.wuser      ('0),\
\
.bid        (p_AXI_LITE_MONITOR_TRANS_ID),\
.bresp      (device_path.b_resp),\
.bvalid     (device_path.b_valid),\
.bready     (device_path.b_ready),\
.buser      (device_path.b_user),\
\
.arid       (p_AXI_LITE_MONITOR_TRANS_ID),\
.araddr     (device_path.ar_bits.addr),\
.arlen      ('0),\
.arsize     (p_AXI_LITE_size),\
.arburst    ('0),\
.arlock     ('0),\
.arprot     (device_path.ar_bits.prot),\
.arqos      ('0),\
.arregion   ('0),\
.arcache    ('0),\
.arvalid    (device_path.ar_valid),\
.arready    (device_path.ar_ready),\
.aruser     ('0),\
\
.rid        (p_AXI_LITE_MONITOR_TRANS_ID),\
.rdata      (device_path.r_data),\
.rlast      (1'b1),\
.rvalid     (device_path.r_valid),\
.rready     (device_path.r_ready),\
.rresp      (device_path.r_resp),\
.ruser      ('0),\
\
.csysreq(1'b1),\
.csysack(0),\
.cactive(0)







`define WF_AXI_MONITOR_PARAMS_BLOCK \
.ID_WIDTH(SYSTEM_ID_WIDTH),\
.ADDR_WIDTH(SYSTEM_ADDR_WIDTH),\
.DATA_WIDTH(SYSTEM_DATA_WIDTH),\
.LEN_WIDTH(SYSTEM_LEN_WIDTH),\
\
.SIZE_WIDTH(3),\
.BURST_WIDTH(2),\
.CACHE_WIDTH(4),\
.PROT_WIDTH(3),\
.RESP_WIDTH(SYSTEM_RESP_WIDTH),\
.LOCK_WIDTH(1),\
.AWUSER_WIDTH(SYSTEM_USER_WIDTH),\
.WUSER_WIDTH(SYSTEM_USER_WIDTH),\
.BUSER_WIDTH(SYSTEM_USER_WIDTH),\
.ARUSER_WIDTH(SYSTEM_USER_WIDTH),\
.RUSER_WIDTH(SYSTEM_USER_WIDTH)



`define AXI_MONITOR_DEFPARAM(inst_name)\
defparam inst_name.COVERAGE_ON                =p_abvip_coverage_collect_on;\
defparam inst_name.CDNS_EXHAUSTIVE_COVER_ON   =p_abvip_coverage_collect_on;\
defparam inst_name.CONFIG_SIMULATION          =1;\
defparam inst_name.EXCL_ACCESS_ON             =p_abvip_excl_access_check;\
defparam inst_name.LOW_POWER_ON               =p_abvip_low_power_support;\
defparam inst_name.MAX_PENDING                =p_pending_trans_qnty;\
defparam inst_name.MAX_PENDING_EXCL           =p_pending_trans_qnty;\
defparam inst_name.BYTE_STROBE_ON             = 1;\
defparam inst_name.DATA_BEFORE_CONTROL_ON     = 0;\
defparam inst_name.READ_RESP_IN_ORDER_ON      = 1;\
defparam inst_name.WRITE_RESP_IN_ORDER_ON     = 1;\
defparam inst_name.XCHECKS_ON                 = 0;



module asic_top_tb;

    parameter real HALF_PERIOD = `SCLK_PERIOD / 2.0;

    //! Input reset. Active HIGH.
    logic                     i_rst;
    //! Differential clock (LVDS) positive/negaive signal.
    wire                     i_sclk_p;
    wire                     i_sclk_n;
    //! GPIO: [11:4] LEDs; [3:0] DIP switch
    wire [11:0] io_gpio;
    wire [11:0] io_gpio_in;
    reg [11:0] io_gpio_out;
    bit [11:0] io_gpio_dir;
    logic i_jtag_trst;
    logic i_jtag_tck;
    logic i_jtag_tms;
    logic i_jtag_tdi;
    logic o_jtag_tdo;
    logic o_jtag_vref;
    //! UART1 signals:
    logic                     i_uart1_rd;
    logic                    o_uart1_td;
    logic clk;
    int clk_cnt;


  initial begin
    clk = 0;
    io_gpio_dir = 12'h0;
  end

  always #HALF_PERIOD clk=~clk;

//  assign io_gpio = '0;
  assign (weak0, weak1) io_gpio[11:4]  = 8'h00;
  assign io_gpio[3:0] = 4'hF;

  // Tie high UART input pins:
  assign (weak0, weak1) i_uart1_rd = 1'b1;

  assign i_sclk_p = clk;
  assign i_sclk_n = ~clk;


  // always_latch begin

  //     for (int i = 0; i < 12; i++) begin
  //         //io_gpio_in(i) = io_gpio(i);
  //         //io_gpio(i) = (io_gpio_dir(i) == 1'b1) ? io_gpio_out(i) : 1'bZ;
  //     end
  // end

  always_ff@(posedge clk) begin
    if (clk_cnt <= 10) begin
        i_rst <= 1'b1;
    end else begin
        i_rst <= 1'b0;
    end
    clk_cnt <= clk_cnt + 1;
  end

  asic_top tt(
    .i_rst (i_rst),
    .i_sclk_p (i_sclk_p),
    .i_sclk_n (i_sclk_n),
    .io_gpio (io_gpio),
    //! JTAG signals:
    .i_jtag_trst(i_jtag_trst),
    .i_jtag_tck(i_jtag_tck),
    .i_jtag_tms(i_jtag_tms),
    .i_jtag_tdi(i_jtag_tdi),
    .o_jtag_tdo(o_jtag_tdo),
    .o_jtag_vref(o_jtag_vref),
    //! UART1 signals:
    .i_uart1_rd(i_uart1_rd),
    .o_uart1_td(o_uart1_td)
  );

  `ifdef SIMULATION_WFSOC_FPGA
    // Global signals for Xilinx unisim modules:
    glbl glbl();
  `endif

  sim_uart_rx
  #(
    .p_inst_num(0),
    `ifdef SIM_UART_SPEED
      .p_uart_clk_half_period   (`SIM_UART_SPEED) // Set UART speed in Makefile
    `else
//      .p_uart_clk_half_period   (3.125ns)
      .p_uart_clk_half_period   (270.3125ns) // True 115200 UART speed
    `endif
  ) UART_RX (
    .scaler  (32'd8),
    .rx      (o_uart1_td),
    .rst_n   (~i_rst),
    .clk_in  (1'b0)
  );

//tap_dpi #(
//    .HALF_PERIOD(33.7ns)   // some async value to system clock
//) tap0 (
//    .i_tdo(o_jtag_tdo),
//    .o_trst(i_jtag_trst),
//    .o_tck(i_jtag_tck),
//    .o_tms(i_jtag_tms),
//    .o_tdi(i_jtag_tdi)
//);
assign i_jtag_trst = 1'b0;
assign i_jtag_tck = 1'b0;
assign i_jtag_tms = 1'b0;
assign i_jtag_tdi = 1'b0;


//--------------------------------------------------------------------------------------------------
// ABVIPs
//--------------------------------------------------------------------------------------------------
import types_amba_pkg::*;
import types_bus0_pkg::*;

localparam SYSTEM_ID_WIDTH = CFG_SYSBUS_ID_BITS;
localparam SYSTEM_LEN_WIDTH = 8;
localparam SYSTEM_ADDR_WIDTH = CFG_SYSBUS_ADDR_BITS;
localparam SYSTEM_DATA_WIDTH = CFG_SYSBUS_DATA_BITS;
localparam SYSTEM_USER_WIDTH = CFG_SYSBUS_USER_BITS;
localparam SYSTEM_RESP_WIDTH = 2;
localparam p_pending_trans_qnty = 8;
localparam p_abvip_low_power_support = 0;
//localparam p_abvip_coverage_collect_on = 0;
localparam p_abvip_coverage_collect_on = 1;
localparam p_abvip_excl_access_check = 1;

localparam p_XIL_GPIO_AXI_ADDR_WIDTH = 32;
localparam p_XIL_GPIO_AXI_DATA_WIDTH = 32;
localparam p_XIL_GPIO_AXI_LITE_size = (p_XIL_GPIO_AXI_DATA_WIDTH==32)?(2):(3);

localparam p_AXI_LITE_MONITOR_TRANS_ID = 0;
/*

CFG_LOG2_SYSBUS_DATA_BYTES



*/




localparam p_AXI_IC_CPU_PORT_NMB = 0;
localparam p_AXI_IC_DPI_PORT_NMB = 1;
localparam p_AXI_IC_UART_PORT_NMB = 2;
localparam p_AXI_IC_JTAG_PORT_NMB = 3;




localparam p_AXI_IC_SLV_BOOTROM   =   0;
localparam p_AXI_IC_SLV_ROMIMAGE  =   1;
localparam p_AXI_IC_SLV_SRAM      =   2;
localparam p_AXI_IC_SLV_UART1     =   3;
localparam p_AXI_IC_SLV_GPIO      =   4;
localparam p_AXI_IC_SLV_IRQCTRL   =   5;
localparam p_AXI_IC_SLV_ENGINE    =   6;
localparam p_AXI_IC_SLV_RFCTRL    =   7;
localparam p_AXI_IC_SLV_FSE_GPS   =   8;
localparam p_AXI_IC_SLV_ETHMAC    =   9;
localparam p_AXI_IC_SLV_DSU       =  10;
localparam p_AXI_IC_SLV_GPTIMERS  =  11;
localparam p_AXI_IC_SLV_PNP       =  12;










`ifdef AXI_ABVIP_L2_OUTP

axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_CPU
    (`WF_AXI_MONITOR_MST_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_CPU_PORT_NMB)
        );

`AXI_MONITOR_DEFPARAM(AXI_MONITOR_CPU)
`endif //AXI_ABVIP_L2_OUTP

`ifdef AXI_ABVIP_ALL_MASTERS_OUTP
axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_DPI
    (`WF_AXI_MONITOR_MST_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_DPI_PORT_NMB)
        );

`AXI_MONITOR_DEFPARAM(AXI_MONITOR_DPI)

axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_UART
    (`WF_AXI_MONITOR_MST_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_UART_PORT_NMB)
        );

`AXI_MONITOR_DEFPARAM(AXI_MONITOR_UART)

axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_JTAG
    (`WF_AXI_MONITOR_MST_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_JTAG_PORT_NMB)
        );

`AXI_MONITOR_DEFPARAM(AXI_MONITOR_JTAG)
`endif //AXI_ABVIP_ALL_MASTERS_OUTP



`ifdef AXI_ABVIP_ALL_SLAVES_OUTP
axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_BOOTROM
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_BOOTROM)
        );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_BOOTROM)



axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_ROMIMAGE
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_ROMIMAGE)
        );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_ROMIMAGE)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_SRAM
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_SRAM)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_SRAM)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_UART1
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_UART1)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_UART1)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_GPIO
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_GPIO)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_GPIO)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_IRQCTRL
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_IRQCTRL)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_IRQCTRL)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_ENGINE
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_ENGINE)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_ENGINE)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_RFCTRL
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_RFCTRL)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_RFCTRL)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_FSE_GPS
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_FSE_GPS)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_FSE_GPS)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_ETHMAC
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_ETHMAC)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_ETHMAC)


axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_DSU
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_DSU)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_DSU)

axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_GPTIMERS
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_GPTIMERS)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_GPTIMERS)

axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_PNP
    (`WF_AXI_MONITOR_SLV_CONN(asic_top_tb.tt.soc0.ctrl0, p_AXI_IC_SLV_PNP)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_PNP)

//`define XIL_GPIO_PATH asic_top_tb.tt.soc0.ctrl0.XIL_GPIO
`define XIL_GPIO_PATH asic_top_tb.tt.soc0.ctrl0.XIL_IC

axi4_monitor
    //#(`WF_AXI_MONITOR_PARAMS_BLOCK )
    #(  .ADDR_WIDTH(p_XIL_GPIO_AXI_ADDR_WIDTH),
        .DATA_WIDTH(p_XIL_GPIO_AXI_DATA_WIDTH))
    AXI_MONITOR_LITE_XIL_GPIO
    (
     .aclk       (`XIL_GPIO_PATH.aclk),
     .aresetn    (`XIL_GPIO_PATH.aresetn),

     .awid       (p_AXI_LITE_MONITOR_TRANS_ID),
     .awaddr     (`XIL_GPIO_PATH.XILGPIO_AXI_S_awaddr),
     .awlen      ('0),
     .awsize     (p_XIL_GPIO_AXI_LITE_size),
     .awburst    ('0),
     .awlock     ('0),
     .awprot     (`XIL_GPIO_PATH.XILGPIO_AXI_S_awprot),
     .awqos      ('0),
     .awregion   ('0),
     .awcache    ('0),
     .awvalid    (`XIL_GPIO_PATH.XILGPIO_AXI_S_awvalid),
     .awready    (`XIL_GPIO_PATH.XILGPIO_AXI_S_awready),
     .awuser     ('0),

     .wdata      (`XIL_GPIO_PATH.XILGPIO_AXI_S_wdata),
     .wstrb      (`XIL_GPIO_PATH.XILGPIO_AXI_S_wstrb),
     .wlast      (1'b1),
     .wvalid     (`XIL_GPIO_PATH.XILGPIO_AXI_S_wvalid),
     .wready     (`XIL_GPIO_PATH.XILGPIO_AXI_S_wready),
     .wuser      ('0),

     .bid        (p_AXI_LITE_MONITOR_TRANS_ID),
     .bresp      (`XIL_GPIO_PATH.XILGPIO_AXI_S_bresp),
     .bvalid     (`XIL_GPIO_PATH.XILGPIO_AXI_S_bvalid),
     .bready     (`XIL_GPIO_PATH.XILGPIO_AXI_S_bready),
     .buser      ('0),

     .arid       (p_AXI_LITE_MONITOR_TRANS_ID),
     .araddr     (`XIL_GPIO_PATH.XILGPIO_AXI_S_araddr),
     .arlen      ('0),
     .arsize     (p_XIL_GPIO_AXI_LITE_size),
     .arburst    ('0),
     .arlock     ('0),
     .arprot     (`XIL_GPIO_PATH.XILGPIO_AXI_S_arprot),
     .arqos      ('0),
     .arregion   ('0),
     .arcache    ('0),
     .arvalid    (`XIL_GPIO_PATH.XILGPIO_AXI_S_arvalid),
     .arready    (`XIL_GPIO_PATH.XILGPIO_AXI_S_arready),
     .aruser     ('0),

     .rid        (p_AXI_LITE_MONITOR_TRANS_ID),
     .rdata      (`XIL_GPIO_PATH.XILGPIO_AXI_S_rdata),
     .rlast      (1'b1),
     .rvalid     (`XIL_GPIO_PATH.XILGPIO_AXI_S_rvalid),
     .rready     (`XIL_GPIO_PATH.XILGPIO_AXI_S_rready),
     .rresp      (`XIL_GPIO_PATH.XILGPIO_AXI_S_rresp),
     .ruser      ('0),

     .csysreq(1'b1),
     .csysack(0),
     .cactive(0)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_LITE_XIL_GPIO)





axi4_monitor
    #(`WF_AXI_MONITOR_PARAMS_BLOCK )
    AXI_MONITOR_ACP
    (`WF_AXI_MONITOR_SLV_CONN_SINGLE(asic_top_tb.tt.soc0.group0, i_clk, i_nrst, i_acpo, o_acpi)
     );
`AXI_MONITOR_DEFPARAM(AXI_MONITOR_ACP)


`endif //AXI_ABVIP_ALL_SLAVES_OUTP


  endmodule
