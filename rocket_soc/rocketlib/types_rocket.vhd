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
library commonlib;
use commonlib.types_common.all;
--! Technology definition library.
library techmap;
use techmap.gencomp.all;
--! CPU, System Bus and common peripheries library.
library ambalib;
use ambalib.types_amba4.all;

--! @brief   Declaration of components visible on SoC top level.
package types_rocket is

--! @name Scala inherited constants.
--! @brief The following constants were define in Rocket-chip generator.
--! @{

--! @brief   Bits allocated for the memory tag value.
--! @details This value is defined \i Config.scala and depends of others
--!          configuration paramters, like number of master, clients, channels
--!          and so on. It is not used in VHDL implemenation.
constant MEM_TAG_BITS  : integer := 6;
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
  --! Total number of HostIO initiators.
  constant CFG_HTIF_SRC_TOTAL   : integer := CFG_HTIF_SRC_DSU + 1;
  --! @}

  --! @name    Memory Transaction types.
  --! @details TileLinkIO interface uses these constant to identify the payload
  --!          size of the transaction.
  --! @{
  constant MT_B  : integer := 0;  --! int8_t   Memory Transaction.
  constant MT_H  : integer := 1;  --! int16_t  Memory Transaction.
  constant MT_W  : integer := 2;  --! int32_t  Memory Transaction.
  constant MT_D  : integer := 3;  --! int64_t  Memory Transaction.
  constant MT_BU : integer := 4;  --! uint8_t  Memory Transaction.
  constant MT_HU : integer := 5;  --! uint16_t Memory Transaction.
  constant MT_WU : integer := 6;  --! uint32_t Memory Transaction.
  constant MT_Q  : integer := 7;  --! AXI data-width Memory Transaction (default 128-bits).
  --! @}

  --! @brief Memory operation types
  --! @details The union bits [5:1] contains information about current transaction
  constant M_XRD     : std_logic_vector(4 downto 0) := "00000"; --! int load
  constant M_XWR     : std_logic_vector(4 downto 0) := "00001"; --! int store
  constant M_PFR     : std_logic_vector(4 downto 0) := "00010"; --! prefetch with intent to read
  constant M_PFW     : std_logic_vector(4 downto 0) := "00011"; --! prefetch with intent to write
  constant M_XA_SWAP : std_logic_vector(4 downto 0) := "00100";
  constant M_NOP     : std_logic_vector(4 downto 0) := "00101";
  constant M_XLR     : std_logic_vector(4 downto 0) := "00110";
  constant M_XSC     : std_logic_vector(4 downto 0) := "00111";
  constant M_XA_ADD  : std_logic_vector(4 downto 0) := "01000";
  constant M_XA_XOR  : std_logic_vector(4 downto 0) := "01001";
  constant M_XA_OR   : std_logic_vector(4 downto 0) := "01010";
  constant M_XA_AND  : std_logic_vector(4 downto 0) := "01011";
  constant M_XA_MIN  : std_logic_vector(4 downto 0) := "01100";
  constant M_XA_MAX  : std_logic_vector(4 downto 0) := "01101";
  constant M_XA_MINU : std_logic_vector(4 downto 0) := "01110";
  constant M_XA_MAXU : std_logic_vector(4 downto 0) := "01111";
  constant M_FLUSH   : std_logic_vector(4 downto 0) := "10000"; --! write back dirty data and cede R/W permissions
  constant M_PRODUCE : std_logic_vector(4 downto 0) := "10001"; --! write back dirty data and cede W permissions
  constant M_CLEAN   : std_logic_vector(4 downto 0) := "10011"; --! write back dirty data and retain R/W permissions

  function isAMO(cmd : std_logic_vector(4 downto 0)) return std_logic;
  --def isPrefetch(cmd: UInt) = cmd === M_PFR || cmd === M_PFW
  --def isRead(cmd: UInt) = cmd === M_XRD || cmd === M_XLR || cmd === M_XSC || isAMO(cmd)
  function isWrite(cmd : std_logic_vector(4 downto 0)) return std_logic;
  --def isWriteIntent(cmd: UInt) = isWrite(cmd) || cmd === M_PFW || cmd === M_XLR

  --! <tilelink.scala> Object Acquire {}
  constant ACQUIRE_GET_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "000";
  constant ACQUIRE_GET_BLOCK_DATA       : std_logic_vector(2 downto 0) := "001"; -- 
  constant ACQUIRE_PUT_SINGLE_DATA_BEAT : std_logic_vector(2 downto 0) := "010"; -- Single beat data.
  constant ACQUIRE_PUT_BLOCK_DATA       : std_logic_vector(2 downto 0) := "011"; -- For acMultibeat data.
  constant ACQUIRE_PUT_ATOMIC_DATA      : std_logic_vector(2 downto 0) := "100"; -- Single beat data. 64 bits width
  constant ACQUIRE_PREFETCH_BLOCK       : std_logic_vector(2 downto 0) := "101";
  
  --! <tilelink.scala> Object Grant {}
  constant GRANT_ACK_RELEASE          : std_logic_vector(3 downto 0) := "0000"; -- For acking Releases
  constant GRANT_ACK_PREFETCH         : std_logic_vector(3 downto 0) := "0001"; -- For acking any kind of Prefetch
  constant GRANT_ACK_NON_PREFETCH_PUT : std_logic_vector(3 downto 0) := "0011"; -- For acking any kind of non-prfetch Put
  constant GRANT_SINGLE_BEAT_GET      : std_logic_vector(3 downto 0) := "0100"; -- Supplying a single beat of Get
  constant GRANT_BLOCK_GET            : std_logic_vector(3 downto 0) := "0101"; -- Supplying all beats of a GetBlock

  --! MESI coherence
  constant CACHED_ACQUIRE_SHARED      : std_logic_vector(2 downto 0) := "000"; -- get 
  constant CACHED_ACQUIRE_EXCLUSIVE   : std_logic_vector(2 downto 0) := "001"; -- put

  constant CACHED_GRANT_SHARED        : std_logic_vector(3 downto 0) := "0000";
  constant CACHED_GRANT_EXCLUSIVE     : std_logic_vector(3 downto 0) := "0001";  
  constant CACHED_GRANT_EXCLUSIVE_ACK : std_logic_vector(3 downto 0) := "0010";  

  --! @brief Memory Operation size decoder
  --! @details TileLink bus has encoded Memory Operation size
  --!          in the union[8:6] bits of the acquire request.
  constant MEMOP_XSIZE_TOTAL : integer := 8;
  type memop_xsize_type is array (0 to MEMOP_XSIZE_TOTAL-1) of std_logic_vector(2 downto 0);
  constant opSizeToXSize : memop_xsize_type := (
    MT_B => "000",
    MT_BU => "000",
    MT_H => "001",
    MT_HU => "001",
    MT_W => "010",
    MT_WU => "010", --! unimplemented in scala
    MT_D => "011",
    MT_Q => conv_std_logic_vector(log2(CFG_NASTI_DATA_BYTES),3)
  );


