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

entity ICacheLru is generic (
    memtech : integer;
    async_reset : boolean;
    index_width : integer
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- Control path:
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    o_resp_ctrl_load_fault : out std_logic;
    i_resp_ctrl_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_req_mem_len : out std_logic_vector(7 downto 0);
    o_req_mem_burst : out std_logic_vector(1 downto 0);
    o_req_mem_last : out std_logic;
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_load_fault : in std_logic;
    -- Debug Signals:
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic;                                      -- address to clear icache is valid
    o_istate : out std_logic_vector(1 downto 0)
  );
end; 
 
architecture arch_ICacheLru of ICacheLru is

  constant FLUSH_ALL_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"FFFF0000";
  constant flush_cnt_zero : std_logic_vector(CFG_IINDEX_WIDTH downto 0) := (others => '0');

  constant MISS : integer := CFG_ICACHE_WAYS;

  constant WAY_EVEN : integer := 0;
  constant WAY_ODD : integer := 1;

  constant State_Idle : std_logic_vector(2 downto 0) := "000";
  constant State_CheckHit : std_logic_vector(2 downto 0) := "001";
  constant State_WaitGrant : std_logic_vector(2 downto 0) := "010";
  constant State_WaitResp : std_logic_vector(2 downto 0) := "011";
  constant State_CheckResp : std_logic_vector(2 downto 0) := "100";
  constant State_SetupReadAdr : std_logic_vector(2 downto 0) := "101";
  constant State_Flush : std_logic_vector(2 downto 0) := "110";

  constant offset_ones : std_logic_vector(CFG_IOFFSET_WIDTH-1 downto 1) := (others => '1');

  type TagMemInType is record
      radr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      wadr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      wstrb : std_logic_vector(3 downto 0);
      wvalid : std_logic_vector(3 downto 0);
      wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      load_fault : std_logic;
  end record;

  type TagMemInVector is array (0 to 1) of TagMemInType;
 

  type TagMemOutType is record
      rtag : std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
      rdata : std_logic_vector(31 downto 0);
      valid : std_logic;
      load_fault : std_logic;
  end record;

  type TagMemOutVector is array (0 to CFG_ICACHE_WAYS-1) of TagMemOutType;


  type WayMuxType is record
      hit : integer range 0 to CFG_ICACHE_WAYS;
      rdata : std_logic_vector(31 downto 0);
      valid : std_logic;
      load_fault : std_logic;
  end record;

  type WayMuxVector is array (0 to 1) of WayMuxType;


  type LruInType is record
      init : std_logic;
      radr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
      wadr : std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
      we : std_logic;
      lru : std_logic_vector(1 downto 0);
  end record;

  type LruInVector is array (0 to 1) of LruInType;

  type RegistersType is record
      requested : std_logic;
      req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_addr_overlay : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      use_overlay : std_logic;
      state : std_logic_vector(2 downto 0);
      req_mem_valid : std_logic;
      mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      burst_cnt : integer range 0 to 3;
      burst_wstrb : std_logic_vector(3 downto 0);
      burst_valid : std_logic_vector(3 downto 0);
      lru_even_wr : std_logic_vector(1 downto 0);
      lru_odd_wr : std_logic_vector(1 downto 0);
      req_flush : std_logic;
      req_flush_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_flush_cnt : std_logic_vector(CFG_IINDEX_WIDTH downto 0);
      flush_cnt : std_logic_vector(CFG_IINDEX_WIDTH downto 0);
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'), (others => '0'),  -- requested, req_addr, req_addr_overlay
    '0', State_Flush,                       -- use_overlay, state
    '0', FLUSH_ALL_ADDR,                    -- req_mem_valid, mem_addr,
    0, "1111", "0000",                      -- burst_cnt, burst_wstrb, burst_valid
    "00", "00", '0',                        -- lru_even_wr, lru_odd_wr, req_flush
    (others => '0'), (others => '0'),       -- req_flush_addr, req_flush_cnt
    (others => '1')                         -- flush_cnt
  );

  signal r, rin : RegistersType;
  signal swapin : TagMemInVector;
  signal memeven : TagMemOutVector;
  signal memodd : TagMemOutVector;
  signal wb_ena_even : std_logic_vector(CFG_ICACHE_WAYS-1 downto 0);
  signal wb_ena_odd : std_logic_vector(CFG_ICACHE_WAYS-1 downto 0);

  signal lrui : LruInVector;
  signal wb_lru_even : std_logic_vector(1 downto 0);
  signal wb_lru_odd : std_logic_vector(1 downto 0);

