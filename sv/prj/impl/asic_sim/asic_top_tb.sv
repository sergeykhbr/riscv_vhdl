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
    // SPI SD-card signals:
    logic o_spi_cs;
    logic o_spi_sclk;
    logic o_spi_mosi;
    logic i_spi_miso;
    logic i_sd_detected;
    logic i_sd_protect;

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

  assign i_sd_detected = 1'b1;
  assign i_sd_protect = 1'b0;

  // always_latch begin

  //     for (int i = 0; i < 12; i++) begin
  //         //io_gpio_in(i) = io_gpio(i);
  //         //io_gpio(i) = (io_gpio_dir(i) == 1'b1) ? io_gpio_out(i) : 1'bZ;
  //     end
  // end

  always_ff@(posedge clk) begin
    if (clk_cnt <= 10) begin
//    if (clk_cnt <= 10 || (clk_cnt > 1000010 && clk_cnt < 1000050)) begin
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
    .o_uart1_td(o_uart1_td),
    // SD-card SPI signals
    .o_spi_cs(o_spi_cs),
    .o_spi_sclk(o_spi_sclk),
    .o_spi_mosi(o_spi_mosi),
    .i_spi_miso(i_spi_miso),
    .i_sd_detected(i_sd_detected),
    .i_sd_protect(i_sd_protect)
  );

  `ifdef SIMULATION_WFSOC_FPGA
    // Global signals for Xilinx unisim modules:
    glbl glbl();
  `endif

  sim_uart_rx
  #(
    .p_inst_num(0),
//      .p_uart_clk_half_period   (3.125ns)
      .p_uart_clk_half_period   (270.3125ns / (2**config_target_pkg::CFG_UART_SPEED_UP_RATE)) // True 115200 UART speed
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
