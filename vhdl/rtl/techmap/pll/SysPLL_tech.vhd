-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Implementation of the SysPLL_tech entity
--! @details   This module file be included in all projects.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;

--! "Virtual" components declarations library.
library techmap;
use techmap.gencomp.all;

--! @brief   SysPLL_tech entity declaration ("Virtual" PLL).
--! @details This module instantiates the certain PLL implementation
--!          depending generic technology argument.
entity SysPLL_tech is
  generic (
    tech    : integer range 0 to NTECH := 0 --! PLL implementation selector
  );
  port
  (
    --! Reset value. Active high.
    i_reset           : in     std_logic;
    --! Input clock from the external oscillator (default 200 MHz)
    i_clk_tcxo        : in     std_logic;
    --! System Bus clock 100MHz/40MHz (Virtex6/Spartan6)
    o_clk_bus         : out    std_logic;
    --! PLL locked status.
    o_locked          : out    std_logic
  );
end SysPLL_tech;

--! SysPLL_tech architecture declaration.
architecture rtl of SysPLL_tech is
  component SysPLL_inferred is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

  component SysPLL_v6 is 
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1	: out    std_logic;
    RESET	: in std_logic;
    LOCKED	: out std_logic );
  end component;

  component SysPLL_k7 is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    RESET     : in     std_logic;
    LOCKED    : out    std_logic );
  end component;
  
  component SysPLL_zynq is
  port
   (
    CLK_IN           : in     std_logic;
    CLK_OUT1          : out    std_logic;
    RESET             : in     std_logic;
    LOCKED            : out    std_logic
   );
  end component;

  component SysPLL_micron180 is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

begin

   xv6 : if tech = virtex6 generate
     pll0 : SysPLL_v6 port map (i_clk_tcxo, o_clk_bus, i_reset, o_locked);
   end generate;

   xv7 : if tech = kintex7 generate
     pll0 : SysPLL_k7 port map (i_clk_tcxo, o_clk_bus, i_reset, o_locked);
   end generate;

   xz7 : if tech = zynq7000 generate
     pll0 : SysPLL_zynq port map (i_clk_tcxo, o_clk_bus, i_reset, o_locked);
   end generate;
   
   inf : if tech = inferred generate
     pll0 : SysPLL_inferred port map (i_clk_tcxo, o_clk_bus, i_reset, o_locked);
   end generate;
   
   m180 : if tech = mikron180 generate
     pll0 : SysPLL_micron180 port map (i_clk_tcxo, o_clk_bus, i_reset, o_locked);
   end generate;

end;
