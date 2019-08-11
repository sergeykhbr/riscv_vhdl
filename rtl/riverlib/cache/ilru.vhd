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

entity ILru is port (
    i_clk : in std_logic;
    i_init : in std_logic;
    i_radr : in std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
    i_wadr : in std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
    i_we : in std_logic;
    i_lru : in std_logic_vector(1 downto 0);
    o_lru : out std_logic_vector(1 downto 0)
  );
end; 
 
architecture arch_ILru of ILru is

  constant LINES_TOTAL : integer := 2**CFG_IINDEX_WIDTH;
  type array_type is array (0 to LINES_TOTAL-1) of std_logic_vector(7 downto 0);

  signal radr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
  signal tbl : array_type;
  signal wb_tbl_rdata : std_logic_vector(7 downto 0);
  signal wb_tbl_wdata : std_logic_vector(7 downto 0);
  signal w_we : std_logic;

begin

  comb : process(i_init, i_radr, i_wadr, i_we, i_lru, wb_tbl_rdata)
    variable vb_tbl_wdata : std_logic_vector(7 downto 0);
    variable v_we : std_logic;
  begin

    v_we := i_we;
    if i_init = '1' then
        v_we := '1';
        vb_tbl_wdata := X"E4";  -- 0x3, 0x2, 0x1, 00
    elsif i_we = '1' then
        if wb_tbl_rdata(7 downto 6) = i_lru then
            vb_tbl_wdata := wb_tbl_rdata;
        elsif wb_tbl_rdata(5 downto 4) = i_lru then
            vb_tbl_wdata := i_lru & wb_tbl_rdata(7 downto 6)
                                  & wb_tbl_rdata(3 downto 0);
        elsif wb_tbl_rdata(3 downto 2) = i_lru then
            vb_tbl_wdata := i_lru & wb_tbl_rdata(7 downto 4)
                                  & wb_tbl_rdata(1 downto 0);
        else
            vb_tbl_wdata := i_lru & wb_tbl_rdata(7 downto 2);
        end if;
    else
        vb_tbl_wdata := wb_tbl_rdata;
    end if;

    w_we <= v_we;
    wb_tbl_wdata <= vb_tbl_wdata;
  end process;

  reg : process (i_clk) begin
    if rising_edge(i_clk) then 
      radr <= i_radr;
      if w_we = '1' then
        tbl(conv_integer(i_wadr)) <= wb_tbl_wdata;
      end if;
    end if;
  end process;

  wb_tbl_rdata <= tbl(conv_integer(radr));
  o_lru <= wb_tbl_rdata(1 downto 0);
 
end;
