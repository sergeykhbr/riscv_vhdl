-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU library external interfaces
-----------------------------------------------------------------------------

--! @page riscv_core_page RISC-V Processor
--!
--! @section core_overview Overview
--!
--! Current repository supports two synthesizable processors: \c Rocket and
--! \c River. Both of them implement open RISC-V ISA. To select what processor
--! to use there's special generic parameter:
--!
--!      CFG_COMMON_RIVER_CPU_ENABLE
--!
--! @section core_rocket Rocket CPU
--!
--! Rocket is the 64-bits single issue, in-order processor developed in Berkley 
--! and shared as the sources writen on SCALA language. It uses specally developed
--! library \c Chisel to generate Verilog implementation from SCALA sources.
--! 
--! Rocket Core usually implements all features of the latest ISA specification,
--! either as multi-core support with L2-cache implementation and many other.
--! But it has a set of disadvantages: bad integration with other devices not
--! writen on SCALA, not very-good integration with RTL simulators, no reference
--! model. It shows worse performance than RIVER CPU (for now).
--!
--! @section core_river River CPU
--!
--! River is my implementation of RISC-V ISA writen on VHDL either as all
--! others parts of shared SoC implementation.
--! There's also availabel precise SystemC model integrated into Simulator
--! which is used as a stimulus during RTL simulation and garantee consistency 
--! of functional and SystemC models either as RTL.
--!
--! River CPU is the 5-stage processor with the classical pipeline structure:
--!
--! <img src="pics/river_top.png" alt="Top Level"> 
--! @latexonly {\includegraphics{pics/river_top.png}} @endlatexonly


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

type dport_in_type is record
    valid : std_logic;
    write : std_logic;
    region : std_logic_vector(1 downto 0);
    addr : std_logic_vector(11 downto 0);
    wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

constant dport_in_none : dport_in_type := (
  '0', '0', (others => '0'), (others => '0'), (others => '0'));

type dport_out_type is record
    ready : std_logic;
    rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

  --! @brief   Declaration of the Debug Support Unit with the AXI interface.
  --! @details This module provides access to processors CSRs via HostIO bus.
  --! @param[in] clk           System clock (BUS/CPU clock).
  --! @param[in] rstn          Reset signal with active LOW level.
  --! @param[in] i_axi         Slave slot input signals.
  --! @param[out] o_axi        Slave slot output signals.
  --! @param[out] o_dporti     Debug port output signals connected to River CPU.
  --! @param[in] i_dporto      River CPU debug port response signals.
  --! @param[out] o_soft_rstn  Software reset CPU and interrupt controller. Active HIGH
  --! @param[in] i_miss_irq    Miss access counter update signal
  --! @param[in] i_miss_addr   Miss accessed memory address
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
    o_dporti : out dport_in_type;
    i_dporto : in dport_out_type;
    o_soft_rst : out std_logic;
    i_miss_irq  : in std_logic;
    i_miss_addr : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
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
