-----------------------------------------------------------------------------
-- Entity: 	   Buffers descriptor
-- File:	      bufgmux.vhd
-- Author:	    Sergey Khabarov - GNSS Sensor Ltd
-- Description:	Multiplexer/buffer  description
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library techmap;
use techmap.gencomp.all;


package bufgmux is

  component bufgmux_tech is
    generic (
      tech : integer := 0
    );
    port (
      O        : out std_ulogic;
      I1       : in std_ulogic;
      I2       : in std_ulogic;
      S        : in std_ulogic);
  end component;


end;
