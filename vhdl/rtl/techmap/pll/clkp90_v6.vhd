-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Clock phase offset generator (90 deg) for FPGA Virtex6.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity clkp90_virtex6 is port (
    i_clk    : in std_logic;
    o_clk    : out std_logic;
    o_clkp90 : out std_logic
);
end clkp90_virtex6;

architecture rtl of clkp90_virtex6 is
   signal clk_buf : std_logic;
begin

   x0 : BUFG port map (
       O => clk_buf,
       I => i_clk
   );

   x1 : ODDR port map (
      Q => o_clkp90,
      C => clk_buf,
      CE => '1',
      D1 => '0',
      D2 => '1',
      R => '0',
      S => '0'
   );
   
   o_clk <= clk_buf;

end;
