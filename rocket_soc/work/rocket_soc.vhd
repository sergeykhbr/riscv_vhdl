-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Network on Chip design top level.
--! @details    RISC-V "Rocket Core" based system with the AMBA AXI4 (NASTI) 
--!             system bus and integrated peripheries.
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
--! "Virtual" PLL declaration.
use techmap.types_pll.all;
--! "Virtual" buffers declaration.
use techmap.types_buf.all;

--! Rocket-chip specific library
library rocketlib;
--! AMBA AXI4 (NASTI) interface configuration and templates
use rocketlib.types_nasti.all;
--! SOC top-level component declaration.
use rocketlib.types_rocket.all;
--! TileLink interface description.
use rocketlib.types_tile.all;

 --! Top-level implementaion library
library work;
--! Target dependable configuration: RTL, FPGA or ASIC.
use work.config_target.all;
--! Target independable configuration.
use work.config_common.all;

--! @brief   SOC Top-level entity declaration.
--! @details This module implements full SOC functionality and all IO signals
--!          are available on FPGA/ASIC IO pins.
entity rocket_soc is port 
( 
  --! Input reset. Active High. Usually assigned to button "Center".
  i_rst     : in std_logic;

  --! @name Clocks:
  --! @{

  --! Differential clock (LVDS) positive signal.
  i_sclk_p  : in std_logic;
  --! Differential clock (LVDS) negative signal.
  i_sclk_n  : in std_logic;
  --! External ADC clock (default 26 MHz).
  i_clk_adc : in std_logic;
  --! @}
 
  --! @name User's IOs:
  --! @{

  --! DIP switch.
  i_dip     : in std_logic_vector(3 downto 0);
  --! LEDs.
  o_led     : out std_logic_vector(7 downto 0);
  --! @}
 
  --! @name  UART1 signals:
  --! @{
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic
);
  --! @}

end rocket_soc;

--! @brief SOC top-level  architecture declaration.
architecture arch_rocket_soc of rocket_soc is

  signal wSysReset  : std_ulogic; -- Internal system reset. MUST NOT USED BY DEVICES.
  signal wReset     : std_ulogic; -- Global reset active HIGH
  signal wNReset    : std_ulogic; -- Global reset active LOW
  signal wClkBus    : std_ulogic; -- bus clock from the internal PLL (100MHz virtex6/40MHz Spartan6)
  signal wClkAdcSim : std_ulogic; -- 26 MHz from the internal PLL
  signal wClkGnss   : std_ulogic; -- clock that goes to GnssEngine (config dependable)
  signal wPllLocked : std_ulogic; -- PLL status signal. 0=Unlocked; 1=locked.

  signal i_starter : starter_in_type;
  signal o_starter : starter_out_type;
  
  signal uart1i : uart_in_type;
  signal uart1o : uart_out_type;

  signal htif_in_valid_delay : std_logic;
  signal htif_in_ready_delay : std_logic;
  signal htif_in_bits_delay : std_logic_vector(HTIF_WIDTH-1 downto 0);
  signal htif_out_bits_delay : std_logic_vector(HTIF_WIDTH-1 downto 0);

  signal htif_clk : std_logic;
  signal htif_out_stats_delay : std_logic;

  signal axi_acquired : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
  signal axi_busy : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);

  --! Arbiter is switching only slaves output signal, data from noc
  --! is connected to all slaves and to the arbiter itself.
  signal noc2cslv   : nasti_slave_in_type;
  signal carb2noc   : nasti_slave_out_type;
  signal cslv2carb  : nasti_slaves_out_vector;
  signal cslv_cfg   : nasti_slave_cfg_vector;

  signal cbridge_in  : bridge_in_type;
  signal cbridge_out : bridge_out_type;

  signal ubridge_in  : bridge_in_type;
  signal ubridge_out : bridge_out_type;
  
 
  signal irq_pins : std_logic_vector(CFG_IRQ_TOTAL-1 downto 0);
  signal tile2host : host_out_type;
  signal host2tile : host_in_type;
