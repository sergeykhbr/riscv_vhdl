--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

entity RiverTop is
  generic (
    memtech : integer := 0;
    hartid : integer := 0;
    async_reset : boolean := false;
    fpu_ena : boolean := true;
    coherence_ena : boolean := false;
    tracer_ena : boolean := false
  );
  port (
    i_clk : in std_logic;                                             -- CPU clock
    i_nrst : in std_logic;                                            -- Reset. Active LOW.
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                   -- AXI request was accepted
    o_req_mem_path : out std_logic;                                   -- 0=ctrl; 1=data path
    o_req_mem_valid : out std_logic;                                  -- AXI memory request is valid
    o_req_mem_type : out std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);-- AXI memory request is write type
    o_req_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- AXI memory request address
    o_req_mem_strob : out std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);-- Writing strob. 1 bit per Byte
    o_req_mem_data : out std_logic_vector(L1CACHE_LINE_BITS-1 downto 0); -- Writing data
    i_resp_mem_valid : in std_logic;                                  -- AXI response is valid
    i_resp_mem_path : in std_logic;                                   -- 0=ctrl; 1=data path
    i_resp_mem_data : in std_logic_vector(L1CACHE_LINE_BITS-1 downto 0); -- Read data
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    -- Interrupt line from external interrupts controller (PLIC).
    i_ext_irq : in std_logic;
    -- D$ Snoop interface
    i_req_snoop_valid : in std_logic;
    i_req_snoop_type : in std_logic_vector(SNOOP_REQ_TYPE_BITS-1 downto 0);
    o_req_snoop_ready : out std_logic;
    i_req_snoop_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_resp_snoop_ready : in std_logic;
    o_resp_snoop_valid : out std_logic;
    o_resp_snoop_data : out std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
    o_resp_snoop_flags : out std_logic_vector(DTAG_FL_TOTAL-1 downto 0);
    -- Debug interface:
    i_dport_req_valid : in std_logic;                         -- Debug access from DSU is valid
    i_dport_write : in std_logic;                             -- Write command flag
    i_dport_addr : in std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0); -- Debug Port address
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Write value
    o_dport_req_ready : out std_logic;                        -- Ready to accept dbg request
    i_dport_resp_ready : in std_logic;                        -- Read to accept response
    o_dport_resp_valid : out std_logic;                       -- Response is valid
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Response value
    o_halted : out std_logic
  );
end;
 
architecture arch_RiverTop of RiverTop is

  -- Control path:
  signal w_req_ctrl_ready : std_logic;
  signal w_req_ctrl_valid : std_logic;
  signal wb_req_ctrl_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal w_resp_ctrl_valid : std_logic;
  signal wb_resp_ctrl_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal wb_resp_ctrl_data : std_logic_vector(31 downto 0);
  signal w_resp_ctrl_load_fault : std_logic;
  signal w_resp_ctrl_executable : std_logic;
  signal w_resp_ctrl_ready : std_logic;
  -- Data path:
  signal w_req_data_ready : std_logic;
  signal w_req_data_valid : std_logic;
  signal w_req_data_write : std_logic;
  signal wb_req_data_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal wb_req_data_wdata : std_logic_vector(63 downto 0);
  signal wb_req_data_wstrb : std_logic_vector(7 downto 0);
  signal w_resp_data_valid : std_logic;
  signal wb_resp_data_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal wb_resp_data_data : std_logic_vector(63 downto 0);
  signal w_resp_data_load_fault : std_logic;
  signal w_resp_data_store_fault : std_logic;
  signal w_resp_data_er_mpu_load : std_logic;
  signal w_resp_data_er_mpu_store : std_logic;
  signal wb_resp_data_store_fault_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal w_resp_data_ready : std_logic;
  signal w_mpu_region_we : std_logic;
  signal wb_mpu_region_idx : std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
  signal wb_mpu_region_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal wb_mpu_region_mask : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal wb_mpu_region_flags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
  signal wb_flush_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal w_flush_valid : std_logic;
  signal wb_data_flush_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal w_data_flush_valid : std_logic;
  signal w_data_flush_end : std_logic;

