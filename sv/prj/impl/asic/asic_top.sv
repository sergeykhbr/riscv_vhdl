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

module asic_top #(
    parameter int sim_uart_speedup_rate = 0                 // simulation UART speed-up: 0=no speed up, 1=2x, 2=4x, etc
)
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
    // SD-card signals:
    output logic o_sd_sclk,
    inout logic io_sd_cmd,                                  // CMD IO Command/Resonse; Data output in SPI mode
    inout logic io_sd_dat0,                                 // Data[0] IO; Data input in SPI mode
    inout logic io_sd_dat1,
    inout logic io_sd_dat2,
    inout logic io_sd_cd_dat3,                              // CD/DAT3 IO CardDetect/Data[3]; CS output in SPI mode
    input logic i_sd_detected,                              // SD-card detected
    input logic i_sd_protect                                // SD-card write protect
);

  import types_amba_pkg::*;
  import types_pnp_pkg::*;
  import config_target_pkg::*;

  logic             ib_rst;
  logic             ib_clk_tcxo;
  logic             ib_sclk_n;  

  logic [11:0]      ob_gpio_direction;
  logic [11:0]      ob_gpio_opins;
  logic [11:0]      ib_gpio_ipins;  
  logic ib_sd_cmd;
  logic ob_sd_cmd;
  logic ob_sd_cmd_direction;
  logic ib_sd_dat0;
  logic ob_sd_dat0;
  logic ob_sd_dat0_direction;
  logic ib_sd_dat1;
  logic ob_sd_dat1;
  logic ob_sd_dat1_direction;
  logic ib_sd_dat2;
  logic ob_sd_dat2;
  logic ob_sd_dat2_direction;
  logic ib_sd_cd_dat3;
  logic ob_sd_cd_dat3;
  logic ob_sd_cd_dat3_direction;
  logic             ib_uart1_rd;  
  logic             ob_uart1_td;  
  //! JTAG signals:  
  logic             ib_jtag_tck;  
  logic             ib_jtag_trst; 
  logic             ib_jtag_tms; 
  logic             ib_jtag_tdi; 
  logic             ob_jtag_tdo; 
  logic             ob_jtag_vref;   
  // SPI SD-card signals:
  logic ob_spi_cs;
  logic ob_spi_sclk;
  logic ob_spi_mosi;
  logic ib_spi_miso;
  logic ib_sd_detected;
  logic ib_sd_protect;

  logic             w_sys_rst;
  logic             w_sys_nrst;
  logic             w_dbg_nrst;
  logic             w_dmreset;
  logic             w_sys_clk;
  logic             w_ddr_clk;
  logic             w_pll_lock;

  // DDR interface
  mapinfo_type ddr_xmapinfo;
  dev_config_type ddr_xdev_cfg;
  axi4_slave_out_type ddr_xslvo;
  axi4_slave_in_type ddr_xslvi;

  mapinfo_type ddr_pmapinfo;
  dev_config_type ddr_pdev_cfg;
  apb_in_type ddr_apbi;
  apb_out_type ddr_apbo;

  logic w_ddr_ui_nrst;
  logic w_ddr_ui_clk;
  logic w_ddr3_init_calib_complete;

  // PRCI intefrace:
  mapinfo_type prci_pmapinfo;
  dev_config_type prci_dev_cfg;
  apb_in_type prci_apbi;
  apb_out_type prci_apbo;
  

  ibuf_tech irst0(.o(ib_rst),.i(i_rst));
  
  idsbuf_tech iclk0(.clk_p(i_sclk_p), .clk_n(i_sclk_n), .o_clk(ib_clk_tcxo));
  
  ibuf_tech ird1(.o(ib_uart1_rd),.i(i_uart1_rd));
  obuf_tech otd1(.o(o_uart1_td),.i(ob_uart1_td));

  genvar i;
  generate 
    for(i=0; i<=11; i++) begin: gpiox  
      iobuf_tech iob0(.o(ib_gpio_ipins[i]), .io(io_gpio[i]), .i(ob_gpio_opins[i]), .t(ob_gpio_direction[i])); 
    end
  endgenerate

