-----------------------------------------------------------------------------
-- Entity:      RF front-end control
-- File:        iobuf_virtex6.vhd
-- Author:      Sergey Khabarov - GNSS Sensor Ltd
-- Description: O buffer for inferred
------------------------------------------------------------------------------

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
