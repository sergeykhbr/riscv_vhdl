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

entity Ram32x2_tech is
generic (
    generic_tech   : integer := 0;
    generic_kWords : integer := 1
);
port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(10+log2(generic_kWords)-1 downto 0);
    i_wr_ena    : in std_logic_vector(1 downto 0);
    i_data      : in std_logic_vector(63 downto 0);
    o_data      : out std_logic_vector(63 downto 0)
);
end;

architecture rtl of Ram32x2_tech is

  component Ram32_inferred
  generic (
    generic_abits    : integer := 10
  );
  port (
    i_clk         : in std_logic;
    i_address     : in std_logic_vector(generic_abits-1 downto 0);
    i_wr_ena      : in std_logic;
    i_data       : in std_logic_vector(31 downto 0);
    o_data       : out std_logic_vector(31 downto 0)
  );
  end component;

begin

  genmem0 : if generic_tech = inferred or is_fpga(generic_tech) /= 0 generate
      ramx0 : Ram32_inferred generic map
      (
        generic_abits => 10+log2(generic_kWords)
      ) port map
      (
        i_clk,
        i_address,
        i_wr_ena(0),
        i_data(31 downto 0),
        o_data(31 downto 0)
      );

      ramx1 : Ram32_inferred generic map
      (
        generic_abits => 10+log2(generic_kWords)
      ) port map
      (
        i_clk,
        i_address,
        i_wr_ena(1),
        i_data(63 downto 32),
        o_data(63 downto 32)
      );
  end generate;

end; 