type tile_cached_in_type is record
    acquire_ready : std_logic;
    grant_valid : std_logic;
    grant_bits_addr_beat : std_logic_vector(1 downto 0);
    --! client's transaction id
    grant_bits_client_xact_id : std_logic_vector(1 downto 0);
    grant_bits_manager_xact_id : std_logic_vector(3 downto 0);
    grant_bits_is_builtin_type : std_logic;
    grant_bits_g_type : std_logic_vector(3 downto 0);
    grant_bits_data : std_logic_vector(127 downto 0);
    probe_valid : std_logic;
    probe_bits_addr_block : std_logic_vector(25 downto 0);
    probe_bits_p_type : std_logic_vector(1 downto 0);
    release_ready : std_logic;
end record;

type tile_cached_out_type is record
    acquire_valid : std_logic;
    acquire_bits_addr_block : std_logic_vector(25 downto 0);
    acquire_bits_client_xact_id : std_logic_vector(1 downto 0);
    acquire_bits_addr_beat : std_logic_vector(1 downto 0);
    acquire_bits_is_builtin_type : std_logic;
    acquire_bits_a_type : std_logic_vector(2 downto 0);
    acquire_bits_union : std_logic_vector(16 downto 0);
    acquire_bits_data : std_logic_vector(127 downto 0);
    grant_ready : std_logic;
    probe_ready : std_logic;
    release_valid : std_logic;
    release_bits_addr_beat : std_logic_vector(1 downto 0);
    release_bits_addr_block : std_logic_vector(25 downto 0);
    release_bits_client_xact_id : std_logic_vector(1 downto 0);
    release_bits_r_type : std_logic_vector(2 downto 0);
    release_bits_voluntary : std_logic;
    release_bits_data : std_logic_vector(127 downto 0);
end record;


--! HostIO tile input signals
type host_in_type is record
    grant : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0);
    csr_req_ready : std_logic;
    csr_resp_valid : std_logic;
    csr_resp_bits : std_logic_vector(63 downto 0);
    debug_stats_csr : std_logic;
end record;

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

--! Full stack of HostIO output signals from all devices.
type host_out_vector is array (0 to CFG_HTIF_SRC_TOTAL-1) 
       of host_out_type;

--! @brief   Empty output signals of HostIO interface.
--! @details If device was included in the owners of the HostIO interface and
--!          was disabled by configuration parameter (for example) then its
--!          outputs must be assigned to this empty signals otherwise 
--!          RTL simulation will fail with undefined states of the processor.
constant host_out_none : host_out_type := (
     '0', '0', '0', '0', (others => '0'), (others => '0'), '0');

  --! @brief Decode Acquire request from the Cached/Uncached TileLink
  --! @param[in] a_type   Request type depends of the built_in flag
  --! @param[in] built_in This flag defines cached or uncached request. For
  --!                     the uncached this value is set to 1.
  --! @param[in] u        Union bits. This value is decoding depending of
  --!                     types operation (rd/wr) and cached/uncached.
  procedure procedureDecodeTileAcquire (
    a_type    : in std_logic_vector(2 downto 0);
    built_in  : in std_logic;
    u         : in std_logic_vector(16 downto 0);
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(3 downto 0);
    beat_cnt  : out integer
  );



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
--! @param[in] soft_rst Software Reset via DSU
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
end component;


