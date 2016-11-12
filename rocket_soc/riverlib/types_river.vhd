-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU configuration parameters
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

--! @brief   Declaration of components visible on SoC top level.
package types_river is

  --! Architecture size difinition.
  constant RISCV_ARCH : integer := 64;

  --! @name System bus parameters
  --! @brief Constants specify AXI bus global settigns
  --! @{

  --! @brief   Address bus bit-size.
  constant BUS_ADDR_WIDTH : integer := 32;
  --! @brief   Data bus bit-size.
  constant BUS_DATA_WIDTH : integer := 64;
  --! @brief   Num of data bytes per transaction.
  constant BUS_DATA_BYTES : integer := BUS_DATA_WIDTH / 8;
  --! @}

  --! @name   Encoded Memory operation size values
  --! @{

  constant MEMOP_8B : std_logic_vector(1 downto 0) := "11";
  constant MEMOP_4B : std_logic_vector(1 downto 0) := "10";
  constant MEMOP_2B : std_logic_vector(1 downto 0) := "01";
  constant MEMOP_1B : std_logic_vector(1 downto 0) := "00";
  --! @}


--! @brief   RIVER CPU component declaration.
--! @details This module implements Risc-V CPU Core named as
--!          "RIVER" with AXI interface.
--! @param[in] xindex AXI master index
--! @param[in] i_rstn     Reset signal with active LOW level.
--! @param[in] i_clk      System clock (BUS/CPU clock).
--! @param[in] i_msti     Bus-to-Master device signals.
--! @param[out] o_msto    CachedTile-to-Bus request signals.
--! @param[in] i_ext_irq  Interrupts line supported by Rocket chip.
component river_axi is 
generic (
    xindex : integer := 0
);
port ( 
    i_rstn   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in nasti_master_in_type;
    o_msto   : out nasti_master_out_type;
    o_mstcfg : out nasti_master_config_type;
    i_ext_irq : in std_logic
);
end component;

end; -- package body
