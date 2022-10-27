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

entity tagmem is generic (
    memtech : integer := 0;
    async_reset : boolean := false;
    wayidx : integer := 0;
    abus : integer := 64;          -- system bus address bus (32 or 64 bits)
    ibits : integer := 7;          -- lines memory addres width (usually 6..8)
    lnbits : integer := 5;         -- One line bits: log2(bytes_per_line)
    flbits : integer := 1;         -- Total flags number saved with address tag
    snoop : boolean := false       -- snoop channel (only with enabled L2-cache)
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_addr : in std_logic_vector(abus-1 downto 0);
    i_wstrb : in std_logic_vector(2**lnbits-1 downto 0);
    i_wdata : in std_logic_vector(8*(2**lnbits)-1 downto 0);
    i_wflags : in std_logic_vector(flbits-1 downto 0);
    o_raddr : out std_logic_vector(abus-1 downto 0);
    o_rdata : out std_logic_vector(8*(2**lnbits)-1 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic;
    -- L2 snoop portm active when snoop = 1
    i_snoop_addr : in std_logic_vector(abus-1 downto 0);
    o_snoop_flags : out std_logic_vector(flbits-1 downto 0)
  );
end; 
 
architecture arch_tagmem of tagmem is

  constant TAG_BITS : integer := abus - ibits - lnbits;
  constant TAG_WITH_FLAGS : integer := TAG_BITS + flbits;

  signal wb_index : std_logic_vector(ibits-1 downto 0);

  signal tago_rdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);
  signal tagi_wdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);
  signal tagi_we : std_logic;

  signal wb_snoop_index : std_logic_vector(ibits-1 downto 0);
  signal wb_snoop_tagaddr : std_logic_vector(TAG_BITS-1 downto 0);
  signal tago_snoop_rdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);

  signal rb_tagaddr : std_logic_vector(TAG_BITS-1 downto 0);
  signal rb_index : std_logic_vector(ibits-1 downto 0);
  signal rb_snoop_tagaddr : std_logic_vector(TAG_BITS-1 downto 0);

begin

  -- 1-byte memory banks combining into cache line
  dx : for n in 0 to 2**lnbits-1 generate
    datax : ram_tech generic map (
      memtech => memtech,
      abits => ibits,
      dbits => 8
    ) port map (
      i_clk => i_clk,
      i_addr => wb_index,
      i_wena => i_wstrb(n),
      i_wdata => i_wdata(8*n+7 downto 8*n),
      o_rdata => o_rdata(8*n+7 downto 8*n)
    );
  end generate;

  tag0 : ram_tech generic map (
    memtech => memtech,
    abits => ibits,
    dbits => TAG_WITH_FLAGS
  ) port map (
    i_clk => i_clk,
    i_addr => wb_index,
    i_wena => tagi_we,
    i_wdata => tagi_wdata,
    o_rdata => tago_rdata
  );

  snoopena : if snoop generate
    tagsnoop0 : ram_tech generic map (
      memtech => memtech,
      abits => ibits,
      dbits => TAG_WITH_FLAGS
    ) port map (
      i_clk => i_clk,
      i_addr => wb_snoop_index,
      i_wena => tagi_we,
      i_wdata => tagi_wdata,
      o_rdata => tago_snoop_rdata
    );
  end generate;
  snoopdis : if not snoop generate
    tago_snoop_rdata <= (others => '0');
  end generate;

  comb : process(i_nrst, i_addr, i_wstrb, i_wdata, i_wflags,
                 tago_rdata, tago_snoop_rdata, rb_tagaddr, rb_index)
    variable vb_index : std_logic_vector(ibits-1 downto 0);
    variable vb_raddr : std_logic_vector(abus-1 downto 0);
    variable vb_tagi_wdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);
    variable v_hit : std_logic;
    variable vb_snoop_index : std_logic_vector(ibits-1 downto 0);
    variable vb_snoop_tagaddr : std_logic_vector(TAG_BITS-1 downto 0);
    variable vb_snoop_flags : std_logic_vector(flbits-1 downto 0);
  begin
    v_hit := '0';
    if rb_tagaddr = tago_rdata(TAG_BITS-1 downto 0) then
        v_hit := tago_rdata(TAG_BITS);  -- valid bit
    end if;

    vb_raddr := (others => '0');
    vb_raddr(abus-1 downto ibits + lnbits) := tago_rdata(TAG_BITS-1 downto 0);
    vb_raddr(ibits + lnbits - 1 downto lnbits) := rb_index;
  
    vb_index := i_addr(ibits + lnbits - 1 downto lnbits);
    vb_tagi_wdata := i_wflags & i_addr(abus-1 downto ibits + lnbits);

    if snoop then
        vb_snoop_flags := tago_snoop_rdata(TAG_WITH_FLAGS-1 downto TAG_BITS);
        vb_snoop_index := i_snoop_addr(ibits + lnbits - 1 downto lnbits);
        vb_snoop_tagaddr := i_snoop_addr(abus - 1 downto ibits + lnbits);

        if or_reduce(i_wstrb) = '1' then
            vb_snoop_index := vb_index;
        end if;

        if rb_snoop_tagaddr /= tago_snoop_rdata(TAG_BITS-1 downto 0) then
            vb_snoop_flags := (others => '0');
        end if;
    else
        vb_snoop_flags := (others => '0');
        vb_snoop_index := (others => '0');
        vb_snoop_tagaddr := (others => '0');
    end if;

    if not async_reset and i_nrst = '0' then
        vb_tagi_wdata := (others => '0');
        vb_index := (others => '0');
    end if;

    wb_index <= vb_index;
    tagi_we <= or_reduce(i_wstrb);
    tagi_wdata <= vb_tagi_wdata;

    o_raddr <= vb_raddr;
    o_rflags <= tago_rdata(TAG_WITH_FLAGS-1 downto TAG_BITS);
    o_hit <= v_hit;

    wb_snoop_index <= vb_snoop_index;
    wb_snoop_tagaddr <= vb_snoop_tagaddr;
    o_snoop_flags <= vb_snoop_flags;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        rb_tagaddr <= (others => '0');
        rb_index <= (others => '0');
        rb_snoop_tagaddr <= (others => '0');
     elsif rising_edge(i_clk) then 
        rb_tagaddr <= tagi_wdata(TAG_BITS-1 downto 0);
        rb_index <= wb_index;
        rb_snoop_tagaddr <= wb_snoop_tagaddr;
     end if; 
  end process;

end;
