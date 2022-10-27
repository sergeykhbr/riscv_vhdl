----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Virtual clock multiplexer with buffered output.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library techmap;
use techmap.gencomp.all;

entity bufgmux_tech is
  generic
  (
    tech : integer := 0;
    rf_frontend_ena : boolean := false
  );
  port (
    O        : out std_ulogic;
    I1       : in std_ulogic;
    I2       : in std_ulogic;
    S        : in std_ulogic
    );
end;


architecture rtl of bufgmux_tech is

 component bufgmux_fpga is
  generic (
    rf_frontend_ena : boolean := false
  );
  port (
    O       : out std_ulogic;
    I1      : in std_ulogic;
    I2      : in std_ulogic;
    S       : in std_ulogic
    );
  end component;

 component bufgmux_micron180 is
  port (
    O       : out std_ulogic;
    I1      : in std_ulogic;
    I2      : in std_ulogic;
    S       : in std_ulogic
    );
  end component;


begin


   inf : if tech = inferred generate
      O <= I1 when S = '0' else I2;
   end generate;

   xlnx : if tech = virtex6 or tech = kintex7 generate
      mux_buf : bufgmux_fpga generic map (
        rf_frontend_ena => rf_frontend_ena
      ) port map (
        O   => O,
        I1  => I1,
        I2  => I2,
        S   => S
      );
   end generate;

   mic0 : if tech = mikron180 generate
      mux_buf : bufgmux_micron180
      port map (
        O   => O,
        I1  => I1,
        I2  => I2,
        S   => S
      );
   end generate;

  
end;  
