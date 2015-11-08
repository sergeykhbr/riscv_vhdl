-----------------------------------------------------------------------------
-- Entity: 	   PLL's descriptor
-- File:	      gnsspll.vhd
-- Author:	    Sergey Khabarov - GNSS Sensor Ltd
-- Description:	PLL in/out description
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library techmap;
use techmap.gencomp.all;


package syspll is

  component SysPLL_tech is
    generic(
      tech    : integer range 0 to NTECH := 0
    );
    port (
    i_reset           : in     std_logic;
    i_int_clkrf       : in     std_logic;-- disable external ADC/enable internal ADC simulation
    i_clkp            : in     std_logic;
    i_clkn            : in     std_logic;
    i_clk_adc         : in     std_logic;
    o_clk_bus         : out    std_logic;-- Bus interface clock = 100MHz/40MHz (Virtex6/Spartan6)
    o_clk_adc         : out    std_logic;-- ADC simulation clock = 26MHz (default)
    o_locked          : out    std_logic);
  end component;

  component SysPLL_inferred is
  port (
    CLK_IN1_P   : in     std_logic;
    CLK_IN1_N   : in     std_logic;
    CLK_OUT1    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

  component SysPLL_v6 is 
  port (
    CLK_IN1_P   : in     std_logic;
    CLK_IN1_N   : in     std_logic;
    CLK_OUT1	: out std_logic;
    RESET	: in std_logic;
    LOCKED	: out std_logic );
  end component;

  component SysPLL_k7 is
  port (
    CLK_IN1_P   : in     std_logic;
    CLK_IN1_N   : in     std_logic;
    CLK_OUT1  : out    std_logic;
    RESET     : in     std_logic;
    LOCKED    : out    std_logic );
  end component;


  component SysPLL_micron180 is
  port (
    CLK_IN1_P   : in     std_logic;
    CLK_IN1_N   : in     std_logic;
    CLK_OUT1    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

end;
