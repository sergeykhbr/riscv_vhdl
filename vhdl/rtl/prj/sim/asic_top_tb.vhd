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

  signal i_dtmcs_re : std_logic;
  signal i_dmi_ena : std_logic;
  signal i_dmi_we : std_logic;
  signal i_dmi_re : std_logic;
  signal i_dmi_addr : std_logic_vector(6 downto 0);
  signal i_dmi_wdata : std_logic_vector(31 downto 0);
  signal jtag_tdi : std_logic;
  signal jtag_tdo : std_logic;
  signal jtag_tms : std_logic;
  signal jtag_tck : std_logic;
  signal jtag_ntrst : std_logic;

  signal iClkCnt : integer := 0;
  signal iEdclCnt : integer := 0;
  signal jtag_test_cnt : integer := 0;

  constant UART_WR_ENA : std_logic := '0';
  constant JTAG_WR_ENA : std_logic := '1';
  constant ETH_WR_ENA : std_logic := '0';

  type jtag_test_type is record
    idx : integer;
    dtmcs_re : std_logic;
    dmi_we : std_logic;
    dmi_re : std_logic;
    dmi_addr : std_logic_vector(7 downto 0);  -- actual width may be different and reported in dtmcs
    wdata : std_logic_vector(31 downto 0);
  end record;
  type jtag_test_vector is array (0 to 64) of jtag_test_type;

  constant JTAG_TESTS : jtag_test_vector := (
  --    | dtm | we | re | adr | wdata
    (0,   '1', '0', '0', X"00", X"00000000"),  
    (1,   '0', '1', '0', X"10", X"07ffffc1"),  -- [dmcontrol] hartselhi=1023 hartsello=1023 dmactive
    (2,   '0', '0', '0', X"10", X"00000000"),  
    (3,   '0', '0', '1', X"10", X"00000000"),  
    (4,   '0', '0', '0', X"10", X"00000000"),  
    (5,   '0', '1', '0', X"10", X"80000000"),  -- [dmcontrol] haltreq
    (6,   '0', '0', '0', X"10", X"00000000"),  
    (7,   '0', '0', '1', X"11", X"00000000"),  -- [dmstatus]
    (8,   '0', '0', '0', X"11", X"00000000"),  

    (9,   '0', '1', '0', X"20", X"0000100f"),  -- [progbuf0]
    (10,  '0', '0', '0', X"20", X"00000000"),  
    (11,  '0', '1', '0', X"21", X"0000000f"),  -- [progbuf1]
    (12,  '0', '0', '0', X"21", X"00000000"),  
    (13,  '0', '1', '0', X"22", X"00100073"),  -- [progbuf2]
    (14,  '0', '0', '0', X"22", X"00000000"),  
    (15,  '0', '1', '0', X"17", X"00241000"),  -- [command] size=32, posexec=1, transfer=0, write=0, regno=0x1000
    (16,  '0', '0', '1', X"16", X"00000000"),  
    (17,  '0', '0', '0', X"16", X"00000000"),  

    (30,  '0', '1', '0', X"05", X"00000000"),  -- [data1] data[63:32]
    (31,  '0', '0', '0', X"05", X"00000000"),  -- 
    (32,  '0', '1', '0', X"04", X"00000000"),  -- [data1] data[31:0], step=1, ebreakm=1, ebreaks=1, ebreaku=1, 
    (33,  '0', '0', '0', X"04", X"00000000"),  -- 
    (34,  '0', '1', '0', X"17", X"003307b0"),  -- [command] size=64, posexec=0, transfer=1, write=1, regno=0x7b0
    (35,  '0', '0', '1', X"16", X"00000000"),  -- 
    (36,  '0', '0', '0', X"16", X"00000000"),  -- 

    (40,  '0', '1', '0', X"17", X"00320301"),  -- [command] [15:0]0x301=MISA; [16]write=0; [17]transfer=1; [18]postexec=0;
    (41,  '0', '0', '0', X"17", X"00000000"),  -- [command] empty
    (42,  '0', '0', '1', X"16", X"00000000"),  -- [abstracs] read
    (43,  '0', '0', '0', X"16", X"00000000"),  -- [abstracs] empty
    (44,  '0', '0', '1', X"05", X"00000000"),  -- [data1] read
    (45,  '0', '0', '0', X"05", X"00000000"),  -- [data1] empty
    (46,  '0', '0', '1', X"04", X"00000000"),  -- [data0] read
    (47,  '0', '0', '0', X"04", X"00000000"),  -- [data0] empty
    (48,  '0', '1', '0', X"04", X"11223344"),
    (49,  '0', '1', '0', X"05", X"aabbccdd"),
    (50,  '0', '0', '1', X"04", X"00000000"),
    others => (-1,  '0', '0', '0', X"00", X"00000000")
  );
  
  component asic_top is port ( 
    i_rst     : in std_logic;
    i_sclk_p  : in std_logic;
    i_sclk_n  : in std_logic;
    io_gpio   : inout std_logic_vector(11 downto 0);
    o_pwm     : out std_logic_vector(1 downto 0);
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
  generic (
    OUTPUT_ENA : std_logic := '1'
  );
  port (
    rst : in std_logic;
    clk : in std_logic;
    o_rxd  : out std_logic_vector(3 downto 0);
    o_rxdv : out std_logic
  );
  end component;

  component jtag_sim is 
  generic (
    clock_rate : integer := 10
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    i_dtmcs_re : in std_logic;
    i_dmi_ena : in std_logic;
    i_dmi_we : in std_logic;
    i_dmi_re : in std_logic;
    i_dmi_addr : in std_logic_vector(6 downto 0);
    i_dmi_wdata : in std_logic_vector(31 downto 0);
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

  jtaggen0 : process (i_sclk_n, iClkCnt, jtag_test_cnt)
    variable test_case : integer;
  begin
    if rising_edge(i_sclk_n) then
        test_case := iClkCnt / 5000;
        i_dtmcs_re <= '0';
        i_dmi_ena <= '0';
        i_dmi_we <= '0';
        i_dmi_re <= '0';

        if (iClkCnt mod 5000) = 4999 then
            if test_case = JTAG_TESTS(jtag_test_cnt).idx then
                i_dtmcs_re <= JTAG_WR_ENA and JTAG_TESTS(jtag_test_cnt).dtmcs_re;
                i_dmi_ena <= JTAG_WR_ENA and not JTAG_TESTS(jtag_test_cnt).dtmcs_re;
                i_dmi_we <= JTAG_WR_ENA and JTAG_TESTS(jtag_test_cnt).dmi_we;
                i_dmi_re <= JTAG_WR_ENA and JTAG_TESTS(jtag_test_cnt).dmi_re;
                i_dmi_addr <= JTAG_TESTS(jtag_test_cnt).dmi_addr(6 downto 0);
                i_dmi_wdata <= JTAG_TESTS(jtag_test_cnt).wdata;
                jtag_test_cnt <= jtag_test_cnt + 1;
            end if;
        end if;
    end if;
  end process;

  udatagen0 : process (i_sclk_n, iClkCnt)
  begin
    if rising_edge(i_sclk_n) then
        uart_wr_str <= '0';
        if iClkCnt = 82000 then
           uart_wr_str <= UART_WR_ENA;
           uart_instr(1 to 4) <= "ping";
           uart_instr(5) <= cr;
           uart_instr(6) <= lf;
        elsif iClkCnt = 108000 then
           uart_wr_str <= UART_WR_ENA;
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
    rtsn => '0',
    rd  => i_uart1_rd,
    ctsn => open,
    busy => uart_busy
  );

  phy0 : ethphy_sim generic map (
    OUTPUT_ENA => ETH_WR_ENA
  ) port map (
    rst => i_rst,
    clk  => i_sclk_p,
    o_rxd  => i_rxd,
    o_rxdv => i_rxdv
  );

  jsim0 : jtag_sim  generic map (
    clock_rate => 4
  ) port map (
    rst => i_rst,
    clk => i_sclk_p,
    i_dtmcs_re => i_dtmcs_re,
    i_dmi_ena => i_dmi_ena,
    i_dmi_we => i_dmi_we,
    i_dmi_re => i_dmi_re,
    i_dmi_addr => i_dmi_addr,
    i_dmi_wdata => i_dmi_wdata,
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
    o_erstn     => open
 );

end;
