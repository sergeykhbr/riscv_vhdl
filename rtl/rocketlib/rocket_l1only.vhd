-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      RockeTile top level.
--! @details    RISC-V "RocketTile" without Uncore subsystem.
------------------------------------------------------------------------------
--! Standard library
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

--! Data transformation and math functions library
library commonlib;
use commonlib.types_common.all;

--! Technology definition library.
library techmap;
--! Technology constants definition.
use techmap.gencomp.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! Rocket-chip specific library
library rocketlib;
--! TileLink interface description.
use rocketlib.types_rocket.all;
library work;
use work.all;

--! @brief   RocketTile entity declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
entity rocket_l1only is 
generic (
    hartid : integer := 0;
    reset_vector : integer := 16#1000#
);
port ( 
    nrst     : in std_logic;
    clk_sys  : in std_logic;
    msti1    : in nasti_master_in_type;
    msto1    : out nasti_master_out_type;
    mstcfg1  : out nasti_master_config_type;
    msti2    : in nasti_master_in_type;
    msto2    : out nasti_master_out_type;
    mstcfg2  : out nasti_master_config_type;
    interrupts : in std_logic_vector(CFG_CORE_IRQ_TOTAL-1 downto 0)
);
  --! @}

end;

--! @brief SOC top-level  architecture declaration.
architecture arch_rocket_l1only of rocket_l1only is

  constant CFG_HARTID : std_logic_vector(63 downto 0) := conv_std_logic_vector(hartid, 64);
  constant CFG_RESET_VECTOR : std_logic_vector(63 downto 0) := conv_std_logic_vector(reset_vector, 64);

  constant xmstconfig1 : nasti_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_CACHED_TILELINK
  );

  constant xmstconfig2 : nasti_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_UNCACHED_TILELINK
  );
  
  signal cpu_rst : std_logic;
  
  signal cto : tile_out_type;
  signal cti : tile_in_type;

  signal uto : tile_out_type;
  signal uti : tile_in_type;
  

  component AxiBridge is port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_in_type
  );
  end component;

  component Tile2Axi is port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_in_type
  );
  end component;

  COMPONENT RocketTile
  PORT(
    clock : IN std_logic;
    reset : IN std_logic;
    io_cached_0_a_ready : IN std_logic;
    io_cached_0_a_valid : OUT std_logic;
    io_cached_0_a_bits_opcode : OUT std_logic_vector(2 downto 0);
    io_cached_0_a_bits_param : OUT std_logic_vector(2 downto 0);
    io_cached_0_a_bits_size : OUT std_logic_vector(3 downto 0);
    io_cached_0_a_bits_source : OUT std_logic_vector(1 downto 0);
    io_cached_0_a_bits_address : OUT std_logic_vector(31 downto 0);
    io_cached_0_a_bits_mask : OUT std_logic_vector(7 downto 0);
    io_cached_0_a_bits_data : OUT std_logic_vector(63 downto 0);
    io_cached_0_b_ready : OUT std_logic;
    io_cached_0_b_valid : IN std_logic;
    io_cached_0_b_bits_opcode : IN std_logic_vector(2 downto 0);
    io_cached_0_b_bits_param : IN std_logic_vector(1 downto 0);
    io_cached_0_b_bits_size : IN std_logic_vector(3 downto 0);
    io_cached_0_b_bits_source : IN std_logic_vector(1 downto 0);
    io_cached_0_b_bits_address : IN std_logic_vector(31 downto 0);
    io_cached_0_b_bits_mask : IN std_logic_vector(7 downto 0);
    io_cached_0_b_bits_data : IN std_logic_vector(63 downto 0);
    io_cached_0_c_ready : IN std_logic;
    io_cached_0_c_valid : OUT std_logic;
    io_cached_0_c_bits_opcode : OUT std_logic_vector(2 downto 0);
    io_cached_0_c_bits_param : OUT std_logic_vector(2 downto 0);
    io_cached_0_c_bits_size : OUT std_logic_vector(3 downto 0);
    io_cached_0_c_bits_source : OUT std_logic_vector(1 downto 0);
    io_cached_0_c_bits_address : OUT std_logic_vector(31 downto 0);
    io_cached_0_c_bits_data : OUT std_logic_vector(63 downto 0);
    io_cached_0_c_bits_error : OUT std_logic;
    io_cached_0_d_ready : OUT std_logic;
    io_cached_0_d_valid : IN std_logic;
    io_cached_0_d_bits_opcode : IN std_logic_vector(2 downto 0);
    io_cached_0_d_bits_param : IN std_logic_vector(1 downto 0);
    io_cached_0_d_bits_size : IN std_logic_vector(3 downto 0);
    io_cached_0_d_bits_source : IN std_logic_vector(1 downto 0);
    io_cached_0_d_bits_sink : IN std_logic_vector(3 downto 0);
    io_cached_0_d_bits_addr_lo : IN std_logic_vector(2 downto 0);
    io_cached_0_d_bits_data : IN std_logic_vector(63 downto 0);
    io_cached_0_d_bits_error : IN std_logic;
    io_cached_0_e_ready : IN std_logic;
    io_cached_0_e_valid : OUT std_logic;
    io_cached_0_e_bits_sink : OUT std_logic_vector(3 downto 0);
    io_uncached_0_a_ready : IN std_logic;
    io_uncached_0_a_valid : OUT std_logic;
    io_uncached_0_a_bits_opcode : OUT std_logic_vector(2 downto 0);
    io_uncached_0_a_bits_param : OUT std_logic_vector(2 downto 0);
    io_uncached_0_a_bits_size : OUT std_logic_vector(3 downto 0);
    io_uncached_0_a_bits_source : OUT std_logic_vector(2 downto 0);
    io_uncached_0_a_bits_address : OUT std_logic_vector(31 downto 0);
    io_uncached_0_a_bits_mask : OUT std_logic_vector(7 downto 0);
    io_uncached_0_a_bits_data : OUT std_logic_vector(63 downto 0);
    io_uncached_0_b_ready : OUT std_logic;
    io_uncached_0_b_valid : IN std_logic;
    io_uncached_0_b_bits_opcode : IN std_logic_vector(2 downto 0);
    io_uncached_0_b_bits_param : IN std_logic_vector(1 downto 0);
    io_uncached_0_b_bits_size : IN std_logic_vector(3 downto 0);
    io_uncached_0_b_bits_source : IN std_logic_vector(2 downto 0);
    io_uncached_0_b_bits_address : IN std_logic_vector(31 downto 0);
    io_uncached_0_b_bits_mask : IN std_logic_vector(7 downto 0);
    io_uncached_0_b_bits_data : IN std_logic_vector(63 downto 0);
    io_uncached_0_c_ready : IN std_logic;
    io_uncached_0_c_valid : OUT std_logic;
    io_uncached_0_c_bits_opcode : OUT std_logic_vector(2 downto 0);
    io_uncached_0_c_bits_param : OUT std_logic_vector(2 downto 0);
    io_uncached_0_c_bits_size : OUT std_logic_vector(3 downto 0);
    io_uncached_0_c_bits_source : OUT std_logic_vector(2 downto 0);
    io_uncached_0_c_bits_address : OUT std_logic_vector(31 downto 0);
    io_uncached_0_c_bits_data : OUT std_logic_vector(63 downto 0);
    io_uncached_0_c_bits_error : OUT std_logic;
    io_uncached_0_d_ready : OUT std_logic;
    io_uncached_0_d_valid : IN std_logic;
    io_uncached_0_d_bits_opcode : IN std_logic_vector(2 downto 0);
    io_uncached_0_d_bits_param : IN std_logic_vector(1 downto 0);
    io_uncached_0_d_bits_size : IN std_logic_vector(3 downto 0);
    io_uncached_0_d_bits_source : IN std_logic_vector(2 downto 0);
    io_uncached_0_d_bits_sink : IN std_logic_vector(3 downto 0);
    io_uncached_0_d_bits_addr_lo : IN std_logic_vector(2 downto 0);
    io_uncached_0_d_bits_data : IN std_logic_vector(63 downto 0);
    io_uncached_0_d_bits_error : IN std_logic;
    io_uncached_0_e_ready : IN std_logic;
    io_uncached_0_e_valid : OUT std_logic;
    io_uncached_0_e_bits_sink : OUT std_logic_vector(3 downto 0);
    io_hartid : IN std_logic_vector(63 downto 0);
    io_interrupts_debug : IN std_logic;
    io_interrupts_mtip : IN std_logic;
    io_interrupts_msip : IN std_logic;
    io_interrupts_meip : IN std_logic;
    io_interrupts_seip : IN std_logic;
    io_resetVector : IN std_logic_vector(63 downto 0)
  );
  END COMPONENT;

