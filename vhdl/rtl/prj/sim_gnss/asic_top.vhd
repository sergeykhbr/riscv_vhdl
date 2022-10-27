--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

--! Standard library
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

--! Data transformation and math functions library
library commonlib;
use commonlib.types_common.all;

--! Technology definition library.
library techmap;
--! Technology constants definition.
use techmap.gencomp.all;
--! "Virtual" PLL declaration.
use techmap.types_pll.all;
-- "Virtual" memory banks
use techmap.types_mem.all;
--! "Virtual" buffers declaration.
use techmap.types_buf.all;

 --! Top-level implementaion library
library work;
--! Target dependable configuration: RTL, FPGA or ASIC.
use work.config_target.all;

entity asic_top is port 
( 
  --! Input reset. Active HIGH.
  i_rst     : in std_logic;
  --! Differential clock (LVDS) positive/negaive signal.
  i_sclk_p  : in std_logic;
  i_sclk_n  : in std_logic;
  --! GPIO: [11:4] LEDs; [3:0] DIP switch
  io_gpio     : inout std_logic_vector(11 downto 0);
  --! GPTimers
  o_pwm : out std_logic_vector(1 downto 0);
  --! JTAG signals:
  i_jtag_tck : in std_logic;
  i_jtag_ntrst : in std_logic;
  i_jtag_tms : in std_logic;
  i_jtag_tdi : in std_logic;
  o_jtag_tdo : out std_logic;
  o_jtag_vref : out std_logic;
  --! UART1 signals:
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  --! UART2 TAP (debug port) signals: DO NOT SUPPORT FIRMWARE OUTPUT!
  i_uart2_rd   : in std_logic;
  o_uart2_td   : out std_logic;
  --! SPI Flash/ext OTP
  i_flash_si : in std_logic;
  o_flash_so : out std_logic;
  o_flash_sck : out std_logic;
  o_flash_csn : out std_logic;
  -- OTP power
  io_otp_gnd : inout std_logic;
  io_otp_vdd  : inout std_logic;
  io_otp_vdd18 : inout std_logic;
  io_otp_upp : inout std_logic;
  --! Ethernet MAC PHY interface signals
  i_gmiiclk_p : in    std_ulogic;
  i_gmiiclk_n : in    std_ulogic;
  o_egtx_clk  : out   std_ulogic;
  i_etx_clk   : in    std_ulogic;
  i_erx_clk   : in    std_ulogic;
  i_erxd      : in    std_logic_vector(3 downto 0);
  i_erx_dv    : in    std_ulogic;
  i_erx_er    : in    std_ulogic;
  i_erx_col   : in    std_ulogic;
  i_erx_crs   : in    std_ulogic;
  i_emdint    : in std_ulogic;
  o_etxd      : out   std_logic_vector(3 downto 0);
  o_etx_en    : out   std_ulogic;
  o_etx_er    : out   std_ulogic;
  o_emdc      : out   std_ulogic;
  io_emdio    : inout std_logic;
  o_erstn     : out   std_ulogic;
  -- GNSS Sub-system signals:
  i_clk_adc : in std_logic;
  i_gps_I : in std_logic_vector(1 downto 0);
  i_gps_Q : in std_logic_vector(1 downto 0);
  i_glo_I : in std_logic_vector(1 downto 0);
  i_glo_Q : in std_logic_vector(1 downto 0);
  o_pps : out std_logic;
  i_gps_ld    : in std_logic;
  i_glo_ld    : in std_logic;
  o_max_sclk  : out std_logic;
  o_max_sdata : out std_logic;
  o_max_ncs   : out std_logic_vector(1 downto 0);
  i_antext_stat   : in std_logic;
  i_antext_detect : in std_logic;
  o_antext_ena    : out std_logic;
  o_antint_contr  : out std_logic
);
end asic_top;

architecture arch_asic_top of asic_top is

