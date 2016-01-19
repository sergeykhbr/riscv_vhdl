----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Declaration types_buf package components.
------------------------------------------------------------------------------
--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library techmap;
use techmap.gencomp.all;

package types_buf is

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


  component ibuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    i  : in std_logic
  );
  end component; 
  
  component obuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    i  : in std_logic
  );
  end component; 

  component iobuf_tech is generic (generic_tech : integer := 0);
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
  end component;


end;
