-----------------------------------------------------------------------------
-- Entity:      RF front-end control
-- File:        iobuf_virtex6.vhd
-- Author:      Sergey Khabarov - GNSS Sensor Ltd
-- Description: I buffer for inferred
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