component riscv_soc is port 
( 
  i_rst     : in std_logic;
  i_clk  : in std_logic;
  --! GPIO.
  i_gpio     : in std_logic_vector(11 downto 0);
  o_gpio     : out std_logic_vector(11 downto 0);
  o_gpio_dir : out std_logic_vector(11 downto 0);
  --! GPTimers
  o_pwm : out std_logic_vector(1 downto 0);
  --! JTAG signals:
  i_jtag_tck : in std_logic;
  i_jtag_ntrst : in std_logic;
  i_jtag_tms : in std_logic;
  i_jtag_tdi : in std_logic;
  o_jtag_tdo : out std_logic;
  o_jtag_vref : out std_logic;
  --! UART1 signals:
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic;
  --! UART2 (debug port) signals:
  i_uart2_ctsn : in std_logic;
  i_uart2_rd   : in std_logic;
  o_uart2_td   : out std_logic;
  o_uart2_rtsn : out std_logic;
  --! SPI Flash
  i_flash_si : in std_logic;
  o_flash_so : out std_logic;
  o_flash_sck : out std_logic;
  o_flash_csn : out std_logic;
  o_flash_wpn : out std_logic;
  o_flash_holdn : out std_logic;
  o_flash_reset : out std_logic;
  --! OTP Memory
  i_otp_d : in std_logic_vector(15 downto 0);
  o_otp_d : out std_logic_vector(15 downto 0);
  o_otp_a : out std_logic_vector(11 downto 0);
  o_otp_we : out std_logic;
  o_otp_re : out std_logic;
  --! Ethernet MAC PHY interface signals
  i_etx_clk   : in    std_ulogic;
  i_erx_clk   : in    std_ulogic;
  i_erxd      : in    std_logic_vector(3 downto 0);
  i_erx_dv    : in    std_ulogic;
  i_erx_er    : in    std_ulogic;
  i_erx_col   : in    std_ulogic;
  i_erx_crs   : in    std_ulogic;
  i_emdint    : in std_ulogic;
  o_etxd      : out   std_logic_vector(3 downto 0);
  o_etx_en    : out   std_ulogic;
  o_etx_er    : out   std_ulogic;
  o_emdc      : out   std_ulogic;
  i_eth_mdio    : in std_logic;
  o_eth_mdio    : out std_logic;
  o_eth_mdio_oe : out std_logic;
  i_eth_gtx_clk    : in std_logic;
  i_eth_gtx_clk_90 : in std_logic;
  o_erstn     : out   std_ulogic;
  -- GNSS Sub-system signals:
  i_clk_adc : in std_logic;
  i_gps_I : in std_logic_vector(1 downto 0);
  i_gps_Q : in std_logic_vector(1 downto 0);
  i_glo_I : in std_logic_vector(1 downto 0);
  i_glo_Q : in std_logic_vector(1 downto 0);
  o_pps : out std_logic;
  i_gps_ld    : in std_logic;
  i_glo_ld    : in std_logic;
  o_max_sclk  : out std_logic;
  o_max_sdata : out std_logic;
  o_max_ncs   : out std_logic_vector(1 downto 0);
  i_antext_stat   : in std_logic;
  i_antext_detect : in std_logic;
  o_antext_ena    : out std_logic;
  o_antint_contr  : out std_logic
);
end component;

  signal ib_rst     : std_logic;
  signal ib_clk_tcxo : std_logic;
  signal ib_sclk_n  : std_logic;

  signal ob_gpio_direction : std_logic_vector(11 downto 0);
  signal ob_gpio_opins    : std_logic_vector(11 downto 0);
  signal ib_gpio_ipins     : std_logic_vector(11 downto 0);
  signal ob_pwm    : std_logic_vector(1 downto 0);
  signal ib_uart1_rd    : std_logic;
  signal ob_uart1_td    : std_logic;
  signal ib_uart2_rd    : std_logic;
  signal ob_uart2_td    : std_logic;
  signal ib_flash_si     : std_logic;
  signal ob_flash_so     : std_logic;
  signal ob_flash_sck    : std_logic;
  signal ob_flash_csn    : std_logic;
  --! JTAG signals:
  signal ib_jtag_tck    : std_logic;
  signal ib_jtag_ntrst  : std_logic;
  signal ib_jtag_tms    : std_logic;
  signal ib_jtag_tdi    : std_logic;
  signal ob_jtag_tdo    : std_logic;
  signal ob_jtag_vref   : std_logic;

  signal ib_gmiiclk : std_logic;
  signal ib_eth_mdio : std_logic;
  signal ob_eth_mdio : std_logic;
  signal ob_eth_mdio_oe : std_logic;
  signal w_eth_gtx_clk : std_logic;
  signal w_eth_gtx_clk_90 : std_logic;

  signal ib_clk_adc : std_logic;
  signal ib_gps_I : std_logic_vector(1 downto 0);
  signal ib_gps_Q : std_logic_vector(1 downto 0);
  signal ib_glo_I : std_logic_vector(1 downto 0);
  signal ib_glo_Q : std_logic_vector(1 downto 0);
  signal ob_pps : std_logic;
  signal ib_gps_ld : std_logic;
  signal ib_glo_ld : std_logic;
  signal ob_max_sclk : std_logic;
  signal ob_max_sdata : std_logic;
  signal ob_max_ncs : std_logic_vector(1 downto 0);
  signal ib_antext_stat : std_logic;
  signal ib_antext_detect : std_logic;
  signal ob_antext_ena : std_logic;
  signal ob_antint_contr : std_logic;

  signal w_ext_reset : std_ulogic; -- External system reset or PLL unlcoked. MUST NOT USED BY DEVICES.
  signal w_glob_rst  : std_ulogic; -- Global reset active HIGH
  signal w_glob_nrst : std_ulogic; -- Global reset active LOW
  signal w_soft_rst : std_ulogic; -- Software reset (acitve HIGH) from DSU
  signal w_bus_nrst : std_ulogic; -- Global reset and Soft Reset active LOW
  signal w_clk_bus  : std_ulogic; -- bus clock from the internal PLL (100MHz virtex6/40MHz Spartan6)
  signal w_pll_lock : std_ulogic; -- PLL status signal. 0=Unlocked; 1=locked.

  signal wb_otp_wdata : std_logic_vector(15 downto 0);
  signal wb_otp_addr : std_logic_vector(11 downto 0);
  signal w_otp_we : std_logic;
  signal w_otp_re : std_logic;
  signal wb_otp_rdata : std_logic_vector(15 downto 0);
  
