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

entity dcache_lru is generic (
    memtech : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- Control path:
    i_req_valid : in std_logic;
    i_req_write : in std_logic;
    i_req_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_wdata : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_req_wstrb : in std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_ready : out std_logic;
    o_resp_valid : out std_logic;
    o_resp_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_resp_er_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_er_load_fault : out std_logic;
    o_resp_er_store_fault : out std_logic;
    o_resp_er_mpu_load : out std_logic;
    o_resp_er_mpu_store : out std_logic;
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
    i_mem_store_fault : in std_logic;
    -- MPU interface
    o_mpu_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mpu_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- Debug Signals:
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_flush_valid : in std_logic;
    o_state : out std_logic_vector(3 downto 0)
  );
end; 
 
architecture arch_dcache_lru of dcache_lru is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  constant State_Idle : std_logic_vector(3 downto 0) := "0000";
  constant State_CheckHit : std_logic_vector(3 downto 0) := "0001";
  constant State_CheckMPU : std_logic_vector(3 downto 0) := "0010";
  constant State_WaitGrant : std_logic_vector(3 downto 0) := "0011";
  constant State_WaitResp : std_logic_vector(3 downto 0) := "0100";
  constant State_CheckResp : std_logic_vector(3 downto 0) := "0101";
  constant State_SetupReadAdr : std_logic_vector(3 downto 0) := "0110";
  constant State_WriteBus : std_logic_vector(3 downto 0) := "0111";
  constant State_FlushAddr : std_logic_vector(3 downto 0) := "1000";
  constant State_FlushCheck : std_logic_vector(3 downto 0) := "1001";

  signal line_cs_i : std_logic;
  signal line_addr_i : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal line_wdata_i : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
  signal line_wstrb_i : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
  signal line_wflags_i : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
  signal line_flush_i : std_logic;
  signal line_raddr_o : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal line_rdata_o : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
  signal line_rflags_o : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
  signal line_hit_o : std_logic;

  type RegistersType is record
      requested : std_logic;
      req_write : std_logic;
      req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_addr_b_resp : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_wdata : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
      req_wstrb : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
      state : std_logic_vector(3 downto 0);
      req_mem_valid : std_logic;
      mem_write : std_logic;
      mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      burst_cnt : integer range 0 to DCACHE_BURST_LEN-1;
      burst_rstrb : std_logic_vector(DCACHE_BURST_LEN-1 downto 0);
      cached : std_logic;
      writable : std_logic;
      readable : std_logic;
      load_fault : std_logic;
      write_first : std_logic;
      write_flush : std_logic;
      mem_wstrb : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
      req_flush : std_logic;
      req_flush_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_flush_cnt : std_logic_vector(CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS-1 downto 0);
      flush_cnt : std_logic_vector(CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS-1 downto 0);
      cache_line_i : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
      cache_line_o : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
      init : std_logic;          -- remove xxx from memory simulation
  end record;

  constant R_RESET : RegistersType := (
    '0', '0',                               -- requested, req_write
    (others => '0'), (others => '0'),       -- req_addr, req_addr_b_resp
    (others => '0'), (others => '0'),       -- req_wdata, req_wstrb
    State_FlushAddr,                        -- state
    '0',                                    -- req_mem_valid
    '0',                                    -- mem_write
    (others => '0'),                        -- mem_addr,
    0,                                      -- burst_cnt
    (others => '1'),                        -- burst_rstrb
    '0',                                    -- cached
    '0',                                    -- writable
    '0',                                    -- readable
    '0',                                    -- load_fault
    '0',                                    -- write_first
    '0',                                    -- write_flush
    (others => '0'),                        -- mem_wstrb
    '0',                                    -- req_flush
    (others => '0'),                        -- req_flush_addr [0]=1 flush all
    (others => '0'),                        -- req_flush_cnt
    (others => '1'),                        -- flush_cnt
    (others => '0'),                        -- cache_line_i
    (others => '0'),                        -- cache_line_o
    '1'                                     -- init
  );

  signal r, rin : RegistersType;

