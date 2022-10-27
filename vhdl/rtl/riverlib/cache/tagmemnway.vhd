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
use ieee.std_logic_misc.all;  -- or_reduce()
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.types_mem.all;
library riverlib;
use riverlib.types_cache.all;

entity tagmemnway is generic (
    memtech : integer := 0;
    async_reset : boolean := false;
    abus : integer := 64;          -- system bus address bus (32 or 64 bits)
    waybits : integer := 2;        -- log2 of number of ways bits (=2 for 4 ways)
    ibits : integer := 7;          -- lines memory addres width (usually 6..8)
    lnbits : integer := 5;         -- One line bits: log2(bytes_per_line)
    flbits : integer := 1;         -- Total flags number saved with address tag
    snoop : boolean := false       -- Snoop port disabled; 1 Enabled (L2 caching)
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_direct_access : in std_logic;
    i_invalidate : in std_logic;
    i_re : in std_logic;
    i_we : in std_logic;
    i_addr : in std_logic_vector(abus-1 downto 0);
    i_wdata : in std_logic_vector(8*(2**lnbits)-1 downto 0);
    i_wstrb : in std_logic_vector(2**lnbits-1 downto 0);
    i_wflags : in std_logic_vector(flbits-1 downto 0);
    o_raddr : out std_logic_vector(abus-1 downto 0);
    o_rdata : out std_logic_vector(8*(2**lnbits)-1 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic;
    -- L2 snoop port, active when snoop = 1
    i_snoop_addr : in std_logic_vector(abus-1 downto 0);
    o_snoop_ready : out std_logic;
    o_snoop_flags : out std_logic_vector(flbits-1 downto 0)
  );
end; 
 
architecture arch_tagmemnway of tagmemnway is

  constant FL_VALID : integer := 0;
  constant NWAYS : integer := 2**waybits;

  type WayInType is record
      addr : std_logic_vector(abus-1 downto 0);
      wstrb : std_logic_vector((2**lnbits)-1 downto 0);
      wdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
      wflags : std_logic_vector(flbits-1 downto 0);
      snoop_addr : std_logic_vector(abus-1 downto 0);
  end record;

  type WayOutType is record
      raddr : std_logic_vector(abus-1 downto 0);
      rdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
      rflags : std_logic_vector(flbits-1 downto 0);
      hit : std_logic;
      snoop_flags : std_logic_vector(flbits-1 downto 0);
  end record;

  type way_in_vector is array (0 to NWAYS-1) of WayInType;
  type way_out_vector is array (0 to NWAYS-1) of WayOutType;

  type RegistersType is record
      req_addr : std_logic_vector(abus-1 downto 0);
      direct_access : std_logic;
      invalidate : std_logic;
      re : std_logic;
  end record;

  constant R_RESET : RegistersType := ((others => '0'), '0', '0', '0');

  signal way_i : way_in_vector;
  signal way_o : way_out_vector;

  signal lrui_init : std_logic;
  signal lrui_raddr : std_logic_vector(ibits-1 downto 0);
  signal lrui_waddr : std_logic_vector(ibits-1 downto 0);
  signal lrui_up : std_logic;
  signal lrui_down : std_logic;
  signal lrui_lru : std_logic_vector(waybits-1 downto 0);
  signal lruo_lru : std_logic_vector(waybits-1 downto 0);

  signal r, rin : RegistersType;

begin

  dx : for i in 0 to NWAYS-1 generate
    wayx : tagmem generic map (
      async_reset => async_reset,
      memtech => memtech,
      abus => abus,
      ibits => ibits,
      lnbits => lnbits,
      flbits => flbits,
      snoop => snoop
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_addr => way_i(i).addr,
      i_wstrb => way_i(i).wstrb,
      i_wdata => way_i(i).wdata,
      i_wflags => way_i(i).wflags,
      o_raddr => way_o(i).raddr,
      o_rdata => way_o(i).rdata,
      o_rflags => way_o(i).rflags,
      o_hit => way_o(i).hit,
      i_snoop_addr => way_i(i).snoop_addr,
      o_snoop_flags => way_o(i).snoop_flags
    );
  end generate;

  lru0 : lrunway generic map (
    abits => ibits,
    waybits => waybits
  ) port map (
    i_clk => i_clk,
    i_init => lrui_init,
    i_raddr => lrui_raddr,
    i_waddr => lrui_waddr,
    i_up => lrui_up,
    i_down => lrui_down,
    i_lru => lrui_lru,
    o_lru => lruo_lru
  );

  comb : process(i_nrst, i_direct_access, i_invalidate, i_re, i_we,
                 i_addr, i_wstrb, i_wdata, i_wflags, i_snoop_addr,
                 way_o, lruo_lru, r)
    variable v : RegistersType;
    variable vb_raddr : std_logic_vector(abus-1 downto 0);
    variable vb_rdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
    variable vb_rflags : std_logic_vector(flbits-1 downto 0);
    variable v_hit : std_logic;
    variable vb_hit_idx : std_logic_vector(waybits-1 downto 0);
    variable v_way_we : std_logic;
    variable vb_wstrb : std_logic_vector((2**lnbits)-1 downto 0);
    variable vb_wflags : std_logic_vector(flbits-1 downto 0);

    variable v_snoop_ready : std_logic;
    variable vb_snoop_flags : std_logic_vector(flbits-1 downto 0);
  begin

    v.direct_access := i_direct_access;
    v.invalidate := i_invalidate;
    v.re := i_re;
    v.req_addr := i_addr;

    vb_hit_idx := lruo_lru;
    if r.direct_access = '1' then
        vb_hit_idx := r.req_addr(waybits-1 downto 0);
    else
        for i in 0 to NWAYS-1 loop
            if way_o(i).hit = '1' then
                vb_hit_idx := conv_std_logic_vector(i, waybits);
            end if;
        end loop;
    end if;

    vb_raddr := way_o(conv_integer(vb_hit_idx)).raddr;
    vb_rdata := way_o(conv_integer(vb_hit_idx)).rdata;
    vb_rflags := way_o(conv_integer(vb_hit_idx)).rflags;
    v_hit := way_o(conv_integer(vb_hit_idx)).hit;

    if r.invalidate = '1' then
        vb_wflags := (others => '0');
        vb_wstrb := (others => '1');
    else
        vb_wflags := i_wflags;
        vb_wstrb := i_wstrb;
    end if;


    --  Warning: we can write only into previously read line,
    --              if the previuosly read line is hit and contains valid flags
    --              HIGH we modify it. Otherwise, we write into displacing line.
    --
    for i in 0 to NWAYS-1 loop
        way_i(i).addr <= i_addr;
        way_i(i).wdata <= i_wdata;
        way_i(i).wstrb <= (others => '0');
        way_i(i).wflags <= vb_wflags;
        way_i(i).snoop_addr <= i_snoop_addr;
    end loop;

    v_way_we := i_we or (r.invalidate and v_hit);
    if v_way_we = '1' then
        way_i(conv_integer(vb_hit_idx)).wstrb <= vb_wstrb;
    end if;

    v_snoop_ready := '1';
    vb_snoop_flags := (others => '0');
    if snoop then
        for i in 0 to NWAYS-1 loop
            -- tagmem already cleared snoop flags if there's no snoop hit
            if way_o(i).snoop_flags(FL_VALID) = '1' then
                vb_snoop_flags := way_o(i).snoop_flags;
            end if;
        end loop;
        -- Writing into snoop tag memory, output value won't be valid on next clock
        if v_way_we = '1' then
            v_snoop_ready := '0';
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    lrui_init <= r.direct_access;
    lrui_raddr <= i_addr(ibits+lnbits-1 downto lnbits);
    lrui_waddr <= r.req_addr(ibits+lnbits-1 downto lnbits);
    lrui_up <= i_we or (v_hit and r.re);
    lrui_down <= v_hit and r.invalidate;
    lrui_lru <= vb_hit_idx;

    rin <= v;

    o_raddr <= vb_raddr;
    o_rdata <= vb_rdata;
    o_rflags <= vb_rflags;
    o_hit <= v_hit;
    o_snoop_ready <= v_snoop_ready;
    o_snoop_flags <= vb_snoop_flags;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