begin

  --! PAD buffers:
  irst0   : ibuf_tech generic map(CFG_PADTECH) port map (ib_rst, i_rst);

  iclk0 : idsbuf_tech generic map (CFG_PADTECH) port map (
         i_sclk_p, i_sclk_n, ib_clk_tcxo);

  ird1 : ibuf_tech generic map(CFG_PADTECH) port map (ib_uart1_rd, i_uart1_rd);
  otd1 : obuf_tech generic map(CFG_PADTECH) port map (o_uart1_td, ob_uart1_td);

  ird2 : ibuf_tech generic map(CFG_PADTECH) port map (ib_uart2_rd, i_uart2_rd);
  otd2 : obuf_tech generic map(CFG_PADTECH) port map (o_uart2_td, ob_uart2_td);

  iflshsi : ibuf_tech generic map(CFG_PADTECH) port map (ib_flash_si, i_flash_si);
  oflshso : obuf_tech generic map(CFG_PADTECH) port map (o_flash_so, ob_flash_so);
  oflshsck : obuf_tech generic map(CFG_PADTECH) port map (o_flash_sck, ob_flash_sck);
  oflshcsn : obuf_tech generic map(CFG_PADTECH) port map (o_flash_csn, ob_flash_csn);

  gpiox : for i in 0 to 11 generate
    iob0  : iobuf_tech generic map(CFG_PADTECH) 
            port map (ib_gpio_ipins(i), io_gpio(i), ob_gpio_opins(i), ob_gpio_direction(i));
  end generate;

  pwmx : for i in 0 to 1 generate
    opwm0  : obuf_tech generic map(CFG_PADTECH) port map (o_pwm(i), ob_pwm(i));
  end generate;

  --! JTAG signals:
  ijtck0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_jtag_tck, i_jtag_tck);
  ijtrst0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_jtag_ntrst, i_jtag_ntrst);
  ijtms0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_jtag_tms, i_jtag_tms);
  ijtdi0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_jtag_tdi, i_jtag_tdi);
  ojtdo0 : obuf_tech generic map(CFG_PADTECH) port map (o_jtag_tdo, ob_jtag_tdo);
  ojvrf0 : obuf_tech generic map(CFG_PADTECH) port map (o_jtag_vref, ob_jtag_vref);

  igbebuf0 : igdsbuf_tech generic map (CFG_PADTECH) port map (
            i_gmiiclk_p, i_gmiiclk_n, ib_gmiiclk);
				
  iomdio : iobuf_tech generic map(CFG_PADTECH)
	        port map (ib_eth_mdio, io_emdio, ob_eth_mdio, ob_eth_mdio_oe);

  --! GNSS sub-system
  iclkadc0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_clk_adc, i_clk_adc);
  adcx : for i in 0 to 1 generate
      igpsi0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_gps_I(i), i_gps_I(i));
      igpsq0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_gps_Q(i), i_gps_Q(i));
      igloi0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_glo_I(i), i_glo_I(i));
      igloq0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_glo_Q(i), i_glo_Q(i));
  end generate;
  opps0 : obuf_tech generic map(CFG_PADTECH) port map (o_pps, ob_pps);
  igpsld0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_gps_ld, i_gps_ld);
  iglold0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_glo_ld, i_glo_ld);
  omaxclk0 : obuf_tech generic map(CFG_PADTECH) port map (o_max_sclk, ob_max_sclk);
  omaxdat0 : obuf_tech generic map(CFG_PADTECH) port map (o_max_sdata, ob_max_sdata);
  omaxcs0 : obuf_tech generic map(CFG_PADTECH) port map (o_max_ncs(0), ob_max_ncs(0));
  omaxcs1 : obuf_tech generic map(CFG_PADTECH) port map (o_max_ncs(1), ob_max_ncs(1));
  iantstat0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_antext_stat, i_antext_stat);
  iantdet0 : ibuf_tech generic map(CFG_PADTECH) port map (ib_antext_detect, i_antext_detect);
  oanten0 : obuf_tech generic map(CFG_PADTECH) port map (o_antext_ena, ob_antext_ena);
  oantctr0 : obuf_tech generic map(CFG_PADTECH) port map (o_antint_contr, ob_antint_contr);

  --! Gigabit clock phase rotator with buffers
  clkrot90 : clkp90_tech  generic map (
      tech    => CFG_FABTECH,
      freq    => 125000   -- KHz = 125 MHz
    ) port map (
      i_rst    => ib_rst,
      i_clk    => ib_gmiiclk,
      o_clk    => w_eth_gtx_clk,
      o_clkp90 => w_eth_gtx_clk_90,
      o_clk2x  => open, -- used in gbe 'io_ref'
      o_lock   => open
    );

  o_egtx_clk <= w_eth_gtx_clk;

  ------------------------------------
  -- @brief Internal PLL device instance.
  pll0 : SysPLL_tech generic map (
    tech => CFG_FABTECH
  ) port map (
    i_reset     => ib_rst,
    i_clk_tcxo	=> ib_clk_tcxo,
    o_clk_bus   => w_clk_bus,
    o_locked    => w_pll_lock
  );
  w_ext_reset <= ib_rst or not w_pll_lock;


  otp0 : otp_tech generic map (
    memtech => CFG_MEMTECH
  ) port map (
    clk      => w_clk_bus,  -- only for FPGA
    i_we     => w_otp_we,
    i_re     => w_otp_re,
    i_addr   => wb_otp_addr,
    i_wdata  => wb_otp_wdata,
    o_rdata  => wb_otp_rdata,
    io_gnd   => io_otp_gnd,
    io_vdd   => io_otp_vdd,
    io_vdd18 => io_otp_vdd18,
    io_upp   => io_otp_upp
  );


  soc0 : riscv_soc port map
  ( 
    i_rst  => w_ext_reset,
    i_clk  => w_clk_bus,
    --! GPIO.
    i_gpio     => ib_gpio_ipins,
    o_gpio     => ob_gpio_opins,
    o_gpio_dir => ob_gpio_direction,
    --! GPTimers
    o_pwm => ob_pwm,
    --! JTAG signals:
    i_jtag_tck => ib_jtag_tck,
    i_jtag_ntrst => ib_jtag_ntrst,
    i_jtag_tms => ib_jtag_tms,
    i_jtag_tdi => ib_jtag_tdi,
    o_jtag_tdo => ob_jtag_tdo,
    o_jtag_vref => ob_jtag_vref,
    --! UART1 signals:
    i_uart1_ctsn => '0',
    i_uart1_rd   => ib_uart1_rd,
    o_uart1_td   => ob_uart1_td,
    o_uart1_rtsn => open,
    --! UART2 (debug port) signals:
    i_uart2_ctsn => '0',
    i_uart2_rd   => ib_uart2_rd,
    o_uart2_td   => ob_uart2_td,
    o_uart2_rtsn => open,
  --! SPI Flash
    i_flash_si => ib_flash_si,
    o_flash_so => ob_flash_so,
    o_flash_sck => ob_flash_sck,
    o_flash_csn => ob_flash_csn,
    o_flash_wpn => open,
    o_flash_holdn => open,
    o_flash_reset => open,
    --! OTP Memory
    i_otp_d => wb_otp_rdata,
    o_otp_d => wb_otp_wdata,
    o_otp_a => wb_otp_addr,
    o_otp_we => w_otp_we,
    o_otp_re => w_otp_re,
    --! Ethernet MAC PHY interface signals
    i_etx_clk   => i_etx_clk,
    i_erx_clk   => i_erx_clk,
    i_erxd      => i_erxd,
    i_erx_dv    => i_erx_dv,
    i_erx_er    => i_erx_er,
    i_erx_col   => i_erx_col,
    i_erx_crs   => i_erx_crs,
    i_emdint    => i_emdint,
    o_etxd      => o_etxd,
    o_etx_en    => o_etx_en,
    o_etx_er    => o_etx_er,
    o_emdc      => o_emdc,
    i_eth_mdio => ib_eth_mdio,
    o_eth_mdio => ob_eth_mdio,
    o_eth_mdio_oe => ob_eth_mdio_oe,
    i_eth_gtx_clk => w_eth_gtx_clk,
    i_eth_gtx_clk_90 => w_eth_gtx_clk_90,
    o_erstn     => o_erstn,
    -- GNSS Sub-system signals:
    i_clk_adc => ib_clk_adc,
    i_gps_I => ib_gps_I,
    i_gps_Q => ib_gps_Q,
    i_glo_I => ib_glo_I,
    i_glo_Q => ib_glo_Q,
    o_pps => ob_pps,
    i_gps_ld => ib_gps_ld,
    i_glo_ld => ib_glo_ld,
    o_max_sclk => ob_max_sclk,
    o_max_sdata => ob_max_sdata,
    o_max_ncs => ob_max_ncs,
    i_antext_stat => ib_antext_stat,
    i_antext_detect => ib_antext_detect,
    o_antext_ena => ob_antext_ena,
    o_antint_contr => ob_antint_contr
  );
  
end arch_asic_top;
