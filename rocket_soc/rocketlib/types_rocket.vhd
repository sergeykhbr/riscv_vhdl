-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     System Top level modules and interconnect declarations.
-----------------------------------------------------------------------------

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
--! Technology definition library.
library techmap;
use techmap.gencomp.all;
--! CPU, System Bus and common peripheries library.
library rocketlib;
--! AXI4 bus configuration and data structures.
use rocketlib.types_nasti.all;
--! TileLink configuration and data structures.
use rocketlib.types_tile.all;  

--! @name Scala inherited constants.
--! @brief The following constants were define in Rocket-chip generator.
--! @{

--! @brief   Declaration of components visible on SoC top level.
package types_rocket is

--! @brief   Bits allocated for the memory tag value.
--! @details This value is defined \i Config.scala and depends of others
--!          configuration paramters, like number of master, clients, channels
--!          and so on. It is not used in VHDL implemenation.
constant MEM_TAG_BITS  : integer := 6;
--! @brief   Memory data bitwise.
--! @details We intentionally select this value equals to AXI4 bus data width.
--!          Default SCALA configurator value uses 64/128 SerDes module to
--!          access from Tile to AXI4 bus.
constant MEM_DATA_BITS : integer := CFG_NASTI_DATA_BITS;
--! @brief   SCALA generated value. Not used in VHDL.
constant MEM_ADDR_BITS : integer := 26;
--! @brief   Multiplexing HTIF bus data width.
--! @details Not used in a case of disabled L2 cache.
--!          If L2 cached is enabled this value defines bitwise of the bus
--!          between \i Uncore module and external transievers.
--!          Standard message size for the HTID request is 128 bits, so this
--!          value defines number of beats required to transmit/recieve such
--!          message.
constant HTIF_WIDTH    : integer := 16;
--! @}

--! @name   HostIO modules unique IDs.
--! @{

--! Interrupt controller
constant CFG_HTIF_SRC_IRQCTRL : integer := 0;
--! Debug Support Unit (DSU)
constant CFG_HTIF_SRC_DSU     : integer := CFG_HTIF_SRC_IRQCTRL + 1;
--! Ether MAC module with DMA and ECL support
constant CFG_HTIF_SRC_ETH     : integer := CFG_HTIF_SRC_DSU + 1;
--! Total number of HostIO initiators.
constant CFG_HTIF_SRC_TOTAL   : integer := CFG_HTIF_SRC_ETH + 1;
--! @}

--! HostIO tile output signals
type host_out_type is  record
    reset : std_logic;
    id : std_logic;
    csr_req_valid : std_logic;
    csr_req_bits_rw : std_logic;
    csr_req_bits_addr : std_logic_vector(11 downto 0);
    csr_req_bits_data : std_logic_vector(63 downto 0);
    csr_resp_ready : std_logic;
end record;

--! @brief   Empty output signals of HostIO interface.
--! @details If device was included in the owners of the HostIO interface and
--!          was disabled by configuration parameter (for example) then its
--!          outputs must be assigned to this empty signals otherwise 
--!          RTL simulation will fail with undefined states of the processor.
constant host_out_none : host_out_type := (
     '0', '0', '0', '0', (others => '0'), (others => '0'), '0');

--! Full stack of HostIO output signals from all devices.
type host_out_vector is array (0 to CFG_HTIF_SRC_TOTAL-1) 
       of host_out_type;


--! HostIO tile input signals
type host_in_type is record
    grant : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0);
    csr_req_ready : std_logic;
    csr_resp_valid : std_logic;
    csr_resp_bits : std_logic_vector(63 downto 0);
    debug_stats_csr : std_logic;
end record;

--! @brief   HostIO (HTIF) controller declaration. 
--! @details This device provides multiplexing of the Host messages
--!          from several sources (interrupt controller, ethernet MAC,
--!          Debug Support Unit and others) on HostIO bus that is 
--!          specific for Rocket-chip implementation of RISC-V.
--! @todo    Make htifii as a vector to support multi-cores 
--!          configuration.
component htifctrl is
  port (
    clk    : in std_logic;
    nrst   : in std_logic;
    srcsi  : in host_out_vector;
    srcso  : out host_out_type;
    htifii : in host_in_type;
    htifio : out host_in_type
);
end component; 

