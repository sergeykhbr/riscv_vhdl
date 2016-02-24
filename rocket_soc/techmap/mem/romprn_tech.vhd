-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Technology specific Galileo PRN ROM codes
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

entity RomPrn_tech is
generic (
    generic_tech    : integer := 0
);
port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(12 downto 0);
    o_data      : out std_logic_vector(31 downto 0)
);
end;

architecture rtl of RomPrn_tech is

component RomPrn_inferred is
  port (
    clk     : in  std_ulogic;
    inAdr   : in  std_logic_vector(12 downto 0);
    outData : out std_logic_vector(31 downto 0)
  );
end component;

component RomPrn_micron180 is
  port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(12 downto 0);
    o_data      : out std_logic_vector(31 downto 0)
  );
end component;


begin

  genrom0 : if generic_tech = inferred or is_fpga(generic_tech) /= 0 generate
      romprn_infer : RomPrn_inferred port map
      (
        i_clk,
        i_address,
        o_data
      );
  end generate;
  genrom1 : if generic_tech = micron180 generate
      romprn_micr : RomPrn_micron180 port map
      (
        i_clk,
        i_address,
        o_data
      );
  end generate;
end; 


