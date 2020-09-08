--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
use ambalib.types_bus0.all;
--! Misc modules library
library misclib;
use misclib.types_misc.all;
--! Ethernet related declarations.
library ethlib;
use ethlib.types_eth.all;
--! gnss sub-system library
library gnsslib;
use gnsslib.types_gnss.all;

--! River CPU specific library
library riverlib;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

 --! Top-level implementaion library
library work;
--! Target dependable configuration: RTL, FPGA or ASIC.
use work.config_target.all;

--! @brief   SOC Top-level entity declaration.
--! @details This module implements full SOC functionality and all IO signals
--!          are available on FPGA/ASIC IO pins.
entity riscv_soc is port 
( 
  i_rst : in std_logic;
  i_clk  : in std_logic;
  --! GPIO.
  i_gpio     : in std_logic_vector(11 downto 0);
  o_gpio     : out std_logic_vector(11 downto 0);
  o_gpio_dir : out std_logic_vector(11 downto 0);
  --! GPTimers
  o_pwm : out std_logic_vector(1 downto 0);
  --! JTAG signals:
  i_jtag_tck : in std_logic;
  i_jtag_ntrst : in std_logic;
  i_jtag_tms : in std_logic;
  i_jtag_tdi : in std_logic;
  o_jtag_tdo : out std_logic;
  o_jtag_vref : out std_logic;
  --! UART1 signals:
  i_uart1_ctsn : in std_logic;
  i_uart1_rd   : in std_logic;
  o_uart1_td   : out std_logic;
  o_uart1_rtsn : out std_logic;
  --! UART2 (debug port) signals:
  i_uart2_ctsn : in std_logic;
  i_uart2_rd   : in std_logic;
  o_uart2_td   : out std_logic;
  o_uart2_rtsn : out std_logic;
  --! SPI Flash
  i_flash_si : in std_logic;
  o_flash_so : out std_logic;
  o_flash_sck : out std_logic;
  o_flash_csn : out std_logic;
  o_flash_wpn : out std_logic;
  o_flash_holdn : out std_logic;
  o_flash_reset : out std_logic;
  --! OTP Memory
  i_otp_d : in std_logic_vector(15 downto 0);
  o_otp_d : out std_logic_vector(15 downto 0);
  o_otp_a : out std_logic_vector(11 downto 0);
  o_otp_we : out std_logic;
  o_otp_re : out std_logic;
  --! Ethernet MAC PHY interface signals
  i_etx_clk   : in    std_ulogic;
  i_erx_clk   : in    std_ulogic;
  i_erxd      : in    std_logic_vector(3 downto 0);
  i_erx_dv    : in    std_ulogic;
  i_erx_er    : in    std_ulogic;
  i_erx_col   : in    std_ulogic;
  i_erx_crs   : in    std_ulogic;
  i_emdint    : in std_ulogic;
  o_etxd      : out   std_logic_vector(3 downto 0);
  o_etx_en    : out   std_ulogic;
  o_etx_er    : out   std_ulogic;
  o_emdc      : out   std_ulogic;
  i_eth_mdio    : in std_logic;
  o_eth_mdio    : out std_logic;
  o_eth_mdio_oe : out std_logic;
  i_eth_gtx_clk    : in std_logic;
  i_eth_gtx_clk_90 : in std_logic;
  o_erstn     : out   std_ulogic;
  -- GNSS Sub-system signals:
  i_clk_adc : in std_logic;                   -- GNSS ADC clock (4..40 MHz)
  i_gps_I : in std_logic_vector(1 downto 0);  -- Channel 0 sampled I value
  i_gps_Q : in std_logic_vector(1 downto 0);  -- Channel 0 sampled Q value
  i_glo_I : in std_logic_vector(1 downto 0);  -- Channel 1 sampled I value
  i_glo_Q : in std_logic_vector(1 downto 0);  -- Channel 1 sampled I value
  o_pps : out std_logic;                      -- Pulse Per Second signal
  i_gps_ld    : in std_logic;                 -- Channel 0 RF front-end Lock detect
  i_glo_ld    : in std_logic;                 -- Channel 1 RF front-end Lock detect
  o_max_sclk  : out std_logic;                -- RF synthesizer SPI clock
  o_max_sdata : out std_logic;                -- RF synthesizer SPI data
  o_max_ncs   : out std_logic_vector(1 downto 0); -- RF synthesizer channel 0/1 selector
  i_antext_stat   : in std_logic;             -- Antenna powered status
  i_antext_detect : in std_logic;             -- Antenna connected status
  o_antext_ena    : out std_logic;            -- Enabling/disabling antenna
  o_antint_contr  : out std_logic             -- Antenna Internal/External selector
);
  --! @}

