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

entity dcache_lru is generic (
    memtech : integer;
    async_reset : boolean;
    coherence_ena : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- Control path:
    i_req_valid : in std_logic;
    i_req_write : in std_logic;
    i_req_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_req_wdata : in std_logic_vector(63 downto 0);
    i_req_wstrb : in std_logic_vector(7 downto 0);
    o_req_ready : out std_logic;
    o_resp_valid : out std_logic;
    o_resp_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_data : out std_logic_vector(63 downto 0);
    o_resp_er_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_er_load_fault : out std_logic;
    o_resp_er_store_fault : out std_logic;
    o_resp_er_mpu_load : out std_logic;
    o_resp_er_mpu_store : out std_logic;
    i_resp_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_type : out std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
    o_req_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_req_mem_strob : out std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
    o_req_mem_data : out std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data : in std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    i_mem_load_fault : in std_logic;
    i_mem_store_fault : in std_logic;
    -- MPU interface
    o_mpu_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mpu_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- D$ Snoop interface
    i_req_snoop_valid : in std_logic;
    i_req_snoop_type : in std_logic_vector(SNOOP_REQ_TYPE_BITS-1 downto 0);
    o_req_snoop_ready : out std_logic;
    i_req_snoop_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_resp_snoop_ready : in std_logic;
    o_resp_snoop_valid : out std_logic;
    o_resp_snoop_data : out std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
    o_resp_snoop_flags : out std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
    -- Debug Signals:
    i_flush_address : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_flush_valid : in std_logic;
    o_flush_end : out std_logic
  );
end; 
 
architecture arch_dcache_lru of dcache_lru is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  constant State_Idle : std_logic_vector(3 downto 0) := "0000";
  constant State_CheckHit : std_logic_vector(3 downto 0) := "0001";
  constant State_TranslateAddress : std_logic_vector(3 downto 0) := "0010";
  constant State_WaitGrant : std_logic_vector(3 downto 0) := "0011";
  constant State_WaitResp : std_logic_vector(3 downto 0) := "0100";
  constant State_CheckResp : std_logic_vector(3 downto 0) := "0101";
  constant State_SetupReadAdr : std_logic_vector(3 downto 0) := "0110";
  constant State_WriteBus : std_logic_vector(3 downto 0) := "0111";
  constant State_FlushAddr : std_logic_vector(3 downto 0) := "1000";
  constant State_FlushCheck : std_logic_vector(3 downto 0) := "1001";
  constant State_ResetAddr : std_logic_vector(3 downto 0) := "1010";
  constant State_ResetWrite : std_logic_vector(3 downto 0) := "1011";
  constant State_SnoopSetupAddr : std_logic_vector(3 downto 0) := "1100";
  constant State_SnoopReadData : std_logic_vector(3 downto 0) := "1101";

  signal line_direct_access_i : std_logic;
  signal line_invalidate_i : std_logic;
  signal line_re_i : std_logic;
  signal line_we_i : std_logic;
  signal line_addr_i : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal line_wdata_i : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
  signal line_wstrb_i : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
  signal line_wflags_i : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
  signal line_raddr_o : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal line_rdata_o : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
  signal line_rflags_o : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
  signal line_hit_o : std_logic;
  -- Snoop signals:
  signal line_snoop_addr_i : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal line_snoop_ready_o : std_logic;
  signal line_snoop_flags_o : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);

  type RegistersType is record
      req_write : std_logic;
      req_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      req_wdata : std_logic_vector(63 downto 0);
      req_wstrb : std_logic_vector(7 downto 0);
      state : std_logic_vector(3 downto 0);
      req_mem_valid : std_logic;
      req_mem_type : std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
      mem_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mpu_er_store : std_logic;
      mpu_er_load : std_logic;
      load_fault : std_logic;
      write_first : std_logic;
      write_flush : std_logic;
      write_share : std_logic;
      mem_wstrb : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
      req_flush : std_logic;
      req_flush_all : std_logic;
      req_flush_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      req_flush_cnt : std_logic_vector(CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS-1 downto 0);
      flush_cnt : std_logic_vector(CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS-1 downto 0);
      cache_line_i : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
      cache_line_o : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
      req_snoop_type : std_logic_vector(SNOOP_REQ_TYPE_BITS-1 downto 0);
      snoop_flags_valid : std_logic;
      snoop_restore_wait_resp : std_logic;
      snoop_restore_write_bus : std_logic;
      req_addr_restore : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
    '0',                                    -- req_write
    (others => '0'),                        -- req_addr
    (others => '0'), (others => '0'),       -- req_wdata, req_wstrb
    State_ResetAddr,                        -- state
    '0',                                    -- req_mem_valid
    (others => '0'),                        -- req_mem_type
    (others => '0'),                        -- mem_addr,
    '0',                                    -- mpu_er_store
    '0',                                    -- mpu_er_load
    '0',                                    -- load_fault
    '0',                                    -- write_first
    '0',                                    -- write_flush
    '0',                                    -- write_share
    (others => '0'),                        -- mem_wstrb
    '0',                                    -- req_flush
    '0',                                    -- req_flush_all
    (others => '0'),                        -- req_flush_addr [0]=1 flush all
    (others => '0'),                        -- req_flush_cnt
    (others => '1'),                        -- flush_cnt
    (others => '0'),                        -- cache_line_i
    (others => '0'),                        -- cache_line_o
    (others => '0'),                        -- req_snoop_type
    '0',                                    -- snoop_flags_valid
    '0',                                    -- snoop_restore_wait_resp
    '0',                                    -- snoop_restore_write_bus
    (others => '0')                         -- req_addr_restore
  );

  signal r, rin : RegistersType;

