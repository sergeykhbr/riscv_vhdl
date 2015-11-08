-----------------------------------------------------------------------------
-- Entity:      System reset former
-- File:        tap_rstn.vhd
-- Author:      Sergey Khabarov - GNSS Sensor Ltd
-- Description: 
------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity bufgmux_fpga is
  port (
    O       : out std_ulogic;
    I1      : in std_ulogic;
    I2      : in std_ulogic;
    S       : in std_ulogic
    );
end;


architecture rtl of bufgmux_fpga is
begin

    mux_buf : BUFGMUX
    port map (
      O   => O,
      I0  => I1,
      I1  => I2,
      S   => S
    );
  
end;  
