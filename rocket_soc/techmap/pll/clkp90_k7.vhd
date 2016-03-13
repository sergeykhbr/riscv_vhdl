-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Clock phase offset generator (90 deg) for Kintex7 FPGA.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity clkp90_kintex7 is
  generic (
    freq    : integer := 125000
  );
  port (
    --! Active High
    i_rst    : in  std_logic;
    i_clk    : in  std_logic;
    o_clk    : out std_logic;
    o_clkp90 : out std_logic;
    o_clk2x  : out std_logic;
    o_lock   : out std_logic
  );
end clkp90_kintex7;

architecture rtl of clkp90_kintex7 is

  constant clk_mul : integer := 8;
  constant clk_div : integer := 8;
  constant period : real := 1000000.0/real(freq);
  constant clkio_div : integer := freq*clk_mul/200000;

  signal CLKFBOUT : std_logic;
  signal CLKFBIN : std_logic;
  signal clk_nobuf : std_logic;
  signal clk90_nobuf : std_logic;
  signal clkio_nobuf : std_logic;

begin

  CLKFBIN <= CLKFBOUT;

  PLLE2_ADV_inst : PLLE2_ADV generic map (
     BANDWIDTH          => "OPTIMIZED",  -- OPTIMIZED, HIGH, LOW
     CLKFBOUT_MULT      => clk_mul,   -- Multiply value for all CLKOUT, (2-64)
     CLKFBOUT_PHASE     => 0.0, -- Phase offset in degrees of CLKFB, (-360.000-360.000).
     -- CLKIN_PERIOD: Input clock period in nS to ps resolution (i.e. 33.333 is 30 MHz).
     CLKIN1_PERIOD      => period,
     CLKIN2_PERIOD      => 0.0,
     -- CLKOUT0_DIVIDE - CLKOUT5_DIVIDE: Divide amount for CLKOUT (1-128)
     CLKOUT0_DIVIDE     => clk_div,
     CLKOUT1_DIVIDE     => clk_div,
     CLKOUT2_DIVIDE     => clkio_div,
     CLKOUT3_DIVIDE     => 1,
     CLKOUT4_DIVIDE     => 1,
     CLKOUT5_DIVIDE     => 1,
     -- CLKOUT0_DUTY_CYCLE - CLKOUT5_DUTY_CYCLE: Duty cycle for CLKOUT outputs (0.001-0.999).
     CLKOUT0_DUTY_CYCLE => 0.5,
     CLKOUT1_DUTY_CYCLE => 0.5,
     CLKOUT2_DUTY_CYCLE => 0.5,
     CLKOUT3_DUTY_CYCLE => 0.5,
     CLKOUT4_DUTY_CYCLE => 0.5,
     CLKOUT5_DUTY_CYCLE => 0.5,
     -- CLKOUT0_PHASE - CLKOUT5_PHASE: Phase offset for CLKOUT outputs (-360.000-360.000).
     CLKOUT0_PHASE      => 0.0,
     CLKOUT1_PHASE      => 90.0,
     CLKOUT2_PHASE      => 0.0,
     CLKOUT3_PHASE      => 0.0,
     CLKOUT4_PHASE      => 0.0,
     CLKOUT5_PHASE      => 0.0,
     COMPENSATION       => "ZHOLD", -- ZHOLD, BUF_IN, EXTERNAL, INTERNAL
     DIVCLK_DIVIDE      => 1, -- Master division value (1-56)
     -- REF_JITTER: Reference input jitter in UI (0.000-0.999).
     REF_JITTER1        => 0.0,
     REF_JITTER2        => 0.0,
     STARTUP_WAIT       => "TRUE" -- Delay DONE until PLL Locks, ("TRUE"/"FALSE")
  ) port map (
     -- Clock Outputs: 1-bit (each) output: User configurable clock outputs
     CLKOUT0           => clk_nobuf,
     CLKOUT1           => clk90_nobuf,
     CLKOUT2           => clkio_nobuf,
     CLKOUT3           => OPEN,
     CLKOUT4           => OPEN,
     CLKOUT5           => OPEN,
     -- DRP Ports: 16-bit (each) output: Dynamic reconfigration ports
     DO                => OPEN,
     DRDY              => OPEN,
     -- Feedback Clocks: 1-bit (each) output: Clock feedback ports
     CLKFBOUT          => CLKFBOUT,
     -- Status Ports: 1-bit (each) output: PLL status ports
     LOCKED            => o_lock,
     -- Clock Inputs: 1-bit (each) input: Clock inputs
     CLKIN1            => i_clk,
     CLKIN2            => '0',
     -- Con trol Ports: 1-bit (each) input: PLL control ports
     CLKINSEL          => '1',
     PWRDWN            => '0',
     RST               => i_rst, 
     -- DRP Ports: 7-bit (each) input: Dynamic reconfigration ports
     DADDR             => "0000000", 
     DCLK              => '0',
     DEN               => '0',
     DI                => "0000000000000000", 
     DWE               => '0',
     -- Feedback Clocks: 1-bit (each) input: Clock feedback ports
     CLKFBIN           => CLKFBIN
    );

  bufgclk0 : BUFG port map (I => clk_nobuf, O => o_clk);
  bufgclk90 : BUFG port map (I => clk90_nobuf, O => o_clkp90);
  bufgclkio : BUFG port map (I => clkio_nobuf, O => o_clk2x);

end;