begin

  tagmem0 : tagmemnway generic map (
      memtech => memtech,
      async_reset => async_reset,
      abus => CFG_CPU_ADDR_BITS,
      waybits => CFG_DLOG2_NWAYS,
      ibits => CFG_DLOG2_LINES_PER_WAY,
      lnbits => CFG_DLOG2_BYTES_PER_LINE,
      flbits => DTAG_FL_TOTAL,
      snoop => coherence_ena
  ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_direct_access => line_direct_access_i,
      i_invalidate => line_invalidate_i,
      i_re => line_re_i,
      i_we => line_we_i,
      i_addr => line_addr_i,
      i_wdata => line_wdata_i,
      i_wstrb => line_wstrb_i,
      i_wflags => line_wflags_i,
      o_raddr => line_raddr_o,
      o_rdata => line_rdata_o,
      o_rflags => line_rflags_o,
      o_hit => line_hit_o,
      i_snoop_addr => line_snoop_addr_i,
      o_snoop_ready => line_snoop_ready_o,
      o_snoop_flags => line_snoop_flags_o
  );


  comb : process(i_nrst, i_req_valid, i_req_write, i_req_addr, i_req_wdata, i_req_wstrb,
                i_resp_ready, i_req_mem_ready,
                i_mem_data_valid, i_mem_data, i_mem_load_fault, i_mem_store_fault,
                i_mpu_flags, i_flush_address, i_flush_valid,
                i_req_snoop_type, i_req_snoop_valid, i_req_snoop_addr,
                line_raddr_o, line_rdata_o, line_hit_o, line_rflags_o,
                line_snoop_ready_o, line_snoop_flags_o, r)
    variable v : RegistersType;
    variable vb_cache_line_i_modified : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_rdata_o_modified : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_rdata_o_wstrb : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
    
    variable v_req_ready : std_logic;
    variable vb_cached_data : std_logic_vector(63 downto 0);
    variable vb_uncached_data : std_logic_vector(63 downto 0);
    variable v_resp_valid : std_logic;
    variable vb_resp_data : std_logic_vector(63 downto 0);
    variable v_resp_er_load_fault : std_logic;
    variable v_resp_er_store_fault : std_logic;
    variable v_direct_access : std_logic;
    variable v_invalidate : std_logic;
    variable v_flush_end : std_logic;
    variable v_line_cs_read : std_logic;
    variable v_line_cs_write : std_logic;
    variable vb_line_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_line_wdata : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_wstrb : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
    variable vb_req_mask : std_logic_vector(63 downto 0);
    variable v_line_wflags : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
    variable ridx : integer range 0 to (DCACHE_BYTES_PER_LINE/8)-1;
    variable v_req_same_line : std_logic;
    variable v_ready_next : std_logic;
    variable v_req_snoop_ready : std_logic;
    variable v_req_snoop_ready_on_wait : std_logic;
    variable v_resp_snoop_valid : std_logic;
    variable vb_addr_direct_next : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_req_snoop_cohena : std_logic;
  begin

    v := r;

    v_ready_next := '0';
    v_req_ready := '0';
    v_resp_valid := '0';
    vb_resp_data := (others => '0');
    v_resp_er_load_fault := '0';
    v_resp_er_store_fault := '0';
    v_direct_access := '0';
    v_invalidate := '0';
    v_flush_end := '0';
    v_req_snoop_ready := '0';
    v_req_snoop_ready_on_wait := '0';
    v_resp_snoop_valid := r.snoop_flags_valid;
    ridx := conv_integer(r.req_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 3));

    vb_cached_data := line_rdata_o((ridx+1)*64 - 1 downto
                                         ridx*64);
    vb_uncached_data := r.cache_line_i(63 downto 0);

    v_req_same_line := '0';
    if r.req_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE)
        = i_req_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) then
        v_req_same_line := '1';
    end if;


    if i_flush_valid = '1' then
        v.req_flush := '1';
        v.req_flush_all := i_flush_address(0);
        if i_flush_address(0) = '1' then
            v.req_flush_cnt := (others => '1');
            v.req_flush_addr := (others => '0');
        else
            v.req_flush_cnt := (others => '0');
            v.req_flush_addr := i_flush_address;
        end if;
    end if;

    for i in 0 to 7 loop
        vb_req_mask(8*i+7 downto 8*i) := (others => r.req_wstrb(i));
    end loop;

    vb_line_rdata_o_modified := line_rdata_o;
    vb_cache_line_i_modified := r.cache_line_i;
    vb_line_rdata_o_wstrb := (others => '0');
    for i in 0 to (DCACHE_BYTES_PER_LINE/8)-1 loop
        if i = ridx then
            vb_line_rdata_o_modified(64*(i+1)-1 downto 64*i) :=
                (vb_line_rdata_o_modified(64*(i+1)-1 downto 64*i)
                 and not vb_req_mask) or (r.req_wdata and vb_req_mask);

            vb_cache_line_i_modified(64*(i+1)-1 downto 64*i) :=
                (vb_cache_line_i_modified(64*(i+1)-1 downto 64*i)
                 and not vb_req_mask) or (r.req_wdata and vb_req_mask);

            vb_line_rdata_o_wstrb(8*(i+1)-1 downto 8*i) :=
                r.req_wstrb;
        end if;
    end loop;

    -- Flush counter when direct access
    if r.req_addr(CFG_DLOG2_NWAYS-1 downto 0) =
       conv_std_logic_vector(DCACHE_WAYS-1, CFG_DLOG2_NWAYS) then
        vb_addr_direct_next(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) := 
            r.req_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) + 1;
        vb_addr_direct_next(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
    else
        vb_addr_direct_next := r.req_addr + 1;
    end if;

    v_line_cs_read := '0';
    v_line_cs_write := '0';
    vb_line_addr := r.req_addr;
    vb_line_wdata := r.cache_line_i;
    vb_line_wstrb := (others => '0');
    v_line_wflags := (others => '0');


    -- System Bus access state machine
    case r.state is
    when State_Idle =>
        v.mpu_er_store := '0';
        v.mpu_er_load := '0';
        v_ready_next := '1';
    when State_CheckHit =>
        vb_resp_data := vb_cached_data;
        if line_hit_o = '1' then
            -- Hit
            v_resp_valid := '1';
            if i_resp_ready = '1' then
                if r.req_write = '1' then
                    -- Modify tagged mem output with request and write back
                    v_line_cs_write := '1';
                    v_line_wflags(TAG_FL_VALID) := '1';
                    v_line_wflags(DTAG_FL_DIRTY) := '1';
                    v.req_write := '0';
                    vb_line_wstrb := vb_line_rdata_o_wstrb;
                    vb_line_wdata := vb_line_rdata_o_modified;
                    if coherence_ena and line_rflags_o(DTAG_FL_SHARED) = '1' then
                        -- Make line: 'shared' -> 'unique' using write request
                        v.write_share := '1';
                        v.state := State_TranslateAddress;
                    else
                        if v_req_same_line = '1' then
                            -- Write address is the same as the next requested, so use it to write
                            -- value and update state machine
                            v_ready_next := '1';
                        end if;
                        v.state := State_Idle;
                    end if;
                else
                    v_ready_next := '1';
                    v.state := State_Idle;
                end if;
            end if;
        else
            -- Miss
            v.state := State_TranslateAddress;
        end if;
    when State_TranslateAddress =>
        if r.req_write = '1' and i_mpu_flags(CFG_MPU_FL_WR) = '0' then
            v.mpu_er_store := '1';
            v.cache_line_i := (others => '1');
            v.state := State_CheckResp;
        elsif r.req_write = '0' and i_mpu_flags(CFG_MPU_FL_RD) = '0' then
            v.mpu_er_load := '1';
            v.cache_line_i := (others => '1');
            v.state := State_CheckResp;
        else
            v.req_mem_valid := '1';
            v.state := State_WaitGrant;

            if i_mpu_flags(CFG_MPU_FL_CACHABLE) = '1' then
                -- Cached:
                if r.write_share = '1' then
                    v.req_mem_type := WriteLineUnique;
                    v.mem_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) := 
                        line_raddr_o(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE);
                    v.mem_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
                elsif line_rflags_o(TAG_FL_VALID) = '1' and
                    line_rflags_o(DTAG_FL_DIRTY) = '1' then
                    v.write_first := '1';
                    v.req_mem_type := WriteBack;
                    v.mem_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) := 
                        line_raddr_o(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE);
                    v.mem_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
                else
                    -- 1. Read -> Save cache
                    -- 2. Read -> Modify -> Save cache
                    v.mem_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE) :=
                        r.req_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE);
                    v.mem_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
                    if r.req_write = '1' then
                        v.req_mem_type := ReadMakeUnique;
                    else
                        v.req_mem_type := ReadShared;
                    end if;
                end if;
                v.mem_wstrb := (others => '1');
                v.cache_line_o := line_rdata_o;
            else
                -- Uncached read/write
                v.mem_addr := r.req_addr(CFG_CPU_ADDR_BITS-1 downto 3) & "000";
                v.mem_wstrb := (others => '0');
                v.mem_wstrb(7 downto 0) := r.req_wstrb;
                if r.req_write = '1' then
                    v.req_mem_type := WriteNoSnoop;
                else
                    v.req_mem_type := ReadNoSnoop;
                end if;

                v.cache_line_o := (others => '0');
                v.cache_line_o(63 downto 0) := r.req_wdata;
            end if;
        end if;
        v.cache_line_i := (others => '0');
        v.load_fault := '0';
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            if r.write_flush = '1' or
                r.write_first = '1' or
                r.write_share = '1' or
                (r.req_write = '1' and r.req_mem_type(REQ_MEM_TYPE_CACHED) = '0') then
                v.state := State_WriteBus;
            else
                -- 1. uncached read
                -- 2. cached read or write
                v.state := State_WaitResp;
            end if;
            v.req_mem_valid := '0';
        end if;
    when State_WaitResp =>
        if i_mem_data_valid = '1' then
            v.cache_line_i := i_mem_data;
            v.state := State_CheckResp;
            if i_mem_load_fault = '1' then
                v.load_fault := '1';
            end if;
        elsif coherence_ena and
            i_req_snoop_valid = '1' and or_reduce(i_req_snoop_type) = '1' then
            -- Access cache data
            v_req_snoop_ready_on_wait := '1';
            v.snoop_restore_wait_resp := '1';
            v.req_addr_restore := r.req_addr;
            v.req_addr := i_req_snoop_addr;
            v.req_snoop_type := i_req_snoop_type;
            v.state := State_SnoopSetupAddr;
        end if;
    when State_CheckResp =>
        if r.req_mem_type(REQ_MEM_TYPE_CACHED) = '0'
           or r.load_fault = '1' then
            -- uncached read only (write goes to WriteBus) or cached load-modify fault
            v_resp_valid := '1';
            vb_resp_data := vb_uncached_data;
            v_resp_er_load_fault := r.load_fault and (not r.req_write);
            v_resp_er_store_fault := r.load_fault and r.req_write;
            if i_resp_ready = '1' then
                v.state := State_Idle;
            end if;
        else
            v.state := State_SetupReadAdr;
            v_line_cs_write := '1';
            v_line_wflags(TAG_FL_VALID) := '1';
            v_line_wflags(DTAG_FL_SHARED) := '1';
            vb_line_wstrb := (others => '1');  -- write full line
            if r.req_write = '1' then
                -- Modify tagged mem output with request before write
                v.req_write := '0';
                v_line_wflags(DTAG_FL_DIRTY) := '1';
                v_line_wflags(DTAG_FL_SHARED) := '0';
                vb_line_wdata := vb_cache_line_i_modified;
                v_resp_valid := '1';
                v.state := State_Idle;
            end if;
        end if;
    when State_SetupReadAdr =>
        v.state := State_CheckHit;
    when State_WriteBus =>
        if i_mem_data_valid = '1' then
            if r.write_share = '1' then
                v.write_share := '0';
                v.state := State_Idle;
            elsif r.write_flush = '1' then
                -- Offloading Cache line on flush request
                v.state := State_FlushAddr;
            elsif r.write_first = '1' then
                v.mem_addr := r.req_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE)
                            & zero64(CFG_DLOG2_BYTES_PER_LINE-1 downto 0);
                v.req_mem_valid := '1';
                v.write_first := '0';
                if r.req_write = '1' then
                    -- read request: read-modify-save cache line
                    v.req_mem_type := ReadMakeUnique;
                else
                    v.req_mem_type := ReadShared;
                end if;
                v.state := State_WaitGrant;
            else
                -- Non-cached write
                v.state := State_Idle;
                v_resp_valid := '1';
                v_resp_er_store_fault := i_mem_store_fault;
            end if;
        elsif coherence_ena and
            i_req_snoop_valid = '1' and or_reduce(i_req_snoop_type) = '1' then
            -- Access cache data cannot be in the same clock as i_mem_data_valid
            v_req_snoop_ready_on_wait := '1';
            v.snoop_restore_write_bus := '1';
            v.req_addr_restore := r.req_addr;
            v.req_addr := i_req_snoop_addr;
            v.req_snoop_type := i_req_snoop_type;
            v.state := State_SnoopSetupAddr;
        end if;
    when State_FlushAddr =>
        v.state := State_FlushCheck;
        v_direct_access := r.req_flush_all;    -- 0=only if hit; 1=will be applied ignoring hit
        v_invalidate := '1';                   -- generate: wstrb='1; wflags='0
        v.write_flush := '0';
        v.cache_line_i := (others => '0');
    when State_FlushCheck =>
        v.cache_line_o := line_rdata_o;
        v_direct_access := r.req_flush_all;
        v_line_cs_write := r.req_flush_all;

        if line_rflags_o(TAG_FL_VALID) = '1' and
           line_rflags_o(DTAG_FL_DIRTY) = '1' then
            -- Off-load valid line
            v.write_flush := '1';
            v.mem_addr := line_raddr_o;
            v.req_mem_valid := '1';
            v.req_mem_type := WriteBack;
            v.mem_wstrb := (others => '1');
            v.state := State_WaitGrant;
        else
            -- Write clean line
            v.state := State_FlushAddr;
            if or_reduce(r.flush_cnt) = '0' then
                v.state := State_Idle;
                v_flush_end := '1';
            end if;
        end if;

        if or_reduce(r.flush_cnt) = '1' then
            v.flush_cnt := r.flush_cnt - 1;
            if r.req_flush_all = '1' then
                v.req_addr := vb_addr_direct_next;
            else
                v.req_addr := r.req_addr + DCACHE_BYTES_PER_LINE;
            end if;
        end if;
    when State_ResetAddr =>
        -- Write clean line
        v_direct_access := '1';
        v_invalidate := '1';                       -- generate: wstrb='1; wflags='0
        v.state := State_ResetWrite;
    when State_ResetWrite =>
        v_direct_access := '1';
        v_line_cs_write := '1';
        v.state := State_ResetAddr;
        if or_reduce(r.flush_cnt) = '1' then
            v.flush_cnt := r.flush_cnt - 1;
            v.req_addr := vb_addr_direct_next;
        else
            v.state := State_Idle;
        end if;

    when State_SnoopSetupAddr =>
        v.state := State_SnoopReadData;
        v_invalidate := r.req_snoop_type(SNOOP_REQ_TYPE_READCLEAN);
    when State_SnoopReadData =>
        v_resp_snoop_valid := '1';
        if r.req_snoop_type(SNOOP_REQ_TYPE_READCLEAN) = '0' then
            v_line_cs_write := '1';
            vb_line_wdata := line_rdata_o;
            vb_line_wstrb := (others => '1');
            v_line_wflags := line_rflags_o;
            v_line_wflags(DTAG_FL_DIRTY) := '0';
            v_line_wflags(DTAG_FL_SHARED) := '1';
        end if;
        -- restore state
        v.snoop_restore_wait_resp := '0';
        v.snoop_restore_write_bus := '0';
        if r.snoop_restore_wait_resp = '1' then
            v.req_addr := r.req_addr_restore;
            v.state := State_WaitResp;
        elsif r.snoop_restore_write_bus = '1' then
            v.req_addr := r.req_addr_restore;
            v.state := State_WriteBus;
        else
            v.state := State_Idle;
        end if;
    when others =>
    end case;

    v_req_snoop_cohena := '0';
    if coherence_ena then
        v_req_snoop_cohena := v_ready_next and or_reduce(i_req_snoop_type);
    end if;

    v_req_snoop_ready :=
        (line_snoop_ready_o and (not or_reduce(i_req_snoop_type))) or
        v_req_snoop_cohena or v_req_snoop_ready_on_wait;

    v.snoop_flags_valid := i_req_snoop_valid and
        line_snoop_ready_o and (not or_reduce(i_req_snoop_type));

    if v_ready_next = '1' then
        if coherence_ena and
            i_req_snoop_valid = '1' and or_reduce(i_req_snoop_type) = '1' then
            -- Access cache data
            v.req_addr := i_req_snoop_addr;
            v.req_snoop_type := i_req_snoop_type;
            v.state := State_SnoopSetupAddr;
        elsif r.req_flush = '1' then
            v.state := State_FlushAddr;
            v.req_flush := '0';
            v.cache_line_i := (others => '0');
            v.req_addr := r.req_flush_addr;
            v.req_addr := r.req_flush_addr(CFG_CPU_ADDR_BITS-1 downto CFG_DLOG2_BYTES_PER_LINE)
                          & zero64(CFG_DLOG2_BYTES_PER_LINE-1 downto 0);
            v.flush_cnt := r.req_flush_cnt;
        else
            v_req_ready := '1';
            v_line_cs_read := i_req_valid;
            vb_line_addr := i_req_addr;
            if i_req_valid = '1' then
                v.req_addr := i_req_addr;
                v.req_wstrb := i_req_wstrb;
                v.req_wdata := i_req_wdata;
                v.req_write := i_req_write;
                v.state := State_CheckHit;
            end if;
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    line_direct_access_i <= v_direct_access;
    line_invalidate_i <= v_invalidate;
    line_re_i <= v_line_cs_read;
    line_we_i <= v_line_cs_write;
    line_addr_i <= vb_line_addr;
    line_wdata_i <= vb_line_wdata;
    line_wstrb_i <= vb_line_wstrb;
    line_wflags_i <= v_line_wflags;
    line_snoop_addr_i <= i_req_snoop_addr;

    o_req_ready <= v_req_ready;

    o_req_mem_valid <= r.req_mem_valid;
    o_req_mem_addr <= r.mem_addr;
    o_req_mem_type <= r.req_mem_type;
    o_req_mem_strob <= r.mem_wstrb;
    o_req_mem_data <= r.cache_line_o;

    o_resp_valid <= v_resp_valid;
    o_resp_data <= vb_resp_data;
    o_resp_addr <= r.req_addr;
    o_resp_er_addr <= r.req_addr;
    o_resp_er_load_fault <= v_resp_er_load_fault;
    o_resp_er_store_fault <= v_resp_er_store_fault;
    o_resp_er_mpu_load <= r.mpu_er_load;
    o_resp_er_mpu_store <= r.mpu_er_store;
    o_mpu_addr <= r.req_addr;
    o_flush_end <= v_flush_end;

    o_req_snoop_ready <= v_req_snoop_ready;
    o_resp_snoop_valid <= v_resp_snoop_valid;
    o_resp_snoop_data <= line_rdata_o;
    o_resp_snoop_flags <= line_snoop_flags_o;
    
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