begin

  ------------------------------------
  -- @brief Internal PLL device instance.
  pll0 : SysPLL_tech generic map
  (
    tech => CFG_FABTECH
  )port map 
  (
    i_reset     => i_rst,
    i_int_clkrf => '0',
    i_clkp	=> i_sclk_p,
    i_clkn	=> i_sclk_n,
    i_clk_adc   => i_clk_adc,
    o_clk_bus   => wClkBus,
    o_clk_adc   => wClkAdcSim,
    o_locked    => wPllLocked
  );
--`ifdef FPGA
  htif_clk <= wClkBus;
--`endif

  wSysReset <= i_rst or not wPllLocked;

  ------------------------------------
  --! @brief System Reset device instance.
  rst0 : reset_global port map
  (
    inSysReset  => wSysReset,
    inSysClk    => wClkBus,
    inPllLock   => wPllLocked,
    outReset    => wReset
  );
  wNReset <= not wReset;


L1toL2ena0 : if CFG_COMMON_L1toL2_ENABLE generate 
  ------------------------------------
  -- Hardware init and MRESET for the CPUs
  htif_in_bits_delay <= o_starter.in_bits;
  htif_in_valid_delay <= o_starter.in_valid;

  i_starter.in_ready  <= htif_in_ready_delay;
  i_starter.out_bits  <= htif_out_bits_delay;
  
  start0 : Starter port map
  (
    clk   => htif_clk,
    nrst  => wNReset,
    i     => i_starter,
    o     => o_starter
  );


  ------------------------------------
  --! @brief NoC core instance.
  rocket0 : Top port map
  (
    clk                       => wClkBus,              --in
    reset                     => wReset,               --in

    io_host_in_valid          => htif_in_valid_delay,  --in
    io_host_in_ready          => htif_in_ready_delay,  --out
    io_host_in_bits           => htif_in_bits_delay,   --in[15:0]
    io_host_out_valid         => i_starter.out_valid,  --out
    io_host_out_ready         => o_starter.out_ready,  --in
    io_host_out_bits          => htif_out_bits_delay,  --out[15:0] goes to Starter and Memory DeSerializer

    io_host_clk               => htif_clk,             --out
    io_host_clk_edge          => open,                 --out
    io_host_debug_stats_csr   => htif_out_stats_delay, --out
    io_mem_backup_ctrl_en     => '0', --in
    io_mem_backup_ctrl_in_valid  => '0',--mem_bk_in_valid_delay, --in
    io_mem_backup_ctrl_out_ready => '0',--mem_bk_out_ready_delay,--in
    io_mem_backup_ctrl_out_valid => open,--mem_bk_out_valid_delay,--out

    --! mem 
    io_mem_0_aw_ready => carb2noc.aw_ready,--in
    io_mem_0_aw_valid => cbridge_out.nasti.aw_valid,--out
    io_mem_0_aw_bits_addr => cbridge_out.nasti.aw_bits.addr,--out[31:0]
    io_mem_0_aw_bits_len => cbridge_out.nasti.aw_bits.len,--out[7:0]
    io_mem_0_aw_bits_size => cbridge_out.nasti.aw_bits.size,--out[2:0]
    io_mem_0_aw_bits_burst => cbridge_out.nasti.aw_bits.burst,--out[1:0]
    io_mem_0_aw_bits_lock => cbridge_out.nasti.aw_bits.lock,--out
    io_mem_0_aw_bits_cache => cbridge_out.nasti.aw_bits.cache,--out[3:0]
    io_mem_0_aw_bits_prot => cbridge_out.nasti.aw_bits.prot,--out[2:0]
    io_mem_0_aw_bits_qos => cbridge_out.nasti.aw_bits.qos,--out[3:0]
    io_mem_0_aw_bits_region => cbridge_out.nasti.aw_bits.region,--out[3:0]
    io_mem_0_aw_bits_id  => cbridge_out.nasti.aw_id,--out[5:0]
    io_mem_0_aw_bits_user => cbridge_out.nasti.aw_user,--out
    io_mem_0_w_ready => carb2noc.w_ready,--in
    io_mem_0_w_valid  => cbridge_out.nasti.w_valid,--out
    io_mem_0_w_bits_data => cbridge_out.nasti.w_data,--out[127:0]
    io_mem_0_w_bits_last => cbridge_out.nasti.w_last,--out
    io_mem_0_w_bits_strb => cbridge_out.nasti.w_strb,--out[15:0]
    io_mem_0_w_bits_user => cbridge_out.nasti.w_user,--out
    io_mem_0_b_ready => cbridge_out.nasti.b_ready,--out
    io_mem_0_b_valid => carb2noc.b_valid,--in
    io_mem_0_b_bits_resp => carb2noc.b_resp,--in[1:0]
    io_mem_0_b_bits_id => carb2noc.b_id,--in[5:0]
    io_mem_0_b_bits_user => carb2noc.b_user,--in
    io_mem_0_ar_ready => carb2noc.ar_ready,--in
    io_mem_0_ar_valid => cbridge_out.nasti.ar_valid,--out
    io_mem_0_ar_bits_addr => cbridge_out.nasti.ar_bits.addr,--out[31:0]
    io_mem_0_ar_bits_len => cbridge_out.nasti.ar_bits.len,--out[7:0]
    io_mem_0_ar_bits_size => cbridge_out.nasti.ar_bits.size,--out[2:0]
    io_mem_0_ar_bits_burst => cbridge_out.nasti.ar_bits.burst,--out[1:0]
    io_mem_0_ar_bits_lock => cbridge_out.nasti.ar_bits.lock,--out
    io_mem_0_ar_bits_cache => cbridge_out.nasti.ar_bits.cache,--out[3:0]
    io_mem_0_ar_bits_prot => cbridge_out.nasti.ar_bits.prot,--out[2:0]
    io_mem_0_ar_bits_qos => cbridge_out.nasti.ar_bits.qos,--out[3:0]
    io_mem_0_ar_bits_region => cbridge_out.nasti.ar_bits.region,--out[3:0]
    io_mem_0_ar_bits_id => cbridge_out.nasti.ar_id,--out[5:0]
    io_mem_0_ar_bits_user => cbridge_out.nasti.ar_user,--out
    io_mem_0_r_ready => cbridge_out.nasti.r_ready,--out
    io_mem_0_r_valid => carb2noc.r_valid,--in
    io_mem_0_r_bits_resp => carb2noc.r_resp,--in[1:0]
    io_mem_0_r_bits_data => carb2noc.r_data,--in[127:0]
    io_mem_0_r_bits_last => carb2noc.r_last,--in
    io_mem_0_r_bits_id => carb2noc.r_id,--in[5:0]
    io_mem_0_r_bits_user => carb2noc.r_user,--in
    --! mmio 
    io_mmio_aw_ready => carb2noc.aw_ready,--in
    io_mmio_aw_valid => ubridge_out.nasti.aw_valid,--out
    io_mmio_aw_bits_addr => ubridge_out.nasti.aw_bits.addr,--out[31:0]
    io_mmio_aw_bits_len => ubridge_out.nasti.aw_bits.len,--out[7:0]
    io_mmio_aw_bits_size => ubridge_out.nasti.aw_bits.size,--out[2:0]
    io_mmio_aw_bits_burst => ubridge_out.nasti.aw_bits.burst,--out[1:0]
    io_mmio_aw_bits_lock => ubridge_out.nasti.aw_bits.lock,--out
    io_mmio_aw_bits_cache => ubridge_out.nasti.aw_bits.cache,--out[3:0]
    io_mmio_aw_bits_prot => ubridge_out.nasti.aw_bits.prot,--out[2:0]
    io_mmio_aw_bits_qos => ubridge_out.nasti.aw_bits.qos,--out[3:0]
    io_mmio_aw_bits_region => ubridge_out.nasti.aw_bits.region,--out[3:0]
    io_mmio_aw_bits_id  => ubridge_out.nasti.aw_id,--out[5:0]
    io_mmio_aw_bits_user => ubridge_out.nasti.aw_user,--out
    io_mmio_w_ready => carb2noc.w_ready,--in
    io_mmio_w_valid  => ubridge_out.nasti.w_valid,--out
    io_mmio_w_bits_data => ubridge_out.nasti.w_data,--out[127:0]
    io_mmio_w_bits_last => ubridge_out.nasti.w_last,--out
    io_mmio_w_bits_strb => ubridge_out.nasti.w_strb,--out[15:0]
    io_mmio_w_bits_user => ubridge_out.nasti.w_user,--out
    io_mmio_b_ready => ubridge_out.nasti.b_ready,--out
    io_mmio_b_valid => carb2noc.b_valid,--in
    io_mmio_b_bits_resp => carb2noc.b_resp,--in[1:0]
    io_mmio_b_bits_id => carb2noc.b_id,--in[5:0]
    io_mmio_b_bits_user => carb2noc.b_user,--in
    io_mmio_ar_ready => carb2noc.ar_ready,--in
    io_mmio_ar_valid => ubridge_out.nasti.ar_valid,--out
    io_mmio_ar_bits_addr => ubridge_out.nasti.ar_bits.addr,--out[31:0]
    io_mmio_ar_bits_len => ubridge_out.nasti.ar_bits.len,--out[7:0]
    io_mmio_ar_bits_size => ubridge_out.nasti.ar_bits.size,--out[2:0]
    io_mmio_ar_bits_burst => ubridge_out.nasti.ar_bits.burst,--out[1:0]
    io_mmio_ar_bits_lock => ubridge_out.nasti.ar_bits.lock,--out
    io_mmio_ar_bits_cache => ubridge_out.nasti.ar_bits.cache,--out[3:0]
    io_mmio_ar_bits_prot => ubridge_out.nasti.ar_bits.prot,--out[2:0]
    io_mmio_ar_bits_qos => ubridge_out.nasti.ar_bits.qos,--out[3:0]
    io_mmio_ar_bits_region => ubridge_out.nasti.ar_bits.region,--out[3:0]
    io_mmio_ar_bits_id => ubridge_out.nasti.ar_id,--out[5:0]
    io_mmio_ar_bits_user => ubridge_out.nasti.ar_user,--out
    io_mmio_r_ready => ubridge_out.nasti.r_ready,--out
    io_mmio_r_valid => carb2noc.r_valid,--in
    io_mmio_r_bits_resp => carb2noc.r_resp,--in[1:0]
    io_mmio_r_bits_data => carb2noc.r_data,--in[127:0]
    io_mmio_r_bits_last => carb2noc.r_last,--in
    io_mmio_r_bits_id => carb2noc.r_id,--in[5:0]
    io_mmio_r_bits_user => carb2noc.r_user--in
  );
