-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Technology specific RAM selector
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

entity Ram32_tech is
generic (
    generic_tech   : integer := 0;
    generic_kWords : integer := 1
);
port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(10+log2(generic_kWords)-1 downto 0);
    i_wr_ena    : in std_logic;
    i_data      : in std_logic_vector(31 downto 0);
    o_data      : out std_logic_vector(31 downto 0)
);
end;

architecture rtl of Ram32_tech is

component Ram32_inferred
generic (
    generic_kWords    : integer := 1
);
port (
    i_clk         : in std_logic;
    i_address     : in std_logic_vector(10+log2(generic_kWords)-1 downto 0);
    i_wr_ena      : in std_logic;
    i_data       : in std_logic_vector(31 downto 0);
    o_data       : out std_logic_vector(31 downto 0)
);
end component;

-- micron 180 nm tech
component micron180_syncram
  generic (abits : integer := 10; dbits : integer := 8 );
port (
    clk      : in std_ulogic;
    address  : in std_logic_vector((abits -1) downto 0);
    datain   : in std_logic_vector((dbits -1) downto 0);
    dataout  : out std_logic_vector((dbits -1) downto 0);
    enable   : in std_ulogic;
    write    : in std_ulogic
   );
end component;

-- TODO: add there other ASIC components

begin

  genmem0 : if generic_tech = inferred or is_fpga(generic_tech) /= 0 generate
      ram_infer : Ram32_inferred generic map
      (
        generic_kWords => generic_kWords
      ) port map
      (
        i_clk,
        i_address,
        i_wr_ena,
        i_data,
        o_data
      );
  end generate;

  genmem1 : if generic_tech = micron180 generate
    k4 : if generic_kWords = 4 generate
      x0 : micron180_syncram 
           generic map (12, 32) 
           port map (i_clk, i_address, i_data, o_data, '1', i_wr_ena);
    end generate;
    k8 : if generic_kWords = 8 generate
      x0 : micron180_syncram 
           generic map (13, 32) 
           port map (i_clk, i_address, i_data, o_data, '1', i_wr_ena);
    end generate;
  end generate;
end; 


