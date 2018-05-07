----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Gigabits buffer with the differential signals.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity igdsbuf_kintex7 is
  generic (
    generic_tech : integer := 0
  );
  port (
    gclk_p : in std_logic;
    gclk_n : in std_logic;
    o_clk  : out std_logic
  );
end; 
 
architecture rtl of igdsbuf_kintex7 is
begin

      x1 : IBUFDS_GTE2   port map (
         I     => gclk_p,
         IB    => gclk_n,
         CEB   => '0',
         O     => o_clk,
         ODIV2 => open
      );

end;