end riscv_soc;

--! @brief SOC top-level  architecture declaration.
architecture arch_riscv_soc of riscv_soc is

  signal w_glob_rst  : std_ulogic; -- Global reset active HIGH
  signal w_glob_nrst : std_ulogic; -- Global reset active LOW
  signal w_soft_rst : std_ulogic; -- Software reset (acitve HIGH) from DSU
  signal w_bus_nrst : std_ulogic; -- Global reset and Soft Reset active LOW
  
  signal uart1i : uart_in_type;
  signal uart1o : uart_out_type;
  signal uart2i : uart_in_type;
  signal uart2o : uart_out_type;
  signal spiflashi : spi_in_type;
  signal spiflasho : spi_out_type;

  --! Arbiter is switching only slaves output signal, data from noc
  --! is connected to all slaves and to the arbiter itself.
  signal aximi   : bus0_xmst_in_vector;
  signal aximo   : bus0_xmst_out_vector;
  signal axisi   : bus0_xslv_in_vector;
  signal axiso   : bus0_xslv_out_vector;
  signal slv_cfg : bus0_xslv_cfg_vector;
  signal mst_cfg : bus0_xmst_cfg_vector;
  signal wb_core_irq : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
  signal w_ext_irq : std_logic;
  signal dport_i : dport_in_vector;
  signal dport_o : dport_out_vector;
  signal dmi_dport_i : dport_in_vector;
  signal dmi_dport_o : dport_out_vector;
  signal dsu_dport_i : dport_in_vector;
  signal dsu_dport_o : dport_out_vector;
  signal wb_bus_util_w : std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0);
  signal wb_bus_util_r : std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0);

  signal w_dmi_jtag_req_valid : std_logic;
  signal w_dmi_jtag_req_ready : std_logic;
  signal w_dmi_jtag_write : std_logic;
  signal wb_dmi_jtag_addr : std_logic_vector(6 downto 0);
  signal wb_dmi_jtag_wdata : std_logic_vector(31 downto 0);
  signal w_dmi_jtag_resp_valid : std_logic;
  signal w_dmi_jtag_resp_ready : std_logic;
  signal wb_dmi_jtag_rdata : std_logic_vector(31 downto 0);

  signal w_dmi_dsu_req_valid : std_logic;
  signal w_dmi_dsu_req_ready : std_logic;
  signal w_dmi_dsu_write : std_logic;
  signal wb_dmi_dsu_addr : std_logic_vector(6 downto 0);
  signal wb_dmi_dsu_wdata : std_logic_vector(31 downto 0);
  signal w_dmi_dsu_resp_valid : std_logic;
  signal w_dmi_dsu_resp_ready : std_logic;
  signal wb_dmi_dsu_rdata : std_logic_vector(31 downto 0);
  signal wb_dmi_hartsel : std_logic_vector(CFG_LOG2_CPU_MAX-1 downto 0);
  
  signal eth_i : eth_in_type;
  signal eth_o : eth_out_type;
 
  signal irq_pins : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);

  signal w_otp_busy : std_logic;
  signal wb_otp_cfg_rsetup : std_logic_vector(3 downto 0);
  signal wb_otp_cfg_wadrsetup : std_logic_vector(3 downto 0);
  signal wb_otp_cfg_wactive : std_logic_vector(31 downto 0);
  signal wb_otp_cfg_whold : std_logic_vector(3 downto 0);
