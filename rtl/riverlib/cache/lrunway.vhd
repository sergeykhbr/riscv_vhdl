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

entity lrunway is generic (
    abits : integer;        -- cache bus address bus (usually 6..8)
    waybits : integer       -- Number of ways bitwidth (=2 for 4-ways cache)
);
port (
    i_clk : in std_logic;
    i_flush : in std_logic;
    i_addr : in std_logic_vector(abits-1 downto 0);
    i_we : in std_logic;
    i_lru : in std_logic_vector(waybits-1 downto 0);
    o_lru : out std_logic_vector(waybits-1 downto 0)
  );
end; 
 
architecture arch_lrunway of lrunway is

  constant LINES_TOTAL : integer := 2**abits;
  constant WAYS_TOTAL : integer := 2**waybits;
  constant LINE_WIDTH : integer := WAYS_TOTAL * waybits;

  type array_type is array (0 to LINES_TOTAL-1) of std_logic_vector(LINE_WIDTH-1 downto 0);

  signal radr : std_logic_vector(abits-1 downto 0);
  signal tbl : array_type;
  signal wb_tbl_rdata : std_logic_vector(LINE_WIDTH-1 downto 0);
  signal wb_tbl_wdata : std_logic_vector(LINE_WIDTH-1 downto 0);
  signal w_we : std_logic;

begin

  comb : process(i_flush, i_addr, i_we, i_lru, wb_tbl_rdata)
    variable vb_tbl_wdata : std_logic_vector(7 downto 0);
    variable v_we : std_logic;
    variable shift_ena : std_logic;
  begin

    shift_ena := '0';
    v_we := i_we;
    vb_tbl_wdata := wb_tbl_rdata;

    if i_flush = '1' then
        v_we := '1';
        for i in 0 to WAYS_TOTAL-1 loop
            vb_tbl_wdata((i+1)*waybits-1 downto i*waybits) := conv_std_logic_vector(i, waybits);
        end loop;
    elsif i_we = '1' then
        if wb_tbl_rdata(LINE_WIDTH-1 downto LINE_WIDTH-waybits) /= i_lru then
           vb_tbl_wdata(LINE_WIDTH-1 downto LINE_WIDTH-waybits) := i_lru;
           shift_ena := '1';

           for i in WAYS_TOTAL-2 downto 0 loop
                if shift_ena = '1' then
                    vb_tbl_wdata((i+1)*waybits-1 downto i*waybits) :=
                            wb_tbl_rdata((i+2)*waybits-1 downto (i+1)*waybits);
                    if wb_tbl_rdata((i+1)*waybits-1 downto i*waybits) = i_lru then
                        shift_ena := '0';
                    end if;
                else
                    vb_tbl_wdata((i+1)*waybits-1 downto i*waybits) :=
                            wb_tbl_rdata((i+1)*waybits-1 downto i*waybits);
                end if;
            end loop;
        end if;
    end if;

    w_we <= v_we;
    wb_tbl_wdata <= vb_tbl_wdata;
  end process;

  wb_tbl_rdata <= tbl(conv_integer(radr));
  o_lru <= wb_tbl_rdata(waybits-1 downto 0);

  reg : process (i_clk) begin
    if rising_edge(i_clk) then 
      radr <= i_addr;
      if w_we = '1' then
        tbl(conv_integer(i_addr)) <= wb_tbl_wdata;
      end if;
    end if;
  end process;


end;
