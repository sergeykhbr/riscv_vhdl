----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Simple output buffer for simulation target.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;


entity obuf_inferred is
  port (
    o  : out std_logic;
    i  : in std_logic
  );
end; 
 
architecture rtl of obuf_inferred is

begin

  o <= i;

end;
