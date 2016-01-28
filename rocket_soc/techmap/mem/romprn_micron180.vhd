----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Galileo Reference E1 codes.
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library tech;
use tech.RAMLIB_80_COMPONENTS.all;

entity RomPrn_micron180 is
  port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(12 downto 0);
    o_data      : out std_logic_vector(31 downto 0)
  );
end;

architecture rtl of RomPrn_micron180 is


begin

  
   m180 : ROMD_8192x32m8d4_R0_M4_ns port map
      (Q   => o_data,
       CK  => i_clk,
       CSN => '0',
       OEN => '0',
       A   => i_address); 
  
end;
