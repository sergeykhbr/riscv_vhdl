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
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity icache_lru is generic (
    memtech : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- Control path:
    i_req_valid : in std_logic;
    i_req_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ready : out std_logic;
    o_resp_valid : out std_logic;
    o_resp_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data : out std_logic_vector(31 downto 0);
    o_resp_load_fault : out std_logic;
    o_resp_executable : out std_logic;
    o_resp_writable : out std_logic;
    o_resp_readable : out std_logic;
    i_resp_ready : in std_logic;
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
    i_mem_data_valid : in std_logic;
    i_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_mem_load_fault : in std_logic;
    -- MPU interface:
    o_mpu_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mpu_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- Debug Signals:
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic;                                      -- address to clear icache is valid
    o_state : out std_logic_vector(3 downto 0)
  );
end; 
 
architecture arch_icache_lru of icache_lru is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  constant State_Idle : std_logic_vector(3 downto 0) := "0000";
  constant State_CheckHit : std_logic_vector(3 downto 0) := "0001";
  constant State_CheckMPU : std_logic_vector(3 downto 0) := "0010";
  constant State_WaitGrant : std_logic_vector(3 downto 0) := "0011";
  constant State_WaitResp : std_logic_vector(3 downto 0) := "0100";
  constant State_CheckResp : std_logic_vector(3 downto 0) := "0101";
  constant State_SetupReadAdr : std_logic_vector(3 downto 0) := "0110";
  constant State_Flush : std_logic_vector(3 downto 0) := "0111";

  type RegistersType is record
      req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_addr_next : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      write_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      state : std_logic_vector(3 downto 0);
      req_mem_valid : std_logic;
      mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      burst_cnt : std_logic_vector(7 downto 0);
      burst_rstrb : std_logic_vector(ICACHE_BURST_LEN-1 downto 0);
      cached : std_logic;
      executable : std_logic;
      load_fault : std_logic;
      req_flush : std_logic;
      req_flush_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_flush_cnt : std_logic_vector(CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS-1 downto 0);
      flush_cnt : std_logic_vector(CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS-1 downto 0);
      cache_line_i : std_logic_vector(ICACHE_LINE_BITS-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
    (others => '0'), (others => '0'),       -- req_addr, req_addr_next
    (others => '0'),                        -- write_addr
    State_Flush,                            -- state
    '0', (others => '0'),                   -- req_mem_valid, mem_addr,
    (others => '0'),                        -- burst_cnt
    (others => '0'),                        -- burst_rstrb
    '0',                                    -- cached
    '0',                                    -- executable
    '0',                                    -- load_fault
    '0',                                    -- req_flush
    (others => '0'), (others => '0'),       -- req_flush_addr, req_flush_cnt
    (others => '1'),                        -- flush_cnt
    (others => '0')                         -- cache_line_i
  );

  signal r, rin : RegistersType;
  signal line_cs_i : std_logic;
  signal line_addr_i : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal line_wdata_i : std_logic_vector(ICACHE_LINE_BITS-1 downto 0);
  signal line_wstrb_i : std_logic_vector(2**CFG_ILOG2_BYTES_PER_LINE-1 downto 0);
  signal line_wflags_i : std_logic_vector(ITAG_FL_TOTAL-1 downto 0);
  signal line_flush_i : std_logic;
  signal line_raddr_o : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal line_rdata_o : std_logic_vector(ICACHE_LINE_BITS+15 downto 0);
  signal line_rflags_o : std_logic_vector(ITAG_FL_TOTAL-1 downto 0);
  signal line_hit_o : std_logic;
  signal line_hit_next_o : std_logic;

