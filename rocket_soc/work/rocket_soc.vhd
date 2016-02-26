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

--! GNSS Sensor Ltd proprietary library
library gnsslib;
use gnsslib.types_gnss.all;

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
  o_uart1_rtsn : out std_logic;
  --! @}
  
  --! @name ADC channel A inputs (1575.4 GHz):
  --! @{
  i_gps_I  : in std_logic_vector(1 downto 0);
  i_gps_Q  : in std_logic_vector(1 downto 0);
  --! @}

  --! @name ADC channel B inputs (1602 GHz):
  --! @{
  i_glo_I  : in std_logic_vector(1 downto 0);
  i_glo_Q  : in std_logic_vector(1 downto 0);
  --! @}
  
  --! @name MAX2769 SPIs and antenna controls signals:
  --! @{
  i_gps_ld    : in std_logic;
  i_glo_ld    : in std_logic;
  o_max_sclk  : out std_logic;
  o_max_sdata : out std_logic;
  o_max_ncs   : out std_logic_vector(1 downto 0);
  i_antext_stat   : in std_logic;
  i_antext_detect : in std_logic;
  o_antext_ena    : out std_logic;
  o_antint_contr  : out std_logic
);
  --! @}

end rocket_soc;

--! @brief SOC top-level  architecture declaration.
architecture arch_rocket_soc of rocket_soc is

  --! @name Buffered in/out signals.
  --! @details All signals that are connected with in/out pads must be passed
  --!          through the dedicated buffere modules. For FPGA they are implemented
  --!          as an empty devices but ASIC couldn't be made without buffering.
  --! @{
  signal ib_rst     : std_logic;
  signal ib_sclk_p  : std_logic;
  signal ib_sclk_n  : std_logic;
  signal ib_clk_adc : std_logic;
  --! @}

  signal wSysReset  : std_ulogic; -- Internal system reset. MUST NOT USED BY DEVICES.
  signal wReset     : std_ulogic; -- Global reset active HIGH
  signal wNReset    : std_ulogic; -- Global reset active LOW
  signal wClkBus    : std_ulogic; -- bus clock from the internal PLL (100MHz virtex6/40MHz Spartan6)
  signal wClkAdc    : std_ulogic; -- 26 MHz from the internal PLL
  signal wClkGnss   : std_ulogic; -- clock that goes to GnssEngine (config dependable)
  signal wPllLocked : std_ulogic; -- PLL status signal. 0=Unlocked; 1=locked.

  
  signal uart1i : uart_in_type;
  signal uart1o : uart_out_type;

  --! Arbiter is switching only slaves output signal, data from noc
  --! is connected to all slaves and to the arbiter itself.
  signal aximi   : nasti_master_in_type;
  signal aximo   : nasti_master_out_vector;
  signal axisi   : nasti_slave_in_type;
  signal axiso   : nasti_slaves_out_vector;
  signal axi_cfg : nasti_slave_cfg_vector;
  
  --! From modules-to-tile requests
  signal htifo : host_out_vector;
  --! Selected request with the highest priority.
  signal htifo_mux : host_out_type;
  
  --! tile-to-module response.
  signal htifi : host_in_type;
  --! response with the 'grant' signal marking the exact recipient.
  signal htifi_grant : host_in_type;
  
  signal gnss_i : gns_in_type;
  signal gnss_o : gns_out_type;
  
  signal fse_i : fse_in_type;
  signal fse_o : fse_out_type;
 
  signal irq_pins : std_logic_vector(CFG_IRQ_TOTAL-1 downto 0);
begin

  --! PAD buffers:
  irst0   : ibuf_tech generic map(CFG_PADTECH) port map (ib_rst, i_rst);
  iclkp0  : ibuf_tech generic map(CFG_PADTECH) port map (ib_sclk_p, i_sclk_p);
  iclkn0  : ibuf_tech generic map(CFG_PADTECH) port map (ib_sclk_n, i_sclk_n);
  iclk1  : ibuf_tech generic map(CFG_PADTECH) port map (ib_clk_adc, i_clk_adc);
  --! @todo all other in/out signals via buffers:

  htifo(CFG_HTIF_SRC_DSU) <= host_out_none;

  ------------------------------------
  -- @brief Internal PLL device instance.
  pll0 : SysPLL_tech generic map
  (
    tech => CFG_FABTECH
  )port map 
  (
    i_reset     => ib_rst,
    i_int_clkrf => '0',
    i_clkp	     => ib_sclk_p,
    i_clkn	     => ib_sclk_n,
    i_clk_adc   => ib_clk_adc,
    o_clk_bus   => wClkBus,
    o_clk_adc   => wClkAdc,
    o_locked    => wPllLocked
  );
  wSysReset <= ib_rst or not wPllLocked;

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

  --! @brief AXI4 controller.
  ctrl0 : axictrl port map (
    clk    => wClkBus,
    nrst   => wNReset,
    slvoi  => axiso,
    mstoi  => aximo,
    slvio  => axisi,
    mstio  => aximi
  );
  
  --! @brief HostIO controller.
  htif0 : htifctrl port map (
    clk    => wClkBus,
    nrst   => wNReset,
    srcsi  => htifo,
    srcso  => htifo_mux,
    htifii => htifi,
    htifio => htifi_grant
);

