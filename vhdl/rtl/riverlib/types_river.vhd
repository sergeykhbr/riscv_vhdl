--!
--! Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
use ambalib.types_bus0.all; -- TODO: REMOVE ME when update dsu

--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;

--! @brief   Declaration of components visible on SoC top level.
package types_river is

  -- Number of CPU per one workgroup:
  constant CFG_LOG2_CPU_MAX : integer := 2;  -- 1=Dual-core (maximum); 2=Quad-core (maximum)
  constant CFG_TOTAL_CPU_MAX : integer := 2**CFG_LOG2_CPU_MAX;
  -- +1 Coherent SBA debug port (not available in River)
  -- +1 ACP coherent port  (not available in River)
  constant CFG_SLOT_L1_TOTAL : integer := CFG_TOTAL_CPU_MAX + 0;

-- AXI4 with ACE channels
type axi4_l1_out_type is record
  aw_valid : std_logic;
  aw_bits : axi4_metadata_type;
  aw_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  aw_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  w_valid : std_logic;
  w_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  w_last : std_logic;
  w_strb : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
  w_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  b_ready : std_logic;
  ar_valid : std_logic;
  ar_bits : axi4_metadata_type;
  ar_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  ar_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  r_ready : std_logic;
  -- ACE signals
  ar_domain : std_logic_vector(1 downto 0);                -- 00=Non-shareable (single master in domain)
  ar_snoop : std_logic_vector(3 downto 0);                 -- Table C3-7:
  ar_bar : std_logic_vector(1 downto 0);                   -- read barrier transaction
  aw_domain : std_logic_vector(1 downto 0);
  aw_snoop : std_logic_vector(3 downto 0);                 -- Table C3-8
  aw_bar : std_logic_vector(1 downto 0);                   -- write barrier transaction
  ac_ready : std_logic;
  cr_valid : std_logic;
  cr_resp : std_logic_vector(4 downto 0);
  cd_valid : std_logic;
  cd_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  cd_last : std_logic;
  rack : std_logic;
  wack : std_logic;
end record;

constant axi4_l1_out_none : axi4_l1_out_type := (
      '0', META_NONE, (others=>'0'), (others => '0'),
      '0', (others=>'0'), '0', (others=>'0'), (others => '0'), 
      '0', '0', META_NONE, (others=>'0'), (others => '0'), '0',
       "00", X"0", "00", "00", X"0", "00", '0', '0',
       "00000", '0', (others => '0'), '0', '0', '0');

type axi4_l1_in_type is record
  aw_ready : std_logic;
  w_ready : std_logic;
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  b_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  ar_ready : std_logic;
  r_valid : std_logic;
  r_resp : std_logic_vector(3 downto 0);
  r_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  r_last : std_logic;
  r_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  r_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  -- ACE signals
  ac_valid : std_logic;
  ac_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  ac_snoop : std_logic_vector(3 downto 0);                  -- Table C3-19
  ac_prot : std_logic_vector(2 downto 0);
  cr_ready : std_logic;
  cd_ready : std_logic;
end record;

constant axi4_l1_in_none : axi4_l1_in_type := (
      '0', '0', '0', AXI_RESP_OKAY, (others=>'0'), (others => '0'),
      '0', '0', (others => '0'), (others=>'0'), '0', (others=>'0'), (others => '0'),
      '0', (others => '0'), X"0", "000", '0', '0');

type axi4_l1_in_vector is array (0 to CFG_SLOT_L1_TOTAL-1) of axi4_l1_in_type;
type axi4_l1_out_vector is array (0 to CFG_SLOT_L1_TOTAL-1) of axi4_l1_out_type;

-- L2 AXI structure
type axi4_l2_out_type is record
  aw_valid : std_logic;
  aw_bits : axi4_metadata_type;
  aw_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  aw_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  w_valid : std_logic;
  w_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  w_last : std_logic;
  w_strb : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
  w_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  b_ready : std_logic;
  ar_valid : std_logic;
  ar_bits : axi4_metadata_type;
  ar_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  ar_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  r_ready : std_logic;