iobuf_tech iosdcmd0 (
    .io(io_sd_cmd),
    .o(ib_sd_cmd),
    .i(ob_sd_cmd),
    .t(ob_sd_cmd_direction)
);


iobuf_tech iosddat0 (
    .io(io_sd_dat0),
    .o(ib_sd_dat0),
    .i(ob_sd_dat0),
    .t(ob_sd_dat0_direction)
);


iobuf_tech iosddat1 (
    .io(io_sd_dat1),
    .o(ib_sd_dat1),
    .i(ob_sd_dat1),
    .t(ob_sd_dat1_direction)
);


iobuf_tech iosddat2 (
    .io(io_sd_dat2),
    .o(ib_sd_dat2),
    .i(ob_sd_dat2),
    .t(ob_sd_dat2_direction)
);


iobuf_tech iosddat3 (
    .io(io_sd_cd_dat3),
    .o(ib_sd_cd_dat3),
    .i(ob_sd_cd_dat3),
    .t(ob_sd_cd_dat3_direction)
);

  ibuf_tech isddet0(.o(ib_sd_detected),.i(i_sd_detected));  
  ibuf_tech isdwp0(.o(ib_sd_protect),.i(i_sd_protect));  
  
  ibuf_tech ijtck0(.o(ib_jtag_tck),.i(i_jtag_tck));  
  ibuf_tech ijtrst0(.o(ib_jtag_trst),.i(i_jtag_trst)); 
  ibuf_tech ijtms0(.o(ib_jtag_tms),.i(i_jtag_tms));   
  ibuf_tech ijtdi0(.o(ib_jtag_tdi),.i(i_jtag_tdi)); 
  obuf_tech ojtdo0(.o(o_jtag_tdo),.i(ob_jtag_tdo));   
  obuf_tech ojvrf0(.o(o_jtag_vref),.i(ob_jtag_vref)); 
  
  //assign o_ddr3_init_calib_complete = w_ddr3_init_calib_complete;

  SysPLL_tech pll0(
    .i_reset(ib_rst),
    .i_clk_tcxo(ib_clk_tcxo),
    .o_clk_sys(w_sys_clk),
    .o_clk_ddr(w_ddr_clk),
    .o_locked(w_pll_lock)
  );  

  
  // PLL and Reset Control Interface:
  apb_prci #(
    .async_reset(async_reset)
  ) prci0 (
    .i_clk(ib_clk_tcxo),
    .i_pwrreset(ib_rst),
    .i_dmireset(w_dmreset),
    .i_sys_locked(w_pll_lock),
    .i_ddr_locked(w_ddr3_init_calib_complete),
    .o_sys_rst(w_sys_rst),
    .o_sys_nrst(w_sys_nrst),
    .o_dbg_nrst(w_dbg_nrst),
    .i_mapinfo(prci_pmapinfo),
    .o_cfg(prci_dev_cfg),
    .i_apbi(prci_apbi),
    .o_apbo(prci_apbo)
  );
  
  riscv_soc #(
    .async_reset(async_reset),
    .cpu_num(CFG_CPU_NUM),
    .ilog2_nways(CFG_ILOG2_NWAYS),
    .ilog2_lines_per_way(CFG_ILOG2_LINES_PER_WAY),
    .dlog2_nways(CFG_DLOG2_NWAYS),
    .dlog2_lines_per_way(CFG_DLOG2_LINES_PER_WAY),
    .l2cache_ena(CFG_L2CACHE_ENA),
    .l2log2_nways(CFG_L2_LOG2_NWAYS),
    .l2log2_lines_per_way(CFG_L2_LOG2_LINES_PER_WAY),
    .bootrom_log2_size(CFG_BOOTROM_LOG2_SIZE),
    .sram_log2_size(CFG_SRAM_LOG2_SIZE),
    .bootfile(CFG_BOOTROM_FILE_HEX),
    .sim_uart_speedup_rate(sim_uart_speedup_rate)
  ) soc0 (
    .i_sys_nrst (w_sys_nrst),
    .i_sys_clk (w_sys_clk),
    .i_dbg_nrst(w_dbg_nrst),
    .i_ddr_nrst (w_ddr_ui_nrst),
    .i_ddr_clk (w_ddr_ui_clk),
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
    // SD-card signals:
    .o_sd_sclk(o_sd_sclk),
    .i_sd_cmd(ib_sd_cmd),
    .o_sd_cmd(ob_sd_cmd),
    .o_sd_cmd_dir(ob_sd_cmd_direction),
    .i_sd_dat0(ib_sd_dat0),
    .o_sd_dat0(ob_sd_dat0),
    .o_sd_dat0_dir(ob_sd_dat0_direction),
    .i_sd_dat1(ib_sd_dat1),
    .o_sd_dat1(ob_sd_dat1),
    .o_sd_dat1_dir(ob_sd_dat1_direction),
    .i_sd_dat2(ib_sd_dat2),
    .o_sd_dat2(ob_sd_dat2),
    .o_sd_dat2_dir(ob_sd_dat2_direction),
    .i_sd_cd_dat3(ib_sd_cd_dat3),
    .o_sd_cd_dat3(ob_sd_cd_dat3),
    .o_sd_cd_dat3_dir(ob_sd_cd_dat3_direction),
    .i_sd_detected(ib_sd_detected),
    .i_sd_protect(ib_sd_protect),
    // PRCI:
    .o_dmreset(w_dmreset),
    .o_prci_pmapinfo(prci_pmapinfo),
    .i_prci_pdevcfg(prci_dev_cfg),
    .o_prci_apbi(prci_apbi),
    .i_prci_apbo(prci_apbo),
    // DDR:
    .o_ddr_pmapinfo(ddr_pmapinfo),
    .i_ddr_pdevcfg(ddr_pdev_cfg),
    .o_ddr_apbi(ddr_apbi),
    .i_ddr_apbo(ddr_apbo),
    .o_ddr_xmapinfo(ddr_xmapinfo),
    .i_ddr_xdevcfg(ddr_xdev_cfg),
    .o_ddr_xslvi(ddr_xslvi),
    .i_ddr_xslvo(ddr_xslvo)
  );


