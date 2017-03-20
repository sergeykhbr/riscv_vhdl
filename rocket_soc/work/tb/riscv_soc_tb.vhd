-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Testbench file for the SoC top-level implementation
------------------------------------------------------------------------------
--! @page verification_page RTL Verification
--!
--! @section sim_tb_link Top-level simulation
--!
--! @par Test-bench example
--! Use file <b>work/tb/riscv_soc_tb.vhd</b> to run simulation scenario. You can
--! get the following time diagram after simulation of 2 ms interval.
--!
--! <img src="pics/soc_sim.png" alt="Simulating top"> 
--! @latexonly {\includegraphics[scale=0.75]{pics/soc_sim.png}} @endlatexonly
--!
--! @note Simulation behaviour depends of current firmware image. It may
--!       significantly differs in a new releases either as Zephyr OS kernel
--!       image is absolutely different relative GNSS FW image.
--!
--! Some FW versions can detect RTL simulation target by reading <i>'Target'
--! Register</i> in PnP device that allows to speed-up simulation
--! by removing some delays and changing Devices IO parameters (UART speed
--! for example).
--!
--! @par Running on FPGA
--! Supported FPGA:
--! <ul>
--!    <li>ML605 with Virtex6 FPGA using ISE 14.7 (default).</li>
--!    <li>KC705 with Kintex7 FPGA using Vivado 2015.4.</li>
--! </ul>
--!
--! @warning In a case of using GNSS FW without connected RF front-end 
--!          don't forget to <em><b>switch ON DIP[0] (i_int_clkrf) to enable
--!          Test Mode</b></em>. Otherwise there 
--!          wouldn't be generated interrupts and, as result, no UART 
--!          output.
--!
--! @section auto_compare_page VCD-files automatic comparision
--!
--! @subsection gen_sysc_vcd Generating VCD-pattern form SystemC model
--!
--! Edit the following attributes in SystemC target script 
--! <i>debugger/targets/sysc_river_gui.json</i> to enable vcd-file generation.
--! <ul>
--!    <li>['InVcdFile','i_river','Non empty string enables generation of stimulus VCD file'].</li>
--!    <li>['OutVcdFile','o_river','Non empty string enables VCD file with reference signals']</li>
--! </ul>
--! Files <em>i_river.vcd</em> and <em>o_river.vcd</em> will be generated.
--! The first one will be used as a RTL simulation stimulus to generate input
--! signals. The second one as a reference.
--!
--! @subsection run_vcd_compare Compare RIVER SystemC model relative RTL
--!
--! Run simulation in ModelSim with the following commands using correct pathes
--! for your host:
--! 
--!      vcd2wlf E:/Projects/GitProjects/riscv_vhdl/debugger/win32build/Debug/i_river.vcd -o e:/i_river.wlf
--!      vcd2wlf E:/Projects/GitProjects/riscv_vhdl/debugger/win32build/Debug/o_river.vcd -o e:/o_river.wlf
--!      wlf2vcd e:/i_river.wlf -o e:/i_river.vcd
--!      vsim -t 1ps -vcdstim E:/i_river.vcd riverlib.RiverTop
--!      vsim -view e:/o_river.wlf
--!      add wave o_river:/SystemC/o_*
--!      add wave sim:/rivertop/*
--!      run 500us
--!      compare start o_river sim
--!      compare add -wave sim:/RiverTop/o_req_mem_valid o_river:/SystemC/o_req_mem_valid
--!      compare add -wave sim:/RiverTop/o_req_mem_write o_river:/SystemC/o_req_mem_write
--!      compare add -wave sim:/RiverTop/o_req_mem_addr o_river:/SystemC/o_req_mem_addr
--!      compare add -wave sim:/RiverTop/o_req_mem_strob o_river:/SystemC/o_req_mem_strob
--!      compare add -wave sim:/RiverTop/o_req_mem_data o_river:/SystemC/o_req_mem_data
--!      compare add -wave sim:/RiverTop/o_dport_ready o_river:/SystemC/o_dport_ready
--!      compare add -wave sim:/RiverTop/o_dport_rdata o_river:/SystemC/o_dport_rdata
--!      compare run
--! 
--! @note In this script I've used \c vcd2wlf and \c wlf2vcd utilities to form
--!       compatible with ModelSim VCD-file. Otherwise there're will be errors because
--!       ModelSim cannot parse std_logic_vector siganls (only std_logic).

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_util.all;
library rocketlib;
--use rocketlib.types_rocket.all;

entity riscv_soc_tb is
  constant INCR_TIME : time := 3571 ps;--100 ns;--3571 ps;
end riscv_soc_tb;

architecture behavior of riscv_soc_tb is
  -- input/output signals:
  signal i_rst : std_logic := '1';
  signal i_sclk_p : std_logic;
  signal i_sclk_n : std_logic;
  signal i_dip : std_logic_vector(3 downto 0);
  signal o_led : std_logic_vector(7 downto 0);
  signal i_uart1_ctsn : std_logic := '0';
  signal i_uart1_rd : std_logic := '1';
  signal o_uart1_td : std_logic;
  signal o_uart1_rtsn : std_logic;
  
  signal o_emdc    : std_logic;
  signal io_emdio  : std_logic;
  signal i_rxd  : std_logic_vector(3 downto 0) := "0000";
  signal i_rxdv : std_logic := '0';
  signal o_txd  : std_logic_vector(3 downto 0);
  signal o_txdv : std_logic;

  signal uart_wr_str : std_logic;
  signal uart_instr : string(1 to 256);
  signal uart_busy : std_logic;

  signal clk_cur: std_logic := '1';
  signal check_clk_bus : std_logic := '0';
  signal iClkCnt : integer := 0;
  signal iErrCnt : integer := 0;
  signal iErrCheckedCnt : integer := 0;
  signal iEdclCnt : integer := 0;
  
component riscv_soc is port 
( 
  i_rst     : in std_logic; -- button "Center"
  i_sclk_p  : in std_logic;
  i_sclk_n  : in std_logic;
  i_dip     : in std_logic_vector(3 downto 0);
  o_led     : out std_logic_vector(7 downto 0);
  -- uart1
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic;
  -- Ethernet
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
    clock_rate : integer := 10
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    wr_str : in std_logic;
    instr : in string;
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

  i_dip <= "0001";

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
    o_rxdv => i_rxdv
  );

  -- signal parsment and assignment
  tt : riscv_soc port map
  (
    i_rst     => i_rst,
    i_sclk_p  => i_sclk_p,
    i_sclk_n  => i_sclk_n,
    i_dip     => i_dip,
    o_led     => o_led,
    i_uart1_ctsn => i_uart1_ctsn,
    i_uart1_rd   => i_uart1_rd,
    o_uart1_td   => o_uart1_td,
    o_uart1_rtsn => o_uart1_rtsn,
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