end record;

constant axi4_l2_out_none : axi4_l2_out_type := (
      '0', META_NONE, (others=>'0'), (others => '0'),
      '0', (others=>'0'), '0', (others=>'0'), (others => '0'), 
      '0', '0', META_NONE, (others=>'0'), (others => '0'), '0'
);

type axi4_l2_in_type is record
  aw_ready : std_logic;
  w_ready : std_logic;
  b_valid : std_logic;
  b_resp : std_logic_vector(1 downto 0);
  b_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  b_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
  ar_ready : std_logic;
  r_valid : std_logic;
  r_resp : std_logic_vector(1 downto 0);
  r_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  r_last : std_logic;
  r_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
  r_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
end record;

constant axi4_l2_in_none : axi4_l2_in_type := (
      '0', '0', '0', AXI_RESP_OKAY, (others=>'0'), (others => '0'),
      '0', '0', (others => '0'), (others=>'0'), '0', (others=>'0'), (others => '0')
);

-- River Debug port interface
type dport_in_type is record
    req_valid : std_logic;
    resp_ready : std_logic;
    write : std_logic;
    addr : std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0);
    wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

constant dport_in_none : dport_in_type := (
  '0', '1', '0', (others => '0'), (others => '0'));

type dport_in_vector is array (0 to CFG_TOTAL_CPU_MAX-1) 
       of dport_in_type;


type dport_out_type is record
    halted : std_logic;
    available : std_logic;
    req_ready : std_logic;
    resp_valid : std_logic;
    rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

constant dport_out_none : dport_out_type := (
    '0', '0', '1', '1', (others => '0'));

type dport_out_vector is array (0 to CFG_TOTAL_CPU_MAX-1) 
     of dport_out_type;

  --! @brief   Declaration of the Debug Support Unit with the AXI interface.
  --! @details This module provides access to processors CSRs via HostIO bus.
  --! @param[in] clk           System clock (BUS/CPU clock).
  --! @param[in] rstn          Reset signal with active LOW level.
  --! @param[in] i_axi         Slave slot input signals.
  --! @param[out] o_axi        Slave slot output signals.
  --! @param[out] o_dporti     Debug port output signals connected to River CPU.
  --! @param[in] i_dporto      River CPU debug port response signals.
  --! @param[out] o_soft_rstn  Software reset CPU and interrupt controller. Active HIGH
  --! @param[in] i_bus_util_w  Write bus access utilization per master statistic
  --! @param[in] i_bus_util_r  Write bus access utilization per master statistic
  component axi_dsu is
  generic (
    async_reset : boolean := false;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out axi4_slave_config_type;
    i_axi  : in axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector;
    i_dmi_hartsel : in std_logic_vector(CFG_LOG2_CPU_MAX-1 downto 0);
    o_dmi_req_valid : out std_logic;
    i_dmi_req_ready : in std_logic;
    o_dmi_write : out std_logic;
    o_dmi_addr : out std_logic_vector(6 downto 0);
    o_dmi_wdata : out std_logic_vector(31 downto 0);
    i_dmi_resp_valid : in std_logic;
    o_dmi_resp_ready : out std_logic;
    i_dmi_rdata : in std_logic_vector(31 downto 0);
    i_bus_util_w : in std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0)
  );
  end component;

  component dmi_regs is
  generic (
    async_reset : boolean := false;
    cpu_available : integer := 1
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    -- port[0] connected to JTAG TAP has access to AXI master interface (SBA registers)
    i_dmi_jtag_req_valid : in std_logic;
    o_dmi_jtag_req_ready : out std_logic;
    i_dmi_jtag_write : in std_logic;
    i_dmi_jtag_addr : in std_logic_vector(6 downto 0);
    i_dmi_jtag_wdata : in std_logic_vector(31 downto 0);
    o_dmi_jtag_resp_valid : out std_logic;
    i_dmi_jtag_resp_ready : in std_logic;
    o_dmi_jtag_rdata : out std_logic_vector(31 downto 0);
    -- port[1] connected to DSU doesn't have access to AXI master interface
    i_dmi_dsu_req_valid : in std_logic;
    o_dmi_dsu_req_ready : out std_logic;
    i_dmi_dsu_write : in std_logic;
    i_dmi_dsu_addr : in std_logic_vector(6 downto 0);
    i_dmi_dsu_wdata : in std_logic_vector(31 downto 0);
    o_dmi_dsu_resp_valid : out std_logic;
    i_dmi_dsu_resp_ready : in std_logic;
    o_dmi_dsu_rdata : out std_logic_vector(31 downto 0);
    -- Common signals
    o_hartsel : out std_logic_vector(CFG_LOG2_CPU_MAX-1 downto 0);
    o_dmstat : out std_logic_vector(1 downto 0);
    o_ndmreset : out std_logic;        -- non-debug module reset
    o_cfg  : out axi4_master_config_type;
    i_xmsti  : in axi4_master_in_type;
    o_xmsto  : out axi4_master_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector
  );
  end component;

  --! Dport interconnect to switch DSU and DMI access
  component ic_dport_2s_1m is
  generic (
    async_reset : boolean := false
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    -- Group <=> DMI interface
    i_sdport0i : in dport_in_vector;
    o_sdport0o : out dport_out_vector;
    -- Group <=> DSU interface
    i_sdport1i : in dport_in_vector;
    o_sdport1o : out dport_out_vector;
    -- Group connection
    o_mdporti : out dport_in_vector;
    i_mdporto : in dport_out_vector
  );
  end component;

