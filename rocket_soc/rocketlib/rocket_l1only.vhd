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

--! Rocket-chip specific library
library rocketlib;
--! AMBA AXI4 (NASTI) interface configuration and templates
use rocketlib.types_nasti.all;
--! TileLink interface description.
use rocketlib.types_tile.all;
--! SOC top-level component declaration.
use rocketlib.types_rocket.all;


--! @brief   RocketTile entity declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
entity rocket_l1only is 
generic (
    --! Cached Tile AXI master index
    xindex1 : integer := 0;
    --! Uncached Tile AXI master index
    xindex2 : integer := 1
);
port ( 
    rst      : in std_logic;
    clk_sys  : in std_logic;
    slvo     : in nasti_slave_in_type;
    msti     : in nasti_master_in_type;
    msto1    : out nasti_master_out_type;
    msto2    : out nasti_master_out_type;
    htifoi   : in host_out_type;
    htifio   : out host_in_type
);
  --! @}

end rocket_l1only;

--! @brief SOC top-level  architecture declaration.
architecture arch_rocket_l1only of rocket_l1only is
  signal nrst : std_logic;
  
  signal cto : tile_cached_out_type;
  signal cti : tile_cached_in_type;

  signal uto : tile_cached_out_type;
  signal uti : tile_cached_in_type;
  

  component rocket_tile is
  port (
    clk : in std_logic;
    reset : in std_logic;
    io_cached_0_acquire_ready : in std_logic;
    io_cached_0_acquire_valid : out std_logic;
    io_cached_0_acquire_bits_addr_block : out std_logic_vector(25 downto 0);
    io_cached_0_acquire_bits_client_xact_id : out std_logic_vector(1 downto 0);
    io_cached_0_acquire_bits_addr_beat : out std_logic_vector(1 downto 0);
    io_cached_0_acquire_bits_is_builtin_type : out std_logic;
    io_cached_0_acquire_bits_a_type : out std_logic_vector(2 downto 0);
    io_cached_0_acquire_bits_union : out std_logic_vector(16 downto 0);
    io_cached_0_acquire_bits_data : out std_logic_vector(127 downto 0);
    io_cached_0_grant_ready : out std_logic;
    io_cached_0_grant_valid : in std_logic;
    io_cached_0_grant_bits_addr_beat : in std_logic_vector(1 downto 0);
    io_cached_0_grant_bits_client_xact_id : in std_logic_vector(1 downto 0);
    io_cached_0_grant_bits_manager_xact_id : in std_logic_vector(3 downto 0);
    io_cached_0_grant_bits_is_builtin_type : in std_logic;
    io_cached_0_grant_bits_g_type : in std_logic_vector(3 downto 0);
    io_cached_0_grant_bits_data : in std_logic_vector(127 downto 0);
    io_cached_0_probe_ready : out std_logic;
    io_cached_0_probe_valid : in std_logic;
    io_cached_0_probe_bits_addr_block : in std_logic_vector(25 downto 0);
    io_cached_0_probe_bits_p_type : in std_logic_vector(1 downto 0);
    io_cached_0_release_ready : in std_logic;
    io_cached_0_release_valid : out std_logic;
    io_cached_0_release_bits_addr_beat : out std_logic_vector(1 downto 0);
    io_cached_0_release_bits_addr_block : out std_logic_vector(25 downto 0);
    io_cached_0_release_bits_client_xact_id : out std_logic_vector(1 downto 0);
    io_cached_0_release_bits_voluntary : out std_logic;
    io_cached_0_release_bits_r_type : out std_logic_vector(2 downto 0);
    io_cached_0_release_bits_data : out std_logic_vector(127 downto 0);
    io_uncached_0_acquire_ready : in std_logic;
    io_uncached_0_acquire_valid : out std_logic;
    io_uncached_0_acquire_bits_addr_block : out std_logic_vector(25 downto 0);
    io_uncached_0_acquire_bits_client_xact_id : out std_logic_vector(1 downto 0);
    io_uncached_0_acquire_bits_addr_beat : out std_logic_vector(1 downto 0);
    io_uncached_0_acquire_bits_is_builtin_type : out std_logic;
    io_uncached_0_acquire_bits_a_type : out std_logic_vector(2 downto 0);
    io_uncached_0_acquire_bits_union : out std_logic_vector(16 downto 0);
    io_uncached_0_acquire_bits_data : out std_logic_vector(127 downto 0);
    io_uncached_0_grant_ready : out std_logic;
    io_uncached_0_grant_valid : in std_logic;
    io_uncached_0_grant_bits_addr_beat : in std_logic_vector(1 downto 0);
    io_uncached_0_grant_bits_client_xact_id : in std_logic_vector(1 downto 0);
    io_uncached_0_grant_bits_manager_xact_id : in std_logic_vector(3 downto 0);
    io_uncached_0_grant_bits_is_builtin_type : in std_logic;
    io_uncached_0_grant_bits_g_type : in std_logic_vector(3 downto 0);
    io_uncached_0_grant_bits_data : in std_logic_vector(127 downto 0);
    io_host_reset : in std_logic;
    io_host_id : in std_logic;
    io_host_csr_req_ready : out std_logic;
    io_host_csr_req_valid : in std_logic;
    io_host_csr_req_bits_rw : in std_logic;
    io_host_csr_req_bits_addr : in std_logic_vector(11 downto 0);
    io_host_csr_req_bits_data : in std_logic_vector(63 downto 0);
    io_host_csr_resp_ready : in std_logic;
    io_host_csr_resp_valid : out std_logic;
    io_host_csr_resp_bits : out std_logic_vector(63 downto 0);
    io_host_debug_stats_csr : out std_logic
  );
  end component;

