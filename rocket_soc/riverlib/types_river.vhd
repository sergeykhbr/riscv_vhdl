-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU library external interfaces
-----------------------------------------------------------------------------

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

type dport_out_type is record
    ready : std_logic;
    rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
end record;

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
    xindex : integer := 0
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