end generate;
  
L1toL2dis0 : if not CFG_COMMON_L1toL2_ENABLE generate 

  tile0 : RocketTile port map
  (
    clk                       => wClkBus,              --in
    reset                     => wReset,               --in
    io_cached_0_acquire_ready => cbridge_out.tile.acquire_ready,
    io_cached_0_acquire_valid => cbridge_in.tile.acquire_valid,
    io_cached_0_acquire_bits_addr_block => cbridge_in.tile.acquire_bits_addr_block,
    io_cached_0_acquire_bits_client_xact_id => cbridge_in.tile.acquire_bits_client_xact_id,
    io_cached_0_acquire_bits_addr_beat  => cbridge_in.tile.acquire_bits_addr_beat,
    io_cached_0_acquire_bits_is_builtin_type  => cbridge_in.tile.acquire_bits_is_builtin_type,
    io_cached_0_acquire_bits_a_type => cbridge_in.tile.acquire_bits_a_type,
    io_cached_0_acquire_bits_union => cbridge_in.tile.acquire_bits_union,
    io_cached_0_acquire_bits_data => cbridge_in.tile.acquire_bits_data,
    io_cached_0_grant_ready => cbridge_in.tile.grant_ready,
    io_cached_0_grant_valid => cbridge_out.tile.grant_valid,
    io_cached_0_grant_bits_addr_beat => cbridge_out.tile.grant_bits_addr_beat,
    io_cached_0_grant_bits_client_xact_id => cbridge_out.tile.grant_bits_client_xact_id,
    io_cached_0_grant_bits_manager_xact_id => cbridge_out.tile.grant_bits_manager_xact_id,
    io_cached_0_grant_bits_is_builtin_type => cbridge_out.tile.grant_bits_is_builtin_type,
    io_cached_0_grant_bits_g_type => cbridge_out.tile.grant_bits_g_type,
    io_cached_0_grant_bits_data => cbridge_out.tile.grant_bits_data,
    io_cached_0_probe_ready => cbridge_in.tile.probe_ready,
    io_cached_0_probe_valid => cbridge_out.tile.probe_valid,
    io_cached_0_probe_bits_addr_block => cbridge_out.tile.probe_bits_addr_block,
    io_cached_0_probe_bits_p_type => cbridge_out.tile.probe_bits_p_type,
    io_cached_0_release_ready => cbridge_out.tile.release_ready,
    io_cached_0_release_valid => cbridge_in.tile.release_valid,
    io_cached_0_release_bits_addr_beat => cbridge_in.tile.release_bits_addr_beat,
    io_cached_0_release_bits_addr_block => cbridge_in.tile.release_bits_addr_block,
    io_cached_0_release_bits_client_xact_id => cbridge_in.tile.release_bits_client_xact_id,
    io_cached_0_release_bits_r_type => cbridge_in.tile.release_bits_r_type,
    io_cached_0_release_bits_voluntary => cbridge_in.tile.release_bits_voluntary,
    io_cached_0_release_bits_data => cbridge_in.tile.release_bits_data,
    io_uncached_0_acquire_ready => ubridge_out.tile.acquire_ready,
    io_uncached_0_acquire_valid => ubridge_in.tile.acquire_valid,
    io_uncached_0_acquire_bits_addr_block => ubridge_in.tile.acquire_bits_addr_block,
    io_uncached_0_acquire_bits_client_xact_id => ubridge_in.tile.acquire_bits_client_xact_id,
    io_uncached_0_acquire_bits_addr_beat => ubridge_in.tile.acquire_bits_addr_beat,
    io_uncached_0_acquire_bits_is_builtin_type => ubridge_in.tile.acquire_bits_is_builtin_type,
    io_uncached_0_acquire_bits_a_type => ubridge_in.tile.acquire_bits_a_type,
    io_uncached_0_acquire_bits_union => ubridge_in.tile.acquire_bits_union,
    io_uncached_0_acquire_bits_data => ubridge_in.tile.acquire_bits_data,
    io_uncached_0_grant_ready => ubridge_in.tile.grant_ready,
    io_uncached_0_grant_valid => ubridge_out.tile.grant_valid,
    io_uncached_0_grant_bits_addr_beat => ubridge_out.tile.grant_bits_addr_beat,
    io_uncached_0_grant_bits_client_xact_id => ubridge_out.tile.grant_bits_client_xact_id,
    io_uncached_0_grant_bits_manager_xact_id => ubridge_out.tile.grant_bits_manager_xact_id,
    io_uncached_0_grant_bits_is_builtin_type => ubridge_out.tile.grant_bits_is_builtin_type,
    io_uncached_0_grant_bits_g_type => ubridge_out.tile.grant_bits_g_type,
    io_uncached_0_grant_bits_data => ubridge_out.tile.grant_bits_data,
    io_host_reset => host2tile.reset,
    io_host_id => host2tile.id,
    io_host_csr_req_ready => tile2host.csr_req_ready,
    io_host_csr_req_valid => host2tile.csr_req_valid,
    io_host_csr_req_bits_rw => host2tile.csr_req_bits_rw,
    io_host_csr_req_bits_addr => host2tile.csr_req_bits_addr,
    io_host_csr_req_bits_data => host2tile.csr_req_bits_data,
    io_host_csr_resp_ready => host2tile.csr_resp_ready,
    io_host_csr_resp_valid => tile2host.csr_resp_valid,
    io_host_csr_resp_bits => tile2host.csr_resp_bits,
    io_host_debug_stats_csr => tile2host.debug_stats_csr
);

  cbridge_in.nasti <= carb2noc;
  ubridge_in.nasti <= carb2noc;

  axi_busy(CFG_NASTI_MASTER_CACHED) <= axi_acquired(CFG_NASTI_MASTER_UNCACHED);
  
  cbridge0 : AxiBridge 
  port map (
    clk => wClkBus,
    nrst => wNReset,
    i_busy => axi_busy(CFG_NASTI_MASTER_CACHED),
    o_acquired => axi_acquired(CFG_NASTI_MASTER_CACHED),
    i => cbridge_in,
    o => cbridge_out
  );

  --! We provide priority to acquire AXI bus to the cached Link
  axi_busy(CFG_NASTI_MASTER_UNCACHED) <= 
    cbridge_in.tile.acquire_valid or axi_acquired(CFG_NASTI_MASTER_CACHED);
  
  ubridge0 : AxiBridge 
  port map (
    clk => wClkBus,
    nrst => wNReset,
    i_busy => axi_busy(CFG_NASTI_MASTER_UNCACHED),
    o_acquired => axi_acquired(CFG_NASTI_MASTER_UNCACHED),
    i => ubridge_in,
    o => ubridge_out
  );

