-----------------------------------------------------------------------------
-- Package:     fse_v2
-- File:        romprn_tech.vhd
-- Author:      Sergey Khabarov - sergeykhbr@gmail.com
-- Description:	Technology specific Bootable ROM
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
use techmap.allmem.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity BootRom_tech is
generic (
    memtech : integer := 0;
    sim_hexfile : string
);
port (
    clk       : in std_logic;
    address   : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
    data      : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
);
end;

architecture rtl of BootRom_tech is
begin

  genrom0 : if memtech = inferred or is_fpga(memtech) /= 0 generate
      infer0 : BootRom_inferred  generic map (sim_hexfile)
               port map (clk, address, data);
  end generate;

end; 


