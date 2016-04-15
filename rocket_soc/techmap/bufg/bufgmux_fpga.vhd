----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Clock multiplexer with buffered output for Xilinx FPGA.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library unisim;
use unisim.vcomponents.all;

entity bufgmux_fpga is
  generic (
    tmode_always_ena : boolean := false
  );
  port (
    O       : out std_ulogic;
    I1      : in std_ulogic;
    I2      : in std_ulogic;
    S       : in std_ulogic
    );
end;


architecture rtl of bufgmux_fpga is
begin
  good : if not tmode_always_ena generate
    mux_buf : BUFGMUX
    port map (
      O   => O,
      I0  => I1,
      I1  => I2,
      S   => S
    );
  end generate;
  
  bad : if tmode_always_ena generate
    mux_buf : BUFG
    port map (
      O   => O,
      I  => I2
    );
  end generate;
  
end;  
