----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Virtual input buffer with the differential signals.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;

entity idsbuf_tech is
  generic (
    generic_tech : integer := 0
  );
  port (
    clk_p : in std_logic;
    clk_n : in std_logic;
    o_clk  : out std_logic
  );
end; 
 
architecture rtl of idsbuf_tech is

  component idsbuf_xilinx is
  port (
    clk_p : in std_logic;
    clk_n : in std_logic;
    o_clk  : out std_logic
  );
  end component; 


begin

  infer : if generic_tech = inferred generate 
      o_clk <= clk_p;
  end generate;

  xil0 : if generic_tech = virtex6 or generic_tech = kintex7 generate 
      x1 : idsbuf_xilinx port map (
         clk_p => clk_p,
         clk_n => clk_n,
         o_clk  => o_clk
      );
  end generate;


end;
