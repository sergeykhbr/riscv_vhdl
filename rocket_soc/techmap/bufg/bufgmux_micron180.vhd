-----------------------------------------------------------------------------
-- Entity:      System reset former
-- File:        tap_rstn.vhd
-- Author:      Sergey Khabarov - GNSS Sensor Ltd
-- Description: Clock multiplexer
------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;

entity bufgmux_micron180 is
  port (
    O       : out std_ulogic;
    I1      : in std_ulogic;
    I2      : in std_ulogic;
    S       : in std_ulogic
    );
end;


architecture rtl of bufgmux_micron180 is
begin

    O <= I1 when S = '0' else I2;
    -- TODO: clock buffer

  
end;  