begin


 ------------------------------------
  --! @brief System Reset device instance.
  rst0 : reset_global port map (
    inSysReset  => i_rst,
    inSysClk    => i_clk,
    outReset    => w_glob_rst
  );
  w_glob_nrst <= not w_glob_rst;
  w_bus_nrst <= not (w_glob_rst or w_soft_rst);

  --! @brief AXI4 controller.
  ctrl0 : axictrl_bus0 generic map (
    async_reset => CFG_ASYNC_RESET
  ) port map (
    i_clk    => i_clk,
    i_nrst   => w_glob_nrst,
    i_slvcfg => slv_cfg,
    i_slvo   => axiso,
    i_msto   => aximo,
    o_slvi   => axisi,
    o_msti   => aximi,
    o_bus_util_w => wb_bus_util_w, -- Bus write access utilization per master statistic
    o_bus_util_r => wb_bus_util_r  -- Bus read access utilization per master statistic
  );

  wb_core_irq(CFG_TOTAL_CPU_MAX-1 downto 1) <= (others => '0');
  wb_core_irq(0) <= w_ext_irq;  -- TODO: other CPU interrupts

  group0 : river_workgroup generic map (
    cpunum => CFG_CPU_NUM,
    memtech => CFG_MEMTECH,
    async_reset => CFG_ASYNC_RESET,
    fpu_ena => true,
    coherence_ena => false,
    tracer_ena => false
  ) port map ( 
    i_nrst   => w_bus_nrst,
    i_clk    => i_clk,
    i_msti   => aximi(CFG_BUS0_XMST_WORKGROUP),
    o_msto   => aximo(CFG_BUS0_XMST_WORKGROUP),
    o_mstcfg => mst_cfg(CFG_BUS0_XMST_WORKGROUP),
    i_dport  => dport_i,
    o_dport  => dport_o,
    i_ext_irq => wb_core_irq
);

  -- Access to Debug port of the CPUs workgroup
  dmregs0 : dmi_regs generic map (
    async_reset => CFG_ASYNC_RESET,
    cpu_available => CFG_CPU_NUM
  ) port map (
    clk    => i_clk,
    nrst   => w_glob_nrst,
    -- port[0] connected to JTAG TAP has access to AXI master interface (SBA registers)
    i_dmi_jtag_req_valid => w_dmi_jtag_req_valid,
    o_dmi_jtag_req_ready => w_dmi_jtag_req_ready,
    i_dmi_jtag_write => w_dmi_jtag_write,
    i_dmi_jtag_addr => wb_dmi_jtag_addr,
    i_dmi_jtag_wdata => wb_dmi_jtag_wdata,
    o_dmi_jtag_resp_valid => w_dmi_jtag_resp_valid,
    i_dmi_jtag_resp_ready => w_dmi_jtag_resp_ready,
    o_dmi_jtag_rdata => wb_dmi_jtag_rdata,
    -- port[1] connected to DSU doesn't have access to AXI master interface
    i_dmi_dsu_req_valid => w_dmi_dsu_req_valid,
    o_dmi_dsu_req_ready => w_dmi_dsu_req_ready,
    i_dmi_dsu_write => w_dmi_dsu_write,
    i_dmi_dsu_addr => wb_dmi_dsu_addr,
    i_dmi_dsu_wdata => wb_dmi_dsu_wdata,
    o_dmi_dsu_resp_valid => w_dmi_dsu_resp_valid,
    i_dmi_dsu_resp_ready => w_dmi_dsu_resp_ready,
    o_dmi_dsu_rdata => wb_dmi_dsu_rdata,
    o_hartsel => wb_dmi_hartsel,
    o_dmstat => open,
    o_ndmreset => w_soft_rst,
    o_cfg => mst_cfg(CFG_BUS0_XMST_DMI),
    i_xmsti  => aximi(CFG_BUS0_XMST_DMI),
    o_xmsto  => aximo(CFG_BUS0_XMST_DMI),
    o_dporti => dmi_dport_i,
    i_dporto => dmi_dport_o
  );

  -- Interconnect between DMI register and DSU debug interfaces
  icdport0 : ic_dport_2s_1m generic map (
    async_reset => CFG_ASYNC_RESET
  ) port map (
    clk    => i_clk,
    nrst   => w_glob_nrst,
    i_sdport0i => dmi_dport_i,
    o_sdport0o => dmi_dport_o,
    i_sdport1i => dsu_dport_i,
    o_sdport1o => dsu_dport_o,
    o_mdporti => dport_i,
    i_mdporto => dport_o
  );

