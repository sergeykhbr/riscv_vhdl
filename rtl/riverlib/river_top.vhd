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
    async_reset : boolean := false
  );
  port (
    i_clk : in std_logic;                                             -- CPU clock
    i_nrst : in std_logic;                                            -- Reset. Active LOW.
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                   -- AXI request was accepted
    o_req_mem_valid : out std_logic;                                  -- AXI memory request is valid
    o_req_mem_write : out std_logic;                                  -- AXI memory request is write type
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- AXI memory request address
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);-- Writing strob. 1 bit per Byte
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Writing data
    o_req_mem_len : out std_logic_vector(7 downto 0);                 -- burst length
    o_req_mem_burst : out std_logic_vector(1 downto 0);               -- burst type: "00" FIX; "01" INCR; "10" WRAP
    i_resp_mem_data_valid : in std_logic;                             -- AXI response is valid
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Read data
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    i_resp_mem_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- Interrupt line from external interrupts controller (PLIC).
    i_ext_irq : in std_logic;
    o_time : out std_logic_vector(63 downto 0);                       -- Timer. Clock counter except halt state.
    -- Debug interface:
    i_dport_valid : in std_logic;                                     -- Debug access from DSU is valid
    i_dport_write : in std_logic;                                     -- Write command flag
    i_dport_region : in std_logic_vector(1 downto 0);                 -- Registers region ID: 0=CSR; 1=IREGS; 2=Control
    i_dport_addr : in std_logic_vector(11 downto 0);                  -- Register idx
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);       -- Write value
    o_dport_ready : out std_logic;                                    -- Response is ready
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);      -- Response value
    o_halted : out std_logic
  );
end;
 
architecture arch_RiverTop of RiverTop is

  -- Control path:
  signal w_req_ctrl_ready : std_logic;
  signal w_req_ctrl_valid : std_logic;
  signal wb_req_ctrl_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal w_resp_ctrl_valid : std_logic;
  signal wb_resp_ctrl_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_resp_ctrl_data : std_logic_vector(31 downto 0);
  signal w_resp_ctrl_load_fault : std_logic;
  signal w_resp_ctrl_ready : std_logic;
  -- Data path:
  signal w_req_data_ready : std_logic;
  signal w_req_data_valid : std_logic;
  signal w_req_data_write : std_logic;
  signal wb_req_data_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_req_data_size : std_logic_vector(1 downto 0);
  signal wb_req_data_data : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal w_resp_data_valid : std_logic;
  signal wb_resp_data_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_resp_data_data : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal w_resp_data_load_fault : std_logic;
  signal w_resp_data_store_fault : std_logic;
  signal wb_resp_data_store_fault_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal w_resp_data_ready : std_logic;
  signal wb_flush_address : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal w_flush_valid : std_logic;
  signal wb_istate : std_logic_vector(1 downto 0);
  signal wb_dstate : std_logic_vector(1 downto 0);
  signal wb_cstate : std_logic_vector(1 downto 0);

begin

    proc0 : Processor generic map (
        hartid => hartid,
        async_reset => async_reset
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
        o_resp_ctrl_ready => w_resp_ctrl_ready,
        i_req_data_ready => w_req_data_ready,
        o_req_data_valid => w_req_data_valid,
        o_req_data_write => w_req_data_write,
        o_req_data_addr => wb_req_data_addr,
        o_req_data_size => wb_req_data_size,
        o_req_data_data => wb_req_data_data,
        i_resp_data_valid => w_resp_data_valid,
        i_resp_data_addr => wb_resp_data_addr,
        i_resp_data_data => wb_resp_data_data,
        i_resp_data_load_fault => w_resp_data_load_fault,
        i_resp_data_store_fault => w_resp_data_store_fault,
        i_resp_data_store_fault_addr => wb_resp_data_store_fault_addr,
        o_resp_data_ready => w_resp_data_ready,
        i_ext_irq => i_ext_irq,
        o_time => o_time,
        i_dport_valid => i_dport_valid,
        i_dport_write => i_dport_write,
        i_dport_region => i_dport_region,
        i_dport_addr => i_dport_addr,
        i_dport_wdata => i_dport_wdata,
        o_dport_ready => o_dport_ready,
        o_dport_rdata => o_dport_rdata,
        o_halted => o_halted,
        o_flush_address => wb_flush_address,
        o_flush_valid => w_flush_valid,
        i_istate => wb_istate,
        i_dstate => wb_dstate,
        i_cstate => wb_cstate);

    cache0 :  CacheTop generic map (
        memtech => memtech,
        async_reset => async_reset
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
        i_resp_ctrl_ready => w_resp_ctrl_ready,
        i_req_data_valid => w_req_data_valid,
        i_req_data_write => w_req_data_write,
        i_req_data_addr => wb_req_data_addr,
        i_req_data_size => wb_req_data_size,
        i_req_data_data => wb_req_data_data,
        o_req_data_ready => w_req_data_ready,
        o_resp_data_valid => w_resp_data_valid,
        o_resp_data_addr => wb_resp_data_addr,
        o_resp_data_data => wb_resp_data_data,
        o_resp_data_load_fault => w_resp_data_load_fault,
        o_resp_data_store_fault => w_resp_data_store_fault,
        o_resp_data_store_fault_addr => wb_resp_data_store_fault_addr,
        i_resp_data_ready => w_resp_data_ready,
        i_req_mem_ready => i_req_mem_ready,
        o_req_mem_valid => o_req_mem_valid,
        o_req_mem_write => o_req_mem_write,
        o_req_mem_addr => o_req_mem_addr,
        o_req_mem_strob => o_req_mem_strob,
        o_req_mem_data => o_req_mem_data,
        o_req_mem_len => o_req_mem_len,
        o_req_mem_burst => o_req_mem_burst,
        i_resp_mem_data_valid => i_resp_mem_data_valid,
        i_resp_mem_data => i_resp_mem_data,
        i_resp_mem_load_fault => i_resp_mem_load_fault,
        i_resp_mem_store_fault => i_resp_mem_store_fault,
        i_resp_mem_store_fault_addr => i_resp_mem_store_fault_addr,
        i_flush_address => wb_flush_address,
        i_flush_valid => w_flush_valid,
        o_istate => wb_istate,
        o_dstate => wb_dstate,
        o_cstate => wb_cstate);

end;
