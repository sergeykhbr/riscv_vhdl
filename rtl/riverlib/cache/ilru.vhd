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
library commonlib;
use commonlib.types_common.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity ILru is generic (
    async_reset : boolean
  );
  port (
    i_nrst : in std_logic;
    i_clk : in std_logic;
    i_adr : in std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
    i_we : in std_logic;
    i_lru : in std_logic_vector(1 downto 0);
    o_lru : out std_logic_vector(1 downto 0)
  );
end; 
 
architecture arch_ILru of ILru is

  constant LINES_TOTAL : integer := 2**CFG_IINDEX_WIDTH;
  type array_type is array (0 to LINES_TOTAL-1) of std_logic_vector(7 downto 0);

  signal tbl : array_type;
  signal adr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);

  type RegistersType is record
      adr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
      tbl : array_type;
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_adr, i_we, i_lru, r)
    variable v : RegistersType;
    variable wb_lru : std_logic_vector(7 downto 0);
  begin

    v := r;

    wb_lru := r.tbl(conv_integer(r.adr));
    
    v.adr := i_adr;

    if i_we = '1' and wb_lru(7 downto 6) /= i_lru then
        v.tbl(conv_integer(i_adr)) := i_lru & wb_lru(7 downto 2);
    end if;

    if not async_reset and i_nrst = '0' then
        v.adr := (others => '0');
        for i in 0 to LINES_TOTAL-1 loop
            v.tbl(i) := X"E4";  -- 0x3, 0x2, 0x1, 00
        end loop;
    end if;

    o_lru <= wb_lru(1 downto 0);
   
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r.adr <= (others => '0');
        for i in 0 to LINES_TOTAL-1 loop
            r.tbl(i) <= X"E4";  -- 0x3, 0x2, 0x1, 00
        end loop;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
