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
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;

--! @brief   Declaration of components visible on SoC top level.
package types_river is

constant CFG_CORES_PER_DSU_MAX : integer := 2;

type dport_in_type is record
    valid : std_logic;
    write : std_logic;
    region : std_logic_vector(1 downto 0);
    addr : std_logic_vector(11 downto 0);
    wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

constant dport_in_none : dport_in_type := (
  '0', '0', (others => '0'), (others => '0'), (others => '0'));

type dport_in_vector is array (0 to CFG_CORES_PER_DSU_MAX-1) 
       of dport_in_type;


type dport_out_type is record
    halted : std_logic;
    ready : std_logic;
    rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

constant dport_out_none : dport_out_type := (
    '0', '1', (others => '0'));

type dport_out_vector is array (0 to CFG_CORES_PER_DSU_MAX-1) 
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
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector;
    o_soft_rst : out std_logic;
    i_bus_util_w : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
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
    async_reset : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in nasti_master_in_type;
    o_msto   : out nasti_master_out_type;
    o_mstcfg : out nasti_master_config_type;
    i_dport  : in dport_in_type;
    o_dport  : out dport_out_type;
    i_ext_irq : in std_logic
  );
end component;

end; -- package body
