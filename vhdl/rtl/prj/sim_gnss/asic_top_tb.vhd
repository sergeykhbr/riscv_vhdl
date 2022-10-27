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

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_util.all;

entity asic_top_tb is
end asic_top_tb;

architecture behavior of asic_top_tb is
  -- input/output signals:
  signal i_rst : std_logic := '1';
  signal i_sclk_p : std_logic := '1';
  signal i_sclk_n : std_logic;
  signal io_gpio : std_logic_vector(11 downto 0);
  signal o_pwm : std_logic_vector(1 downto 0);
  signal i_uart1_rd : std_logic := '1';
  signal o_uart1_td : std_logic;
  signal i_uart2_rd : std_logic := '1';
  signal o_uart2_td : std_logic;
  signal i_flash_si : std_logic;
  signal o_flash_so : std_logic;
  signal o_flash_sck : std_logic;
  signal o_flash_csn : std_logic;
  
  signal o_emdc    : std_logic;
  signal io_emdio  : std_logic;
  signal i_rxd  : std_logic_vector(3 downto 0) := "0000";
  signal i_rxdv : std_logic := '0';
  signal o_txd  : std_logic_vector(3 downto 0);
  signal o_txdv : std_logic;

  signal uart_wr_str : std_logic;
  signal uart_instr : string(1 to 256);
  signal uart_busy : std_logic;
  signal uart_bin_data : std_logic_vector(63 downto 0);
  signal uart_bin_bytes_sz : integer;

  signal jtag_test_ena : std_logic;
  signal jtag_test_burst : std_logic_vector(7 downto 0);
  signal jtag_test_addr : std_logic_vector(31 downto 0);
  signal jtag_test_we : std_logic;
  signal jtag_test_wdata : std_logic_vector(31 downto 0);
  signal jtag_tdi : std_logic;
  signal jtag_tdo : std_logic;
  signal jtag_tms : std_logic;
  signal jtag_tck : std_logic;
  signal jtag_ntrst : std_logic;

  signal i_clk_adc : std_logic := '0';
  signal i_gps_ld    : std_logic := '0';--'1';
  signal i_glo_ld    : std_logic := '0';--'1';
  signal o_max_sclk  : std_logic;
  signal o_max_sdata : std_logic;
  signal o_max_ncs   : std_logic_vector(1 downto 0);
  signal i_antext_stat   : std_logic := '0';
  signal i_antext_detect : std_logic := '0';
  signal o_antext_ena    : std_logic;
  signal o_antint_contr  : std_logic;

  signal iClkCnt : integer := 0;
  signal iEdclCnt : integer := 0;
  
  component asic_top is port ( 
    i_rst     : in std_logic;
    i_sclk_p  : in std_logic;
    i_sclk_n  : in std_logic;
    io_gpio   : inout std_logic_vector(11 downto 0);
    o_pwm : out std_logic_vector(1 downto 0);
    i_jtag_tck : in std_logic;
    i_jtag_ntrst : in std_logic;
    i_jtag_tms : in std_logic;
    i_jtag_tdi : in std_logic;
    o_jtag_tdo : out std_logic;
    o_jtag_vref : out std_logic;
    i_uart1_rd   : in std_logic;
    o_uart1_td   : out std_logic;
    i_uart2_rd   : in std_logic;
    o_uart2_td   : out std_logic;
    i_flash_si : in std_logic;
    o_flash_so : out std_logic;
    o_flash_sck : out std_logic;
    o_flash_csn : out std_logic;
    io_otp_gnd : inout std_logic;
    io_otp_vdd : inout std_logic;
    io_otp_vdd18 : inout std_logic;
    io_otp_upp : inout std_logic;
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

  component uart_sim is 
  generic (
    clock_rate : integer := 10;
    binary_bytes_max : integer := 8;
    use_binary : boolean := false
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    wr_str : in std_logic;
    instr : in string;
    bin_data : in std_logic_vector(8*binary_bytes_max-1 downto 0);
    bin_bytes_sz : in integer;
    td  : in std_logic;
    rtsn : in std_logic;
    rd  : out std_logic;
    ctsn : out std_logic;
    busy : out std_logic
  );
  end component;

  component ethphy_sim is 
  port (
    rst : in std_logic;
    clk : in std_logic;
    o_rxd  : out std_logic_vector(3 downto 0);
    o_rxdv : out std_logic
  );
  end component;

  component jtag_sim is 
  generic (
    clock_rate : integer := 10;
    irlen : integer := 4
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    i_test_ena : in std_logic;
    i_test_burst : in std_logic_vector(7 downto 0);
    i_test_addr : in std_logic_vector(31 downto 0);
    i_test_we : in std_logic;
    i_test_wdata : in std_logic_vector(31 downto 0);
    i_tdi  : in std_logic;
    o_tck : out std_logic;
    o_ntrst : out std_logic;
    o_tms : out std_logic;
    o_tdo : out std_logic
  );
  end component;

  component M25AA1024 is port (
     SI : in std_logic;
     SO : out std_logic;
     SCK : in std_logic;
     CS_N : in std_logic;
     WP_N : in std_logic;
     HOLD_N : in std_logic;
     RESET : in std_logic
  );
  end component;

begin

  i_sclk_p <= not i_sclk_p after 12.5 ns;
  i_sclk_n <= not i_sclk_p;

  i_clk_adc <= not i_clk_adc after 192.3 ns;

  procSignal : process (i_sclk_p, iClkCnt)
  begin
    if rising_edge(i_sclk_p) then
      iClkCnt <= iClkCnt + 1;
      --! @note to make sync. reset  of the logic that are clocked by
      --!       htif_clk which is clock/512 by default.
      if iClkCnt = 15 then
        i_rst <= '0';
      end if;
    end if;
  end process procSignal;

  io_gpio <= X"001";

  udatagen0 : process (i_sclk_n, iClkCnt)
  begin
    if rising_edge(i_sclk_n) then
        uart_wr_str <= '0';
        if iClkCnt = 82000 then
           uart_wr_str <= '1';
           uart_instr(1 to 4) <= "ping";
           uart_instr(5) <= cr;
           uart_instr(6) <= lf;
        elsif iClkCnt = 108000 then
           uart_wr_str <= '1';
           uart_instr(1 to 3) <= "pnp";
           uart_instr(4) <= cr;
           uart_instr(5) <= lf;
        end if;

        jtag_test_ena <= '0';
        if iClkCnt = 3000 then
           jtag_test_ena <= '1';
           jtag_test_burst <= (others => '0');
           jtag_test_addr <= X"10000000";
           jtag_test_we <= '0';
           jtag_test_wdata <= (others => '0');
        elsif iClkCnt = 5000 then
           jtag_test_ena <= '1';
           jtag_test_burst <= (others => '0');
           jtag_test_addr <= X"fffff004";
           jtag_test_we <= '1';
           jtag_test_wdata <= X"12345678";
        elsif iClkCnt = 7000 then
           jtag_test_ena <= '1';
           jtag_test_burst <= X"01";
           jtag_test_addr <= X"10000004";
           jtag_test_we <= '0';
           jtag_test_wdata <= (others => '0');
        elsif iClkCnt = 10000 then
           jtag_test_ena <= '1';
           jtag_test_burst <= X"02";
           jtag_test_addr <= X"FFFFF004";
           jtag_test_we <= '1';
           jtag_test_wdata <= X"DEADBEEF";
       end if;    
    end if;
  end process;


  uart0 : uart_sim generic map (
    clock_rate => 2*20
  ) port map (
    rst => i_rst,
    clk => i_sclk_p,
    wr_str => uart_wr_str,
    instr => uart_instr,
    bin_data => uart_bin_data,
    bin_bytes_sz => uart_bin_bytes_sz,
    td  => o_uart1_td,
    rtsn => '0',
    rd  => i_uart1_rd,
    ctsn => open,
    busy => uart_busy
  );

  phy0 : ethphy_sim port map (
    rst => i_rst,
    clk  => i_sclk_p,
    o_rxd  => i_rxd,
    o_rxdv => i_rxdv
  );

  jsim0 : jtag_sim  generic map (
    clock_rate => 4,
    irlen => 4
  ) port map (
    rst => i_rst,
    clk => i_sclk_p,
    i_test_ena => jtag_test_ena,
    i_test_burst => jtag_test_burst,
    i_test_addr => jtag_test_addr,
    i_test_we => jtag_test_we,
    i_test_wdata => jtag_test_wdata,
    i_tdi => jtag_tdi,
    o_tck => jtag_tck,
    o_ntrst => jtag_ntrst,
    o_tms => jtag_tms,
    o_tdo => jtag_tdo
  );

  flash0 : M25AA1024 port map (
     SI => o_flash_so,
     SO => i_flash_si,
     SCK => o_flash_sck,
     CS_N => o_flash_csn,
     WP_N => '1',
     HOLD_N => '1',
     RESET => '0'
  );

  -- signal parsment and assignment
  tt : asic_top port map
  (
    i_rst     => i_rst,
    i_sclk_p  => i_sclk_p,
    i_sclk_n  => i_sclk_n,
    io_gpio   => io_gpio,
    o_pwm => o_pwm,
    i_jtag_tck => jtag_tck,
    i_jtag_ntrst => jtag_ntrst,
    i_jtag_tms => jtag_tms,
    i_jtag_tdi => jtag_tdo,
    o_jtag_tdo => jtag_tdi,
    o_jtag_vref => open,
    i_uart1_rd   => i_uart1_rd,
    o_uart1_td   => o_uart1_td,
    i_uart2_rd   => i_uart2_rd,
    o_uart2_td   => o_uart2_td,
    i_flash_si => i_flash_si,
    o_flash_so => o_flash_so,
    o_flash_sck => o_flash_sck,
    o_flash_csn => o_flash_csn,
    io_otp_gnd => open,
    io_otp_vdd => open,
    io_otp_vdd18 => open,
    io_otp_upp => open,
    i_gmiiclk_p => '0',
    i_gmiiclk_n => '1',
    o_egtx_clk  => open,
    i_etx_clk   => i_sclk_p,
    i_erx_clk   => i_sclk_p,
    i_erxd      => i_rxd,
    i_erx_dv    => i_rxdv,
    i_erx_er    => '0',
    i_erx_col   => '0',
    i_erx_crs   => '0',
    i_emdint    => '0',
    o_etxd      => o_txd,
    o_etx_en    => o_txdv,
    o_etx_er    => open,
    o_emdc      => o_emdc,
    io_emdio    => io_emdio,
    o_erstn     => open,
    i_clk_adc => i_clk_adc,
    i_gps_I  => "01",
    i_gps_Q  => "11",
    i_glo_I  => "11",
    i_glo_Q  => "01",
    o_pps => open,
    i_gps_ld    => i_gps_ld,
    i_glo_ld    => i_glo_ld,
    o_max_sclk  => o_max_sclk,
    o_max_sdata => o_max_sdata,
    o_max_ncs   => o_max_ncs,
    i_antext_stat   => i_antext_stat,
    i_antext_detect => i_antext_detect,
    o_antext_ena    => o_antext_ena,
    o_antint_contr  => o_antint_contr
 );

end;
