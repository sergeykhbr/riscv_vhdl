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
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity CacheTop is generic (
    memtech : integer;
    async_reset : boolean;
    coherence_ena : boolean
  );
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    o_resp_ctrl_load_fault : out std_logic;
    o_resp_ctrl_executable : out std_logic;
    i_resp_ctrl_ready : in std_logic;
    -- Data path:
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_req_data_wdata : in std_logic_vector(63 downto 0);
    i_req_data_wstrb : in std_logic_vector(7 downto 0);
    o_req_data_ready : out std_logic;
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_data_data : out std_logic_vector(63 downto 0);
    o_resp_data_store_fault_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_data_load_fault : out std_logic;
    o_resp_data_store_fault : out std_logic;
    o_resp_data_er_mpu_load : out std_logic;
    o_resp_data_er_mpu_store : out std_logic;
    i_resp_data_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                    -- AXI request was accepted
    o_req_mem_path : out std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_type : out std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
    o_req_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_req_mem_strob : out std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
    o_req_mem_data : out std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);  -- burst transaction length
    i_resp_mem_valid : in std_logic;
    i_resp_mem_path : in std_logic;
    i_resp_mem_data : in std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    -- MPU interface:
    i_mpu_region_we : in std_logic;
    i_mpu_region_idx : in std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    i_mpu_region_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mpu_region_mask : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mpu_region_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- D$ Snoop interface
    i_req_snoop_valid : in std_logic;
    i_req_snoop_type : in std_logic_vector(SNOOP_REQ_TYPE_BITS-1 downto 0);
    o_req_snoop_ready : out std_logic;
    i_req_snoop_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_resp_snoop_ready : in std_logic;
    o_resp_snoop_valid : out std_logic;
    o_resp_snoop_data : out std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
    o_resp_snoop_flags : out std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
    -- Debug signals:
    i_flush_address : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic;                                      -- address to clear icache is valid
    i_data_flush_address : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- clear D$ address
    i_data_flush_valid : in std_logic;                                      -- address to clear D$ is valid
    o_data_flush_end : out std_logic
  );
end; 
 
architecture arch_CacheTop of CacheTop is
  constant DATA_PATH : std_logic := '0';
  constant CTRL_PATH : std_logic := '1';
  constant CACHE_QUEUE_WIDTH : integer :=
        CFG_CPU_ADDR_BITS      -- addr
        + REQ_MEM_TYPE_BITS    -- req_type
        + 1                     -- 1=instruction; 0=data
        ;


  type CacheOutputType is record
      req_mem_valid : std_logic;
      req_mem_type : std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
      req_mem_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      req_mem_strob : std_logic_vector(DCACHE_BYTES_PER_LINE-1 downto 0);
      req_mem_wdata : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
      mpu_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;


  signal i :  CacheOutputType;
  signal d :  CacheOutputType;

  signal queue_we_i : std_logic;
  signal queue_wdata_i : std_logic_vector(CACHE_QUEUE_WIDTH-1 downto 0);
  signal queue_rdata_o : std_logic_vector(CACHE_QUEUE_WIDTH-1 downto 0);
  signal queue_nempty_o : std_logic;
  signal queue_full_o : std_logic;

  -- Memory Control interface:
  signal w_ctrl_resp_mem_data_valid : std_logic;
  signal w_ctrl_resp_mem_load_fault : std_logic;
  signal w_resp_ctrl_writable_unused : std_logic;
  signal w_resp_ctrl_readable_unused : std_logic;
  signal w_ctrl_req_ready : std_logic;

  -- Memory Data interface:
  signal w_data_resp_mem_data_valid : std_logic;
  signal w_data_resp_mem_load_fault : std_logic;
  signal w_data_req_ready : std_logic;

  signal wb_mpu_iflags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
  signal wb_mpu_dflags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);

  signal wb_ctrl_bus : std_logic_vector(CACHE_QUEUE_WIDTH-1 downto 0);
  signal wb_data_bus : std_logic_vector(CACHE_QUEUE_WIDTH-1 downto 0);

