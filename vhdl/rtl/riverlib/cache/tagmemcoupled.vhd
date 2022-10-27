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

entity tagmemcoupled is generic (
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
    i_direct_access : in std_logic;
    i_invalidate : in std_logic;
    i_re : in std_logic;
    i_we : in std_logic;
    i_addr : in std_logic_vector(abus-1 downto 0);
    i_wdata : in std_logic_vector(8*(2**lnbits)-1 downto 0);
    i_wstrb : in std_logic_vector(2**lnbits-1 downto 0);
    i_wflags : in std_logic_vector(flbits-1 downto 0);
    o_raddr : out std_logic_vector(abus-1 downto 0);
    o_rdata : out std_logic_vector(8*(2**lnbits)+15 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic;
    o_hit_next : out std_logic
  );
end; 
 
architecture arch_tagmemcoupled of tagmemcoupled is

  constant TAG_START : integer := abus -  (ibits + lnbits);
  constant EVEN : integer := 0;
  constant ODD : integer := 1;
  constant MemTotal : integer := 2;

  type tagmem_in_type is record
      direct_access : std_logic;
      invalidate : std_logic;
      re : std_logic;
      we : std_logic;
      addr : std_logic_vector(abus-1 downto 0);
      wdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
      wstrb : std_logic_vector((2**lnbits)-1 downto 0);
      wflags : std_logic_vector(flbits-1 downto 0);
      snoop_addr : std_logic_vector(abus-1 downto 0);
  end record;

  type tagmem_out_type is record
      raddr : std_logic_vector(abus-1 downto 0);
      rdata : std_logic_vector(8*(2**lnbits)-1 downto 0);
      rflags : std_logic_vector(flbits-1 downto 0);
      hit : std_logic;
      snoop_ready : std_logic;
      snoop_flags : std_logic_vector(flbits-1 downto 0);
  end record;

  type tagmem_in_vector is array (0 to MemTotal-1) of tagmem_in_type;
  type tagmem_out_vector is array (0 to MemTotal-1) of tagmem_out_type;

  signal r_req_addr : std_logic_vector(abus-1 downto 0);

  signal linei : tagmem_in_vector;
  signal lineo : tagmem_out_vector;

begin

  dx : for i in 0 to MemTotal-1 generate
    memx : tagmemnway generic map (
      async_reset => async_reset,
      memtech => memtech,
      abus => abus,
      waybits => waybits,
      ibits => ibits - 1,
      lnbits => lnbits,
      flbits => flbits,
      snoop => false
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_direct_access => linei(i).direct_access,
      i_invalidate => linei(i).invalidate,
      i_re => linei(i).re,
      i_we => linei(i).we,
      i_addr => linei(i).addr,
      i_wdata => linei(i).wdata,
      i_wstrb => linei(i).wstrb,
      i_wflags => linei(i).wflags,
      o_raddr => lineo(i).raddr,
      o_rdata => lineo(i).rdata,
      o_rflags => lineo(i).rflags,
      o_hit => lineo(i).hit,
      i_snoop_addr => linei(i).snoop_addr,
      o_snoop_ready => lineo(i).snoop_ready,
      o_snoop_flags => lineo(i).snoop_flags
    );
  end generate;


  comb : process(i_nrst, i_direct_access, i_invalidate, i_re, i_we,
                 i_addr, i_wstrb, i_wdata, i_wflags,
                 lineo, r_req_addr)
    variable v_addr_sel : std_logic;
    variable v_addr_sel_r : std_logic;
    variable v_use_overlay : std_logic;
    variable v_use_overlay_r : std_logic;
    variable vb_index : std_logic_vector(ibits-1 downto 0);
    variable vb_index_next : std_logic_vector(ibits-1 downto 0);
    variable vb_addr_next : std_logic_vector(abus-1 downto 0);
    variable vb_addr_tag_direct : std_logic_vector(abus-1 downto 0);
    variable vb_addr_tag_next : std_logic_vector(abus-1 downto 0);
    variable vb_raddr_tag : std_logic_vector(abus-1 downto 0);
    variable vb_o_raddr : std_logic_vector(abus-1 downto 0);
    variable vb_o_rdata : std_logic_vector(8*(2**lnbits)+15 downto 0);
    variable v_o_hit : std_logic;
    variable v_o_hit_next : std_logic;
    variable vb_o_rflags : std_logic_vector(flbits-1 downto 0);
  begin

    v_addr_sel := i_addr(lnbits);
    v_addr_sel_r := r_req_addr(lnbits);

    vb_addr_next := i_addr + (2**lnbits);

    vb_index := i_addr(ibits+lnbits-1 downto lnbits);
    vb_index_next := vb_addr_next(ibits+lnbits-1 downto lnbits);

    v_use_overlay := and_reduce(i_addr(lnbits-1 downto 1));
    v_use_overlay_r := and_reduce(r_req_addr(lnbits-1 downto 1));


    -- Change the bit order in the requested address:
    --    [tag][line_idx][odd/evenbit][line_bytes] on
    --    [tag][1'b0]    [line_idx]   [line_bytes]
    --
    -- Example (abus=32; ibits=7; lnbits=5;):
    --   [4:0]   byte in line           [4:0]
    --   [11:5]  line index             {[1'b0],[11:6]}
    --   [31:12] tag                    [31:12]
    vb_addr_tag_direct := i_addr;
    vb_addr_tag_direct(ibits + lnbits - 1 downto lnbits) := '0' & vb_index(ibits-1 downto 1);

    vb_addr_tag_next := vb_addr_next;
    vb_addr_tag_next(ibits + lnbits - 1 downto lnbits) := '0' & vb_index_next(ibits-1 downto 1);

    if v_addr_sel = '0' then
        linei(EVEN).addr <= vb_addr_tag_direct;
        linei(EVEN).wstrb <= i_wstrb;
        linei(ODD).addr <= vb_addr_tag_next;
        linei(ODD).wstrb <= (others => '0');
    else
        linei(EVEN).addr <= vb_addr_tag_next;
        linei(EVEN).wstrb <= (others => '0');
        linei(ODD).addr <=  vb_addr_tag_direct;
        linei(ODD).wstrb <= i_wstrb;
    end if;

    linei(EVEN).direct_access <= i_direct_access and ((not v_addr_sel) or v_use_overlay);
    linei(ODD).direct_access <= i_direct_access and (v_addr_sel or v_use_overlay);

    linei(EVEN).invalidate <= i_invalidate and ((not v_addr_sel) or v_use_overlay);
    linei(ODD).invalidate <= i_invalidate and (v_addr_sel or v_use_overlay);

    linei(EVEN).re <= i_re and ((not v_addr_sel) or v_use_overlay);
    linei(ODD).re <= i_re and (v_addr_sel or v_use_overlay);

    linei(EVEN).we <= i_we and ((not v_addr_sel) or v_use_overlay);
    linei(ODD).we <= i_we and (v_addr_sel or v_use_overlay);

    linei(EVEN).wdata <= i_wdata;
    linei(ODD).wdata <= i_wdata;

    linei(EVEN).wflags <= i_wflags;
    linei(ODD).wflags <= i_wflags;

    -- Form output:
    if v_addr_sel_r = '0' then
        vb_o_rdata := lineo(ODD).rdata(15 downto 0) & lineo(EVEN).rdata;
        vb_raddr_tag := lineo(EVEN).raddr;
        vb_o_rflags := lineo(EVEN).rflags;

        v_o_hit := lineo(EVEN).hit;
        if v_use_overlay_r = '0' then
            v_o_hit_next := lineo(EVEN).hit;
        else
            v_o_hit_next := lineo(ODD).hit;
        end if;
    else
        vb_o_rdata := lineo(EVEN).rdata(15 downto 0) & lineo(ODD).rdata;
        vb_raddr_tag := lineo(ODD).raddr;
        vb_o_rflags := lineo(ODD).rflags;

        v_o_hit := lineo(ODD).hit;
        if v_use_overlay_r = '0' then
            v_o_hit_next := lineo(ODD).hit;
        else
            v_o_hit_next := lineo(EVEN).hit;
        end if;
    end if;

    vb_o_raddr := vb_raddr_tag;
    vb_o_raddr(lnbits) := v_addr_sel_r;
    vb_o_raddr(ibits + lnbits - 1 downto lnbits + 1) :=
                    vb_raddr_tag(ibits + lnbits - 2 downto lnbits);

    o_raddr <= vb_o_raddr;
    o_rdata <= vb_o_rdata;
    o_rflags <= vb_o_rflags;
    o_hit <= v_o_hit;
    o_hit_next <= v_o_hit_next;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r_req_addr <= (others => '0');
     elsif rising_edge(i_clk) then 
        r_req_addr <= i_addr;
     end if; 
  end process;

end;