dsu_ena : if CFG_DSU_ENABLE generate
  ------------------------------------
  --! @brief Debug Support Unit with access to the CSRs
  --! @details Map address:
  --!          0x80080000..0x8009ffff (128 KB total)
  dsu0 : axi_dsu generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#80080#,
    xmask    => 16#fffe0#
  ) port map (
    clk    => i_clk,
    nrst   => w_glob_nrst,
    o_cfg  => slv_cfg(CFG_BUS0_XSLV_DSU),
    i_axi  => axisi(CFG_BUS0_XSLV_DSU),
    o_axi  => axiso(CFG_BUS0_XSLV_DSU),
    o_dporti => dsu_dport_i,
    i_dporto => dsu_dport_o,
    i_dmi_hartsel => wb_dmi_hartsel,
    o_dmi_req_valid => w_dmi_dsu_req_valid,
    i_dmi_req_ready => w_dmi_dsu_req_ready,
    o_dmi_write => w_dmi_dsu_write,
    o_dmi_addr => wb_dmi_dsu_addr,
    o_dmi_wdata => wb_dmi_dsu_wdata,
    i_dmi_resp_valid => w_dmi_dsu_resp_valid,
    o_dmi_resp_ready => w_dmi_dsu_resp_ready,
    i_dmi_rdata => wb_dmi_dsu_rdata,
    -- Run time platform statistic signals (move to tracer):
    i_bus_util_w => wb_bus_util_w, -- Write access bus utilization per master statistic
    i_bus_util_r => wb_bus_util_r  -- Read access bus utilization per master statistic
  );
end generate;
dsu_dis : if not CFG_DSU_ENABLE generate
    slv_cfg(CFG_BUS0_XSLV_DSU) <= axi4_slave_config_none;
    axiso(CFG_BUS0_XSLV_DSU) <= axi4_slave_out_none;
    dsu_dport_i <= (others => dport_in_none);
    w_dmi_dsu_req_valid <= '0';
    w_dmi_dsu_write <= '0';
    wb_dmi_dsu_addr <= (others => '0');
    wb_dmi_dsu_wdata <= (others => '0');
    w_dmi_dsu_resp_ready <= '0';
