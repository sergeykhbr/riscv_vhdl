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
library techmap;
use techmap.types_mem.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity IWayMem is generic (
    memtech : integer := 0;
    async_reset : boolean;
    wayidx : integer
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_radr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_wadr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_wena : in std_logic;
    i_wstrb : in std_logic_vector(3 downto 0);
    i_wvalid : in std_logic_vector(3 downto 0);
    i_wdata : in std_logic_vector(63 downto 0);
    i_load_fault : in std_logic;
    o_rtag : out std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
    o_rdata : out std_logic_vector(31 downto 0);
    o_valid : out std_logic;
    o_load_fault : out std_logic
  );
end; 
 
architecture arch_IWayMem of IWayMem is

  constant RAM64_BLOCK_TOTAL : integer := (2**CFG_IOFFSET_WIDTH) / 8;

  type RegistersType is record
      roffset : std_logic_vector(CFG_IOFFSET_WIDTH-2 downto 0);  -- 2-bytes alignment
  end record;

  signal r, rin : RegistersType;

  signal datan : std_logic_vector(64*RAM64_BLOCK_TOTAL-1 downto 0);
  signal wb_radr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
  signal wb_wadr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);

  signal wb_tag_rdata : std_logic_vector(CFG_ITAG_WIDTH_TOTAL-1 downto 0);
  signal w_tag_wena : std_logic;
  signal wb_tag_wdata : std_logic_vector(CFG_ITAG_WIDTH_TOTAL-1 downto 0);

  signal wb_data_rdata : std_logic_vector(64*RAM64_BLOCK_TOTAL-1 downto 0);
  signal w_data_wena : std_logic_vector(RAM64_BLOCK_TOTAL-1 downto 0);


begin

  sp : if CFG_SINGLEPORT_CACHE generate
    tag0 : ram_tech generic map (
      memtech => memtech,
      abits => CFG_IINDEX_WIDTH,
      dbits => CFG_ITAG_WIDTH_TOTAL
    ) port map (
      i_clk => i_clk,
      i_addr => wb_radr,
      o_rdata => wb_tag_rdata,
      i_wena => w_tag_wena,
      i_wdata => wb_tag_wdata
    );

    dx : for n in 0 to RAM64_BLOCK_TOTAL-1 generate
      data0 : ram_tech generic map (
        memtech => memtech,
        abits => CFG_IINDEX_WIDTH,
        dbits => 64
      ) port map (
        i_clk => i_clk,
        i_addr => wb_radr,
        o_rdata => wb_data_rdata(64*(n+1)-1 downto 64*n),
        i_wena => w_data_wena(n),
        i_wdata => i_wdata
      );
    end generate;
  end generate;

  dp : if not CFG_SINGLEPORT_CACHE generate
    tag0 : dpram_tech generic map (
      memtech => memtech,
      abits => CFG_IINDEX_WIDTH,
      dbits => CFG_ITAG_WIDTH_TOTAL
    ) port map (
      i_clk => i_clk,
      i_raddr => wb_radr,
      o_rdata => wb_tag_rdata,
      i_waddr => wb_wadr,
      i_wena => w_tag_wena,
      i_wdata => wb_tag_wdata
    );

    dx : for n in 0 to RAM64_BLOCK_TOTAL-1 generate
      data0 : dpram_tech generic map (
        memtech => memtech,
        abits => CFG_IINDEX_WIDTH,
        dbits => 64
      ) port map (
        i_clk => i_clk,
        i_raddr => wb_radr,
        o_rdata => wb_data_rdata(64*(n+1)-1 downto 64*n),
        i_waddr => wb_wadr,
        i_wena => w_data_wena(n),
        i_wdata => i_wdata
      );
    end generate;
  end generate;

  comb : process(i_nrst, i_radr, i_wadr, i_wena, i_wstrb, i_wvalid,
                 i_wdata, i_load_fault, wb_tag_rdata, wb_data_rdata, r)
    variable v : RegistersType;
    variable vb_rdata : std_logic_vector(31 downto 0);
    variable v_valid : std_logic;
  begin

    v := r;

    wb_radr <= i_radr(IINDEX_END downto IINDEX_START);
    wb_wadr <= i_wadr(IINDEX_END downto IINDEX_START);

    w_tag_wena <= i_wena;
    wb_tag_wdata <= i_wadr(ITAG_END downto ITAG_START) &
                   i_load_fault &     -- [4]
                   i_wvalid;          -- [3:0]

    w_data_wena <= (i_wena & i_wena & i_wena & i_wena) and i_wstrb;

    v_valid := '0';
    v.roffset := i_radr(CFG_IOFFSET_WIDTH-1 downto 1);
    case r.roffset is
    when X"0" =>
        vb_rdata := wb_data_rdata(31 downto 0);
        v_valid := wb_tag_rdata(0);
    when X"1" =>
        vb_rdata := wb_data_rdata(47 downto 16);
        v_valid := wb_tag_rdata(0);
    when X"2" =>
        vb_rdata := wb_data_rdata(63 downto 32);
        v_valid := wb_tag_rdata(0);
    when X"3" =>
        vb_rdata := wb_data_rdata(79 downto 48);
        v_valid := wb_tag_rdata(0) and wb_tag_rdata(1);
    when X"4" =>
        vb_rdata := wb_data_rdata(95 downto 64);
        v_valid := wb_tag_rdata(1);
    when X"5" =>
        vb_rdata := wb_data_rdata(111 downto 80);
        v_valid := wb_tag_rdata(1);
    when X"6" =>
        vb_rdata := wb_data_rdata(127 downto 96);
        v_valid := wb_tag_rdata(1);
    when X"7" =>
        vb_rdata := wb_data_rdata(143 downto 112);
        v_valid := wb_tag_rdata(1) and wb_tag_rdata(2);
    when X"8" =>
        vb_rdata := wb_data_rdata(159 downto 128);
        v_valid := wb_tag_rdata(2);
    when X"9" =>
        vb_rdata := wb_data_rdata(175 downto 144);
        v_valid := wb_tag_rdata(2);
    when X"A" =>
        vb_rdata := wb_data_rdata(191 downto 160);
        v_valid := wb_tag_rdata(2);
    when X"B" =>
        vb_rdata := wb_data_rdata(207 downto 176);
        v_valid := wb_tag_rdata(2) and wb_tag_rdata(3);
    when X"C" =>
        vb_rdata := wb_data_rdata(223 downto 192);
        v_valid := wb_tag_rdata(3);
    when X"D" =>
        vb_rdata := wb_data_rdata(239 downto 208);
        v_valid := wb_tag_rdata(3);
    when X"E" =>
        vb_rdata := wb_data_rdata(255 downto 224);
        v_valid := wb_tag_rdata(3);
    when X"F" =>
        vb_rdata(15 downto 0) := wb_data_rdata(255 downto 240);
        vb_rdata(31 downto 16) := (others => '0');
        v_valid := wb_tag_rdata(3);
    when others =>
    end case;


    if not async_reset and i_nrst = '0' then
        v.roffset := (others => '0');
    end if;

    o_rdata <= vb_rdata;
    o_rtag <= wb_tag_rdata(CFG_ITAG_WIDTH_TOTAL-1 downto 5);
    o_valid <= v_valid;
    o_load_fault <= wb_tag_rdata(4);
   
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r.roffset <= (others => '0');
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
