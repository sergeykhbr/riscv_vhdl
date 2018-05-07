----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Virtual Gigabits buffer with the differential signals.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;

entity igdsbuf_tech is
  generic (
    generic_tech : integer := 0
  );
  port (
    gclk_p : in std_logic;
    gclk_n : in std_logic;
    o_clk  : out std_logic
  );
end; 
 
architecture rtl of igdsbuf_tech is

  component igdsbuf_kintex7 is
  generic (
    generic_tech : integer := 0
  );
  port (
    gclk_p : in std_logic;
    gclk_n : in std_logic;
    o_clk  : out std_logic
  );
  end component; 

  component igdsbuf_virtex6 is
  generic (
    generic_tech : integer := 0
  );
  port (
    gclk_p : in std_logic;
    gclk_n : in std_logic;
    o_clk  : out std_logic
  );
  end component; 

begin

  infer : if generic_tech = inferred generate 
      o_clk <= gclk_p;
  end generate;

  xv6 : if generic_tech = virtex6 generate 
      x1 : igdsbuf_virtex6 port map (
         gclk_p => gclk_p,
         gclk_n => gclk_n,
         o_clk  => o_clk
      );
  end generate;

  xk7 : if generic_tech = kintex7 generate 
      x1 : igdsbuf_kintex7 port map (
         gclk_p => gclk_p,
         gclk_n => gclk_n,
         o_clk  => o_clk
      );
  end generate;


end;