end generate;

  ------------------------------------
  -- @brief request multiplexer from cached/uncached TileLink into AXI4 bus
  bridgemux0 : TileBridgeArbiter
  port map (
    i_cached => cbridge_out.nasti,
    i_uncached => ubridge_out.nasti,  
    o => noc2cslv
  );

  ------------------------------------
  -- @brief Cached memory access arbiter:
  carb0 : NastiArbiter port map (
    clk  => wClkBus,
    i    => cslv2carb,
    o    => carb2noc
  );

  ------------------------------------
  --! @brief BOOT ROM module isntance with the AXI4 interface.
  --! @details Map address:
  --!          0x00000000..0x00001fff (8 KB total)
  boot0 : nasti_bootrom generic map (
    memtech  => CFG_MEMTECH,
    xindex   => CFG_NASTI_SLAVE_BOOTROM,
    xaddr    => 0,
    xmask    => 16#ffffe#,
    sim_hexfile => CFG_SIM_BOOTROM_HEX
  ) port map (
    clk  => wClkBus,
    nrst => wNReset,
    cfg  => cslv_cfg(CFG_NASTI_SLAVE_BOOTROM),
    i    => noc2cslv,
    o    => cslv2carb(CFG_NASTI_SLAVE_BOOTROM)
  );

  ------------------------------------
  --! @brief Firmware Image ROM with the AXI4 interface.
  --! @details Map address:
  --!          0x00100000..0x0013ffff (256 KB total)
  img0 : nasti_romimage generic map (
    memtech  => CFG_MEMTECH,
    xindex   => CFG_NASTI_SLAVE_ROMIMAGE,
    xaddr    => 16#00100#,
    xmask    => 16#fffc0#,
    sim_hexfile => CFG_SIM_FWIMAGE_HEX
  ) port map (
    clk  => wClkBus,
    nrst => wNReset,
    cfg  => cslv_cfg(CFG_NASTI_SLAVE_ROMIMAGE),
    i    => noc2cslv,
    o    => cslv2carb(CFG_NASTI_SLAVE_ROMIMAGE)
  );

  ------------------------------------
  --! Internal SRAM module instance with the AXI4 interface.
  --! @details Map address:
  --!          0x10000000..0x1007ffff (512 KB total)
  sram0 : nasti_sram generic map (
    memtech  => CFG_MEMTECH,
    xindex   => CFG_NASTI_SLAVE_SRAM,
    xaddr    => 16#10000#,
    xmask    => 16#fff80#,            -- 512 KB mask
    abits    => (10 + log2(512)),     -- 512 KB address
    init_file => CFG_SIM_FWIMAGE_HEX  -- Used only for inferred
  ) port map (
    clk  => wClkBus,
    nrst => wNReset,
    cfg  => cslv_cfg(CFG_NASTI_SLAVE_SRAM),
    i    => noc2cslv,
    o    => cslv2carb(CFG_NASTI_SLAVE_SRAM)
  );


  ------------------------------------
  --! @brief Controller of the LEDs, DIPs and GPIO with the AXI4 interface.
  --! @details Map address:
  --!          0x80000000..0x80000fff (4 KB total)
  gpio0 : nasti_gpio generic map (
    xindex   => CFG_NASTI_SLAVE_GPIO,
    xaddr    => 16#80000#,
    xmask    => 16#fffff#
  ) port map (
    clk   => wClkBus,
    nrst  => wNReset,
    cfg   => cslv_cfg(CFG_NASTI_SLAVE_GPIO),
    i     => noc2cslv,
    o     => cslv2carb(CFG_NASTI_SLAVE_GPIO),
    i_dip => i_dip,
    o_led => o_led
  );
  
  
  ------------------------------------
  uart1i.cts   <= not i_uart1_ctsn;
  uart1i.rd    <= i_uart1_rd;

  --! @brief UART Controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80001000..0x80001fff (4 KB total)
  uart1 : nasti_uart generic map
  (
    xindex   => CFG_NASTI_SLAVE_UART1,
    xaddr    => 16#80001#,
    xmask    => 16#FFFFF#,
    fifosz   => 16,
    parity_bit => 0
  ) port map (
    nrst   => wNReset, 
    clk    => wClkbus, 
    cfg    => cslv_cfg(CFG_NASTI_SLAVE_UART1),
    i_uart => uart1i, 
    o_uart => uart1o,
    i_axi  => noc2cslv,
    o_axi  => cslv2carb(CFG_NASTI_SLAVE_UART1)
  );
  o_uart1_td  <= uart1o.td;
  o_uart1_rtsn <= not uart1o.rts;


  ------------------------------------
  --! @brief Interrupt controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80002000..0x80002fff (4 KB total)
  irq0 : nasti_irqctrl generic map
  (
    xindex   => CFG_NASTI_SLAVE_IRQCTRL,
    xaddr    => 16#80002#,
    xmask    => 16#FFFFF#,
    L2_ena   => CFG_COMMON_L1toL2_ENABLE
  ) port map (
    clk    => wClkBus,
    nrst   => wNReset,
    i_irqs => irq_pins,
    o_cfg  => cslv_cfg(CFG_NASTI_SLAVE_IRQCTRL),
    i_axi  => noc2cslv,
    o_axi  => cslv2carb(CFG_NASTI_SLAVE_IRQCTRL),
    i_host => tile2host,
    o_host => host2tile
  );

  ------------------------------------
  --! @brief GNSS Engine stub with the AXI4 interface.
  --! @details Map address:
  --!          0x80003000..0x80003fff (4 KB total)