begin

  mstcfg1 <= xmstconfig1;
  mstcfg2 <= xmstconfig2;
  cpu_rst <= not nrst;
   
  cto.a_source(2) <= '0';
  cti.b_source(2) <= '0';
  cto.c_source(2) <= '0';
  cti.d_source(2) <= '0';
  
  inst_tile: RocketTile PORT MAP(
      clock => clk_sys,
      reset => cpu_rst,

      io_cached_0_a_ready => cti.a_ready,
      io_cached_0_a_valid => cto.a_valid,
      io_cached_0_a_bits_opcode => cto.a_opcode,
      io_cached_0_a_bits_param => cto.a_param,
      io_cached_0_a_bits_size => cto.a_size,
      io_cached_0_a_bits_source => cto.a_source(1 downto 0),
      io_cached_0_a_bits_address => cto.a_address,
      io_cached_0_a_bits_mask => cto.a_mask,
      io_cached_0_a_bits_data => cto.a_data,
      io_cached_0_b_ready => cto.b_ready,
      io_cached_0_b_valid => cti.b_valid,
      io_cached_0_b_bits_opcode => cti.b_opcode,
      io_cached_0_b_bits_param => cti.b_param,
      io_cached_0_b_bits_size => cti.b_size,
      io_cached_0_b_bits_source => cti.b_source(1 downto 0),
      io_cached_0_b_bits_address => cti.b_address,
      io_cached_0_b_bits_mask => cti.b_mask,
      io_cached_0_b_bits_data => cti.b_data,
      io_cached_0_c_ready => cti.c_ready,
      io_cached_0_c_valid => cto.c_valid,
      io_cached_0_c_bits_opcode => cto.c_opcode,
      io_cached_0_c_bits_param => cto.c_param,
      io_cached_0_c_bits_size => cto.c_size,
      io_cached_0_c_bits_source => cto.c_source(1 downto 0),
      io_cached_0_c_bits_address => cto.c_address,
      io_cached_0_c_bits_data => cto.c_data,
      io_cached_0_c_bits_error => cto.c_error,
      io_cached_0_d_ready => cto.d_ready,
      io_cached_0_d_valid => cti.d_valid,
      io_cached_0_d_bits_opcode => cti.d_opcode,
      io_cached_0_d_bits_param => cti.d_param,
      io_cached_0_d_bits_size => cti.d_size,
      io_cached_0_d_bits_source => cti.d_source(1 downto 0),
      io_cached_0_d_bits_sink => cti.d_sink,
      io_cached_0_d_bits_addr_lo => cti.d_addr_lo,
      io_cached_0_d_bits_data => cti.d_data,
      io_cached_0_d_bits_error => cti.d_error,
      io_cached_0_e_ready => cti.e_ready,
      io_cached_0_e_valid => cto.e_valid,
      io_cached_0_e_bits_sink => cto.e_sink,

      io_uncached_0_a_ready => uti.a_ready,
      io_uncached_0_a_valid => uto.a_valid,
      io_uncached_0_a_bits_opcode => uto.a_opcode,
      io_uncached_0_a_bits_param => uto.a_param,
      io_uncached_0_a_bits_size => uto.a_size,
      io_uncached_0_a_bits_source => uto.a_source,
      io_uncached_0_a_bits_address => uto.a_address,
      io_uncached_0_a_bits_mask => uto.a_mask,
      io_uncached_0_a_bits_data => uto.a_data,
      io_uncached_0_b_ready => uto.b_ready,
      io_uncached_0_b_valid => uti.b_valid,
      io_uncached_0_b_bits_opcode => uti.b_opcode,
      io_uncached_0_b_bits_param => uti.b_param,
      io_uncached_0_b_bits_size => uti.b_size,
      io_uncached_0_b_bits_source => uti.b_source,
      io_uncached_0_b_bits_address => uti.b_address,
      io_uncached_0_b_bits_mask => uti.b_mask,
      io_uncached_0_b_bits_data => uti.b_data,
      io_uncached_0_c_ready => uti.c_ready,
      io_uncached_0_c_valid => uto.c_valid,
      io_uncached_0_c_bits_opcode => uto.c_opcode,
      io_uncached_0_c_bits_param => uto.c_param,
      io_uncached_0_c_bits_size => uto.c_size,
      io_uncached_0_c_bits_source => uto.c_source,
      io_uncached_0_c_bits_address => uto.c_address,
      io_uncached_0_c_bits_data => uto.c_data,
      io_uncached_0_c_bits_error => uto.c_error,
      io_uncached_0_d_ready => uto.d_ready,
      io_uncached_0_d_valid => uti.d_valid,
      io_uncached_0_d_bits_opcode => uti.d_opcode,
      io_uncached_0_d_bits_param => uti.d_param,
      io_uncached_0_d_bits_size => uti.d_size,
      io_uncached_0_d_bits_source => uti.d_source,
      io_uncached_0_d_bits_sink => uti.d_sink,
      io_uncached_0_d_bits_addr_lo => uti.d_addr_lo,
      io_uncached_0_d_bits_data => uti.d_data,
      io_uncached_0_d_bits_error => uti.d_error,
      io_uncached_0_e_ready => uti.e_ready,
      io_uncached_0_e_valid => uto.e_valid,
      io_uncached_0_e_bits_sink => uto.e_sink,

      io_hartid => CFG_HARTID,
      io_interrupts_debug  => interrupts(CFG_CORE_IRQ_DEBUG),
      io_interrupts_mtip  => interrupts(CFG_CORE_IRQ_MTIP),
      io_interrupts_msip  => interrupts(CFG_CORE_IRQ_MSIP),
      io_interrupts_meip  => interrupts(CFG_CORE_IRQ_MEIP),
      io_interrupts_seip  => interrupts(CFG_CORE_IRQ_SEIP),
      io_resetVector => CFG_RESET_VECTOR
  );
 
  cbridge0 : Tile2Axi  port map (
    clk => clk_sys,
    nrst => nrst,
    --! Tile-to-AXI direction
    tloi => cto,
    msto => msto1,
    --! AXI-to-Tile direction
    msti => msti1,
    tlio => cti
  );

  ubridge0 : Tile2Axi port map (
    clk => clk_sys,
    nrst => nrst,
    --! Tile-to-AXI direction
    tloi => uto,
    msto => msto2,
    --! AXI-to-Tile direction
    msti => msti2,
    tlio => uti
  );

end arch_rocket_l1only;
