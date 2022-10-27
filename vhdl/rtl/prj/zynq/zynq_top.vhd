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

library unisim;
use unisim.vcomponents.all;

--! Data transformation and math functions library
library commonlib;
use commonlib.types_common.all;

--! Technology definition library.
library techmap;
--! Technology constants definition.
use techmap.gencomp.all;
--! "Virtual" PLL declaration.
use techmap.types_pll.all;
--! "Virtual" buffers declaration.
use techmap.types_buf.all;

 --! Top-level implementaion library
library work;
--! Target dependable configuration: RTL, FPGA or ASIC.
use work.config_target.all;

--! Warning: this project wasn't verified on real FPGA (2018 Nov 18). No board is available.
entity zynq_top is port 
( 
  io_gpio   : inout std_logic_vector(11 downto 0);
  --! UART1 signals:
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  --! UART2 (TAP) signals:
  i_uart2_rd   : in std_logic;
  o_uart2_td   : out std_logic;
  --! JTAG
  i_jtag_tck : in std_logic;
  i_jtag_ntrst : in std_logic;
  i_jtag_tms : in std_logic;
  i_jtag_tdi : in std_logic;
  o_jtag_tdo : out std_logic;
  o_jtag_vref : out std_logic
);
end zynq_top;

architecture arch_zynq_top of zynq_top is

  component riscv_soc is port ( 
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

  COMPONENT processing_system7_0
  PORT (
    M_AXI_GP0_ARVALID : OUT STD_LOGIC;
    M_AXI_GP0_AWVALID : OUT STD_LOGIC;
    M_AXI_GP0_BREADY : OUT STD_LOGIC;
    M_AXI_GP0_RREADY : OUT STD_LOGIC;
    M_AXI_GP0_WLAST : OUT STD_LOGIC;
    M_AXI_GP0_WVALID : OUT STD_LOGIC;
    M_AXI_GP0_ARID : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
    M_AXI_GP0_AWID : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
    M_AXI_GP0_WID : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
    M_AXI_GP0_ARBURST : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_ARLOCK : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_ARSIZE : OUT STD_LOGIC_VECTOR(2 DOWNTO 0);
    M_AXI_GP0_AWBURST : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_AWLOCK : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_AWSIZE : OUT STD_LOGIC_VECTOR(2 DOWNTO 0);
    M_AXI_GP0_ARPROT : OUT STD_LOGIC_VECTOR(2 DOWNTO 0);
    M_AXI_GP0_AWPROT : OUT STD_LOGIC_VECTOR(2 DOWNTO 0);
    M_AXI_GP0_ARADDR : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    M_AXI_GP0_AWADDR : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    M_AXI_GP0_WDATA : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    M_AXI_GP0_ARCACHE : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_ARLEN : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_ARQOS : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_AWCACHE : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_AWLEN : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_AWQOS : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_WSTRB : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    M_AXI_GP0_ACLK : IN STD_LOGIC;
    M_AXI_GP0_ARREADY : IN STD_LOGIC;
    M_AXI_GP0_AWREADY : IN STD_LOGIC;
    M_AXI_GP0_BVALID : IN STD_LOGIC;
    M_AXI_GP0_RLAST : IN STD_LOGIC;
    M_AXI_GP0_RVALID : IN STD_LOGIC;
    M_AXI_GP0_WREADY : IN STD_LOGIC;
    M_AXI_GP0_BID : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    M_AXI_GP0_RID : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    M_AXI_GP0_BRESP : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_RRESP : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
    M_AXI_GP0_RDATA : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    FCLK_CLK0 : OUT STD_LOGIC;
    FCLK_RESET0_N : OUT STD_LOGIC;
    MIO : INOUT STD_LOGIC_VECTOR(53 DOWNTO 0);
    DDR_CAS_n : INOUT STD_LOGIC;
    DDR_CKE : INOUT STD_LOGIC;
    DDR_Clk_n : INOUT STD_LOGIC;
    DDR_Clk : INOUT STD_LOGIC;
    DDR_CS_n : INOUT STD_LOGIC;
    DDR_DRSTB : INOUT STD_LOGIC;
    DDR_ODT : INOUT STD_LOGIC;
    DDR_RAS_n : INOUT STD_LOGIC;
    DDR_WEB : INOUT STD_LOGIC;
    DDR_BankAddr : INOUT STD_LOGIC_VECTOR(2 DOWNTO 0);
    DDR_Addr : INOUT STD_LOGIC_VECTOR(14 DOWNTO 0);
    DDR_VRN : INOUT STD_LOGIC;
    DDR_VRP : INOUT STD_LOGIC;
    DDR_DM : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    DDR_DQ : INOUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    DDR_DQS_n : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    DDR_DQS : INOUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    PS_SRSTB : INOUT STD_LOGIC;
    PS_CLK : INOUT STD_LOGIC;
    PS_PORB : INOUT STD_LOGIC
  );
  END COMPONENT;

  signal FCLK_RESET0_N : std_logic;
  signal FCLK_RESET0 : std_logic;

  signal locked : std_logic;
  signal w_ext_clk : std_logic;
  signal w_ext_clk_buf : std_logic;
  signal w_pll_clk : std_logic;
  signal w_pll_lock : std_logic;
  signal w_rst : std_logic;

  signal ob_gpio_direction : std_logic_vector(11 downto 0);
  signal ob_gpio_opins    : std_logic_vector(11 downto 0);
  signal ib_gpio_ipins     : std_logic_vector(11 downto 0);

begin

  procsys0 : processing_system7_0
  PORT MAP (
    M_AXI_GP0_ARVALID => open,
    M_AXI_GP0_AWVALID => open,
    M_AXI_GP0_BREADY => open,
    M_AXI_GP0_RREADY => open,
    M_AXI_GP0_WLAST => open,
    M_AXI_GP0_WVALID => open,
    M_AXI_GP0_ARID => open,
    M_AXI_GP0_AWID => open,
    M_AXI_GP0_WID => open,
    M_AXI_GP0_ARBURST => open,
    M_AXI_GP0_ARLOCK => open,
    M_AXI_GP0_ARSIZE => open,
    M_AXI_GP0_AWBURST => open,
    M_AXI_GP0_AWLOCK => open,
    M_AXI_GP0_AWSIZE => open,
    M_AXI_GP0_ARPROT => open,
    M_AXI_GP0_AWPROT => open,
    M_AXI_GP0_ARADDR => open,
    M_AXI_GP0_AWADDR => open,
    M_AXI_GP0_WDATA => open,
    M_AXI_GP0_ARCACHE => open,
    M_AXI_GP0_ARLEN => open,
    M_AXI_GP0_ARQOS => open,
    M_AXI_GP0_AWCACHE => open,
    M_AXI_GP0_AWLEN => open,
    M_AXI_GP0_AWQOS => open,
    M_AXI_GP0_WSTRB => open,
    M_AXI_GP0_ACLK => w_ext_clk,
    M_AXI_GP0_ARREADY => '1',
    M_AXI_GP0_AWREADY => '1',
    M_AXI_GP0_BVALID => '0',
    M_AXI_GP0_RLAST => '0',
    M_AXI_GP0_RVALID => '0',
    M_AXI_GP0_WREADY => '1',
    M_AXI_GP0_BID => X"000",
    M_AXI_GP0_RID => X"000",
    M_AXI_GP0_BRESP => "00",
    M_AXI_GP0_RRESP => "00",
    M_AXI_GP0_RDATA => X"00000000",
    FCLK_CLK0 => w_ext_clk,
    FCLK_RESET0_N => FCLK_RESET0_N,
    MIO => open,
    DDR_CAS_n => open,
    DDR_CKE => open,
    DDR_Clk_n => open,
    DDR_Clk => open,
    DDR_CS_n => open,
    DDR_DRSTB => open,
    DDR_ODT => open,
    DDR_RAS_n => open,
    DDR_WEB => open,
    DDR_BankAddr => open,
    DDR_Addr => open,
    DDR_VRN => open,
    DDR_VRP => open,
    DDR_DM => open,
    DDR_DQ => open,
    DDR_DQS_n => open,
    DDR_DQS => open,
    PS_SRSTB => open,
    PS_CLK => open,
    PS_PORB => open
  );

  FCLK_RESET0 <= not FCLK_RESET0_N;

  buf0 : BUFG port map (
    I => w_ext_clk,
    O => w_ext_clk_buf
  );

  gpiox : for i in 0 to 11 generate
    iob0  : iobuf_tech generic map(zynq7000) 
            port map (ib_gpio_ipins(i), io_gpio(i), ob_gpio_opins(i), ob_gpio_direction(i));
  end generate;

  pll0 : SysPLL_tech generic map (
    tech => zynq7000
  ) port map (
    i_reset     => FCLK_RESET0,
    i_clk_tcxo	=> w_ext_clk_buf,
    o_clk_bus   => w_pll_clk,
    o_locked    => w_pll_lock
  );
  w_rst <= w_pll_lock;

 
  soc0 : riscv_soc port map ( 
    i_rst  => w_rst,
    i_clk  => w_pll_lock,
    --! GPIO.
    i_gpio     => ib_gpio_ipins,
    o_gpio     => ob_gpio_opins,
    o_gpio_dir => ob_gpio_direction,
    --! GPTimers
    o_pwm => open,
    --! JTAG signals:
    i_jtag_tck => i_jtag_tck,
    i_jtag_ntrst => i_jtag_ntrst,
    i_jtag_tms => i_jtag_tms,
    i_jtag_tdi => i_jtag_tdi,
    o_jtag_tdo => o_jtag_tdo,
    o_jtag_vref => o_jtag_vref,
    --! UART1 signals:
    i_uart1_ctsn => '0',
    i_uart1_rd   => i_uart1_rd,
    o_uart1_td   => o_uart1_td,
    o_uart1_rtsn => open,
    --! UART2 (debug port) signals:
    i_uart2_ctsn => '0',
    i_uart2_rd   => i_uart2_rd,
    o_uart2_td   => o_uart2_td,
    o_uart2_rtsn => open,
    --! SPI Flash
    i_flash_si => '0',
    o_flash_so => open,
    o_flash_sck => open,
    o_flash_csn => open,
    o_flash_wpn => open,
    o_flash_holdn => open,
    o_flash_reset => open,
    --! OTP Memory
    i_otp_d => X"0000",
    o_otp_d => open,
    o_otp_a => open,
    o_otp_we => open,
    o_otp_re => open,
    --! Ethernet MAC PHY interface signals
    i_etx_clk   => '0',
    i_erx_clk   => '0',
    i_erxd      => X"0",
    i_erx_dv    => '0',
    i_erx_er    => '0',
    i_erx_col   => '0',
    i_erx_crs   => '0',
    i_emdint    => '0',
    o_etxd      => open,
    o_etx_en    => open,
    o_etx_er    => open,
    o_emdc      => open,
    i_eth_mdio    => '0',
    o_eth_mdio    => open,
    o_eth_mdio_oe => open,
    i_eth_gtx_clk    => '0',
    i_eth_gtx_clk_90 => '0',
    o_erstn     => open,
    -- GNSS Sub-system signals:
    i_clk_adc => '0',
    i_gps_I => "00",
    i_gps_Q => "00",
    i_glo_I => "00",
    i_glo_Q => "00",
    o_pps => open,
    i_gps_ld => '0',
    i_glo_ld => '0',
    o_max_sclk => open,
    o_max_sdata => open,
    o_max_ncs => open,
    i_antext_stat => '0',
    i_antext_detect => '0',
    o_antext_ena => open,
    o_antint_contr => open
  );


end arch_zynq_top;
