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
    flbits : integer := 1          -- Total flags number saved with address tag
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_cs : in std_logic;
    i_flush : in std_logic;
    i_addr : in std_logic_vector(abus-1 downto 0);
    i_wdata : in std_logic_vector(8*(2**lnbits)-1 downto 0);
    i_wstrb : in std_logic_vector(2**lnbits-1 downto 0);
    i_wflags : in std_logic_vector(flbits-1 downto 0);
    o_raddr : out std_logic_vector(abus-1 downto 0);
    o_rdata : out std_logic_vector(8*(2**lnbits)-1 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic
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
  end record;

  type WayOutType is record
      raddr : std_logic_vector(abus-1 downto 0);
      rdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
      rflags : std_logic_vector(flbits-1 downto 0);
      hit : std_logic;
  end record;

  type way_in_vector is array (0 to NWAYS-1) of WayInType;
  type way_out_vector is array (0 to NWAYS-1) of WayOutType;

  type RegistersType is record
      req_addr : std_logic_vector(abus-1 downto 0);
      re : std_logic;
  end record;

  constant R_RESET : RegistersType := ((others => '0'), '0');

  signal way_i : way_in_vector;
  signal way_o : way_out_vector;

  signal lrui_flush : std_logic;
  signal lrui_addr : std_logic_vector(ibits-1 downto 0);
  signal lrui_we : std_logic;
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
      flbits => flbits
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
      o_hit => way_o(i).hit
    );
  end generate;

  lru0 : lrunway generic map (
    abits => ibits,
    waybits => waybits
  ) port map (
    i_clk => i_clk,
    i_flush => lrui_flush,
    i_addr => lrui_addr,
    i_we => lrui_we,
    i_lru => lrui_lru,
    o_lru => lruo_lru
  );

  comb : process(i_nrst, i_cs, i_flush, i_addr, i_wstrb, i_wdata, i_wflags,
                 way_o, lruo_lru, r)
    variable v : RegistersType;
    variable vb_wayidx_o : std_logic_vector(waybits-1 downto 0);
    variable vb_lineadr : std_logic_vector(ibits-1 downto 0);
    variable v_lrui_we : std_logic;
    variable hit : std_logic;
    variable mux_raddr : std_logic_vector(abus-1 downto 0);
    variable mux_rdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
    variable mux_rflags : std_logic_vector(flbits-1 downto 0);
  begin

    hit := '0';
    v_lrui_we := '0';
    v.req_addr := i_addr;
    v.re := i_cs;

    vb_lineadr := i_addr(ibits+lnbits-1 downto lnbits);

    v_lrui_we := i_cs and or_reduce(i_wstrb);

    vb_wayidx_o := lruo_lru;
    for i in 0 to NWAYS-1 loop
        if way_o(i).hit = '1' and
                    way_o(i).rflags(FL_VALID) = '1' then
            hit := '1';
            vb_wayidx_o := conv_std_logic_vector(i, waybits);
            v_lrui_we := r.re;
        end if;
    end loop;

    mux_raddr := way_o(conv_integer(vb_wayidx_o)).raddr;
    mux_rdata := way_o(conv_integer(vb_wayidx_o)).rdata;
    mux_rflags := way_o(conv_integer(vb_wayidx_o)).rflags;

    --  Warning: we can write only into previously read line,
    --              if the previuosly read line is hit and contains valid flags
    --              HIGH we modify it. Otherwise, we write into displacing line.
    --
    for i in 0 to NWAYS-1 loop
        way_i(i).addr <= i_addr;
        way_i(i).wdata <= i_wdata;
        if (i_flush = '1' and i_addr(waybits-1 downto 0) = conv_std_logic_vector(i, waybits))
            or (i_cs = '1' and vb_wayidx_o = conv_std_logic_vector(i, waybits)) then
            way_i(i).wstrb <= i_wstrb;
        else
            way_i(i).wstrb <= (others => '0');
        end if;
        way_i(i).wflags <= i_wflags;
    end loop;

    lrui_flush <= i_flush;
    lrui_addr <= vb_lineadr;
    lrui_we <= v_lrui_we;
    lrui_lru <= vb_wayidx_o;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;

    o_raddr <= mux_raddr;
    o_rdata <= mux_rdata;
    o_rflags <= mux_rflags;
    o_hit <= hit;
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
