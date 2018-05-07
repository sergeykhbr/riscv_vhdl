----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Xilinx clock buffered output.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity ibufg_xilinx is
  port (
    O    : out std_ulogic;
    I    : in std_ulogic
    );
end;


architecture rtl of ibufg_xilinx is

begin

    bufg0 : BUFG port map (
      O  => O,
      I  => I
    );

end;  
