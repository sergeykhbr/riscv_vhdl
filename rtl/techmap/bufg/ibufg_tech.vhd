----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Virtual clock buffered output.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library techmap;
use techmap.gencomp.all;

entity ibufg_tech is
  generic
  (
    tech : integer := 0
  );
  port (
    O    : out std_ulogic;
    I    : in std_ulogic
    );
end;

architecture rtl of ibufg_tech is

  component ibufg_xilinx is
  port (
    O    : out std_ulogic;
    I    : in std_ulogic
    );
  end component;
  signal w_o : std_logic;
begin


   inf : if tech = inferred generate
      w_o <= I;
   end generate;

   xlnx : if tech = virtex6 or tech = kintex7 generate
      x0 : ibufg_xilinx port map (
        O  => w_o,
        I  => I
      );
   end generate;

   O <= w_o;

end;  
