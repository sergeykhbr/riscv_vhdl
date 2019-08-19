--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
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
--! Technology definition library.
library techmap;
use techmap.gencomp.all;
--! CPU, System Bus and common peripheries library.
library ambalib;
use ambalib.types_amba4.all;

--! @brief   Declaration of components visible on SoC top level.
package types_misc is

--! @brief SOC global reset former.
--! @details This module produces output reset signal in a case if
--!          button 'Reset' was pushed or PLL isn't a 'lock' state.
--! param[in]  inSysReset Button generated signal
--! param[in]  inSysClk Clock from the PLL. Bus clock.
--! param[out] outReset Output reset signal with active 'High' (1 = reset).
component reset_global
port (
  inSysReset  : in std_ulogic;
  inSysClk    : in std_ulogic;
  outReset    : out std_ulogic );
end component;


--! Boot ROM with AXI4 interface declaration.
component axi4_rom is
generic (
    memtech  : integer := inferred;
    async_reset : boolean := false;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    sim_hexfile : string
  );
port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type
  );
end component; 

--! Internal RAM with AXI4 interface declaration.
component axi4_sram is
  generic (
    memtech  : integer := inferred;
    async_reset : boolean := false;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    abits    : integer := 17;
    init_file : string := "" -- only for 'inferred'
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type
  );
end component; 

--! AXI4 to SPI brdige for external Flash IC Micron M25AA1024
type spi_in_type is record
    SDI : std_logic;
end record;

type spi_out_type is record
    SDO : std_logic;
    SCK : std_logic;
    nCS : std_logic;
    nWP : std_logic;
    nHOLD : std_logic;
    RESET : std_logic;
end record;

component axi4_flashspi is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out  nasti_slave_config_type;
    i_spi  : in  spi_in_type;
    o_spi  : out spi_out_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type  );
end component; 

--! @brief NASTI (AXI4) GPIO controller
component nasti_gpio is
  generic (
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
	 xirq     : integer := 0;
	 width    : integer := 12
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type;
    i_gpio : in std_logic_vector(width-1 downto 0);
	 o_gpio : out std_logic_vector(width-1 downto 0);
    o_gpio_dir : out std_logic_vector(width-1 downto 0)
  );
end component; 

type uart_in_type is record
  rd   	: std_ulogic;
  cts   : std_ulogic;
end record;

type uart_out_type is record
  td   	: std_ulogic;
  rts   : std_ulogic;
end record;

--! UART with the AXI4 interface declaration.
component axi4_uart is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    fifosz  : integer := 16
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out  nasti_slave_config_type;
    i_uart : in  uart_in_type;
    o_uart : out uart_out_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq  : out std_logic);
end component;

--! Test Access Point via UART (debug access)
component uart_tap is
  port (
    nrst     : in std_logic;
    clk      : in std_logic;
    i_uart   : in  uart_in_type;
    o_uart   : out uart_out_type;
    i_msti   : in nasti_master_in_type;
    o_msto   : out nasti_master_out_type;
    o_mstcfg : out nasti_master_config_type
  );
end component; 

-- JTAG TAP
component tap_jtag is
  generic (
    ainst  : integer range 0 to 255 := 2;
    dinst  : integer range 0 to 255 := 3);
  port (
    nrst  : in std_logic;
    clk  : in std_logic;
    i_tck   : in std_logic;   -- in: Test Clock
    i_ntrst   : in std_logic;   -- in: 
    i_tms   : in std_logic;   -- in: Test Mode State
    i_tdi   : in std_logic;   -- in: Test Data Input
    o_tdo   : out std_logic;   -- out: Test Data Output
    o_jtag_vref : out std_logic;
    i_msti   : in nasti_master_in_type;
    o_msto   : out nasti_master_out_type;
    o_mstcfg : out nasti_master_config_type
    );
end component;


--! @brief   Interrupt controller with the AXI4 interface declaration.
--! @details To rise interrupt on certain CPU HostIO interface is used.
component nasti_irqctrl is
  generic (
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
 (
    clk    : in std_logic;
    nrst   : in std_logic;
    i_irqs : in std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq_meip : out std_logic
  );
  end component;

  --! @brief   General Purpose Timers with the AXI interface.
  --! @details This module provides high precision counter and
  --!          generic number of GP timers.
  component nasti_gptimers is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    tmr_total  : integer := 2
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out nasti_slave_config_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq  : out std_logic
  );
  end component; 

--! @brief   Plug-n-Play support module with AXI4 interface declaration.
--! @details Each device in a system hase to implements sideband signal
--!          structure 'nasti_slave_config_type' that allows FW to
--!          detect Hardware configuration in a run-time.
--! @todo Implements PnP signals for all Masters devices.
component nasti_pnp is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0;
    hw_id   : std_logic_vector(31 downto 0) := X"20170101"
  );
  port (
    sys_clk : in  std_logic;
    adc_clk : in  std_logic;
    nrst   : in  std_logic;
    mstcfg : in  nasti_master_cfg_vector;
    slvcfg : in  nasti_slave_cfg_vector;
    cfg    : out  nasti_slave_config_type;
    i      : in  nasti_slave_in_type;
    o      : out nasti_slave_out_type
  );
end component; 



end; -- package declaration
