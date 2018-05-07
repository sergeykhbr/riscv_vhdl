----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Clock multiplexer with buffered output for Mikron 180 nm.
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
