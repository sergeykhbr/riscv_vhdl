----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Input buffer with the differential signals.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity idsbuf_xilinx is
  port (
    clk_p : in std_logic;
    clk_n : in std_logic;
    o_clk  : out std_logic
  );
end; 
 
architecture rtl of idsbuf_xilinx is
begin

      x1 : IBUFDS  port map (
         I     => clk_p,
         IB    => clk_n,
         O     => o_clk
      );

end;