begin

  wayx : for n in 0 to CFG_ICACHE_WAYS-1 generate
      wayevenx : IWayMem generic map (
          memtech => memtech,
          async_reset => async_reset,
          wayidx => 2*n
      ) port map (
          i_clk => i_clk,
          i_nrst => i_nrst,
          i_radr => swapin(WAY_EVEN).radr,
          i_wadr => swapin(WAY_EVEN).wadr,
          i_wena => wb_ena_even(n),
          i_wstrb => swapin(WAY_EVEN).wstrb,
          i_wvalid => swapin(WAY_EVEN).wvalid,
          i_wdata => swapin(WAY_EVEN).wdata,
          i_load_fault => swapin(WAY_EVEN).load_fault,
          o_rtag => memeven(n).rtag,
          o_rdata => memeven(n).rdata,
          o_valid => memeven(n).valid,
          o_load_fault => memeven(n).load_fault
      );

      wayoddx : IWayMem generic map (
          async_reset => async_reset,
          wayidx => 2*n + 1
      ) port map (
          i_clk => i_clk,
          i_nrst => i_nrst,
          i_radr => swapin(WAY_ODD).radr,
          i_wadr => swapin(WAY_ODD).wadr,
          i_wena => wb_ena_odd(n),
          i_wstrb => swapin(WAY_ODD).wstrb,
          i_wvalid => swapin(WAY_ODD).wvalid,
          i_wdata => swapin(WAY_ODD).wdata,
          i_load_fault => swapin(WAY_ODD).load_fault,
          o_rtag => memodd(n).rtag,
          o_rdata => memodd(n).rdata,
          o_valid => memodd(n).valid,
          o_load_fault => memodd(n).load_fault
      );
  end generate;

  lrueven0 : ILru port map (
      i_clk => i_clk,
      i_init => lrui(WAY_EVEN).init,
      i_radr => lrui(WAY_EVEN).radr,
      i_wadr => lrui(WAY_EVEN).wadr,
      i_we => lrui(WAY_EVEN).we,
      i_lru => lrui(WAY_EVEN).lru,
      o_lru => wb_lru_even
  );

  lruodd0 : ILru port map (
      i_clk => i_clk,
      i_init => lrui(WAY_ODD).init,
      i_radr => lrui(WAY_ODD).radr,
      i_wadr => lrui(WAY_ODD).wadr,
      i_we => lrui(WAY_ODD).we,
      i_lru => lrui(WAY_ODD).lru,
      o_lru => wb_lru_odd
  );

  comb : process(i_nrst, i_req_ctrl_valid, i_req_ctrl_addr,
                i_resp_ctrl_ready, i_req_mem_ready, 
                i_resp_mem_data_valid, i_resp_mem_data, i_resp_mem_load_fault, 
                i_flush_address, i_flush_valid,
                memeven, memodd, wb_lru_even, wb_lru_odd, r)
    variable v : RegistersType;
    variable waysel : WayMuxVector;
    variable w_raddr5 : std_logic;
    variable w_raddr5_r : std_logic;
    variable w_use_overlay : std_logic;
    variable wb_req_adr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_radr_overlay : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_rtag : std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
    variable wb_rtag_overlay : std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
    variable wb_rtag_even : std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
    variable wb_rtag_odd : std_logic_vector(CFG_ITAG_WIDTH-1 downto 0);
    variable wb_hit0 : integer range 0 to CFG_ICACHE_WAYS;
    variable wb_hit1 : integer range 0 to CFG_ICACHE_WAYS;
    variable w_hit0_valid : std_logic;
    variable w_hit1_valid : std_logic;
    variable wb_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_o_resp_data : std_logic_vector(31 downto 0);
    variable v_init : std_logic;
    variable w_ena : std_logic;
    variable w_last : std_logic;
    variable w_o_resp_valid : std_logic;
    variable w_o_resp_load_fault : std_logic;
    variable w_o_req_ctrl_ready : std_logic;
    variable wb_wstrb_next : std_logic_vector(3 downto 0);
    variable v_swapin : TagMemInVector;
    variable v_lrui : LruInVector;
    variable vb_ena_even : std_logic_vector(CFG_ICACHE_WAYS-1 downto 0);
    variable vb_ena_odd : std_logic_vector(CFG_ICACHE_WAYS-1 downto 0);
  begin

    v := r;

    wb_mem_addr := (others => '0');

    if i_req_ctrl_valid = '1' then
        wb_req_adr := i_req_ctrl_addr;
    else
        wb_req_adr := r.req_addr;
    end if;

    w_raddr5 := wb_req_adr(CFG_IOFFSET_WIDTH);
    w_raddr5_r := r.req_addr(CFG_IOFFSET_WIDTH);

    w_use_overlay := '0';
    if wb_req_adr(CFG_IOFFSET_WIDTH-1 downto 1) = offset_ones then
        w_use_overlay := '1';
    end if;

    wb_radr_overlay(BUS_ADDR_WIDTH-1 downto CFG_IOFFSET_WIDTH) := 
        wb_req_adr(BUS_ADDR_WIDTH-1 downto CFG_IOFFSET_WIDTH) + 1;
    wb_radr_overlay(CFG_IOFFSET_WIDTH-1 downto 0) := (others => '0');

    -- flush request via debug interface
    if i_flush_valid = '1' then
        v.req_flush := '1';
        if i_flush_address(0) = '1' then
            v.req_flush_cnt := (others => '1');
            v.req_flush_addr := FLUSH_ALL_ADDR;
        elsif i_flush_address(CFG_IOFFSET_WIDTH-1 downto 1) = offset_ones then
            v.req_flush_cnt := conv_std_logic_vector(1, CFG_IINDEX_WIDTH+1);
            v.req_flush_addr := i_flush_address;
        else
            v.req_flush_cnt := (others => '0');
            v.req_flush_addr := i_flush_address;
        end if;
    end if;

    -- Check read tag and select hit way
    wb_rtag := r.req_addr(ITAG_END downto ITAG_START);
    wb_rtag_overlay := r.req_addr_overlay(ITAG_END downto ITAG_START);
    waysel(WAY_EVEN).hit := MISS;
    waysel(WAY_EVEN).rdata := (others => '0');
    waysel(WAY_EVEN).valid := '0';
    waysel(WAY_EVEN).load_fault := '0';
    waysel(WAY_ODD).hit := MISS;
    waysel(WAY_ODD).rdata := (others => '0');
    waysel(WAY_ODD).valid := '0';
    waysel(WAY_ODD).load_fault := '0';
    if r.use_overlay = '0' then
        wb_rtag_even := wb_rtag;
        wb_rtag_odd := wb_rtag;
    elsif w_raddr5_r = '0' then
        wb_rtag_even := wb_rtag;
        wb_rtag_odd := wb_rtag_overlay;
    else
        wb_rtag_even := wb_rtag_overlay;
        wb_rtag_odd := wb_rtag;
    end if;
    for n in 0 to CFG_ICACHE_WAYS-1 loop
        if waysel(WAY_EVEN).hit = MISS and memeven(n).rtag = wb_rtag_even then
            waysel(WAY_EVEN).hit := n;
            waysel(WAY_EVEN).rdata := memeven(n).rdata;
            waysel(WAY_EVEN).valid := memeven(n).valid;
            waysel(WAY_EVEN).load_fault := memeven(n).load_fault;
        end if;

        if waysel(WAY_ODD).hit = MISS and memodd(n).rtag = wb_rtag_odd then
            waysel(WAY_ODD).hit := n;
            waysel(WAY_ODD).rdata := memodd(n).rdata;
            waysel(WAY_ODD).valid := memodd(n).valid;
            waysel(WAY_ODD).load_fault := memodd(n).load_fault;
        end if;
    end loop;

    -- swap back rdata
    w_o_resp_load_fault := '0';
    if w_raddr5_r = '0' then
        if r.use_overlay = '0' then
            wb_hit0 := waysel(WAY_EVEN).hit;
            wb_hit1 := waysel(WAY_EVEN).hit;
            w_hit0_valid := waysel(WAY_EVEN).valid;
            w_hit1_valid := waysel(WAY_EVEN).valid;
            wb_o_resp_data := waysel(WAY_EVEN).rdata;
            w_o_resp_load_fault := waysel(WAY_EVEN).load_fault;
        else
            wb_hit0 := waysel(WAY_EVEN).hit;
            wb_hit1 := waysel(WAY_ODD).hit;
            w_hit0_valid := waysel(WAY_EVEN).valid;
            w_hit1_valid := waysel(WAY_ODD).valid;
            wb_o_resp_data(15 downto 0) := waysel(WAY_EVEN).rdata(15 downto 0);
            wb_o_resp_data(31 downto 16) := waysel(WAY_ODD).rdata(15 downto 0);
            w_o_resp_load_fault :=
                waysel(WAY_EVEN).load_fault or waysel(WAY_ODD).load_fault;
        end if;
    else
        if r.use_overlay = '0' then
            wb_hit0 := waysel(WAY_ODD).hit;
            wb_hit1 := waysel(WAY_ODD).hit;
            w_hit0_valid := waysel(WAY_ODD).valid;
            w_hit1_valid := waysel(WAY_ODD).valid;
            wb_o_resp_data := waysel(WAY_ODD).rdata;
            w_o_resp_load_fault := waysel(WAY_ODD).load_fault;
        else
            wb_hit0 := waysel(WAY_ODD).hit;
            wb_hit1 := waysel(WAY_EVEN).hit;
            w_hit0_valid := waysel(WAY_ODD).valid;
            w_hit1_valid := waysel(WAY_EVEN).valid;
            wb_o_resp_data(15 downto 0) := waysel(WAY_ODD).rdata(15 downto 0);
            wb_o_resp_data(31 downto 16) := waysel(WAY_EVEN).rdata(15 downto 0);
            w_o_resp_load_fault :=
                waysel(WAY_ODD).load_fault or waysel(WAY_EVEN).load_fault;
        end if;
    end if;

    v_lrui(WAY_EVEN).init := '0';
    v_lrui(WAY_EVEN).radr := (others => '0');
    v_lrui(WAY_EVEN).wadr := (others => '0');
    v_lrui(WAY_EVEN).we := '0';
    v_lrui(WAY_EVEN).lru := (others => '0');
    v_lrui(WAY_ODD).init := '0';
    v_lrui(WAY_ODD).radr := (others => '0');
    v_lrui(WAY_ODD).wadr := (others => '0');
    v_lrui(WAY_ODD).we := '0';
    v_lrui(WAY_ODD).lru := (others => '0');
    w_o_resp_valid := '0';
    if r.state = State_Flush then
        v_lrui(WAY_EVEN).init := not r.mem_addr(CFG_IOFFSET_WIDTH);
        v_lrui(WAY_EVEN).wadr := r.mem_addr(IINDEX_END downto IINDEX_START);
        v_lrui(WAY_ODD).init := r.mem_addr(CFG_IOFFSET_WIDTH);
        v_lrui(WAY_ODD).wadr := r.mem_addr(IINDEX_END downto IINDEX_START);
    elsif CFG_SINGLEPORT_CACHE and (r.state = State_WaitGrant
            or r.state = State_WaitResp or r.state = State_CheckResp
            or r.state = State_SetupReadAdr) then
            -- Do nothing while memory writing
    elsif w_hit0_valid = '1' and w_hit1_valid = '1'
        and wb_hit0 /= MISS and wb_hit1 /= MISS and r.requested = '1' then
        w_o_resp_valid := '1';

        -- Update LRU table
        if w_raddr5_r = '0' then
            v_lrui(WAY_EVEN).we := '1';
            v_lrui(WAY_EVEN).lru := conv_std_logic_vector(wb_hit0, 2)(1 downto 0);
            v_lrui(WAY_EVEN).wadr :=
                r.req_addr(IINDEX_END downto IINDEX_START);
            if r.use_overlay = '1' then
                v_lrui(WAY_ODD).we := '1';
                v_lrui(WAY_ODD).lru := conv_std_logic_vector(wb_hit1, 2)(1 downto 0);
                v_lrui(WAY_ODD).wadr :=
                    r.req_addr_overlay(IINDEX_END downto IINDEX_START);
            end if;
        else
            v_lrui(WAY_ODD).we := '1';
            v_lrui(WAY_ODD).lru := conv_std_logic_vector(wb_hit0, 2)(1 downto 0);
            v_lrui(WAY_ODD).wadr :=
                r.req_addr(IINDEX_END downto IINDEX_START);
            if r.use_overlay = '1' then
                v_lrui(WAY_EVEN).we := '1';
                v_lrui(WAY_EVEN).lru := conv_std_logic_vector(wb_hit1, 2)(1 downto 0);
                v_lrui(WAY_EVEN).wadr :=
                    r.req_addr_overlay(IINDEX_END downto IINDEX_START);
            end if;
        end if;
    end if;

    w_o_req_ctrl_ready := not r.req_flush
                       and (not r.requested or w_o_resp_valid);
    if i_req_ctrl_valid = '1' and w_o_req_ctrl_ready = '1' then
        v.req_addr := i_req_ctrl_addr;
        v.req_addr_overlay := wb_radr_overlay;
        v.use_overlay := w_use_overlay;
        v.requested := '1';
    elsif w_o_resp_valid = '1' and i_resp_ctrl_ready = '1' then
        v.requested := '0';
    end if;

    -- System Bus access state machine
    w_last := '0';
    w_ena := '0';
    v_init := '0';
    wb_wstrb_next := r.burst_wstrb(2 downto 0) & r.burst_wstrb(3);
    case r.state is
    when State_Idle =>
        if r.req_flush = '1' then
            v.state := State_Flush;
            if r.req_flush_addr(0) = '1' then
                v.mem_addr := FLUSH_ALL_ADDR;
                v.flush_cnt := (others => '1');
            else
                v.mem_addr := r.req_flush_addr;
                v.flush_cnt := r.req_flush_cnt;
            end if;
            v.burst_wstrb := (others => '1');    -- All qwords in line
            v.burst_valid := (others => '0');    -- All qwords in line
        elsif (i_req_ctrl_valid = '1' and w_o_req_ctrl_ready = '1')
              or r.requested = '1' then
            --! Check hit even there's no new request only the previous one.
            --! This must be done in a case of CPU is halted and cache was flushed
            --!
            v.state := State_CheckHit;
        end if;
    when State_CheckHit =>
        if w_o_resp_valid = '1' then
            -- Hit
            if i_req_ctrl_valid = '1' and w_o_req_ctrl_ready = '1' then
                v.state := State_CheckHit;
            else
                v.state := State_Idle;
            end if;

        else
            -- Miss
            v.req_mem_valid := '1';
            if w_hit0_valid = '0' or wb_hit0 = MISS then
                wb_mem_addr := r.req_addr;
            else
                wb_mem_addr := r.req_addr_overlay;
            end if;
            if i_req_mem_ready = '1' then
                v.state := State_WaitResp;
            else
                v.state := State_WaitGrant;
            end if;

            v.mem_addr := wb_mem_addr(BUS_ADDR_WIDTH-1 downto 3) & "000";
            v.burst_cnt := 3;
            case wb_mem_addr(CFG_IOFFSET_WIDTH-1 downto 3) is
            when "00" =>
                wb_wstrb_next := X"1";
            when "01" =>
                wb_wstrb_next := X"2";
            when "10" =>
                wb_wstrb_next := X"4";
            when "11" =>
                wb_wstrb_next := X"8";
            when others =>
            end case;
            v.burst_wstrb := wb_wstrb_next;
            v.burst_valid := wb_wstrb_next;
            v.lru_even_wr := wb_lru_even;
            v.lru_odd_wr := wb_lru_odd;
        end if;
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            v.req_mem_valid := '0';
            v.state := State_WaitResp;
        end if;
    when State_WaitResp =>
        if r.burst_cnt = 0 then
            w_last := '1';
        end if;
        if i_resp_mem_data_valid = '1' then
            w_ena := '1';
            if r.burst_cnt = 0 then
                v.state := State_CheckResp;
            else
                v.burst_cnt := r.burst_cnt - 1;
            end if;
            -- Suppose using WRAP burst transaction
            v.burst_wstrb := wb_wstrb_next;
            v.burst_valid := r.burst_valid or wb_wstrb_next;
        end if;
    when State_CheckResp =>
        if CFG_SINGLEPORT_CACHE then
            v.state := State_SetupReadAdr;
        elsif (w_o_req_ctrl_ready = '1' and i_req_ctrl_valid = '1')
            or (r.requested = '1' and w_o_resp_valid = '0') then
            v.state := State_CheckHit;
        else
            v.state := State_Idle;
        end if;
    when State_SetupReadAdr =>
        v.state := State_CheckHit;
    when State_Flush =>
        v_init := '1';
        if r.flush_cnt = flush_cnt_zero then
            v.req_flush := '0';
            v.state := State_Idle;
        else
            v.flush_cnt := r.flush_cnt - 1;
            v.mem_addr(BUS_ADDR_WIDTH-1 downto CFG_IOFFSET_WIDTH) := 
                   r.mem_addr(BUS_ADDR_WIDTH-1 downto CFG_IOFFSET_WIDTH) + 1;
        end if;
    when others =>
    end case;

    -- Write signals:
    vb_ena_even := (others => '0');
    vb_ena_odd := (others => '0');
    if r.mem_addr(CFG_IOFFSET_WIDTH) = '0' then
        vb_ena_even := (others => v_init);
        vb_ena_even(conv_integer(r.lru_even_wr)) := w_ena or v_init;
    else
        vb_ena_odd := (others => v_init);
        vb_ena_odd(conv_integer(r.lru_odd_wr)) := w_ena or v_init;
    end if;

    v_swapin(WAY_EVEN).wadr := r.mem_addr;
    v_swapin(WAY_EVEN).wstrb := r.burst_wstrb;
    v_swapin(WAY_EVEN).wvalid := r.burst_valid;
    v_swapin(WAY_EVEN).wdata := i_resp_mem_data;
    v_swapin(WAY_EVEN).load_fault := i_resp_mem_load_fault;
    v_swapin(WAY_ODD).wadr := r.mem_addr;
    v_swapin(WAY_ODD).wstrb := r.burst_wstrb;
    v_swapin(WAY_ODD).wvalid := r.burst_valid;
    v_swapin(WAY_ODD).wdata := i_resp_mem_data;
    v_swapin(WAY_ODD).load_fault := i_resp_mem_load_fault;

    if CFG_SINGLEPORT_CACHE and (r.state = State_WaitResp
        or r.state = State_CheckResp
        or r.state = State_Flush) then
        v_swapin(WAY_EVEN).radr := r.mem_addr;
        v_swapin(WAY_ODD).radr := r.mem_addr;
    elsif r.state = State_Idle or w_o_resp_valid = '1' then
        if w_raddr5 = '0' then
            v_swapin(WAY_EVEN).radr := wb_req_adr;
            v_swapin(WAY_ODD).radr := wb_radr_overlay;
            v_lrui(WAY_EVEN).radr := wb_req_adr(IINDEX_END downto IINDEX_START);
            v_lrui(WAY_ODD).radr := wb_radr_overlay(IINDEX_END downto IINDEX_START);
        else
            v_swapin(WAY_EVEN).radr := wb_radr_overlay;
            v_swapin(WAY_ODD).radr := wb_req_adr;
            v_lrui(WAY_EVEN).radr := wb_radr_overlay(IINDEX_END downto IINDEX_START);
            v_lrui(WAY_ODD).radr := wb_req_adr(IINDEX_END downto IINDEX_START);
        end if;
    else
        if w_raddr5_r = '0' then
            v_swapin(WAY_EVEN).radr := r.req_addr;
            v_swapin(WAY_ODD).radr := r.req_addr_overlay;
            v_lrui(WAY_EVEN).radr :=
                r.req_addr(IINDEX_END downto IINDEX_START);
            v_lrui(WAY_ODD).radr :=
                r.req_addr_overlay(IINDEX_END downto IINDEX_START);
        else
            v_swapin(WAY_EVEN).radr := r.req_addr_overlay;
            v_swapin(WAY_ODD).radr := r.req_addr;
            v_lrui(WAY_EVEN).radr :=
                r.req_addr_overlay(IINDEX_END downto IINDEX_START);
            v_lrui(WAY_ODD).radr :=
                r.req_addr(IINDEX_END downto IINDEX_START);
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    lrui(WAY_EVEN) <= v_lrui(WAY_EVEN);
    lrui(WAY_ODD) <= v_lrui(WAY_ODD);

    swapin(WAY_EVEN) <= v_swapin(WAY_EVEN);
    swapin(WAY_ODD) <= v_swapin(WAY_ODD);

    wb_ena_even <= vb_ena_even;
    wb_ena_odd <= vb_ena_odd;

    o_req_ctrl_ready <= w_o_req_ctrl_ready;

    o_req_mem_valid <= r.req_mem_valid;
    o_req_mem_addr <= r.mem_addr;
    o_req_mem_write <= '0';
    o_req_mem_strob <= (others => '0');
    o_req_mem_data <= (others => '0');
    o_req_mem_len <= conv_std_logic_vector(3, 8);
    o_req_mem_burst <= "10";  -- WRAP burst transaction
    o_req_mem_last <= w_last;

    o_resp_ctrl_valid <= w_o_resp_valid;
    o_resp_ctrl_data <= wb_o_resp_data;
    o_resp_ctrl_addr <= r.req_addr;
    o_resp_ctrl_load_fault <= w_o_resp_load_fault;
    o_istate <= r.state(1 downto 0);
    
    rin <= v;
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
