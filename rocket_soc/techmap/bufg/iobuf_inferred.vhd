----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      IO buffer for inferred tech.
----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;


entity iobuf_inferred is
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end; 
 
architecture rtl of iobuf_inferred is
  signal ivalue : std_logic;
begin

  o <= '0';
  io <= 'Z' when t = '1' else to_X01(i);

end;