--! @brief   RIVER CPU component declaration.
--! @details This module implements Risc-V CPU Core named as
--!          "RIVER" with AXI interface.
--! @param[in] xindex AXI master index
--! @param[in] i_rstn     Reset signal with active LOW level.
--! @param[in] i_clk      System clock (BUS/CPU clock).
--! @param[in] i_msti     Bus-to-Master device signals.
--! @param[out] o_msto    CachedTile-to-Bus request signals.
--! @param[in] i_ext_irq  Interrupts line supported by Rocket chip.
  component river_amba is 
  generic (
    memtech : integer;
    hartid : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    coherence_ena : boolean;
    tracer_ena : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in axi4_l1_in_type;
    o_msto   : out axi4_l1_out_type;
    i_dport  : in dport_in_type;
    o_dport  : out dport_out_type;
    i_ext_irq : in std_logic
  );
  end component;

  -- Processor stub should be instantiated for unused CPU slot
  component river_dummycpu is 
  port ( 
    o_msto   : out axi4_l1_out_type;
    o_dport  : out dport_out_type;
    o_flush_l2 : out std_logic
  );
  end component;

  -- L2 cache dummy implementation. Real L2 implemented in Wasserfall SoC.
  component RiverL2Dummy is
  generic (
    async_reset : boolean := false
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- CPUs Workgroup
    i_l1o : in axi4_l1_out_vector;
    o_l1i : out axi4_l1_in_vector;
    -- System bus
    i_l2i : in axi4_l2_in_type;
    o_l2o : out axi4_l2_out_type;
    i_flush_valid : std_logic
  );
  end component;

  -- Convert L2 cache lines into system bus transactions
  component river_l2serdes is 
  generic (
    async_reset : boolean
  );
  port ( 
    i_nrst  : in std_logic;
    i_clk   : in std_logic;
    i_l2o   : in axi4_l2_out_type;
    o_l2i   : out axi4_l2_in_type;
    i_msti  : in axi4_master_in_type;
    o_msto  : out axi4_master_out_type
  );
  end component;


  -- River CPU group with L2-cache (stub or real)
  component river_workgroup is 
  generic (
    cpunum : integer;
    memtech : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    coherence_ena : boolean;
    tracer_ena : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in axi4_master_in_type;
    o_msto   : out axi4_master_out_type;
    o_mstcfg : out axi4_master_config_type;
    i_dport  : in dport_in_vector;
    o_dport  : out dport_out_vector;
    i_ext_irq : in std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0)
  );
  end component;

end; -- package body
