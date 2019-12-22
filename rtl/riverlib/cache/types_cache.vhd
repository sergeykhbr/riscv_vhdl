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

  --! ICacheLru config
  constant CFG_IOFFSET_WIDTH   : integer := 5;    -- [4:0]  log2(ICACHE_LINE_BYTES)
  constant CFG_IODDEVEN_WIDTH  : integer := 1;    -- [5]    0=even; 1=odd
  constant CFG_ICACHE_WAYS     : integer := 4;    -- 4 odds, 4 even

  constant CFG_ILINES_PER_WAY : integer := 2**CFG_IINDEX_WIDTH;
  -- [31:14] tag when 64 KB
  -- [31:13] tag when 32 KB
  -- [31:12] tag when 16 KB
  constant CFG_ITAG_WIDTH : integer := BUS_ADDR_WIDTH
    - (CFG_IOFFSET_WIDTH + CFG_IODDEVEN_WIDTH + CFG_IINDEX_WIDTH);

  --! Store tag data as:
  --!     [3:0]            qword is valid flag
  --!     [4]              load_fault
  --!     [ITAG_WIDTH+5:5] tag value
  constant CFG_ITAG_WIDTH_TOTAL : integer := CFG_ITAG_WIDTH + 5;


  constant IINDEX_START : integer := CFG_IOFFSET_WIDTH + CFG_IODDEVEN_WIDTH;
  constant IINDEX_END : integer := IINDEX_START + CFG_IINDEX_WIDTH - 1;

  constant ITAG_START : integer := IINDEX_START + CFG_IINDEX_WIDTH;
  constant ITAG_END : integer := BUS_ADDR_WIDTH-1;

  component IWayMem is generic (
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
  end component;

  component ILru is port (
    i_clk : in std_logic;
    i_init : in std_logic;
    i_radr : in std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
    i_wadr : in std_logic_vector(CFG_IINDEX_WIDTH-1 downto 0);
    i_we : in std_logic;
    i_lru : in std_logic_vector(1 downto 0);
    o_lru : out std_logic_vector(1 downto 0)
  );
  end component;

  component lrunway is generic (
    abits : integer;        -- cache bus address bus (usually 6..8)
    waybits : integer       -- Number of ways bitwidth (=2 for 4-ways cache)
  );
  port (
    i_clk : in std_logic;
    i_flush : in std_logic;
    i_addr : in std_logic_vector(abits-1 downto 0);
    i_we : in std_logic;
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
  end component;

  component tagmemnway is generic (
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
    i_cs : in std_logic;
    i_flush : in std_logic;
    i_addr : in std_logic_vector(abus-1 downto 0);
    i_wdata : in std_logic_vector(8*(2**lnbits)-1 downto 0);
    i_wstrb : in std_logic_vector(2**lnbits-1 downto 0);
    i_wflags : in std_logic_vector(flbits-1 downto 0);
    o_raddr : out std_logic_vector(abus-1 downto 0);
    o_rdata : out std_logic_vector(8*(2**lnbits)-1 downto 0);
    o_rflags : out std_logic_vector(flbits-1 downto 0);
    o_hit : out std_logic
  );
  end component;

  component dcache_lru is generic (
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
  end component; 

end;