--! @brief   HTIF serializer input.
--! @details In a case of using L2-cache, 'Uncore' module implements
--!          additional layer of the transformation of 128-bits HTIF 
--!          messages into chunks of HTIF_WIDTH. So we have to 
--!          implement the same serdes on upper level.
type htif_serdes_in_type is record
   --! Chunk was accepted by Uncore subsytem.
   ready  : std_logic;
   --! Current chunk output is valid
   valid : std_logic;
   --! Chunk bits itself.
   bits  : std_logic_vector(HTIF_WIDTH-1 downto 0);
end record;

--! @brief   HTIF serializer output.
type htif_serdes_out_type is record
   valid     : std_logic;
   bits      : std_logic_vector(HTIF_WIDTH-1 downto 0);
   ready     : std_logic;
end record;

--! @brief   RocketTile component declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
--! @param[in] xindex1 Cached Tile AXI master index
--! @param[in] xindex2 Uncached Tile AXI master index
--! @param[in] rst     Reset signal with active HIGH level.
--! @param[in] clk_sys System clock (BUS/CPU clock).
--! @param[in] slvo    Bus-to-Slave device signals.
--! @param[in] msti    Bus-to-Master device signals.
--! @param[out] msto1  CachedTile-to-Bus request signals.
--! @param[out] msto2  UncachedTile-to-Bus request signals.
--! @param[in] htifoi  Requests from the HostIO-connected devices.
--! @param[out] htifio Response to HostIO-connected devices.
component rocket_l1only is 
generic (
    xindex1 : integer := 0;
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
end component;

--! @brief   RocketTile + Uncore component declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
--! @param[in] xindex1 Cached Tile AXI master index
--! @param[in] xindex2 Uncached Tile AXI master index
--! @param[in] rst     Reset signal with active HIGH level.
--! @param[in] clk_sys System clock (BUS/CPU clock).
--! @param[in] slvo    Bus-to-Slave device signals.
--! @param[in] msti    Bus-to-Master device signals.
--! @param[out] msto1  CachedTile-to-Bus request signals.
--! @param[out] msto2  UncachedTile-to-Bus request signals.
--! @param[in] htifoi  Requests from the HostIO-connected devices.
--! @param[out] htifio Response to HostIO-connected devices.
component rocket_l2cache is 
generic (
    xindex1 : integer := 0;
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
end component;

--! @brief SOC global reset former.
--! @details This module produces output reset signal in a case if
--!          button 'Reset' was pushed or PLL isn't a 'lock' state.
--! param[in]  inSysReset Button generated signal
--! param[in]  inSysClk Clock from the PLL. Bus clock.
--! param[in]  inPllLock PLL status.
--! param[out] outReset Output reset signal with active 'High' (1 = reset).
component reset_global
port (
  inSysReset  : in std_ulogic;
  inSysClk    : in std_ulogic;
  inPllLock   : in std_ulogic;
  outReset    : out std_ulogic );
end component;


--! Boot ROM with AXI4 interface declaration.
component nasti_bootrom is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
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

--! AXI4 ROM with the default FW version declaration.
  component nasti_romimage is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
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
component nasti_sram is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
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


--! @brief NASTI (AXI4) GPIO controller
component nasti_gpio is
  generic (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type;
    i_dip : in std_logic_vector(3 downto 0);
    o_led : out std_logic_vector(7 downto 0)
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
component nasti_uart is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    fifosz  : integer := 16;
    parity_bit : integer := 1
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out  nasti_slave_config_type;
    i_uart : in  uart_in_type;
    o_uart : out uart_out_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type);
end component;

--! @brief   Interrupt controller with the AXI4 interface declaration.
--! @details To rise interrupt on certain CPU HostIO interface is used.
component nasti_irqctrl is
  generic (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    htif_index  : integer := 0
  );
  port 
 (
    clk    : in std_logic;
    nrst   : in std_logic;
    i_irqs : in std_logic_vector(CFG_IRQ_TOTAL-1 downto 0);
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    i_host : in host_in_type;
    o_host : out host_out_type
  );
end component;

--! @brief   Plug-n-Play support module with AXI4 interface declaration.
--! @details Each device in a system hase to implements sideband signal
--!          structure 'nasti_slave_config_type' that allows FW to
--!          detect Hardware configuration in a run-time.
--! @todo Implements PnP signals for all Masters devices.
component nasti_pnp is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfgvec : in  nasti_slave_cfg_vector;
    cfg    : out  nasti_slave_config_type;
    i      : in  nasti_slave_in_type;
    o      : out nasti_slave_out_type
  );
end component; 

end; -- package declaration
