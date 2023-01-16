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

`ifndef SCLK_PERIOD
  // FPGA 200 MHz
  `define SCLK_PERIOD 5ns
`endif


module kc705_top_tb;

    parameter real HALF_PERIOD = `SCLK_PERIOD / 2.0;

    localparam DDR3_CS_WIDTH              = 1;
    localparam DDR3_MEMORY_WIDTH          = 8;
    localparam DDR3_DQ_WIDTH              = 64;
    localparam DDR3_DQS_WIDTH             = 8;
    localparam DDR3_NUM_COMP              = DDR3_DQ_WIDTH/DDR3_MEMORY_WIDTH;

    //! Input reset. Active HIGH.
    logic i_rst;
    logic sys_rst_n;
    //! Differential clock (LVDS) positive/negaive signal.
    wire i_sclk_p;
    wire i_sclk_n;
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
    logic i_uart1_rd;
    logic o_uart1_td;
    // SPI SD-card signals:
    logic o_spi_cs;
    logic o_spi_sclk;
    logic o_spi_mosi;
    logic i_spi_miso;
    logic i_sd_detected;
    logic i_sd_protect;
    // ddr3
    wire o_ddr3_reset_n;
    wire [0:0] o_ddr3_ck_n;
    wire [0:0] o_ddr3_ck_p;
    wire [0:0] o_ddr3_cke;
    wire [0:0] o_ddr3_cs_n;
    wire o_ddr3_ras_n;
    wire o_ddr3_cas_n;
    wire o_ddr3_we_n;
    wire [7:0] o_ddr3_dm;
    wire [2:0] o_ddr3_ba;
    wire [13:0] o_ddr3_addr;
    wire [63:0] io_ddr3_dq;
    wire [7:0] io_ddr3_dqs_p;
    wire [7:0] io_ddr3_dqs_n;
    wire [0:0] o_ddr3_odt;
    wire o_ddr3_init_calib_complete;
    // delayed
    wire [DDR3_DQ_WIDTH-1:0]  wb_ddr3_dq_sdram;
    wire [DDR3_DQS_WIDTH-1:0] wb_ddr3_dqs_p_sdram;
    wire [DDR3_DQS_WIDTH-1:0] wb_ddr3_dqs_n_sdram;


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
  assign i_uart1_rd = 1'b1;

  assign i_sd_detected = 1'b1;
  assign i_sd_protect = 1'b0;

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
  assign sys_rst_n = ~i_rst;

  kc705_top #(
    .SIM_BYPASS_INIT_CAL("FAST"),  // "FAST"-for simulation true; "OFF"
    .SIMULATION("TRUE")
  ) tt (
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
    .o_uart1_td(o_uart1_td),
    // SPI SD-card signals:
    .o_spi_cs(o_spi_cs),
    .o_spi_sclk(o_spi_sclk),
    .o_spi_mosi(o_spi_mosi),
    .i_spi_miso(i_spi_miso),
    .i_sd_detected(i_sd_detected),
    .i_sd_protect(i_sd_protect),
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
    .io_ddr3_dqs_p(io_ddr3_dqs_p),
    .io_ddr3_dqs_n(io_ddr3_dqs_n),
    .o_ddr3_odt(o_ddr3_odt),
    .o_ddr3_init_calib_complete(o_ddr3_init_calib_complete)
  );

  // Global signals for Xilinx unisim modules:
  glbl glbl();

  sim_uart_rx
  #(
    .p_inst_num(0),