begin

  tagmem0 : tagmemnway generic map (
      memtech => memtech,
      async_reset => async_reset,
      abus => BUS_ADDR_WIDTH,
      waybits => CFG_DLOG2_NWAYS,
      ibits => CFG_DLOG2_LINES_PER_WAY,
      lnbits => CFG_DLOG2_BYTES_PER_LINE,
      flbits => DTAG_FL_TOTAL
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
      o_hit => line_hit_o
  );


  comb : process(i_nrst, i_req_valid, i_req_write, i_req_addr, i_req_wdata, i_req_wstrb,
                i_resp_ready, i_req_mem_ready,
                i_mem_data_valid, i_mem_data, i_mem_load_fault, i_mem_store_fault,
                i_mpu_flags, i_flush_address, i_flush_valid,
                line_raddr_o, line_rdata_o, line_hit_o, line_rflags_o, r)
    variable v : RegistersType;
    variable vb_cache_line_i_modified : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_rdata_o_modified : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_rdata_o_wstrb : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
    
    variable v_last : std_logic;
    variable v_req_ready : std_logic;
    variable v_req_mem_len : std_logic_vector(7 downto 0);
    variable vb_cached_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable vb_uncached_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable v_resp_valid : std_logic;
    variable vb_resp_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable v_resp_er_load_fault : std_logic;
    variable v_resp_er_mpu_load : std_logic;
    variable v_resp_er_mpu_store : std_logic;
    variable v_flush : std_logic;
    variable v_line_cs : std_logic;
    variable vb_line_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_line_wdata : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_line_wstrb : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
    variable vb_req_mask : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    variable v_line_wflags : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
    variable vb_err_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable ridx : integer range 0 to DCACHE_BURST_LEN-1;
    variable v_req_same_line : std_logic;
  begin

    v := r;

    v_req_ready := '0';
    v_resp_valid := '0';
    v_resp_er_load_fault := '0';
    v_resp_er_mpu_load := '0';
    v_resp_er_mpu_store := '0';
    v_flush := '0';
    v_last := '0';
    v_req_mem_len := conv_std_logic_vector(DCACHE_BURST_LEN-1, 8);
    ridx := conv_integer(r.req_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto CFG_LOG2_DATA_BYTES));

    vb_cached_data := line_rdata_o((ridx+1)*BUS_DATA_WIDTH - 1 downto
                                         ridx*BUS_DATA_WIDTH);
    vb_uncached_data := r.cache_line_i(BUS_DATA_WIDTH-1 downto 0);

    if i_mem_store_fault = '1' then
        vb_err_addr := r.req_addr_b_resp;
    else
        vb_err_addr := r.req_addr;
    end if;

    v_req_same_line := '0';
    if r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE)
        = i_req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE) then
        v_req_same_line := '1';
    end if;


    if i_flush_valid = '1' then
        v.req_flush := '1';
        if i_flush_address(0) = '1' then
            v.req_flush_cnt := (others => '1');
            v.req_flush_addr := (others => '0');
        else
            v.req_flush_cnt := (others => '0');
            v.req_flush_addr := i_flush_address;
        end if;
    end if;

    for i in 0 to BUS_DATA_BYTES-1 loop
        vb_req_mask(8*i+7 downto 8*i) := (others => r.req_wstrb(i));
    end loop;

    vb_line_rdata_o_modified := line_rdata_o;
    vb_cache_line_i_modified := r.cache_line_i;
    vb_line_rdata_o_wstrb := (others => '0');
    for i in 0 to DCACHE_BURST_LEN-1 loop
        if i = ridx then
            vb_line_rdata_o_modified(BUS_DATA_WIDTH*(i+1)-1 downto BUS_DATA_WIDTH*i) :=
                (vb_line_rdata_o_modified(BUS_DATA_WIDTH*(i+1)-1 downto BUS_DATA_WIDTH*i)
                 and not vb_req_mask) or (r.req_wdata and vb_req_mask);

            vb_cache_line_i_modified(BUS_DATA_WIDTH*(i+1)-1 downto BUS_DATA_WIDTH*i) :=
                (vb_cache_line_i_modified(BUS_DATA_WIDTH*(i+1)-1 downto BUS_DATA_WIDTH*i)
                 and not vb_req_mask) or (r.req_wdata and vb_req_mask);

            vb_line_rdata_o_wstrb(BUS_DATA_BYTES*(i+1)-1 downto BUS_DATA_BYTES*i) :=
                r.req_wstrb;
        end if;
    end loop;

    v_line_cs := '0';
    vb_line_addr := r.req_addr;
    vb_line_wdata := r.cache_line_i;
    vb_line_wstrb := (others => '0');
    v_line_wflags := (others => '0');


    -- System Bus access state machine
    case r.state is
    when State_Idle =>
        if r.req_flush = '1' then
            v.state := State_FlushAddr;
            v.req_flush := '0';
            v.cache_line_i := (others => '1');
            if r.req_flush_addr(0) = '1' then
                v.req_addr := (others => '0');
                v.flush_cnt := (others => '1');
            else
                v.req_addr := r.req_flush_addr;
                v.req_addr(LOG2_DATA_BYTES_MASK-1 downto 0) := (others => '0');
                v.flush_cnt := r.req_flush_cnt;
            end if;
        else
            v_line_cs := i_req_valid;
            v_req_ready := '1';
            vb_line_addr := i_req_addr;
            if i_req_valid = '1' then
                v.requested := '1';
                v.req_addr := i_req_addr;
                v.req_wstrb := i_req_wstrb;
                v.req_wdata := i_req_wdata;
                v.req_write := i_req_write;
                v.state := State_CheckHit;
            else
                v.requested := '0';
            end if;
        end if;
    when State_CheckHit =>
        vb_resp_data := vb_cached_data;
        if line_hit_o = '1' then
            -- Hit
            v_resp_valid := '1';
            if r.req_write = '1' then
                -- Modify tagged mem output with request and write back
                v_line_cs := '1';
                v_line_wflags(TAG_FL_VALID) := '1';
                v_line_wflags(DTAG_FL_DIRTY) := '1';
                v.req_write := '0';
                vb_line_wstrb := vb_line_rdata_o_wstrb;
                vb_line_wdata := vb_line_rdata_o_modified;
                if i_resp_ready = '0' then
                    -- Do nothing: wait accept
                elsif v_req_same_line = '1' and i_req_valid = '1' then
                    -- 1 clock write cycle using previously set read address
                    v_req_ready := '1';
                    v.state := State_CheckHit;
                    v_line_cs := i_req_valid;
                    v.req_addr := i_req_addr;
                    v.req_wstrb := i_req_wstrb;
                    v.req_wdata := i_req_wdata;
                    v.req_write := i_req_write;
                    vb_line_addr := i_req_addr;
                else
                    v.state := State_Idle;
                    v.requested := '0';
                end if;
            else
                v_req_ready := '1';
                if i_resp_ready = '0' then
                    -- Do nothing: wait accept
                elsif i_req_valid = '1' then
                    v.state := State_CheckHit;
                    v_line_cs := i_req_valid;
                    v.req_addr := i_req_addr;
                    v.req_wstrb := i_req_wstrb;
                    v.req_wdata := i_req_wdata;
                    v.req_write := i_req_write;
                    vb_line_addr := i_req_addr;
                else
                    v.state := State_Idle;
                    v.requested := '0';
                end if;
            end if;
        else
            -- Miss
            v.state := State_CheckMPU;
            if r.req_write = '1' then
                v_resp_valid := '1';
            end if;
        end if;
    when State_CheckMPU =>
        v.req_mem_valid := '1';
        v.mem_write := '0';
        v.state := State_WaitGrant;

        if i_mpu_flags(CFG_MPU_FL_CACHABLE) = '1' then
            if line_rflags_o(TAG_FL_VALID) = '1' and
                line_rflags_o(DTAG_FL_DIRTY) = '1' then
                v.write_first := '1';
                v.mem_write := '1';
                v.mem_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE) := 
                    line_raddr_o(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE);
                v.mem_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
            else
                v.mem_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE) :=
                    r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE);
                v.mem_addr(CFG_DLOG2_BYTES_PER_LINE-1 downto 0) := (others => '0');
            end if;
            v.mem_wstrb := (others => '1');
            v.burst_cnt := DCACHE_BURST_LEN-1;
            v.cached := '1';
            v.cache_line_o := line_rdata_o;
        else
            v.mem_addr(BUS_ADDR_WIDTH-1 downto CFG_LOG2_DATA_BYTES) :=
                r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_LOG2_DATA_BYTES);
            v.mem_addr(CFG_LOG2_DATA_BYTES-1 downto 0) := (others => '0');
            v.mem_wstrb := r.req_wstrb;
            v.mem_write := r.req_write;
            v.burst_cnt := 0;
            v.cached := '0';
            v_req_mem_len := (others => '0');

            v.cache_line_o := (others => '0');
            v.cache_line_o(BUS_DATA_WIDTH-1 downto 0) := r.req_wdata;
        end if;
        v.burst_rstrb := (others => '0');
        v.burst_rstrb(0) := '1';
        v.cache_line_i := (others => '0');
        v.load_fault := '0';
        v.writable := i_mpu_flags(CFG_MPU_FL_WR);
        v.readable := i_mpu_flags(CFG_MPU_FL_RD);
    when State_WaitGrant =>
        if i_req_mem_ready = '1' then
            if r.write_flush = '1' or
                r.write_first = '1' or
                (r.req_write = '1' and r.cached = '0') then
                v.state := State_WriteBus;
            else
                v.state := State_WaitResp;
            end if;
            v.req_mem_valid := '0';
        end if;
        if r.cached = '0' then
            v_req_mem_len := (others => '0');
        end if;
    when State_WaitResp =>
        if r.burst_cnt = 0 then
            v_last := '1';
        end if;
        if i_mem_data_valid = '1' then
            for k in 0 to DCACHE_BURST_LEN-1 loop
                if r.burst_rstrb(k) = '1' then
                    v.cache_line_i((k+1)*BUS_DATA_WIDTH-1 downto
                                    k*BUS_DATA_WIDTH) := i_mem_data;
                end if;
            end loop;
            if r.burst_cnt = 0 then
                v.state := State_CheckResp;
            else
                v.burst_cnt := r.burst_cnt - 1;
            end if;
            v.burst_rstrb := r.burst_rstrb(DCACHE_BURST_LEN-2 downto 0) & '0';
            if i_mem_load_fault = '1' then
                v.load_fault := '1';
            end if;
        end if;
    when State_CheckResp =>
        if r.cached = '1' then
            v.state := State_SetupReadAdr;
            v_line_cs := '1';
            v_line_wflags(TAG_FL_VALID) := '1';
            vb_line_wstrb := (others => '1');  -- write full line
            if r.req_write = '1' then
                -- Modify tagged mem output with request before write
                v.req_write := '0';
                v_line_wflags(DTAG_FL_DIRTY) := '1';
                vb_line_wdata := vb_cache_line_i_modified;
            end if;
        else
            if r.req_write = '0' then
                v_resp_valid := '1';
            end if;
            vb_resp_data := vb_uncached_data;
            v_resp_er_load_fault := r.load_fault;
            if i_resp_ready = '1' then
                v.state := State_Idle;
                v.requested := '0';
            end if;
        end if;
    when State_SetupReadAdr =>
        v.state := State_CheckHit;
    when State_WriteBus =>
        if r.burst_cnt = 0 then
            v_last := '1';
        end if;
        if i_mem_data_valid = '1' then
            -- Shift right to send lower part on AXI
            v.cache_line_o := zero64(BUS_DATA_WIDTH-1 downto 0)
                            & r.cache_line_o(DCACHE_LINE_BITS-1 downto BUS_DATA_WIDTH);
            if r.burst_cnt = 0 then
                v.req_addr_b_resp := r.req_addr;
                if r.write_flush = '1' then
                    -- Offloading Cache line on flush request
                    v.state := State_FlushAddr;
                elsif r.write_first = '1' then
                    v.mem_addr := r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE)
                                & zero64(CFG_DLOG2_BYTES_PER_LINE-1 downto 0);
                    v.req_mem_valid := '1';
                    v.burst_cnt := DCACHE_BURST_LEN-1;
                    v.write_first := '0';
                    v.mem_write := '0';
                    v.state := State_WaitGrant;
                else
                    -- Non-cached write
                    v.state := State_Idle;
                end if;
            else
                v.burst_cnt := r.burst_cnt - 1;
            end if;
            --if i_resp_mem_store_fault = '1' then
            --    v.store_fault := '1';
            --end if;
        end if;
    when State_FlushAddr =>
        v_line_cs := '1';
        v_flush := '1';
        v.state := State_FlushCheck;
        v.write_flush := '0';
        v.cache_line_i := (others => '0');
        if r.flush_cnt = zero64(CFG_DLOG2_LINES_PER_WAY+CFG_DLOG2_NWAYS-1 downto 0) then
            v.state := State_Idle;
            v.init := '0';
        end if;
    when State_FlushCheck =>
        v.cache_line_o := line_rdata_o;
        v_line_wflags := (others => '0');      -- flag valid = 0
        vb_line_wstrb := (others => '1');      -- write full line
        v_line_cs := '1';
        v_flush := '1';

        if r.init = '0' and
            line_rflags_o(TAG_FL_VALID) = '1' and
            line_rflags_o(DTAG_FL_DIRTY) = '1' then
            -- Off-load valid line
            v.write_flush := '1';
            v.mem_addr := line_raddr_o;
            v.req_mem_valid := '1';
            v.burst_cnt := DCACHE_BURST_LEN-1;
            v.mem_write := '1';
            v.state := State_WaitGrant;
        else
            -- Write clean line
            v.state := State_FlushAddr;
            if r.flush_cnt = zero64(CFG_DLOG2_LINES_PER_WAY+CFG_DLOG2_NWAYS-1 downto 0) then
                v.state := State_Idle;
                v.init := '0';
            end if;
        end if;

        if r.flush_cnt /= zero64(CFG_DLOG2_LINES_PER_WAY+CFG_DLOG2_NWAYS-1 downto 0) then
            v.flush_cnt := r.flush_cnt - 1;
            -- Use lsb address bits to manually select memory WAY bank:
            if r.req_addr(CFG_DLOG2_NWAYS-1 downto 0) =
               conv_std_logic_vector(DCACHE_WAYS-1, CFG_DLOG2_NWAYS) then
                v.req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE) := 
                    r.req_addr(BUS_ADDR_WIDTH-1 downto CFG_DLOG2_BYTES_PER_LINE) + 1;
               v.req_addr(LOG2_DATA_BYTES_MASK-1 downto 0) := (others => '0');
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
    o_req_mem_write <= r.mem_write;
    o_req_mem_strob <= r.mem_wstrb;
    o_req_mem_data <= r.cache_line_o(BUS_DATA_WIDTH-1 downto 0);
    o_req_mem_len <= v_req_mem_len;
    o_req_mem_burst <= "01";    -- 00=FIX; 01=INCR; 10=WRAP
    o_req_mem_last <= v_last;

    o_resp_valid <= v_resp_valid;
    o_resp_data <= vb_resp_data;
    o_resp_addr <= r.req_addr;
    o_resp_er_addr <= vb_err_addr;
    o_resp_er_load_fault <= v_resp_er_load_fault;
    o_resp_er_store_fault <= i_mem_store_fault;
    o_resp_er_mpu_load <= v_resp_er_mpu_load;
    o_resp_er_mpu_store <= v_resp_er_mpu_store;
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