ddr_tech #(
    .async_reset(async_reset),
    .SYSCLK_TYPE("NO_BUFFER"), // "NO_BUFFER,"DIFFERENTIAL"
    .SIM_BYPASS_INIT_CAL("FAST"),  // "FAST"-for simulation true; "OFF"
    .SIMULATION("TRUE")
//    .SIM_BYPASS_INIT_CAL("OFF"),  // "FAST"-for simulation true; "OFF"
//    .SIMULATION("FALSE")
) ddr0 (
     // AXI memory access (ddr clock)
    .i_xslv_nrst(w_sys_nrst),
    .i_xslv_clk(ib_clk_tcxo),
    .i_xmapinfo(ddr_xmapinfo),
    .o_xcfg(ddr_xdev_cfg),
    .i_xslvi(ddr_xslvi),
    .o_xslvo(ddr_xslvo),
    // APB control interface (sys clock):
    .i_apb_nrst(w_sys_nrst),
    .i_apb_clk(w_sys_clk),
    .i_pmapinfo(ddr_pmapinfo),
    .o_pcfg(ddr_pdev_cfg),
    .i_apbi(ddr_apbi),
    .o_apbo(ddr_apbo),
    // to SOC:
    .o_ui_nrst(w_ddr_ui_nrst),  // xilinx generte ddr clock inside ddr controller
    .o_ui_clk(w_ddr_ui_clk),  // xilinx generte ddr clock inside ddr controller
    // DDR signals:
    .io_ddr3_dq(),
    .io_ddr3_dqs_n(),
    .io_ddr3_dqs_p(),
    .o_ddr3_addr(),
    .o_ddr3_ba(),
    .o_ddr3_ras_n(),
    .o_ddr3_cas_n(),
    .o_ddr3_we_n(),
    .o_ddr3_reset_n(),
    .o_ddr3_ck_p(),
    .o_ddr3_ck_n(),
    .o_ddr3_cke(),
    .o_ddr3_cs_n(),
    .o_ddr3_dm(),
    .o_ddr3_odt(),
    .o_init_calib_done(w_ddr3_init_calib_complete)
);

  
endmodule