end generate;

  ------------------------------------
  -- JTAG TAP interface
  jtag0 : tap_jtag  port map (
    nrst   => w_glob_nrst, 
    clk    => i_clk, 
    i_tck  => i_jtag_tck,
    i_ntrst  => i_jtag_ntrst,
    i_tms  => i_jtag_tms,
    i_tdi  => i_jtag_tdi,
    o_tdo  => o_jtag_tdo,
    o_jtag_vref => o_jtag_vref,
    -- DMI interface
    o_dmi_req_valid => w_dmi_jtag_req_valid,
    i_dmi_req_ready => w_dmi_jtag_req_ready,
    o_dmi_write => w_dmi_jtag_write,
    o_dmi_addr => wb_dmi_jtag_addr,
    o_dmi_wdata => wb_dmi_jtag_wdata,
    i_dmi_resp_valid => w_dmi_jtag_resp_valid,
    o_dmi_resp_ready => w_dmi_jtag_resp_ready,
    i_dmi_rdata => wb_dmi_jtag_rdata
  );

  ------------------------------------
  --! @brief TAP via UART (debug port) with master interface.
  uart2i.cts   <= not i_uart2_ctsn;
  uart2i.rd    <= i_uart2_rd;
  uart2 : uart_tap  port map (
    nrst   => w_glob_nrst, 
    clk    => i_clk, 
    i_uart   => uart2i,
    o_uart   => uart2o,
    i_msti   => aximi(CFG_BUS0_XMST_MSTUART),
    o_msto   => aximo(CFG_BUS0_XMST_MSTUART),
    o_mstcfg => mst_cfg(CFG_BUS0_XMST_MSTUART)
  );
  o_uart2_td  <= uart2o.td;
  o_uart2_rtsn <= not uart2o.rts;

  ------------------------------------
  --! @brief BOOT ROM module instance with the AXI4 interface.
  --! @details Map address:
  --!          0x00000000..0x00007fff (32 KB total)
  boot0 : axi4_rom generic map (
    memtech  => CFG_MEMTECH,
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#00000#,
    xmask    => 16#ffff8#,
    sim_hexfile => CFG_SIM_BOOTROM_HEX
  ) port map (
    clk  => i_clk,
    nrst => w_glob_nrst,
    cfg  => slv_cfg(CFG_BUS0_XSLV_BOOTROM),
    i    => axisi(CFG_BUS0_XSLV_BOOTROM),
    o    => axiso(CFG_BUS0_XSLV_BOOTROM)
  );

  ------------------------------------
  --! @brief OTP module instance with the AXI4 interface.
  --! @details Map address:
  --!          0x00010000..0x00011fff (8 KB total)
  otp_ena : if CFG_OTP8KB_ENA generate
    otp0 : axi4_otp generic map (
      async_reset => CFG_ASYNC_RESET,
      xaddr => 16#00010#,
      xmask => 16#ffffe#
    ) port map (
      clk    => i_clk,
      nrst   => w_glob_nrst,
      cfg    => slv_cfg(CFG_BUS0_XSLV_OTP),
      i_axi => axisi(CFG_BUS0_XSLV_OTP),
      o_axi => axiso(CFG_BUS0_XSLV_OTP),
      o_otp_we     => o_otp_we,
      o_otp_re     => o_otp_re,
      o_otp_addr   => o_otp_a,
      o_otp_wdata  => o_otp_d,
      i_otp_rdata  => i_otp_d,
      i_cfg_rsetup => wb_otp_cfg_rsetup,
      i_cfg_wadrsetup => wb_otp_cfg_wadrsetup,
      i_cfg_wactive => wb_otp_cfg_wactive,
      i_cfg_whold => wb_otp_cfg_whold,
      o_busy => w_otp_busy
    );
  end generate;
  otp_dis : if not CFG_OTP8KB_ENA generate
      slv_cfg(CFG_BUS0_XSLV_OTP) <= axi4_slave_config_none;
      axiso(CFG_BUS0_XSLV_OTP) <= axi4_slave_out_none;
      o_otp_d <= X"0000";
      o_otp_a <= X"000";
      o_otp_we <= '0';
      o_otp_re <= '0';
      w_otp_busy <= '0';
  end generate;

  ------------------------------------
  --! @brief Firmware Image ROM with the AXI4 interface.
  --! @details Map address:
  --!          0x00100000..0x0013ffff (256 KB total)
  --! @warning Don't forget to change ROM_ADDR_WIDTH in rom implementation
  img0 : axi4_rom generic map (
    memtech  => CFG_MEMTECH,
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#00100#,
    xmask    => 16#fffc0#,
    sim_hexfile => CFG_SIM_FWIMAGE_HEX
  ) port map (
    clk  => i_clk,
    nrst => w_glob_nrst,
    cfg  => slv_cfg(CFG_BUS0_XSLV_ROMIMAGE),
    i    => axisi(CFG_BUS0_XSLV_ROMIMAGE),
    o    => axiso(CFG_BUS0_XSLV_ROMIMAGE)
  );

  ------------------------------------
  --! @brief SPI FLASH module isntance with the AXI4 interface.
  --! @details Map address:
  --!          0x00200000..0x0023ffff (256 KB total)
  spiflashi.SDI <= i_flash_si;

  flash_ena : if CFG_EXT_FLASH_ENA generate
    flash0 : axi4_flashspi generic map (
      async_reset => CFG_ASYNC_RESET,
      xaddr    => 16#00200#,
      xmask    => 16#fffc0#,
      wait_while_write => true
    ) port map (
      clk => i_clk,
      nrst => w_glob_nrst,
      cfg => slv_cfg(CFG_BUS0_XSLV_EXTFLASH),
      i_spi => spiflashi,
      o_spi => spiflasho,
      i_axi => axisi(CFG_BUS0_XSLV_EXTFLASH),
      o_axi => axiso(CFG_BUS0_XSLV_EXTFLASH)
    );
  end generate;
  flash_dis : if not CFG_EXT_FLASH_ENA generate
      slv_cfg(CFG_BUS0_XSLV_EXTFLASH) <= axi4_slave_config_none;
      axiso(CFG_BUS0_XSLV_EXTFLASH) <= axi4_slave_out_none;
      spiflasho <= spi_out_none;
  end generate;

  o_flash_so <= spiflasho.SDO;
  o_flash_sck <= spiflasho.SCK;
  o_flash_csn <= spiflasho.nCS;
  o_flash_wpn <= spiflasho.nWP;
  o_flash_holdn <= spiflasho.nHOLD;
  o_flash_reset <= spiflasho.RESET;


  ------------------------------------
  --! Internal SRAM module instance with the AXI4 interface.
  --! @details Map address:
  --!          0x10000000..0x1007ffff (512 KB total)
  sram0 : axi4_sram generic map (
    memtech  => CFG_MEMTECH,
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#10000#,
    xmask    => 16#fff80#,            -- 512 KB mask
    abits    => (10 + log2(512)),     -- 512 KB address
    init_file => CFG_SIM_FWIMAGE_HEX  -- Used only for inferred
  ) port map (
    clk  => i_clk,
    nrst => w_glob_nrst,
    cfg  => slv_cfg(CFG_BUS0_XSLV_SRAM),
    i    => axisi(CFG_BUS0_XSLV_SRAM),
    o    => axiso(CFG_BUS0_XSLV_SRAM)
  );


  ------------------------------------
  --! @brief Controller of the LEDs, DIPs and GPIO with the AXI4 interface.
  --! @details Map address:
  --!          0x80000000..0x80000fff (4 KB total)
  gpio0 : axi4_gpio generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#80000#,
    xmask    => 16#fffff#,
    xirq     => 0,
    width    => 12
  ) port map (
    clk   => i_clk,
    nrst  => w_glob_nrst,
    cfg   => slv_cfg(CFG_BUS0_XSLV_GPIO),
    i     => axisi(CFG_BUS0_XSLV_GPIO),
    o     => axiso(CFG_BUS0_XSLV_GPIO),
    i_gpio => i_gpio,
    o_gpio => o_gpio,
    o_gpio_dir => o_gpio_dir
  );
  
  
  ------------------------------------
  uart1i.cts   <= not i_uart1_ctsn;
  uart1i.rd    <= i_uart1_rd;

  --! @brief UART Controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80001000..0x80001fff (4 KB total)
  uart1 : axi4_uart generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr    => 16#80001#,
    xmask    => 16#FFFFF#,
    xirq     => CFG_IRQ_UART1,
    fifosz   => 16
  ) port map (
    nrst   => w_glob_nrst, 
    clk    => i_clk, 
    cfg    => slv_cfg(CFG_BUS0_XSLV_UART1),
    i_uart => uart1i, 
    o_uart => uart1o,
    i_axi  => axisi(CFG_BUS0_XSLV_UART1),
    o_axi  => axiso(CFG_BUS0_XSLV_UART1),
    o_irq  => irq_pins(CFG_IRQ_UART1)
  );
  o_uart1_td  <= uart1o.td;
  o_uart1_rtsn <= not uart1o.rts;


  ------------------------------------
  --! @brief Interrupt controller with the AXI4 interface.
  --! @details Map address:
  --!          0x80002000..0x80002fff (4 KB total)
  irq0 : axi4_irqctrl generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr      => 16#80002#,
    xmask      => 16#FFFFF#
  ) port map (
    clk    => i_clk,
    nrst   => w_bus_nrst,
    i_irqs => irq_pins,
    o_cfg  => slv_cfg(CFG_BUS0_XSLV_IRQCTRL),
    i_axi  => axisi(CFG_BUS0_XSLV_IRQCTRL),
    o_axi  => axiso(CFG_BUS0_XSLV_IRQCTRL),
    o_irq_meip => w_ext_irq
  );

  --! @brief Timers with the AXI4 interface.
  --! @details Map address:
  --!          0x80005000..0x80005fff (4 KB total)
  gptmr0 : axi4_gptimers  generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr     => 16#80005#,
    xmask     => 16#fffff#,
    xirq      => CFG_IRQ_GPTIMERS,
    tmr_total => 2
  ) port map (
    clk    => i_clk,
    nrst   => w_glob_nrst,
    cfg    => slv_cfg(CFG_BUS0_XSLV_GPTIMERS),
    i_axi  => axisi(CFG_BUS0_XSLV_GPTIMERS),
    o_axi  => axiso(CFG_BUS0_XSLV_GPTIMERS),
    o_pwm  => o_pwm,
    o_irq  => irq_pins(CFG_IRQ_GPTIMERS)
  );

  --! @brief GNSS Sub-System with the AXI4 interface.
  --! @details Map address:
  --!          0x80008000..0x8000ffff (32 KB total)
  --!
  --!          0x80008000..0x80008fff (4 KB total) RF Controller
  --!          0x80009000..0x80009fff (4 KB total) Engine
  --!          0x8000a000..0x8000afff (4 KB total) GPS FSE
  gnss_ena : if CFG_GNSS_SS_ENA generate
    gnss0 : gnss_ss generic map (
      async_reset => CFG_ASYNC_RESET,
      tech        => CFG_MEMTECH,
      xaddr       => 16#80008#,
      xmask       => 16#FFFF8#,
      xirq        => CFG_IRQ_GNSSENGINE
    ) port map ( 
      i_nrst => w_glob_nrst,
      i_clk_bus => i_clk,
      i_clk_adc => i_clk_adc,
      i_gps_I => i_gps_I,
      i_gps_Q => i_gps_Q,
      i_glo_I => i_glo_I,
      i_glo_Q => i_glo_Q,
      o_pps => o_pps,
      i_gps_ld => i_gps_ld,
      i_glo_ld => i_glo_ld,
      o_max_sclk => o_max_sclk,
      o_max_sdata => o_max_sdata,
      o_max_ncs => o_max_ncs,
      i_antext_stat => i_antext_stat,
      i_antext_detect => i_antext_detect,
      o_antext_ena => o_antext_ena,
      o_antint_contr => o_antint_contr,
      o_cfg => slv_cfg(CFG_BUS0_XSLV_GNSS_SS),
      i_axi => axisi(CFG_BUS0_XSLV_GNSS_SS),
      o_axi => axiso(CFG_BUS0_XSLV_GNSS_SS),
      o_irq => irq_pins(CFG_IRQ_GNSSENGINE)
    );
  end generate;
  gnss_dis : if not CFG_GNSS_SS_ENA generate
      axiso(CFG_BUS0_XSLV_GNSS_SS) <= axi4_slave_out_none;
      slv_cfg(CFG_BUS0_XSLV_GNSS_SS) <= axi4_slave_config_none;
      irq_pins(CFG_IRQ_GNSSENGINE) <= '0';
  end generate;


  --! @brief Ethernet MAC with the AXI4 interface.
  --! @details Map address:
  --!          0x80040000..0x8007ffff (256 KB total)
  --!          EDCL IP: 192.168.1.51 = C0.A8.01.33
  eth0_ena : if CFG_ETHERNET_ENABLE generate 
  
    eth_i.tx_clk <= i_etx_clk;
    eth_i.rx_clk <= i_erx_clk;
    eth_i.rxd <= i_erxd;
    eth_i.rx_dv <= i_erx_dv;
    eth_i.rx_er <= i_erx_er;
    eth_i.rx_col <= i_erx_col;
    eth_i.rx_crs <= i_erx_crs;
    eth_i.mdint <= i_emdint;
    eth_i.mdio_i <= i_eth_mdio;
    eth_i.gtx_clk <= i_eth_gtx_clk;
    
    mac0 : grethaxi generic map (
      xaddr => 16#80040#,
      xmask => 16#FFFC0#,
      xirq => CFG_IRQ_ETHMAC,
      memtech => CFG_MEMTECH,
      mdcscaler => 60,  --! System Bus clock in MHz
      enable_mdio => 1,
      fifosize => 16,
      nsync => 1,
      edcl => 1,
      edclbufsz => 16,
      macaddrh => 16#20789#,
      macaddrl => 16#123#,
      ipaddrh => 16#C0A8#,
      ipaddrl => 16#0033#,
      phyrstadr => 7,
      enable_mdint => 1,
      maxsize => 1518
   ) port map (
      rst => w_glob_nrst,
      clk => i_clk,
      msti => aximi(CFG_BUS0_XMST_ETHMAC),
      msto => aximo(CFG_BUS0_XMST_ETHMAC),
      mstcfg => mst_cfg(CFG_BUS0_XMST_ETHMAC),
      msto2 => open,    -- EDCL separate access is disabled
      mstcfg2 => open,  -- EDCL separate access is disabled
      slvi => axisi(CFG_BUS0_XSLV_ETHMAC),
      slvo => axiso(CFG_BUS0_XSLV_ETHMAC),
      slvcfg => slv_cfg(CFG_BUS0_XSLV_ETHMAC),
      ethi => eth_i,
      etho => eth_o,
      irq => irq_pins(CFG_IRQ_ETHMAC)
    );
	 
  end generate;
  
  --! Ethernet disabled
  eth0_dis : if not CFG_ETHERNET_ENABLE generate 
      slv_cfg(CFG_BUS0_XSLV_ETHMAC) <= axi4_slave_config_none;
      axiso(CFG_BUS0_XSLV_ETHMAC) <= axi4_slave_out_none;
      mst_cfg(CFG_BUS0_XMST_ETHMAC) <= axi4_master_config_none;
      aximo(CFG_BUS0_XMST_ETHMAC) <= axi4_master_out_none;
      irq_pins(CFG_IRQ_ETHMAC) <= '0';
		eth_i.gtx_clk <= '0';
      eth_o   <= eth_out_none;
  end generate;

  o_etxd <= eth_o.txd;
  o_etx_en <= eth_o.tx_en;
  o_etx_er <= eth_o.tx_er;
  o_emdc <= eth_o.mdc;
  o_eth_mdio <= eth_o.mdio_o;
  o_eth_mdio_oe <= eth_o.mdio_oe;
  o_erstn <= w_glob_nrst;


  --! @brief Plug'n'Play controller of the current configuration with the
  --!        AXI4 interface.
  --! @details Map address:
  --!          0xfffff000..0xffffffff (4 KB total)
  pnp0 : axi4_pnp generic map (
    async_reset => CFG_ASYNC_RESET,
    xaddr   => 16#fffff#,
    xmask   => 16#fffff#,
    tech    => CFG_MEMTECH,
    hw_id   => CFG_HW_ID
  ) port map (
    sys_clk => i_clk, 
    adc_clk => '0',
    nrst   => w_glob_nrst,
    mstcfg => mst_cfg,
    slvcfg => slv_cfg,
    cfg    => slv_cfg(CFG_BUS0_XSLV_PNP),
    i      => axisi(CFG_BUS0_XSLV_PNP),
    o      => axiso(CFG_BUS0_XSLV_PNP),
    -- OTP Timing control
    i_otp_busy => w_otp_busy,
    o_otp_cfg_rsetup => wb_otp_cfg_rsetup,
    o_otp_cfg_wadrsetup => wb_otp_cfg_wadrsetup,
    o_otp_cfg_wactive => wb_otp_cfg_wactive,
    o_otp_cfg_whold => wb_otp_cfg_whold
  );


end arch_riscv_soc;
