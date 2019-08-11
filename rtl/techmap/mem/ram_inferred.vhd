--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.ALL;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;

entity ram_inferred is generic (
    abits : integer := 12;
    dbits : integer := 64
);
port (
    i_clk   : in std_logic;
    i_addr : in std_logic_vector(abits-1 downto 0);
    o_rdata : out std_logic_vector(dbits-1 downto 0);
    i_wena  : in std_logic;
    i_wdata : in std_logic_vector(dbits-1 downto 0)
);
end;

architecture rtl of ram_inferred is

constant SRAM_LENGTH : integer := 2**abits;
type ram_type is array (0 to SRAM_LENGTH-1) of std_logic_vector(dbits-1 downto 0);

signal ram : ram_type;
signal radr : std_logic_vector(abits-1 downto 0);

begin

  reg : process (i_clk) begin
    if rising_edge(i_clk) then 
      radr <= i_addr;
      if i_wena = '1' then
        ram(conv_integer(i_addr)) <= i_wdata;
      end if;
    end if;
  end process;

  o_rdata <= ram(conv_integer(radr));

end; 


