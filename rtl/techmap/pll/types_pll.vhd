-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Components declaration of the types_pll package.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
--! Target names declaration
library techmap;
use techmap.gencomp.all;

--! @brief Declaration of 'virtual' PLL components
package types_pll is

  --! @brief   Declaration of the "virtual" PLL component.
  --! @details This module instantiates the certain PLL implementation
  --!          depending generic argument.
  --! @param[in] tech Generic PLL implementation selector
  --! @param[in] i_reset Reset value. Active high.
  --! @param[in] i_clk_tcxo Input clock from the external oscillator (default 200 MHz)
  --! @param[out] o_clk_bus System Bus clock 100MHz/40MHz (Virtex6/Spartan6)
  --! @param[out] o_locked PLL locked status.
  component SysPLL_tech is
    generic(
      tech    : integer range 0 to NTECH := 0
    );
    port (
    i_reset           : in     std_logic;
    i_clk_tcxo        : in     std_logic;
    o_clk_bus         : out    std_logic;
    o_locked          : out    std_logic );
  end component;

  --! @brief   Virtual Clock phase rotator.
  --! @param[in] tech Technology selector.
  --! @param[in] freq Clock frequency in KHz.
  --! @param[in] i_rst Reset signal. Active HIGH.
  component clkp90_tech is
  generic (
    tech    : integer range 0 to NTECH := 0;
    freq    : integer := 125000
  );
  port (
    i_rst    : in  std_logic;
    i_clk    : in  std_logic;
    o_clk    : out std_logic;
    o_clkp90 : out std_logic;
    o_clk2x  : out std_logic;
    o_lock   : out std_logic
  );
  end component;


  --! @}
end;
