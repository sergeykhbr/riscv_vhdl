-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     32-bits RAM implementation based on registers
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;

entity Ram32_inferred is
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
end;

architecture rtl of Ram32_inferred is

constant RAM32_ADR_WIDTH : integer := 10+log2(generic_kWords);

type ram_type is array ((2**RAM32_ADR_WIDTH)-1 downto 0) of std_logic_vector (31 downto 0);
signal RAM       : ram_type;
signal adr       : std_logic_vector(10+log2(generic_kWords)-1 downto 0);

begin

  -- registers:
  regs : process(i_clk) begin 
    if rising_edge(i_clk) then 
      if(i_wr_ena='1') then
        RAM(conv_integer(i_address)) <= i_data; 
      end if;
      adr <= i_address;
    end if;
  end process;
  
  o_data <= RAM(conv_integer(adr));

end; 


