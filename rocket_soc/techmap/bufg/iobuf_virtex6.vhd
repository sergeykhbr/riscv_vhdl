----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      IO buffer for fpga virtex6.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
Library UNISIM;
use UNISIM.vcomponents.all;


entity iobuf_virtex6 is
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end; 
 
architecture rtl of iobuf_virtex6 is

begin


  io_inst : IOBUF generic map
  (
    DRIVE => 12,
    IOSTANDARD => "DEFAULT",
    SLEW => "SLOW"
  ) port map
  (
    O => o,
    IO => io,
    I => i,
    T => t
  );


end;