begin

  memcouple : tagmemcoupled generic map (
      memtech => memtech,
      async_reset => async_reset,
      abus => BUS_ADDR_WIDTH,
      waybits => CFG_ILOG2_NWAYS,
      ibits => CFG_ILOG2_LINES_PER_WAY,
      lnbits => CFG_ILOG2_BYTES_PER_LINE,
      flbits => ITAG_FL_TOTAL
  ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_cs => line_cs_i,
      i_flush => line_flush_i,
      i_addr => line_addr_i,
      i_wdata => line_wdata_i,
      i_wstrb => line_wstrb_i,
      i_wflags => line_wflags_i,
      o_raddr => line_raddr_o,
      o_rdata => line_rdata_o,
      o_rflags => line_rflags_o,
      o_hit => line_hit_o,
      o_hit_next => line_hit_next_o
  );


  comb : process(i_nrst, i_req_valid, i_req_addr,
                i_resp_ready, i_req_mem_ready, 
                i_mem_data_valid, i_mem_data, i_mem_load_fault, 
                i_mpu_flags, i_flush_address, i_flush_valid,
                line_raddr_o, line_rdata_o, line_rflags_o, line_hit_o, line_hit_next_o, r)
    variable v : RegistersType;
    variable v_last : std_logic;
    variable v_req_ready : std_logic;
    variable v_resp_valid : std_logic;
    variable v_req_mem_len : std_logic_vector(7 downto 0);
    variable vb_cached_data : std_logic_vector(31 downto 0);
    variable vb_uncached_data : std_logic_vector(31 downto 0);
    variable vb_resp_data : std_logic_vector(31 downto 0);
    variable v_resp_er_load_fault : std_logic;
    variable v_flush : std_logic;
    variable v_line_cs : std_logic;
    variable vb_line_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_line_wdata : std_logic_vector(ICACHE_LINE_BITS-1 downto 0);
    variable vb_line_wstrb : std_logic_vector(ICACHE_BYTES_PER_LINE-1 downto 0);
    variable v_line_wflags : std_logic_vector(ITAG_FL_TOTAL-1 downto 0);
    variable sel_cached : integer;
    variable sel_uncached : integer;
  begin

    v := r;

    v_req_ready := '0';
    v_resp_valid := '0';
    vb_resp_data := (others => '0');
    v_resp_er_load_fault := '0';
    v_flush := '0';
    v_last := '0';
    v_req_mem_len := conv_std_logic_vector(ICACHE_BURST_LEN-1, 8);
    sel_cached := conv_integer(r.req_addr(CFG_ILOG2_BYTES_PER_LINE-1 downto 1));
    sel_uncached := conv_integer(r.req_addr(2 downto 1));

    vb_cached_data := line_rdata_o(16*sel_cached + 31 downto 16*sel_cached);
    vb_uncached_data := r.cache_line_i(16*sel_uncached + 31 downto 16*sel_uncached);

    -- flush request via debug interface
    if i_flush_valid = '1' then
        v.req_flush := '1';
        if i_flush_address(0) = '1' then
            v.req_flush_cnt := (others => '1');
            v.req_flush_addr := (others => '0');
        elsif and_reduce(i_flush_address(CFG_ILOG2_BYTES_PER_LINE-1 downto 1)) = '1' then
            v.req_flush_cnt := conv_std_logic_vector(2*ICACHE_WAYS-1,
                               CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS);
            v.req_flush_addr := i_flush_address;
        else
            v.req_flush_cnt := conv_std_logic_vector(ICACHE_WAYS-1,
                               CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS);
            v.req_flush_addr := i_flush_address;
        end if;
    end if;

    v_line_cs := '0';
    vb_line_addr := r.req_addr;
    vb_line_wdata := r.cache_line_i;
    vb_line_wstrb := (others => '0');
    v_line_wflags := (others => '0');

    case r.state is
    when State_Idle =>
        v.executable := '1';
        if r.req_flush = '1' then
            v.state := State_Flush;
            v.req_flush := '0';
            v.cache_line_i := (others => '0');
            if r.req_flush_addr(0) = '1' then
                v.req_addr := (others => '0');
                v.flush_cnt := (others => '1');
            else
                v.req_addr := r.req_flush_addr(BUS_ADDR_WIDTH-1 downto CFG_ILOG2_BYTES_PER_LINE)
                              & zero64(CFG_ILOG2_BYTES_PER_LINE-1 downto 0);
                v.flush_cnt := r.req_flush_cnt;
            end if;
        else
            v_req_ready := '1';
            v_line_cs := i_req_valid;
            vb_line_addr := i_req_addr;
            if i_req_valid = '1' then
                v.req_addr := i_req_addr;
                v.req_addr_next := i_req_addr + ICACHE_BYTES_PER_LINE;
                v.state := State_CheckHit;
            end if;
        end if;
    when State_CheckHit =>
        vb_resp_data := vb_cached_data;
        if line_hit_o = '1' and line_hit_next_o = '1' then
            -- Hit
            v_req_ready := not r.req_flush;
            v_resp_valid := '1';
            if i_resp_ready = '0' then
                -- Do nothing: wait accept
            elsif i_req_valid = '0' or r.req_flush = '1' then
                v.state := State_Idle;
            else
                v.state := State_CheckHit;
                v_line_cs := i_req_valid;
                v.req_addr := i_req_addr;
                v.req_addr_next := i_req_addr + ICACHE_BYTES_PER_LINE;
                vb_line_addr := i_req_addr;
            end if;
        else
            -- Miss
            v.state := State_CheckMPU;
        end if;
    when State_CheckMPU =>
        if i_mpu_flags(CFG_MPU_FL_EXEC) = '0' then
            v.cache_line_i := (others => '1');
            v.state := State_CheckResp;
            v.cached := '0';
        else
            v.req_mem_valid := '1';
            v.state := State_WaitGrant;
            v.write_addr := r.req_addr;

            if i_mpu_flags(CFG_MPU_FL_CACHABLE) = '1' then
                if line_hit_o = '0' then
                    v.mem_addr := r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_ILOG2_BYTES_PER_LINE)
                                & zero64(CFG_ILOG2_BYTES_PER_LINE-1 downto 0);
                else
                    v.write_addr := r.req_addr_next;
                    v.mem_addr := r.req_addr_next(BUS_ADDR_WIDTH-1 downto CFG_ILOG2_BYTES_PER_LINE)
                                & zero64(CFG_ILOG2_BYTES_PER_LINE-1 downto 0);
                end if;
                v.burst_cnt := conv_std_logic_vector(ICACHE_BURST_LEN-1, 8);
                v.cached := '1';
            else
                v.mem_addr := r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_LOG2_DATA_BYTES)
                             & zero64(CFG_LOG2_DATA_BYTES-1 downto 0);
                v.cached := '0';
                v_req_mem_len := X"01";  -- burst = 2
                v.burst_cnt := X"01";
            end if;
        end if;

        v.burst_rstrb := conv_std_logic_vector(1, ICACHE_BURST_LEN);
        v.load_fault := '0';
        v.executable := i_mpu_flags(CFG_MPU_FL_EXEC);
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            v.state := State_WaitResp;
            v.req_mem_valid := '0';
        end if;
        v_req_mem_len := r.burst_cnt;
    when State_WaitResp =>
        if or_reduce(r.burst_cnt) = '0' then
            v_last := '1';
        end if;
        if i_mem_data_valid = '1' then
            for k in 0 to ICACHE_BURST_LEN-1 loop
                if r.burst_rstrb(k) = '1'then
                    v.cache_line_i((k+1)*BUS_DATA_WIDTH-1 downto
                                    k*BUS_DATA_WIDTH) := i_mem_data;
                end if;
            end loop;
            if or_reduce(r.burst_cnt) = '0' then
                v.state := State_CheckResp;
                v.write_addr := r.req_addr;      -- Swap addres for 1 clock to write line
                v.req_addr := r.write_addr;
            else
                v.burst_cnt := r.burst_cnt - 1;
            end if;
            v.burst_rstrb := r.burst_rstrb(ICACHE_BURST_LEN-2 downto 0) & '0';
            if i_mem_load_fault = '1' then
                v.load_fault := '1';
            end if;
        end if;
    when State_CheckResp =>
        v.req_addr := r.write_addr;              -- Restore req_addr after line write
        if r.cached = '0' or r.load_fault = '1' then
            v_resp_valid := '1';
            vb_resp_data := vb_uncached_data;
            v_resp_er_load_fault := r.load_fault;
            if i_resp_ready = '1' then
                v.state := State_Idle;
            end if;
        else
            v.state := State_SetupReadAdr;
            v_line_cs := '1';
            v_line_wflags(TAG_FL_VALID) := '1';
            vb_line_wstrb := (others => '1');  -- write full line
        end if;
    when State_SetupReadAdr =>
        v.state := State_CheckHit;
    when State_Flush =>
        v_line_wflags := (others => '0');      -- flag valid = 0
        vb_line_wstrb := (others => '1');   -- write full line
        v_flush := '1';
        if or_reduce(r.flush_cnt) = '0' then
            v.state := State_Idle;
        else 
            v.flush_cnt := r.flush_cnt - 1;
            if r.req_addr(CFG_ILOG2_NWAYS-1 downto 0) =
               conv_std_logic_vector(ICACHE_WAYS-1, CFG_ILOG2_NWAYS) then
                v.req_addr(BUS_ADDR_WIDTH-1 downto CFG_ILOG2_BYTES_PER_LINE) := 
                    r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_ILOG2_BYTES_PER_LINE) + 1;
                v.req_addr(CFG_ILOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
            else
                v.req_addr := r.req_addr + 1;
            end if;
        end if;
    when others =>
    end case;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    line_cs_i <= v_line_cs;
    line_addr_i <= vb_line_addr;
    line_wdata_i <= vb_line_wdata;
    line_wstrb_i <= vb_line_wstrb;
    line_wflags_i <= v_line_wflags;
    line_flush_i <= v_flush;

    o_req_ready <= v_req_ready;

    o_req_mem_valid <= r.req_mem_valid;
    o_req_mem_addr <= r.mem_addr;
    o_req_mem_write <= '0';
    o_req_mem_strob <= (others => '0');
    o_req_mem_data <= (others => '0');
    o_req_mem_len <= v_req_mem_len;
    o_req_mem_burst <= "01";  -- 00=FIX; 01=INCR; 10=WRAP
    o_req_mem_last <= v_last;

    o_resp_valid <= v_resp_valid;
    o_resp_data <= vb_resp_data;
    o_resp_addr <= r.req_addr;
    o_resp_load_fault <= v_resp_er_load_fault;
    o_resp_executable <= r.executable;
    o_resp_writable <= '0';
    o_resp_readable <= '0';
    o_mpu_addr <= r.req_addr;
    o_state <= r.state;
    
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
