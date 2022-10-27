-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Technology specific dual-port RAM.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

entity syncram_2p_tech is
generic (
    tech : integer := 0;
    abits : integer := 6;
    dbits : integer := 8;
    sepclk : integer := 0;
    wrfst : integer := 0;
    testen : integer := 0;
    words : integer := 0;
    custombits : integer := 1
);
port (
    rclk     : in std_ulogic;
    renable  : in std_ulogic;
    raddress : in std_logic_vector((abits -1) downto 0);
    dataout  : out std_logic_vector((dbits -1) downto 0);
    wclk     : in std_ulogic;
    write    : in std_ulogic;
    waddress : in std_logic_vector((abits -1) downto 0);
    datain   : in std_logic_vector((dbits -1) downto 0)
);
end;

architecture rtl of syncram_2p_tech is

  component syncram_2p_inferred is
  generic (
    abits : integer := 8;
    dbits : integer := 32;
    sepclk: integer := 0
  );
  port (
    rclk : in std_ulogic;
    wclk : in std_ulogic;
    rdaddress: in std_logic_vector (abits -1 downto 0);
    wraddress: in std_logic_vector (abits -1 downto 0);
    data: in std_logic_vector (dbits -1 downto 0);
    wren : in std_ulogic;
    q: out std_logic_vector (dbits -1 downto 0)
  );
  end component;

begin


  inf : if tech = inferred generate
    x0 : syncram_2p_inferred generic map (abits, dbits, sepclk)
         port map (rclk, wclk, raddress, waddress, datain, write, dataout);
  end generate;

  xilinx6 : if tech = virtex6 or tech = kintex7 generate
    x0 : syncram_2p_inferred generic map (abits, dbits, sepclk)
         port map (rclk, wclk, raddress, waddress, datain, write, dataout);
  end generate;

end;