begin

    proc0 : Processor generic map (
        hartid => hartid,
        async_reset => async_reset,
        fpu_ena => fpu_ena,
        tracer_ena => tracer_ena
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_ctrl_ready => w_req_ctrl_ready,
        o_req_ctrl_valid => w_req_ctrl_valid,
        o_req_ctrl_addr => wb_req_ctrl_addr,
        i_resp_ctrl_valid => w_resp_ctrl_valid,
        i_resp_ctrl_addr => wb_resp_ctrl_addr,
        i_resp_ctrl_data => wb_resp_ctrl_data,
        i_resp_ctrl_load_fault => w_resp_ctrl_load_fault,
        i_resp_ctrl_executable => w_resp_ctrl_executable,
        o_resp_ctrl_ready => w_resp_ctrl_ready,
        i_req_data_ready => w_req_data_ready,
        o_req_data_valid => w_req_data_valid,
        o_req_data_write => w_req_data_write,
        o_req_data_addr => wb_req_data_addr,
        o_req_data_wdata => wb_req_data_wdata,
        o_req_data_wstrb => wb_req_data_wstrb,
        i_resp_data_valid => w_resp_data_valid,
        i_resp_data_addr => wb_resp_data_addr,
        i_resp_data_data => wb_resp_data_data,
        i_resp_data_store_fault_addr => wb_resp_data_store_fault_addr,
        i_resp_data_load_fault => w_resp_data_load_fault,
        i_resp_data_store_fault => w_resp_data_store_fault,
        i_resp_data_er_mpu_load => w_resp_data_er_mpu_load,
        i_resp_data_er_mpu_store => w_resp_data_er_mpu_store,
        o_resp_data_ready => w_resp_data_ready,
        i_ext_irq => i_ext_irq,
        o_mpu_region_we => w_mpu_region_we,
        o_mpu_region_idx => wb_mpu_region_idx,
        o_mpu_region_addr => wb_mpu_region_addr,
        o_mpu_region_mask => wb_mpu_region_mask,
        o_mpu_region_flags => wb_mpu_region_flags,
        i_dport_req_valid => i_dport_req_valid,
        i_dport_write => i_dport_write,
        i_dport_addr => i_dport_addr,
        i_dport_wdata => i_dport_wdata,
        o_dport_req_ready => o_dport_req_ready,
        i_dport_resp_ready => i_dport_resp_ready,
        o_dport_resp_valid => o_dport_resp_valid,
        o_dport_rdata => o_dport_rdata,
        o_halted => o_halted,
        o_flush_address => wb_flush_address,
        o_flush_valid => w_flush_valid,
        o_data_flush_address => wb_data_flush_address,
        o_data_flush_valid => w_data_flush_valid,
        i_data_flush_end => w_data_flush_end);

    cache0 :  CacheTop generic map (
        memtech => memtech,
        async_reset => async_reset,
        coherence_ena => coherence_ena
     ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_ctrl_valid => w_req_ctrl_valid,
        i_req_ctrl_addr => wb_req_ctrl_addr,
        o_req_ctrl_ready => w_req_ctrl_ready,
        o_resp_ctrl_valid => w_resp_ctrl_valid,
        o_resp_ctrl_addr => wb_resp_ctrl_addr,
        o_resp_ctrl_data => wb_resp_ctrl_data,
        o_resp_ctrl_load_fault => w_resp_ctrl_load_fault,
        o_resp_ctrl_executable => w_resp_ctrl_executable,
        i_resp_ctrl_ready => w_resp_ctrl_ready,
        i_req_data_valid => w_req_data_valid,
        i_req_data_write => w_req_data_write,
        i_req_data_addr => wb_req_data_addr,
        i_req_data_wdata => wb_req_data_wdata,
        i_req_data_wstrb => wb_req_data_wstrb,
        o_req_data_ready => w_req_data_ready,
        o_resp_data_valid => w_resp_data_valid,
        o_resp_data_addr => wb_resp_data_addr,
        o_resp_data_data => wb_resp_data_data,
        o_resp_data_store_fault_addr => wb_resp_data_store_fault_addr,
        o_resp_data_load_fault => w_resp_data_load_fault,
        o_resp_data_store_fault => w_resp_data_store_fault,
        o_resp_data_er_mpu_load => w_resp_data_er_mpu_load,
        o_resp_data_er_mpu_store => w_resp_data_er_mpu_store,
        i_resp_data_ready => w_resp_data_ready,
        i_req_mem_ready => i_req_mem_ready,
        o_req_mem_path => o_req_mem_path,
        o_req_mem_valid => o_req_mem_valid,
        o_req_mem_type => o_req_mem_type,
        o_req_mem_addr => o_req_mem_addr,
        o_req_mem_strob => o_req_mem_strob,
        o_req_mem_data => o_req_mem_data,
        i_resp_mem_valid => i_resp_mem_valid,
        i_resp_mem_path => i_resp_mem_path,
        i_resp_mem_data => i_resp_mem_data,
        i_resp_mem_load_fault => i_resp_mem_load_fault,
        i_resp_mem_store_fault => i_resp_mem_store_fault,
        i_mpu_region_we => w_mpu_region_we,
        i_mpu_region_idx => wb_mpu_region_idx,
        i_mpu_region_addr => wb_mpu_region_addr,
        i_mpu_region_mask => wb_mpu_region_mask,
        i_mpu_region_flags => wb_mpu_region_flags,
        i_req_snoop_valid => i_req_snoop_valid,
        i_req_snoop_type => i_req_snoop_type,
        o_req_snoop_ready => o_req_snoop_ready,
        i_req_snoop_addr => i_req_snoop_addr,
        i_resp_snoop_ready => i_resp_snoop_ready,
        o_resp_snoop_valid => o_resp_snoop_valid,
        o_resp_snoop_data => o_resp_snoop_data,
        o_resp_snoop_flags => o_resp_snoop_flags,
        i_flush_address => wb_flush_address,
        i_flush_valid => w_flush_valid,
        i_data_flush_address => wb_data_flush_address,
        i_data_flush_valid => w_data_flush_valid,
        o_data_flush_end => w_data_flush_end);

end;
