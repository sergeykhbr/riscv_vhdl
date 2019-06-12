-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Testbench file for the SoC top-level implementation
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_util.all;
library rocketlib;
--use rocketlib.types_rocket.all;

entity riscv_soc_gnss_tb is
  constant INCR_TIME : time := 3571 ps;--100 ns;--3571 ps;
end riscv_soc_gnss_tb;

architecture behavior of riscv_soc_gnss_tb is
  -- input/output signals:
  signal i_rst : std_logic := '1';
  signal i_sclk_p : std_logic;
  signal i_sclk_n : std_logic;
  signal i_clk_adc : std_logic := '0';
  signal io_gpio : std_logic_vector(11 downto 0);
  signal i_uart1_ctsn : std_logic := '0';
  signal i_uart1_rd : std_logic := '1';
  signal o_uart1_td : std_logic;
  signal o_uart1_rtsn : std_logic;
  signal i_uart2_ctsn : std_logic := '1';
  signal i_uart2_rd   : std_logic := '1';
  signal o_uart2_td   : std_logic;
  signal o_uart2_rtsn : std_logic;
  
  signal i_gps_ld    : std_logic := '0';--'1';
  signal i_glo_ld    : std_logic := '0';--'1';
  signal o_max_sclk  : std_logic;
  signal o_max_sdata : std_logic;
  signal o_max_ncs   : std_logic_vector(1 downto 0);
  signal i_antext_stat   : std_logic := '0';
  signal i_antext_detect : std_logic := '0';
  signal o_antext_ena    : std_logic;
  signal o_antint_contr  : std_logic;

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

  signal adc_cnt : integer := 0;
  signal clk_cur: std_logic := '1';
  signal check_clk_bus : std_logic := '0';
  signal iClkCnt : integer := 0;
  signal iErrCnt : integer := 0;
  signal iErrCheckedCnt : integer := 0;
  signal iEdclCnt : integer := 0;
  
component riscv_soc_gnss is port 
( 
  i_rst     : in std_logic; -- button "Center"
  i_sclk_p  : in std_logic;
  i_sclk_n  : in std_logic;
  i_clk_adc : in std_logic;
  io_gpio     : inout std_logic_vector(11 downto 0);
  -- uart1
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic;
  -- uart2 (debug port)
  i_uart2_ctsn : in std_logic;
  i_uart2_rd   : in std_logic;
  o_uart2_td   : out std_logic;
  o_uart2_rtsn : out std_logic;
  -- ADC samples
  i_gps_I  : in std_logic_vector(1 downto 0);
  i_gps_Q  : in std_logic_vector(1 downto 0);
  i_glo_I  : in std_logic_vector(1 downto 0);
  i_glo_Q  : in std_logic_vector(1 downto 0);
  -- rf front-end
  i_gps_ld    : in std_logic;
  i_glo_ld    : in std_logic;
  o_max_sclk  : out std_logic;
  o_max_sdata : out std_logic;
  o_max_ncs   : out std_logic_vector(1 downto 0);
  i_antext_stat   : in std_logic;
  i_antext_detect : in std_logic;
  o_antext_ena    : out std_logic;
  o_antint_contr  : out std_logic;
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
  o_erstn     : out   std_ulogic
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

begin


  -- Process of reading
  procReadingFile : process
    variable clk_next: std_logic;
  begin

    wait for INCR_TIME;
    if (adc_cnt + 26000000) >= 40000000 then
      adc_cnt <= (adc_cnt + 26000000) - 40000000;
      i_clk_adc <= not i_clk_adc;
    else
      adc_cnt <= (adc_cnt + 26000000);
    end if;

    while true loop
      clk_next := not clk_cur;
      if (clk_next = '1' and clk_cur = '0') then
        check_clk_bus <= '1';
      end if;

      wait for 1 ps;
      check_clk_bus <= '0';
      clk_cur <= clk_next;

      wait for INCR_TIME;
      if clk_cur = '1' then
        iClkCnt <= iClkCnt + 1;
      end if;
      if (adc_cnt + 26000000) >= 40000000 then
        adc_cnt <= (adc_cnt + 26000000) - 40000000;
        i_clk_adc <= not i_clk_adc;
      else
        adc_cnt <= (adc_cnt + 26000000);
      end if;

    end loop;
    report "Total clocks checked: " & tost(iErrCheckedCnt) & " Errors: " & tost(iErrCnt);
    wait for 1 sec;
  end process procReadingFile;


  i_sclk_p <= clk_cur;
  i_sclk_n <= not clk_cur;

  procSignal : process (i_sclk_p, iClkCnt)

  begin
    if rising_edge(i_sclk_p) then
      
      --! @note to make sync. reset  of the logic that are clocked by
      --!       htif_clk which is clock/512 by default.
      if iClkCnt = 15 then
        i_rst <= '0';
      end if;
    end if;
  end process procSignal;

  io_gpio(3 downto 0) <= "0000";

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
    rtsn => o_uart1_rtsn,
    rd  => i_uart1_rd,
    ctsn => i_uart1_ctsn,
    busy => uart_busy
  );
  
  phy0 : ethphy_sim port map (
    rst => i_rst,
    clk  => i_sclk_p,
    o_rxd  => i_rxd,
    o_rxdv => i_rxdv );


  -- signal parsment and assignment
  tt : riscv_soc_gnss port map
  (
    i_rst     => i_rst,
    i_sclk_p  => i_sclk_p,
    i_sclk_n  => i_sclk_n,
    i_clk_adc => i_clk_adc,
    io_gpio     => io_gpio,
    i_uart1_ctsn => i_uart1_ctsn,
    i_uart1_rd   => i_uart1_rd,
    o_uart1_td   => o_uart1_td,
    o_uart1_rtsn => o_uart1_rtsn,
    i_uart2_ctsn => i_uart2_ctsn,
    i_uart2_rd   => i_uart2_rd,
    o_uart2_td   => o_uart2_td,
    o_uart2_rtsn => o_uart2_rtsn,
    i_gps_I  => "01",
    i_gps_Q  => "11",
    i_glo_I  => "11",
    i_glo_Q  => "01",
    i_gps_ld    => i_gps_ld,
    i_glo_ld    => i_glo_ld,
    o_max_sclk  => o_max_sclk,
    o_max_sdata => o_max_sdata,
    o_max_ncs   => o_max_ncs,
    i_antext_stat   => i_antext_stat,
    i_antext_detect => i_antext_detect,
    o_antext_ena    => o_antext_ena,
    o_antint_contr  => o_antint_contr,
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
    o_erstn     => open
 );

  procCheck : process (i_rst, check_clk_bus)
  begin
    if rising_edge(check_clk_bus) then
      if i_rst = '0' then
        iErrCheckedCnt <= iErrCheckedCnt + 1;
      end if;
    end if;
  end process procCheck;

end;
