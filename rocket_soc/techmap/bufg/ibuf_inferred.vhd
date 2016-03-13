----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Input buffer for simulation.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity ibuf_inferred is
  port (
    o  : out std_logic;
    i  : in std_logic
  );
end; 
 
architecture rtl of ibuf_inferred is

begin

  o <= i;

end;