//      .p_uart_clk_half_period   (3.125ns)
      .p_uart_clk_half_period   (270.3125ns) // True 115200 UART speed
  ) UART_RX (
    .scaler  (32'd8),
    .rx      (o_uart1_td),
    .rst_n   (~i_rst),
    .clk_in  (1'b0)
  );

  sd_hc #(
    .half_period_clk(50ns), // 20 MHz = 50ns
    .block_size(512)
  ) SD0 (
    .i_csn(o_spi_cs),
    .i_sck(o_spi_sclk),
    .i_mosi(o_spi_mosi),
    .o_miso(i_spi_miso)
  );


  //===========================================================================
  // DDR3 env. simulation:
  //===========================================================================
  genvar dqwd;
  generate
    for (dqwd = 1; dqwd < DDR3_DQ_WIDTH; dqwd = dqwd+1) begin : dq_delay
      WireDelay # (
        .Delay_g    (0.00),
        .Delay_rd   (0.00),
        .ERR_INSERT ("OFF")
       ) u_delay_dq (
        .A             (io_ddr3_dq[dqwd]),
        .B             (wb_ddr3_dq_sdram[dqwd]),
        .reset         (sys_rst_n),
        .phy_init_done (o_ddr3_init_calib_complete)
       );
    end
    WireDelay # (
      .Delay_g    (0.00),
      .Delay_rd   (0.00),
      .ERR_INSERT ("OFF")
    ) u_delay_dq_0 (
      .A             (io_ddr3_dq[0]),
      .B             (wb_ddr3_dq_sdram[0]),
      .reset         (sys_rst_n),
      .phy_init_done (o_ddr3_init_calib_complete)
    );
  endgenerate

  genvar dqswd;
  generate
    for (dqswd = 0; dqswd < DDR3_DQS_WIDTH; dqswd = dqswd+1) begin : dqs_delay
      WireDelay # (
        .Delay_g    (0.00),
        .Delay_rd   (0.00),
        .ERR_INSERT ("OFF")
       ) u_delay_dqs_p (
        .A             (io_ddr3_dqs_p[dqswd]),
        .B             (wb_ddr3_dqs_p_sdram[dqswd]),
        .reset         (sys_rst_n),
        .phy_init_done (o_ddr3_init_calib_complete)
       );

      WireDelay # (
        .Delay_g    (0.00),
        .Delay_rd   (0.00),
        .ERR_INSERT ("OFF")
      ) u_delay_dqs_n (
        .A             (io_ddr3_dqs_n[dqswd]),
        .B             (wb_ddr3_dqs_n_sdram[dqswd]),
        .reset         (sys_rst_n),
        .phy_init_done (o_ddr3_init_calib_complete)
       );
    end
  endgenerate

  genvar r,i;
  generate
    for (r = 0; r < DDR3_CS_WIDTH; r = r + 1) begin: mem_rnk
      for (i = 0; i < DDR3_NUM_COMP; i = i + 1) begin: gen_mem
        ddr3_model u_comp_ddr3
          (
           .rst_n   (o_ddr3_reset_n),
           .ck      (o_ddr3_ck_p[(i*DDR3_MEMORY_WIDTH)/72]),
           .ck_n    (o_ddr3_ck_n[(i*DDR3_MEMORY_WIDTH)/72]),
           .cke     (o_ddr3_cke[((i*DDR3_MEMORY_WIDTH)/72)+(1*r)]),
           .cs_n    (o_ddr3_cs_n[((i*DDR3_MEMORY_WIDTH)/72)+(1*r)]),
           .ras_n   (o_ddr3_ras_n),
           .cas_n   (o_ddr3_cas_n),
           .we_n    (o_ddr3_we_n),
           .dm_tdqs (o_ddr3_dm[i]),
           .ba      (o_ddr3_ba), //[r]), // mirror_ca is disabled
           .addr    (o_ddr3_addr), //[r]), // mirror_ca is disabled
           .dq      (wb_ddr3_dq_sdram[DDR3_MEMORY_WIDTH*(i+1)-1:DDR3_MEMORY_WIDTH*(i)]),
           .dqs     (wb_ddr3_dqs_p_sdram[i]),
           .dqs_n   (wb_ddr3_dqs_n_sdram[i]),
           .tdqs_n  (),
           .odt     (o_ddr3_odt[((i*DDR3_MEMORY_WIDTH)/72)+(1*r)])
           );
      end
    end
  endgenerate


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

endmodule