L1toL2ena0 : if CFG_COMMON_L1toL2_ENABLE generate 
  --! @brief RISC-V Processor core + Uncore.
  cpu0 : rocket_l2cache generic map (
    xindex1 => CFG_NASTI_MASTER_CACHED,
    xindex2 => CFG_NASTI_MASTER_UNCACHED
  ) port map ( 
    rst      => wReset,
    clk_sys  => wClkBus,
    slvo     => axisi,
    msti     => aximi,
    msto1    => aximo(CFG_NASTI_MASTER_CACHED),
    msto2    => aximo(CFG_NASTI_MASTER_UNCACHED),
    htifoi   => htifo_mux,
    htifio   => htifi
  );
end generate;
  
L1toL2dis0 : if not CFG_COMMON_L1toL2_ENABLE generate 
  --! @brief RISC-V Processor core.
  cpu0 : rocket_l1only generic map (
    xindex1 => CFG_NASTI_MASTER_CACHED,
    xindex2 => CFG_NASTI_MASTER_UNCACHED
  ) port map ( 
    rst      => wReset,
    clk_sys  => wClkBus,
    slvo     => axisi,
    msti     => aximi,
    msto1    => aximo(CFG_NASTI_MASTER_CACHED),
    msto2    => aximo(CFG_NASTI_MASTER_UNCACHED),
    htifoi   => htifo_mux,
    htifio   => htifi
  );
end generate;

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
    cfg  => axi_cfg(CFG_NASTI_SLAVE_BOOTROM),
    i    => axisi,
    o    => axiso(CFG_NASTI_SLAVE_BOOTROM)
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
    cfg  => axi_cfg(CFG_NASTI_SLAVE_ROMIMAGE),
    i    => axisi,
    o    => axiso(CFG_NASTI_SLAVE_ROMIMAGE)
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
    cfg  => axi_cfg(CFG_NASTI_SLAVE_SRAM),
    i    => axisi,
    o    => axiso(CFG_NASTI_SLAVE_SRAM)
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
    cfg   => axi_cfg(CFG_NASTI_SLAVE_GPIO),
    i     => axisi,
    o     => axiso(CFG_NASTI_SLAVE_GPIO),
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
    cfg    => axi_cfg(CFG_NASTI_SLAVE_UART1),
    i_uart => uart1i, 
    o_uart => uart1o,
    i_axi  => axisi,
    o_axi  => axiso(CFG_NASTI_SLAVE_UART1)
  );
  o_uart1_td  <= uart1o.td;
  o_uart1_rtsn <= not uart1o.rts;


  ------------------------------------
  --! @brief Interrupt controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80002000..0x80002fff (4 KB total)
  irq0 : nasti_irqctrl generic map
  (
    xindex     => CFG_NASTI_SLAVE_IRQCTRL,
    xaddr      => 16#80002#,
    xmask      => 16#FFFFF#,
    htif_index => CFG_HTIF_SRC_IRQCTRL
  ) port map (
    clk    => wClkBus,
    nrst   => wNReset,
    i_irqs => irq_pins,
    o_cfg  => axi_cfg(CFG_NASTI_SLAVE_IRQCTRL),
    i_axi  => axisi,
    o_axi  => axiso(CFG_NASTI_SLAVE_IRQCTRL),
    i_host => htifi_grant,
    o_host => htifo(CFG_HTIF_SRC_IRQCTRL)
  );

  --! @brief RF front-end controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80004000..0x80004fff (4 KB total)
  rf0 : axi_rfctrl generic map (
    xindex => CFG_NASTI_SLAVE_RFCTRL,
    xaddr  => 16#80004#,
    xmask  => 16#fffff#
  ) port map (
    nrst           => wNReset,
    clk            => wClkBus,
    o_cfg          => axi_cfg(CFG_NASTI_SLAVE_RFCTRL),
    i_axi          => axisi,
    o_axi          => axiso(CFG_NASTI_SLAVE_RFCTRL),
    i_gps_ld       => i_gps_ld,
    i_glo_ld       => i_glo_ld,
    outSCLK        => o_max_sclk,
    outSDATA       => o_max_sdata,
    outCSn         => o_max_ncs,
    inExtAntStat   => i_antext_stat,
    inExtAntDetect => i_antext_detect,
    outExtAntEna   => o_antext_ena,
    outIntAntContr => o_antint_contr
  );

  ------------------------------------
  --! @brief GNSS Engine stub with the AXI4 interface.
  --! @details Map address:
  --!          0x80003000..0x80003fff (4 KB total)
