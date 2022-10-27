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

entity queue is generic (
    async_reset : boolean := false;
    szbits : integer := 2;
    dbits : integer := 32
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_re : in std_logic;
    i_we : in std_logic;
    i_wdata : in std_logic_vector(dbits-1 downto 0);
    o_rdata : out std_logic_vector(dbits-1 downto 0);
    o_full : out std_logic;
    o_nempty : out std_logic
  );
end; 
 
architecture arch_queue of queue is

  constant QUEUE_DEPTH : integer := 2**szbits;

  constant QUEUE_FULL : std_logic_vector(szbits downto 0) :=
	conv_std_logic_vector(QUEUE_DEPTH, szbits+1);
  constant QUEUE_ALMOST_FULL : std_logic_vector(szbits downto 0) :=
	conv_std_logic_vector(QUEUE_DEPTH-1, szbits+1);
  constant cnt_zero : std_logic_vector(szbits downto 0) := (others => '0');

  type MemoryType is array (0 to QUEUE_DEPTH-1) 
         of std_logic_vector(dbits-1 downto 0);

  type RegistersType is record
      wcnt : std_logic_vector(szbits downto 0);
      mem : MemoryType;
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_we, i_re, i_wdata, r)
    variable v : RegistersType;
    variable vb_data_o : std_logic_vector(dbits-1 downto 0);
    variable nempty : std_logic;
    variable full : std_logic;
    variable show_full : std_logic;
  begin

    v := r;

    full := '0';
    show_full := '0';
    if r.wcnt = QUEUE_FULL then
        full := '1';
    end if;
    if r.wcnt >= QUEUE_ALMOST_FULL then
        show_full := '1';
    end if;

    if i_re = '1' and i_we = '1' then
        for i in 1 to QUEUE_DEPTH-1 loop
            v.mem(i-1) := r.mem(i);
        end loop;
        if r.wcnt /= cnt_zero then
            v.mem(conv_integer(r.wcnt) - 1) := i_wdata;
        else
            -- do nothing, it will directly pass to output
        end if;
    elsif i_re = '0' and i_we = '1' then
        if full = '0' then
            v.wcnt := r.wcnt + 1;
            v.mem(conv_integer(r.wcnt)) := i_wdata;
        end if;
    elsif i_re = '1' and i_we = '0' then
        if r.wcnt /= cnt_zero then
            v.wcnt := r.wcnt - 1;
        end if;
        for i in 1 to QUEUE_DEPTH-1 loop
            v.mem(i-1) := r.mem(i);
        end loop;
    end if;

    if r.wcnt = cnt_zero then
        vb_data_o := i_wdata;
    else
        vb_data_o := r.mem(0);
    end if;

    nempty := '0';
    if i_we = '1' or r.wcnt /= cnt_zero then
        nempty := '1';
    end if;


    if not async_reset and i_nrst = '0' then
        v.wcnt := (others => '0');
        for i in 0 to QUEUE_DEPTH-1 loop
            v.mem(i) := (others => '0');
        end loop;
    end if;

    rin <= v;
    
    o_nempty <= nempty;
    o_full <= show_full;
    o_rdata <= vb_data_o;
  end process;

  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        r.wcnt <= (others => '0');
        for i in 0 to QUEUE_DEPTH-1 loop
            r.mem(i) <= (others => '0');
        end loop;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
