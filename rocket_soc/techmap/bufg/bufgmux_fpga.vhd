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
    rf_frontend_ena : boolean := false
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
  good : if rf_frontend_ena generate
    --! @details BUFGMUX suits much better to switch clock depending DIP[0]
    --!          signal, but ISE studio doesn't properly synth. such logic.
    --!          So here we will use ADC signal only.
    --mux_buf : BUFGMUX
    --port map (
    --  O   => O,
    --  I0  => I1,
    --  I1  => I2,
    --  S   => S
    --);
    mux_buf : BUFG
    port map (
      O  => O,
      I  => I1
    );
  end generate;
  
  bad : if not rf_frontend_ena generate
    mux_buf : BUFG
    port map (
      O  => O,
      I  => I2
    );
  end generate;
  
end;  
