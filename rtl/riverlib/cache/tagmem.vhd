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
    flbits : integer := 1          -- Total flags number saved with address tag
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
    o_hit : out std_logic
  );
end; 
 
architecture arch_tagmem of tagmem is

  constant TAG_BITS : integer := abus - ibits - lnbits;
  constant TAG_WITH_FLAGS : integer := TAG_BITS + flbits;

  signal wb_index : std_logic_vector(ibits-1 downto 0);

  signal tago_rdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);
  signal tagi_wdata : std_logic_vector(TAG_WITH_FLAGS-1 downto 0);
  signal tagi_we : std_logic;

  signal r_tagi : std_logic_vector(TAG_BITS-1 downto 0);
  signal rin_tagi : std_logic_vector(TAG_BITS-1 downto 0);
  signal r_index : std_logic_vector(ibits-1 downto 0);
  signal rin_index : std_logic_vector(ibits-1 downto 0);

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

  comb : process(i_nrst, i_addr, i_wstrb, i_wdata, i_wflags,
                 tago_rdata, r_tagi, r_index)
    variable v_tagi : std_logic_vector(TAG_BITS-1 downto 0);
    variable v_index : std_logic_vector(ibits-1 downto 0);
    variable v_hit : std_logic;
    variable vb_raddr : std_logic_vector(abus-1 downto 0);
  begin
    v_tagi := r_tagi;

    v_hit := '0';
    if r_tagi = tago_rdata(TAG_BITS-1 downto 0) then
        v_hit := '1';
    end if;

    vb_raddr := (others => '0');
    vb_raddr(abus-1 downto ibits + lnbits) := tago_rdata(TAG_BITS-1 downto 0);
    vb_raddr(ibits + lnbits - 1 downto lnbits) := r_index;
  
    v_index := i_addr(ibits + lnbits - 1 downto lnbits);
    v_tagi := i_addr(abus-1 downto ibits + lnbits);

    if not async_reset and i_nrst = '0' then
        v_index := (others => '0');
        v_tagi := (others => '0');
    end if;

    rin_tagi <= v_tagi;
    rin_index <= v_index;

    tagi_we <= or_reduce(i_wstrb);
	 wb_index <= v_index;
    tagi_wdata <= i_wflags & i_addr(abus - 1 downto ibits + lnbits);

    o_raddr <= vb_raddr;
    o_rflags <= tago_rdata(TAG_WITH_FLAGS-1 downto TAG_BITS);
    o_hit <= v_hit;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r_index <= (others => '0');
        r_tagi <= (others => '0');
     elsif rising_edge(i_clk) then 
        r_index <= rin_index;
        r_tagi <= rin_tagi;
     end if; 
  end process;

end;
