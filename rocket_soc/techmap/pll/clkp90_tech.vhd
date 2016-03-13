-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Virtual clock phase offset generator (90 deg)
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;

entity clkp90_tech is
  generic (
    tech    : integer range 0 to NTECH := 0;
    --! clock frequency in KHz
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
end clkp90_tech;

architecture rtl of clkp90_tech is

  component clkp90_virtex6 is
  port (
    i_clk    : in std_logic;
    o_clk    : out std_logic;
    o_clkp90 : out std_logic
  );
  end component;

  component clkp90_kintex7 is
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
  end component;

begin

   xv6 : if tech = virtex6 generate
      v1 : clkp90_virtex6 port map (
           i_clk    => i_clk, 
           o_clk    => o_clk,
           o_clkp90 => o_clkp90
      );
      o_clk2x <= '0';
      o_lock <= '0';
   end generate;

   xl7 : if tech = kintex7  or tech = artix7 or tech = zynq7000 generate
      v1 : clkp90_kintex7 generic map (
          freq => freq
      ) port map (
          i_rst    => i_rst,
          i_clk    => i_clk,
          o_clk    => o_clk,
          o_clkp90 => o_clkp90,
          o_clk2x  => o_clk2x,
          o_lock   => o_lock
     );
   end generate;

   inf : if tech = inferred generate
      o_clk    <= i_clk;
      o_clkp90 <= i_clk;
      o_clk2x  <= '0';
      o_lock  <= '0';
   end generate;
   
   m180 : if tech = micron180 generate
   end generate;

end;