begin

  nrst <= not rst;

  tile0 : rocket_tile port map
  (
    clk                       => clk_sys,              --in
    reset                     => rst,               --in
    io_cached_0_acquire_ready => cti.acquire_ready,
    io_cached_0_acquire_valid => cto.acquire_valid,
    io_cached_0_acquire_bits_addr_block => cto.acquire_bits_addr_block,
    io_cached_0_acquire_bits_client_xact_id => cto.acquire_bits_client_xact_id,
    io_cached_0_acquire_bits_addr_beat  => cto.acquire_bits_addr_beat,
    io_cached_0_acquire_bits_is_builtin_type  => cto.acquire_bits_is_builtin_type,
    io_cached_0_acquire_bits_a_type => cto.acquire_bits_a_type,
    io_cached_0_acquire_bits_union => cto.acquire_bits_union,
    io_cached_0_acquire_bits_data => cto.acquire_bits_data,
    io_cached_0_grant_ready => cto.grant_ready,
    io_cached_0_grant_valid => cti.grant_valid,
    io_cached_0_grant_bits_addr_beat => cti.grant_bits_addr_beat,
    io_cached_0_grant_bits_client_xact_id => cti.grant_bits_client_xact_id,
    io_cached_0_grant_bits_manager_xact_id => cti.grant_bits_manager_xact_id,
    io_cached_0_grant_bits_is_builtin_type => cti.grant_bits_is_builtin_type,
    io_cached_0_grant_bits_g_type => cti.grant_bits_g_type,
    io_cached_0_grant_bits_data => cti.grant_bits_data,
    io_cached_0_probe_ready => cto.probe_ready,
    io_cached_0_probe_valid => cti.probe_valid,
    io_cached_0_probe_bits_addr_block => cti.probe_bits_addr_block,
    io_cached_0_probe_bits_p_type => cti.probe_bits_p_type,
    io_cached_0_release_ready => cti.release_ready,
    io_cached_0_release_valid => cto.release_valid,
    io_cached_0_release_bits_addr_beat => cto.release_bits_addr_beat,
    io_cached_0_release_bits_addr_block => cto.release_bits_addr_block,
    io_cached_0_release_bits_client_xact_id => cto.release_bits_client_xact_id,
    io_cached_0_release_bits_voluntary => cto.release_bits_voluntary,
    io_cached_0_release_bits_r_type => cto.release_bits_r_type,
    io_cached_0_release_bits_data => cto.release_bits_data,
    io_uncached_0_acquire_ready => uti.acquire_ready,
    io_uncached_0_acquire_valid => uto.acquire_valid,
    io_uncached_0_acquire_bits_addr_block => uto.acquire_bits_addr_block,
    io_uncached_0_acquire_bits_client_xact_id => uto.acquire_bits_client_xact_id,
    io_uncached_0_acquire_bits_addr_beat => uto.acquire_bits_addr_beat,
    io_uncached_0_acquire_bits_is_builtin_type => uto.acquire_bits_is_builtin_type,
    io_uncached_0_acquire_bits_a_type => uto.acquire_bits_a_type,
    io_uncached_0_acquire_bits_union => uto.acquire_bits_union,
    io_uncached_0_acquire_bits_data => uto.acquire_bits_data,
    io_uncached_0_grant_ready => uto.grant_ready,
    io_uncached_0_grant_valid => uti.grant_valid,
    io_uncached_0_grant_bits_addr_beat => uti.grant_bits_addr_beat,
    io_uncached_0_grant_bits_client_xact_id => uti.grant_bits_client_xact_id,
    io_uncached_0_grant_bits_manager_xact_id => uti.grant_bits_manager_xact_id,
    io_uncached_0_grant_bits_is_builtin_type => uti.grant_bits_is_builtin_type,
    io_uncached_0_grant_bits_g_type => uti.grant_bits_g_type,
    io_uncached_0_grant_bits_data => uti.grant_bits_data,
    io_host_reset => htifoi.reset,
    io_host_id => htifoi.id,
    io_host_csr_req_ready => htifio.csr_req_ready,
    io_host_csr_req_valid => htifoi.csr_req_valid,
    io_host_csr_req_bits_rw => htifoi.csr_req_bits_rw,
    io_host_csr_req_bits_addr => htifoi.csr_req_bits_addr,
    io_host_csr_req_bits_data => htifoi.csr_req_bits_data,
    io_host_csr_resp_ready => htifoi.csr_resp_ready,
    io_host_csr_resp_valid => htifio.csr_resp_valid,
    io_host_csr_resp_bits => htifio.csr_resp_bits,
    io_host_debug_stats_csr => htifio.debug_stats_csr
  );

 
  cbridge0 : AxiBridge 
  generic map (
    xindex => xindex1
  ) port map (
    clk => clk_sys,
    nrst => nrst,
    --! Tile-to-AXI direction
    tloi => cto,
    msto => msto1,
    --! AXI-to-Tile direction
    msti => msti,
    tlio => cti
  );

  ubridge0 : AxiBridge 
  generic map (
    xindex => xindex2
  ) port map (
    clk => clk_sys,
    nrst => nrst,
    --! Tile-to-AXI direction
    tloi => uto,
    msto => msto2,
    --! AXI-to-Tile direction
    msti => msti,
    tlio => uti
  );

end arch_rocket_l1only;