begin

    wb_ctrl_bus <= CTRL_PATH &
                   i.req_mem_type &
                   i.req_mem_addr;

    wb_data_bus <= DATA_PATH &
                   d.req_mem_type &
                   d.req_mem_addr;

    queue_wdata_i <= wb_data_bus when d.req_mem_valid = '1' else wb_ctrl_bus;
    queue_we_i <= i.req_mem_valid or d.req_mem_valid;

    w_data_req_ready <= '1';
    w_ctrl_req_ready <= not d.req_mem_valid;

    w_ctrl_resp_mem_data_valid <= i_resp_mem_valid when i_resp_mem_path = CTRL_PATH else '0';
    w_data_resp_mem_data_valid <= '0' when i_resp_mem_path = CTRL_PATH else i_resp_mem_valid;

    w_ctrl_resp_mem_load_fault <= i_resp_mem_load_fault when i_resp_mem_path = CTRL_PATH else '0';
    w_data_resp_mem_load_fault <= '0' when i_resp_mem_path = CTRL_PATH else i_resp_mem_load_fault;

    o_req_mem_valid <= queue_nempty_o;

    o_req_mem_path <= queue_rdata_o(CFG_CPU_ADDR_BITS+REQ_MEM_TYPE_BITS);
    o_req_mem_type <= queue_rdata_o(CFG_CPU_ADDR_BITS+REQ_MEM_TYPE_BITS-1 downto CFG_CPU_ADDR_BITS);
    o_req_mem_addr <= queue_rdata_o(CFG_CPU_ADDR_BITS-1 downto 0);

    o_req_mem_strob <= d.req_mem_strob;
    o_req_mem_data <= d.req_mem_wdata;

    queue0 : Queue generic map (
        async_reset => async_reset,
        szbits => 2,
        dbits => CACHE_QUEUE_WIDTH
    ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_re => i_req_mem_ready,
        i_we => queue_we_i,
        i_wdata => queue_wdata_i,
        o_rdata => queue_rdata_o,
        o_full => queue_full_o,
        o_nempty => queue_nempty_o
    );

    i0 : icache_lru generic map (
        memtech => memtech,
        async_reset => async_reset
     ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_valid => i_req_ctrl_valid,
        i_req_addr => i_req_ctrl_addr,
        o_req_ready => o_req_ctrl_ready,
        o_resp_valid => o_resp_ctrl_valid,
        o_resp_addr => o_resp_ctrl_addr,
        o_resp_data => o_resp_ctrl_data,
        o_resp_load_fault => o_resp_ctrl_load_fault,
        o_resp_executable => o_resp_ctrl_executable,
        o_resp_writable => w_resp_ctrl_writable_unused,
        o_resp_readable => w_resp_ctrl_readable_unused,
        i_resp_ready => i_resp_ctrl_ready,
        i_req_mem_ready => w_ctrl_req_ready,
        o_req_mem_valid => i.req_mem_valid,
        o_req_mem_type => i.req_mem_type,
        o_req_mem_addr => i.req_mem_addr,
        o_req_mem_strob => i.req_mem_strob,
        o_req_mem_data => i.req_mem_wdata,
        i_mem_data_valid => w_ctrl_resp_mem_data_valid,
        i_mem_data => i_resp_mem_data,
        i_mem_load_fault => w_ctrl_resp_mem_load_fault,
        o_mpu_addr => i.mpu_addr,
        i_mpu_flags => wb_mpu_iflags,
        i_flush_address => i_flush_address,
        i_flush_valid => i_flush_valid
    );

    d0 : dcache_lru generic map (
        memtech => memtech,
        async_reset => async_reset,
        coherence_ena => coherence_ena
     ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_valid => i_req_data_valid,
        i_req_write => i_req_data_write,
        i_req_addr => i_req_data_addr,
        i_req_wdata => i_req_data_wdata,
        i_req_wstrb => i_req_data_wstrb,
        o_req_ready => o_req_data_ready,
        o_resp_valid => o_resp_data_valid,
        o_resp_addr => o_resp_data_addr,
        o_resp_data => o_resp_data_data,
        o_resp_er_addr => o_resp_data_store_fault_addr,
        o_resp_er_load_fault => o_resp_data_load_fault,
        o_resp_er_store_fault => o_resp_data_store_fault,
        o_resp_er_mpu_load => o_resp_data_er_mpu_load,
        o_resp_er_mpu_store => o_resp_data_er_mpu_store,
        i_resp_ready => i_resp_data_ready,
        i_req_mem_ready => w_data_req_ready,
        o_req_mem_valid => d.req_mem_valid,
        o_req_mem_type => d.req_mem_type,
        o_req_mem_addr => d.req_mem_addr,
        o_req_mem_strob => d.req_mem_strob,
        o_req_mem_data => d.req_mem_wdata,
        i_mem_data_valid => w_data_resp_mem_data_valid,
        i_mem_data => i_resp_mem_data,
        i_mem_load_fault => w_data_resp_mem_load_fault,
        i_mem_store_fault => i_resp_mem_store_fault,
        o_mpu_addr => d.mpu_addr,
        i_mpu_flags => wb_mpu_dflags,
        i_req_snoop_valid => i_req_snoop_valid,
        i_req_snoop_type => i_req_snoop_type,
        o_req_snoop_ready => o_req_snoop_ready,
        i_req_snoop_addr => i_req_snoop_addr,
        i_resp_snoop_ready => i_resp_snoop_ready,
        o_resp_snoop_valid => o_resp_snoop_valid,
        o_resp_snoop_data => o_resp_snoop_data,
        o_resp_snoop_flags => o_resp_snoop_flags,
        i_flush_address => i_data_flush_address,
        i_flush_valid => i_data_flush_valid,
        o_flush_end => o_data_flush_end
    );

    mpu0 : mpu generic map (
        async_reset => async_reset
    ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_iaddr => i.mpu_addr,
        i_daddr => d.mpu_addr,
        i_region_we => i_mpu_region_we,
        i_region_idx => i_mpu_region_idx,
        i_region_addr => i_mpu_region_addr,
        i_region_mask => i_mpu_region_mask,
        i_region_flags => i_mpu_region_flags,
        o_iflags => wb_mpu_iflags,
        o_dflags => wb_mpu_dflags
    );

end;