--! @brief   RocketTile + Uncore component declaration.
--! @details This module implements Risc-V Core with L1-cache, 
--!          branch predictor and other stuffs of the RocketTile.
--! @param[in] xindex1 Cached Tile AXI master index
--! @param[in] xindex2 Uncached Tile AXI master index
--! @param[in] rst     Reset signal with active HIGH level.
--! @param[in] soft_rst Software Reset via DSU
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

  --! @brief   Declaration of the Debug Support Unit with the AXI interface.
  --! @details This module provides access to processors CSRs via HostIO bus.
  component nasti_dsu is
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
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    i_host : in host_in_type;
    o_host : out host_out_type;
    o_soft_reset : out std_logic
  );
  end component;

  --! @brief   General Purpose Timers with the AXI interface.
  --! @details This module provides high precision counter and
  --!          generic number of GP timers.
  component nasti_gptimers is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
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
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0
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

--! -----------------
package body types_rocket is
  
  function isAMO(cmd : std_logic_vector(4 downto 0))
    return std_logic is
    variable t1 : std_logic;
  begin
    t1 := '0';
    if cmd = M_XA_SWAP then
      t1 := '1';
    end if;
    return (cmd(3) or t1);
  end;

  function isWrite(cmd : std_logic_vector(4 downto 0))  
    return std_logic is
    variable ret : std_logic;
  begin
    ret := isAMO(cmd);
    if cmd = M_XWR then ret := '1'; end if;
    if cmd = M_XSC then ret := '1'; end if;
    return (ret);
  end;

  --! @brief Decode Acquire request from the Cached/Uncached TileLink
  --! @param[in] a_type   Request type depends of the built_in flag
  --! @param[in] built_in This flag defines cached or uncached request. For
  --!                     the uncached this value is set to 1.
  --! @param[in] u        Union bits. This value is decoding depending of
  --!                     types operation (rd/wr) and cached/uncached.
  procedure procedureDecodeTileAcquire(
    a_type    : in std_logic_vector(2 downto 0);
    built_in  : in std_logic;
    u         : in std_logic_vector(16 downto 0);
    write     : out std_logic;
    wmask     : out std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    axi_sz    : out std_logic_vector(2 downto 0);
    byte_addr : out std_logic_vector(3 downto 0);
    beat_cnt  : out integer
  ) is
  begin

    if built_in = '1' then
      -- Cached request
      case a_type is
      when ACQUIRE_GET_SINGLE_DATA_BEAT =>
          write := '0';
          wmask := (others => '0');
          byte_addr := u(12 downto 9);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0;
      when ACQUIRE_PREFETCH_BLOCK |
           ACQUIRE_GET_BLOCK_DATA =>
          -- cache line size / data bits width
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 3;--tlDataBeats-1; 
      when ACQUIRE_PUT_SINGLE_DATA_BEAT =>
          -- Single beat data.
          write := '1';
          wmask := u(CFG_NASTI_DATA_BYTES downto 1);
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 0;
      when ACQUIRE_PUT_BLOCK_DATA =>
          -- Multibeat data.
          write := '1';
          wmask := (others => '1');
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 3;--tlDataBeats-1; 
      when ACQUIRE_PUT_ATOMIC_DATA =>
          -- Single beat data. 64 bits width
          write := '1';
          if CFG_NASTI_DATA_BITS = 128 then
              if u(12) = '0' then
                  wmask(7 downto 0) := (others => '1');
                  wmask(15 downto 8) := (others => '0');
              else 
                  wmask(7 downto 0) := (others => '0');
                  wmask(15 downto 8) := (others => '1');
              end if;
          else
              wmask := (others => '1');
          end if;
          byte_addr := (others => '0');
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0; 
      when others =>
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := (others => '0');
          beat_cnt := 0;
      end case;
    else --! built_in = '0'
      -- Cached request
      case a_type is
      when CACHED_ACQUIRE_SHARED =>
          write := '0';
          wmask := (others => '0');
          byte_addr := u(12 downto 9);--tst.block.byte_addr;
          axi_sz := opSizeToXSize(conv_integer(u(8 downto 6)));
          beat_cnt := 0;
      when CACHED_ACQUIRE_EXCLUSIVE =>
          -- Single beat data.
          write := '1';
          wmask := u(CFG_NASTI_DATA_BYTES downto 1);
          byte_addr := (others => '0');
          axi_sz := conv_std_logic_vector(CFG_NASTI_ADDR_OFFSET,3);
          beat_cnt := 0;
      when others =>
          write := '0';
          wmask := (others => '0');
          byte_addr := (others => '0');
          axi_sz := (others => '0');
          beat_cnt := 0;
      end case;
    end if;
  end procedure;

end; -- package body
