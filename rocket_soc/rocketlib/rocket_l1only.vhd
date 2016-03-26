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
    --! Cached Tile AXI master index
    xindex1 : integer := 0;
    --! Uncached Tile AXI master index
    xindex2 : integer := 1
);
port ( 
    rst      : in std_logic;
    soft_rst : in std_logic;
    clk_sys  : in std_logic;
    slvo     : in nasti_slave_in_type;
    msti     : in nasti_master_in_type;
    msto1    : out nasti_master_out_type;
    mstcfg1  : out nasti_master_config_type;
    msto2    : out nasti_master_out_type;
    mstcfg2  : out nasti_master_config_type;
    htifoi   : in host_out_type;
    htifio   : out host_in_type
);
  --! @}

end;

--! @brief SOC top-level  architecture declaration.
architecture arch_rocket_l1only of rocket_l1only is

  constant xmstconfig1 : nasti_master_config_type := (
     xindex => xindex1,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_CACHED_TILELINK,
     descrtype => PNP_CFG_TYPE_MASTER,
     descrsize => PNP_CFG_MASTER_DESCR_BYTES
  );

  constant xmstconfig2 : nasti_master_config_type := (
     xindex => xindex2,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_UNCACHED_TILELINK,
     descrtype => PNP_CFG_TYPE_MASTER,
     descrsize => PNP_CFG_MASTER_DESCR_BYTES
  );
  
  signal nrst : std_logic;
  signal cpu_rst : std_logic;
  
  signal cto : tile_cached_out_type;
  signal cti : tile_cached_in_type;

  signal uto : tile_cached_out_type;
  signal uti : tile_cached_in_type;
  

  component AxiBridge is generic (
     xindex : integer := 0
  );
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_cached_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_cached_in_type
  );
  end component;

	COMPONENT RocketTile
	PORT(
		clk : IN std_logic;
		reset : IN std_logic;
		io_cached_0_acquire_ready : IN std_logic;
		io_cached_0_grant_valid : IN std_logic;
		io_cached_0_grant_bits_addr_beat : IN std_logic_vector(1 downto 0);
		io_cached_0_grant_bits_client_xact_id : IN std_logic_vector(1 downto 0);
		io_cached_0_grant_bits_manager_xact_id : IN std_logic_vector(3 downto 0);
		io_cached_0_grant_bits_is_builtin_type : IN std_logic;
		io_cached_0_grant_bits_g_type : IN std_logic_vector(3 downto 0);
		io_cached_0_grant_bits_data : IN std_logic_vector(127 downto 0);
		io_cached_0_probe_valid : IN std_logic;
		io_cached_0_probe_bits_addr_block : IN std_logic_vector(25 downto 0);
		io_cached_0_probe_bits_p_type : IN std_logic_vector(1 downto 0);
		io_cached_0_release_ready : IN std_logic;
		io_uncached_0_acquire_ready : IN std_logic;
		io_uncached_0_grant_valid : IN std_logic;
		io_uncached_0_grant_bits_addr_beat : IN std_logic_vector(1 downto 0);
		io_uncached_0_grant_bits_client_xact_id : IN std_logic_vector(1 downto 0);
		io_uncached_0_grant_bits_manager_xact_id : IN std_logic_vector(3 downto 0);
		io_uncached_0_grant_bits_is_builtin_type : IN std_logic;
		io_uncached_0_grant_bits_g_type : IN std_logic_vector(3 downto 0);
		io_uncached_0_grant_bits_data : IN std_logic_vector(127 downto 0);
		io_host_reset : IN std_logic;
		io_host_id : IN std_logic;
		io_host_csr_req_valid : IN std_logic;
		io_host_csr_req_bits_rw : IN std_logic;
		io_host_csr_req_bits_addr : IN std_logic_vector(11 downto 0);
		io_host_csr_req_bits_data : IN std_logic_vector(63 downto 0);
		io_host_csr_resp_ready : IN std_logic;          
		io_cached_0_acquire_valid : OUT std_logic;
		io_cached_0_acquire_bits_addr_block : OUT std_logic_vector(25 downto 0);
		io_cached_0_acquire_bits_client_xact_id : OUT std_logic_vector(1 downto 0);
		io_cached_0_acquire_bits_addr_beat : OUT std_logic_vector(1 downto 0);
		io_cached_0_acquire_bits_is_builtin_type : OUT std_logic;
		io_cached_0_acquire_bits_a_type : OUT std_logic_vector(2 downto 0);
		io_cached_0_acquire_bits_union : OUT std_logic_vector(16 downto 0);
		io_cached_0_acquire_bits_data : OUT std_logic_vector(127 downto 0);
		io_cached_0_grant_ready : OUT std_logic;
		io_cached_0_probe_ready : OUT std_logic;
		io_cached_0_release_valid : OUT std_logic;
		io_cached_0_release_bits_addr_beat : OUT std_logic_vector(1 downto 0);
		io_cached_0_release_bits_addr_block : OUT std_logic_vector(25 downto 0);
		io_cached_0_release_bits_client_xact_id : OUT std_logic_vector(1 downto 0);
		io_cached_0_release_bits_voluntary : OUT std_logic;
		io_cached_0_release_bits_r_type : OUT std_logic_vector(2 downto 0);
		io_cached_0_release_bits_data : OUT std_logic_vector(127 downto 0);
		io_uncached_0_acquire_valid : OUT std_logic;
		io_uncached_0_acquire_bits_addr_block : OUT std_logic_vector(25 downto 0);
		io_uncached_0_acquire_bits_client_xact_id : OUT std_logic_vector(1 downto 0);
		io_uncached_0_acquire_bits_addr_beat : OUT std_logic_vector(1 downto 0);
		io_uncached_0_acquire_bits_is_builtin_type : OUT std_logic;
		io_uncached_0_acquire_bits_a_type : OUT std_logic_vector(2 downto 0);
		io_uncached_0_acquire_bits_union : OUT std_logic_vector(16 downto 0);
		io_uncached_0_acquire_bits_data : OUT std_logic_vector(127 downto 0);
		io_uncached_0_grant_ready : OUT std_logic;
		io_host_csr_req_ready : OUT std_logic;
		io_host_csr_resp_valid : OUT std_logic;
		io_host_csr_resp_bits : OUT std_logic_vector(63 downto 0);
		io_host_debug_stats_csr : OUT std_logic
		);
	END COMPONENT;

begin

  mstcfg1 <= xmstconfig1;
  mstcfg2 <= xmstconfig2;
  nrst <= not rst;
  cpu_rst <= rst or soft_rst;
   
	inst_tile: RocketTile PORT MAP(
		clk => clk_sys,
		reset => cpu_rst,
		io_cached_0_acquire_ready => cti.acquire_ready,
		io_cached_0_acquire_valid => cto.acquire_valid,
		io_cached_0_acquire_bits_addr_block => cto.acquire_bits_addr_block,
		io_cached_0_acquire_bits_client_xact_id => cto.acquire_bits_client_xact_id,
		io_cached_0_acquire_bits_addr_beat => cto.acquire_bits_addr_beat,
		io_cached_0_acquire_bits_is_builtin_type => cto.acquire_bits_is_builtin_type,
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
