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

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library riverlib;
use riverlib.river_cfg.all;

package types_cache is

  component lrunway is generic (
    abits : integer;        -- cache bus address bus (usually 6..8)
    waybits : integer       -- Number of ways bitwidth (=2 for 4-ways cache)
  );
  port (
    i_clk : in std_logic;
    i_init : in std_logic;
    i_raddr : in std_logic_vector(abits-1 downto 0);
    i_waddr : in std_logic_vector(abits-1 downto 0);
    i_up : in std_logic;
    i_down : in std_logic;
    i_lru : in std_logic_vector(waybits-1 downto 0);
    o_lru : out std_logic_vector(waybits-1 downto 0)
  );
  end component; 

  component tagmem is generic (
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
    i_snoop_addr : in std_logic_vector(abus-1 downto 0);
    o_snoop_flags : out std_logic_vector(flbits-1 downto 0)
  );
  end component;

  component tagmemnway is generic (
    memtech : integer := 0;
    async_reset : boolean := false;
    abus : integer := 64;          -- system bus address bus (32 or 64 bits)
    waybits : integer := 2;        -- log2 of number of ways bits (=2 for 4 ways)
    ibits : integer := 7;          -- lines memory addres width (usually 6..8)
    lnbits : integer := 5;         -- One line bits: log2(bytes_per_line)
    flbits : integer := 1;         -- Total flags number saved with address tag
    snoop : boolean := false       -- Snoop port disabled; 1 Enabled (L2 caching)
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
    o_rdata : out std_logic_vector(8*(2**lnbits)-1 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic;
    -- L2 snoop port, active when snoop = 1
    i_snoop_addr : in std_logic_vector(abus-1 downto 0);
    o_snoop_ready : out std_logic;
    o_snoop_flags : out std_logic_vector(flbits-1 downto 0)
  );
  end component;

  component tagmemcoupled is generic (
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
  end component;

  component icache_lru is generic (
    memtech : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- Control path:
    i_req_valid : in std_logic;
    i_req_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_req_ready : out std_logic;
    o_resp_valid : out std_logic;
    o_resp_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_resp_data : out std_logic_vector(31 downto 0);
    o_resp_load_fault : out std_logic;
    o_resp_executable : out std_logic;
    o_resp_writable : out std_logic;
    o_resp_readable : out std_logic;
    i_resp_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_type : out std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
    o_req_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_req_mem_strob : out std_logic_vector(ICACHE_BYTES_PER_LINE-1 downto 0);
    o_req_mem_data : out std_logic_vector(ICACHE_LINE_BITS-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data : in std_logic_vector(ICACHE_LINE_BITS-1 downto 0);
    i_mem_load_fault : in std_logic;
    -- MPU interface:
    o_mpu_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mpu_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- Debug Signals:
    i_flush_address : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic                                      -- address to clear icache is valid
  );
  end component; 


  component dcache_lru is generic (
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
  end component; 


  component mpu is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_iaddr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_daddr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_we : in std_logic;
    i_region_idx : in std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    i_region_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_mask : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);  -- {ena, cachable, r, w, x}
    o_iflags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    o_dflags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0)
  );
  end component; 

end;