geneng_dis : if not CFG_GNSSLIB_ENABLE generate 
  gnss0 : nasti_gnssstub  generic map
  (
    xindex  => CFG_NASTI_SLAVE_ENGINE,
    xaddr   => 16#80003#,
    xmask   => 16#FFFFF#
  ) port map (
    clk    => wClkBus,
    nrst   => wNReset,
    cfg    => cslv_cfg(CFG_NASTI_SLAVE_ENGINE),
    i      => noc2cslv,
    o      => cslv2carb(CFG_NASTI_SLAVE_ENGINE),
    irq    => irq_pins(CFG_IRQ_GNSSENGINE)
  );
  
  -- Stub for the RF-controller
  cslv_cfg(CFG_NASTI_SLAVE_RFCTRL) <= nasti_slave_config_none;
  
end generate;


  --! @brief Plug'n'Play controller of the current configuration with the
  --!        AXI4 interface.
  --! @details Map address:
  --!          0xfffff000..0xffffffff (4 KB total)
  pnp0 : nasti_pnp generic map (
    xindex  => CFG_NASTI_SLAVE_PNP,
    xaddr   => 16#fffff#,
    xmask   => 16#fffff#,
    tech    => CFG_MEMTECH
  ) port map (
    clk    => wClkbus, 
    nrst   => wNReset,
    cfgvec => cslv_cfg,
    cfg    => cslv_cfg(CFG_NASTI_SLAVE_PNP),
    i      => noc2cslv,
    o      => cslv2carb(CFG_NASTI_SLAVE_PNP)
  );


end arch_rocket_soc;