geneng_ena : if CFG_GNSSLIB_ENABLE generate 
  gnss_i.nrst     <= wNReset;
  gnss_i.clk_bus  <= wClkBus;
  gnss_i.axi      <= axisi;
  gnss_i.clk_adc  <= wClkAdc;
  gnss_i.gps_I    <= i_gps_I;
  gnss_i.gps_Q    <= i_gps_Q;
  gnss_i.glo_I    <= i_glo_I;
  gnss_i.glo_Q    <= i_glo_I;

  gnss0 : gnssengine  generic map
  (
    tech    => CFG_MEMTECH,
    xindex  => CFG_NASTI_SLAVE_ENGINE,
    xaddr   => 16#80003#,
    xmask   => 16#FFFFF#
  ) port map (
    i      => gnss_i,
    o      => gnss_o
  );
  
  axiso(CFG_NASTI_SLAVE_ENGINE) <= gnss_o.axi;
  axi_cfg(CFG_NASTI_SLAVE_ENGINE)  <= gnss_o.cfg;
  irq_pins(CFG_IRQ_GNSSENGINE)      <= gnss_o.ms_pulse;
end generate;
geneng_dis : if not CFG_GNSSLIB_ENABLE generate 
  axiso(CFG_NASTI_SLAVE_ENGINE) <= nasti_slave_out_none;
  axi_cfg(CFG_NASTI_SLAVE_ENGINE)  <= nasti_slave_config_none;
  irq_pins(CFG_IRQ_GNSSENGINE)      <= '0';
end generate;


  --! @brief GPS-CA Fast Search Engine with the AXI4 interface.
  --! @details Map address:
  --!          0x80005000..0x80005fff (4 KB total)
  fse0_ena : if CFG_GNSSLIB_ENABLE and CFG_GNSSLIB_FSEGPS_ENABLE = 1 generate 
      fse_i.nrst       <= wNReset;
      fse_i.clk_bus    <= wClkBus;
      fse_i.clk_fse    <= wClkBus;
      fse_i.axi        <= axisi;
      fse_i.clk_adc    <= wClkAdc;
      fse_i.I          <= i_gps_I;
      fse_i.Q          <= i_gps_Q;
      fse_i.ms_pulse   <= gnss_o.ms_pulse;
      fse_i.pps        <= gnss_o.pps;
      fse_i.test_mode  <= '0';

      fse0 : TopFSE generic map (
        tech   => CFG_MEMTECH,
        xindex => CFG_NASTI_SLAVE_FSE_GPS,
        xaddr  => 16#80005#,
        xmask  => 16#fffff#,
        sys    => GEN_SYSTEM_GPSCA
      ) port map (
        i => fse_i,
        o => fse_o
      );
  
      axi_cfg(CFG_NASTI_SLAVE_FSE_GPS) <= fse_o.cfg;
      axiso(CFG_NASTI_SLAVE_FSE_GPS) <= fse_o.axi;
  end generate;
  --! FSE GPS disable
  fse0_dis : if CFG_GNSSLIB_FSEGPS_ENABLE = 0 generate 
      axi_cfg(CFG_NASTI_SLAVE_FSE_GPS) <= nasti_slave_config_none;
      axiso(CFG_NASTI_SLAVE_FSE_GPS) <= nasti_slave_out_none;
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
    cfgvec => axi_cfg,
    cfg    => axi_cfg(CFG_NASTI_SLAVE_PNP),
    i      => axisi,
    o      => axiso(CFG_NASTI_SLAVE_PNP)
  );


end arch_rocket_soc;
