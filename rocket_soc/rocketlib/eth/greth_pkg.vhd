------------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2003 - 2008, Gaisler Research
--  Copyright (C) 2008 - 2014, Aeroflex Gaisler
--  Copyright (C) 2015 - 2016, Cobham Gaisler
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

package grethpkg is
  --gigabit sync types
  type data_sync_type is array (0 to 3) of std_logic_vector(31 downto 0);
  type ctrl_sync_type is array (0 to 3) of std_logic_vector(1 downto 0);

  constant HTRANS_IDLE:   std_logic_vector(1 downto 0) := "00";
  constant HTRANS_NONSEQ: std_logic_vector(1 downto 0) := "10";
  constant HTRANS_SEQ:    std_logic_vector(1 downto 0) := "11";

  constant HBURST_INCR:   std_logic_vector(2 downto 0) := "001";

  constant HSIZE_WORD:    std_logic_vector(2 downto 0) := "010";

  constant HRESP_OKAY:    std_logic_vector(1 downto 0) := "00";
  constant HRESP_ERROR:   std_logic_vector(1 downto 0) := "01";
  constant HRESP_RETRY:   std_logic_vector(1 downto 0) := "10";
  constant HRESP_SPLIT:   std_logic_vector(1 downto 0) := "11";

  --receiver constants
  constant maxsizerx : std_logic_vector(15 downto 0) :=
    conv_std_logic_vector(1500, 16);
  constant minpload  : std_logic_vector(10 downto 0) :=
    conv_std_logic_vector(60, 11);

  type ahb_fifo_in_type is record
    renable   : std_ulogic;
    raddress  : std_logic_vector(4 downto 0);
    write     : std_ulogic;
    data      : std_logic_vector(31 downto 0);
    waddress  : std_logic_vector(4 downto 0);
  end record;

  type ahb_fifo_out_type is record
    data      : std_logic_vector(31 downto 0);
  end record;

  type nchar_fifo_in_type is record
    renable   : std_ulogic;
    raddress  : std_logic_vector(5 downto 0);
    write     : std_ulogic;
    data      : std_logic_vector(8 downto 0);
    waddress  : std_logic_vector(5 downto 0);
  end record;

  type nchar_fifo_out_type is record
    data      : std_logic_vector(8 downto 0);
  end record;

  type rmapbuf_in_type is record
    renable   : std_ulogic;
    raddress  : std_logic_vector(7 downto 0);
    write     : std_ulogic;
    data      : std_logic_vector(7 downto 0);
    waddress  : std_logic_vector(7 downto 0);
  end record;

  type rmapbuf_out_type is record
    data      : std_logic_vector(7 downto 0);
  end record;

  type ahbc_mst_in_type is record
    hgrant	: std_ulogic;                           -- bus grant
    hready	: std_ulogic;                         	-- transfer done
    hresp	: std_logic_vector(1 downto 0); 	-- response type
    hrdata	: std_logic_vector(31 downto 0); 	-- read data bus
  end record;

  type ahbc_mst_out_type is record
    hbusreq	: std_ulogic;                         	-- bus request
    hlock	: std_ulogic;                         	-- lock request
    htrans	: std_logic_vector(1 downto 0); 	-- transfer type
    haddr	: std_logic_vector(31 downto 0); 	-- address bus (byte)
    hwrite	: std_ulogic;                         	-- read/write
    hsize	: std_logic_vector(2 downto 0); 	-- transfer size
    hburst	: std_logic_vector(2 downto 0); 	-- burst type
    hprot	: std_logic_vector(3 downto 0); 	-- protection control
    hwdata	: std_logic_vector(31 downto 0); 	-- write data bus
  end record;

  type apbc_slv_in_type is record
    psel	: std_ulogic;                           -- slave select
    penable	: std_ulogic;                         	-- strobe
    paddr	: std_logic_vector(31 downto 0); 	-- address bus (byte)
    pwrite	: std_ulogic;                         	-- write
    pwdata	: std_logic_vector(31 downto 0); 	-- write data bus
  end record;

  type apbc_slv_out_type is record
    prdata	: std_logic_vector(31 downto 0); 	-- read data bus
  end record;

  type eth_tx_ahb_in_type is record
    req     : std_ulogic;
    write   : std_ulogic;
    addr    : std_logic_vector(31 downto 0);
    data    : std_logic_vector(31 downto 0);
    burst_bytes : std_logic_vector(10 downto 0);
  end record;

  type eth_tx_ahb_out_type is record
    grant    : std_ulogic;
    data     : std_logic_vector(31 downto 0);
    ready    : std_ulogic;
    error    : std_ulogic;
    retry    : std_ulogic;
  end record;

  type eth_rx_ahb_in_type is record
    req     : std_ulogic;
    write   : std_ulogic;
    addr    : std_logic_vector(31 downto 0);
    data    : std_logic_vector(31 downto 0);
    burst_bytes : std_logic_vector(10 downto 0);
  end record;
  
  constant eth_rx_in_none : eth_rx_ahb_in_type := (
    '0', '0', (others => '0'), (others => '0'), (others => '0'));

  type eth_rx_ahb_out_type is record
    grant   : std_ulogic;
    ready   : std_ulogic;
    error   : std_ulogic;
    retry   : std_ulogic;
    data    : std_logic_vector(31 downto 0);
  end record;

  type eth_rx_gbit_ahb_in_type is record
    req     : std_ulogic;
    write   : std_ulogic;
    addr    : std_logic_vector(31 downto 0);
    data    : std_logic_vector(31 downto 0);
    size    : std_logic_vector(1 downto 0);
  end record;

  type gbit_host_tx_type is record
    full_duplex : std_ulogic;
    start       : std_ulogic;
    read_ack    : std_ulogic;
    data        : std_logic_vector(31 downto 0);
    datavalid   : std_ulogic;
    valid       : std_ulogic;
    len         : std_logic_vector(10 downto 0);
    rx_col      : std_ulogic;
    rx_crs      : std_ulogic;
  end record;

  type gbit_tx_host_type is record
    txd          : std_logic_vector(3 downto 0);
    tx_en        : std_ulogic;
    done         : std_ulogic;
    read         : std_ulogic;
    restart      : std_ulogic;
    status       : std_logic_vector(1 downto 0);
  end record;

  type gbit_rx_host_type is record
    sync_start   : std_ulogic;
    done         : std_ulogic;
    write        : std_logic_vector(3 downto 0);
    dataout      : data_sync_type;
    byte_count   : std_logic_vector(10 downto 0);
    status       : std_logic_vector(3 downto 0);
    gotframe     : std_ulogic;
    mcasthash    : std_logic_vector(5 downto 0);
  end record;

  type gbit_host_rx_type is record
    full_duplex  : std_ulogic;
    gbit         : std_ulogic;
    doneack      : std_ulogic;
    writeack     : std_logic_vector(3 downto 0);
    speed        : std_ulogic;
    writeok      : std_logic_vector(3 downto 0);
    rxenable     : std_ulogic;
    rxd          : std_logic_vector(7 downto 0);
    rx_dv        : std_ulogic;
    rx_er        : std_ulogic;
    rx_col       : std_ulogic;
    rx_crs       : std_ulogic;
    rx_en        : std_ulogic;
  end record;

  type gbit_gtx_host_type is record
    txd          : std_logic_vector(7 downto 0);
    tx_en        : std_ulogic;
    tx_er        : std_ulogic;
    done         : std_ulogic;
    restart      : std_ulogic;
    read         : std_logic_vector(3 downto 0);
    status       : std_logic_vector(2 downto 0);
  end record;

  type gbit_host_gtx_type is record
    rx_col        : std_ulogic;
    rx_crs        : std_ulogic;
    full_duplex   : std_ulogic;
    burstmode     : std_ulogic;
    txen          : std_ulogic;
    start_sync    : std_ulogic;
    readack       : std_logic_vector(3 downto 0);
    valid         : std_logic_vector(3 downto 0);
    data          : data_sync_type;
    len           : std_logic_vector(10 downto 0);
  end record;

  type host_tx_type is record
    rx_col      : std_ulogic;
    rx_crs      : std_ulogic;
    full_duplex : std_ulogic;
    start       : std_ulogic;
    readack     : std_ulogic;
    speed       : std_ulogic;
    data        : std_logic_vector(31 downto 0);
    datavalid   : std_ulogic;
    valid       : std_ulogic;
    len         : std_logic_vector(10 downto 0);
  end record;

  type tx_host_type is record
    txd         : std_logic_vector(3 downto 0);
    tx_en       : std_ulogic;
    tx_er       : std_ulogic;
    done        : std_ulogic;
    read        : std_ulogic;
    restart     : std_ulogic;
    status      : std_logic_vector(1 downto 0);
  end record;

  type rx_host_type is record
    dataout    : std_logic_vector(31 downto 0);
    start      : std_ulogic;
    done       : std_ulogic;
    write      : std_ulogic;
    status     : std_logic_vector(3 downto 0);
    gotframe   : std_ulogic;
    byte_count : std_logic_vector(10 downto 0);
    lentype    : std_logic_vector(15 downto 0);
    mcasthash  : std_logic_vector(5 downto 0);
  end record;

  type host_rx_type is record
    writeack : std_ulogic;
    doneack  : std_ulogic;
    speed    : std_ulogic;
    writeok  : std_ulogic;
    rxd      : std_logic_vector(3 downto 0);
    rx_dv    : std_ulogic;
    rx_crs   : std_ulogic;
    rx_er    : std_ulogic;
    enable   : std_ulogic;
    rx_en    : std_ulogic;
  end record;
 

  component greth_rx is
    generic(
      nsync          : integer range 1 to 2 := 2;
      rmii           : integer range 0 to 1 := 0;
      multicast      : integer range 0 to 1 := 0;
      maxsize        : integer;
      gmiimode       : integer range 0 to 1 := 0
      );
    port(
      rst            : in  std_ulogic;
      clk            : in  std_ulogic;
      rxi            : in  host_rx_type;
      rxo            : out rx_host_type
    );
  end component;

  component greth_tx is
    generic(
      ifg_gap        : integer := 24;
      attempt_limit  : integer := 16;
      backoff_limit  : integer := 10;
      nsync          : integer range 1 to 2 := 2;
      rmii           : integer range 0 to 1  := 0;
      gmiimode       : integer range 0 to 1 := 0
      );
    port(
      rst            : in  std_ulogic;
      clk            : in  std_ulogic;
      txi            : in  host_tx_type;
      txo            : out tx_host_type
    );
  end component;

  component eth_rstgen is
    generic(acthigh : integer := 0);
    port (
      rstin     : in  std_ulogic;
      clk       : in  std_ulogic;
      clklock   : in  std_ulogic;
      rstout    : out std_ulogic;
      rstoutraw : out std_ulogic
    );
  end component;

  component greth_gbit_tx is
    generic(
      ifg_gap        : integer := 24;
      attempt_limit  : integer := 16;
      backoff_limit  : integer := 10;
      nsync          : integer range 1 to 2 := 2;
      gmiimode       : integer range 0 to 1 := 0
      );
    port(
      rst            : in  std_ulogic;
      clk            : in  std_ulogic;
      txi            : in  gbit_host_tx_type;
      txo            : out gbit_tx_host_type);
  end component;

  component greth_gbit_gtx is
    generic(
      ifg_gap        : integer := 24;
      attempt_limit  : integer := 16;
      backoff_limit  : integer := 10;
      nsync          : integer range 1 to 2 := 2;
      iotest         : integer := 0);
    port(
      rst            : in   std_ulogic;
      clk            : in   std_ulogic;
      gtxi           : in   gbit_host_gtx_type;
      gtxo           : out  gbit_gtx_host_type;
      iotmact        : in   std_ulogic;
      iotdata        : in   std_logic_vector(9 downto 0)
    );
  end component;

  component greth_gbit_rx is
    generic(
      multicast      : integer range 0 to 1 := 0;
      nsync          : integer range 1 to 2 := 2;
      gmiimode       : integer range 0 to 1 := 0
      );
    port(
      rst            : in  std_ulogic;
      clk            : in  std_ulogic;
      rxi            : in  gbit_host_rx_type;
      rxo            : out gbit_rx_host_type;
      iotdata        : out std_logic_vector(9 downto 0));
  end component;

  component eth_ahb_mst is
    port(
      rst     : in  std_ulogic;
      clk     : in  std_ulogic;
      ahbmi   : in  ahbc_mst_in_type;
      ahbmo   : out ahbc_mst_out_type;
      tmsti   : in  eth_tx_ahb_in_type;
      tmsto   : out eth_tx_ahb_out_type;
      rmsti   : in  eth_rx_ahb_in_type;
      rmsto   : out eth_rx_ahb_out_type
    );
  end component;

  component eth_ahb_mst_gbit is
    port(
      rst         : in  std_ulogic;
      clk         : in  std_ulogic;
      ahbmi       : in  ahbc_mst_in_type;
      ahbmo       : out ahbc_mst_out_type;
      tmsti       : in  eth_tx_ahb_in_type;
      tmsto       : out eth_tx_ahb_out_type;
      rmsti       : in  eth_rx_gbit_ahb_in_type;
      rmsto       : out eth_rx_ahb_out_type);
  end component;

  component eth_edcl_ahb_mst is
  port(
    rst         : in  std_ulogic;
    clk         : in  std_ulogic;
    ahbmi       : in  ahbc_mst_in_type;
    ahbmo       : out ahbc_mst_out_type;
    tmsti       : in  eth_tx_ahb_in_type;
    tmsto       : out eth_tx_ahb_out_type
  );
  end component;
  
  component eth_axi_mst is
  generic (
    xindex : integer := 0
  );
  port(
    rst     : in  std_ulogic;
    clk     : in  std_ulogic;
    aximi   : in  nasti_master_in_type;
    aximo   : out nasti_master_out_type;
    tmsti   : in  eth_tx_ahb_in_type;
    tmsto   : out eth_tx_ahb_out_type;
    rmsti   : in  eth_rx_ahb_in_type;
    rmsto   : out eth_rx_ahb_out_type
  );
  end component;


  function mirror(din : in std_logic_vector) return std_logic_vector;

  function crc32_4(d   : in std_logic_vector(3 downto 0);
                   crc : in std_logic_vector(31 downto 0))
                         return std_logic_vector;
  function crc16_2(d1   : in std_logic_vector(15 downto 0);
                   d2   : in std_logic_vector(25 downto 0))
                          return std_logic_vector;
  function crc16(d1   : in std_logic_vector(15 downto 0);
                 d2   : in std_logic_vector(15 downto 0))
                        return std_logic_vector;

  function validlen(len   : in std_logic_vector(10 downto 0);
                    bcnt  : in std_logic_vector(10 downto 0);
                    usesz : in std_ulogic)
                            return std_ulogic;

  function getfifosize(edcl, fifosize, ebufsize : in integer) return integer;

  function setburstlength(fifosize : in integer) return integer;

  function calccrc(d   : in std_logic_vector(3 downto 0);
                   crc : in std_logic_vector(31 downto 0))
                         return std_logic_vector;

  --16-bit one's complement adder
  function crcadder(d1   : in std_logic_vector(15 downto 0);
                    d2   : in std_logic_vector(17 downto 0))
                         return std_logic_vector;


  -- ETH registers

  type eth_mdio_command_type is record
    valid    : std_ulogic;
    regadr   : std_logic_vector(4 downto 0);
    write    : std_ulogic;
    read     : std_ulogic;
    data     : std_logic_vector(15 downto 0);
  end record;
  constant eth_mdio_command_none : eth_mdio_command_type :=  (
    '0', (others => '0'), '0', '0', (others => '0')
  );

  type eth_mdio_status_type is record
    cmd      : eth_mdio_command_type;
    busy     : std_ulogic;
    linkfail : std_ulogic;
  end record;

  type eth_mac_status_type is record
    txdsel      : std_logic_vector(9 downto 3);
    rxdsel      : std_logic_vector(9 downto 3);
    txen        : std_ulogic;
    rxen        : std_ulogic;
    tx_int      : std_ulogic;
    rx_int      : std_ulogic;
    tx_err      : std_ulogic;
    rx_err      : std_ulogic;
    edcltx_idle : std_ulogic;
    edclrx_idle : std_ulogic;
    txahberr    : std_ulogic;
    rxahberr    : std_ulogic;
    toosmall    : std_ulogic;
    invaddr     : std_ulogic;
    phystat     : std_ulogic;
    full_duplex : std_ulogic; 
    speed       : std_ulogic;
    reset       : std_ulogic;
    mdio        : eth_mdio_status_type;
  end record;

  --! Latched values set via external Bus Interface
  type eth_control_type is record
    tx_irqen    : std_ulogic;
    rx_irqen    : std_ulogic;
    prom        : std_ulogic;
    pstatirqen  : std_ulogic;
    mcasten     : std_ulogic;
    --! Enable access to the internal FIFOs via system BUS (disabled default)
    ramdebugen  : std_ulogic;
    --! Disable EDCL access
    edcldis     : std_ulogic;
    disableduplex : std_ulogic;
    --! Physical address.
    --! Can be changed in a runtime, but become actual only after system reset.
    mdio_phyadr   : std_logic_vector(4 downto 0);

    mac_addr : std_logic_vector(47 downto 0);
    --! Tx descriptor
    txdesc : std_logic_vector(31 downto 10);
    --! Rx descriptor
    rxdesc : std_logic_vector(31 downto 10);
    --! EDCL IP
    edclip : std_logic_vector(31 downto 0);
    --! Multicast enabling hash value
    hash : std_logic_vector(63 downto 0);
    emacaddr : std_logic_vector(47 downto 0);
  end record;

  --! @name   DBG access unique IDs to the internal FIFOs blocks.
  --! @{
  constant DBG_ACCESS_NONE : std_logic_vector(1 downto 0) := "00";
  constant DBG_ACCESS_TX_BUFFER : std_logic_vector(1 downto 0) := "01";
  constant DBG_ACCESS_RX_BUFFER : std_logic_vector(1 downto 0) := "10";
  constant DBG_ACCESS_EDCL_BUFFER : std_logic_vector(1 downto 0) := "11";
  --! @}

  --! Bus interface read/write actions transforming into these commands.
  type eth_command_type is record
    --! Tx/Rx can be enabled externally but they're cleared inside of MAC
    --! in a case of disabled Descriptor or in a case of BUS error.
    set_txena : std_ulogic;
    clr_txena : std_ulogic;
    set_rxena : std_ulogic;
    clr_rxena : std_ulogic;
    --! Set new descriptor index in the array of descriptors table
    set_txdsel : std_ulogic;
    set_rxdsel : std_ulogic;
    txdsel     : std_logic_vector(9 downto 3);
    rxdsel     : std_logic_vector(9 downto 3);
    --! The following values can be changed during initialization stage.
    set_full_duplex : std_ulogic; 
    clr_full_duplex : std_ulogic; 
    set_speed       : std_ulogic;
    clr_speed       : std_ulogic;
    set_reset       : std_ulogic;
    clr_reset       : std_ulogic;
    --! Clear status bits commands:
    clr_status_tx_int      : std_ulogic;
    clr_status_rx_int      : std_ulogic;
    clr_status_tx_err      : std_ulogic;
    clr_status_rx_err      : std_ulogic;
    clr_status_txahberr    : std_ulogic;
    clr_status_rxahberr    : std_ulogic;
    clr_status_toosmall    : std_ulogic;
    clr_status_invaddr     : std_ulogic;
    clr_status_phystat     : std_ulogic;
    --! mdi interface command
    mdio_cmd               : eth_mdio_command_type;
    --! Request ID values:
    dbg_access_id : std_logic_vector(1 downto 0);
    dbg_wr_ena : std_logic;
    dbg_rd_ena : std_logic;
    dbg_addr   : std_logic_vector(13 downto 0);
    dbg_wdata  : std_logic_vector(31 downto 0);
  end record;
  
  constant eth_command_none : eth_command_type := (
    '0', '0', '0', '0', '0', '0', (others => '0'), (others => '0'),
    '0', '0', '0', '0', '0', '0', 
    '0', '0', '0', '0', '0', '0', '0', '0', '0', eth_mdio_command_none,
    DBG_ACCESS_NONE, '0', '0', (others => '0'), (others => '0')
  );
  
  type eth_in_type is record
    gtx_clk        : std_ulogic;                     
    rmii_clk       : std_ulogic;
    tx_clk         : std_ulogic;
    tx_clk_90  : std_ulogic;
    rx_clk         : std_ulogic;
    tx_dv          : std_ulogic;
    rxd            : std_logic_vector(3 downto 0);   
    rx_dv          : std_ulogic; 
    rx_er          : std_ulogic; 
    rx_col         : std_ulogic;
    rx_en          : std_ulogic;
    rx_crs         : std_ulogic;
    mdio_i         : std_ulogic;
    mdint          : std_ulogic;
    phyrstaddr     : std_logic_vector(4 downto 0);
    edcladdr       : std_logic_vector(3 downto 0);
    edclsepahb     : std_ulogic;
    edcldisable    : std_ulogic;
  end record;

  constant eth_in_none : eth_in_type := (
    '0', '0', '0', '0', '0', '0', (others => '0'), '0', '0', '0', '0', '0',
     '0', '0', (others => '0'), (others => '0'), '0', '0');

  
  type eth_out_type is record
    reset          : std_ulogic;
    txd            : std_logic_vector(3 downto 0);   
    tx_en          : std_ulogic; 
    tx_er          : std_ulogic;
    tx_clk         : std_ulogic;  
    mdc            : std_ulogic;    
    mdio_o         : std_ulogic; 
    mdio_oe        : std_ulogic;
    gbit          : std_ulogic;
    speed          : std_ulogic;
  end record;

  constant eth_out_none : eth_out_type := (
    '0', (others => '0'), '0', '0', '0', '0', '0', '1', '0', '0');

  component grethc64 is
  generic(
    memtech        : integer := 0;
    ifg_gap        : integer := 24; 
    attempt_limit  : integer := 16;
    backoff_limit  : integer := 10;
    mdcscaler      : integer range 0 to 255 := 25; 
    enable_mdio    : integer range 0 to 1 := 0;
    fifosize       : integer range 4 to 512 := 8;
    nsync          : integer range 1 to 2 := 2;
    edcl           : integer range 0 to 3 := 0;
    edclbufsz      : integer range 1 to 64 := 1;
    macaddrh       : integer := 16#00005E#;
    macaddrl       : integer := 16#000000#;
    ipaddrh        : integer := 16#c0a8#;
    ipaddrl        : integer := 16#0035#;
    phyrstadr      : integer range 0 to 32 := 0;
    rmii           : integer range 0 to 1  := 0;
    oepol          : integer range 0 to 1  := 0; 
    scanen         : integer range 0 to 1  := 0;
    mdint_pol      : integer range 0 to 1  := 0;
    enable_mdint   : integer range 0 to 1  := 0;
    multicast      : integer range 0 to 1  := 0;
    edclsepahbg    : integer range 0 to 1  := 0;
    ramdebug       : integer range 0 to 2  := 0;
    mdiohold       : integer := 1;
    maxsize        : integer := 1500;
    gmiimode       : integer range 0 to 1 := 0
    );
  port(
    rst           : in  std_ulogic;
    clk           : in  std_ulogic;
    ctrli         : in eth_control_type;
    cmdi          : in eth_command_type;
    statuso       : out eth_mac_status_type;
    --! Debug value read from internal buffers suing external bus interface
    rdbgdatao     : out  std_logic_vector(31 downto 0);
    --irq
    irq            : out  std_logic;
    --ethernet input signals
    rmii_clk       : in   std_ulogic;
    tx_clk         : in   std_ulogic;
    rx_clk         : in   std_ulogic;
    tx_dv          : in   std_ulogic;
    rxd            : in   std_logic_vector(3 downto 0);
    rx_dv          : in   std_ulogic; 
    rx_er          : in   std_ulogic; 
    rx_col         : in   std_ulogic;
    rx_en          : in   std_ulogic;
    rx_crs         : in   std_ulogic;
    mdio_i         : in   std_ulogic;
    phyrstaddr     : in   std_logic_vector(4 downto 0);
    mdint          : in   std_ulogic; 
    --ethernet output signals
    reset          : out  std_ulogic;
    txd            : out  std_logic_vector(3 downto 0);   
    tx_en          : out  std_ulogic; 
    tx_er          : out  std_ulogic; 
    mdc            : out  std_ulogic;    
    mdio_o         : out  std_ulogic; 
    mdio_oe        : out  std_ulogic;
    --scantest
    testrst        : in   std_ulogic;
    testen         : in   std_ulogic;
    testoen        : in   std_ulogic;
    edcladdr       : in   std_logic_vector(3 downto 0) := "0000";
    edclsepahb     : in   std_ulogic;
    edcldisable    : in   std_ulogic;
    speed          : out  std_ulogic;
    tmsto          : out eth_tx_ahb_in_type;
    tmsti          : in eth_tx_ahb_out_type;
    tmsto2         : out eth_tx_ahb_in_type;
    tmsti2         : in eth_tx_ahb_out_type;
    rmsto          : out eth_rx_ahb_in_type;
    rmsti          : in eth_rx_ahb_out_type
  );
  end component;

  component grethaxi is
  generic(
    xslvindex      : integer := 0;
    xmstindex      : integer := 0;
    xmstindex2     : integer := 1;
    xaddr          : integer := 0;
    xmask          : integer := 16#FFFFF#;
    xirq           : integer := 0;
    memtech        : integer := 0; 
    ifg_gap        : integer := 24; 
    attempt_limit  : integer := 16;
    backoff_limit  : integer := 10;
    slot_time      : integer := 128;
    mdcscaler      : integer range 0 to 255 := 25; 
    enable_mdio    : integer range 0 to 1 := 0;
    fifosize       : integer range 4 to 512 := 8;
    nsync          : integer range 1 to 2 := 2;
    edcl           : integer range 0 to 3 := 0;
    edclbufsz      : integer range 1 to 64 := 1;
    macaddrh       : integer := 16#00005E#;
    macaddrl       : integer := 16#000000#;
    ipaddrh        : integer := 16#c0a8#;
    ipaddrl        : integer := 16#0035#;
    phyrstadr      : integer range 0 to 32 := 0;
    rmii           : integer range 0 to 1  := 0;
    oepol          : integer range 0 to 1  := 0; 
    scanen         : integer range 0 to 1  := 0;
    ft             : integer range 0 to 2  := 0;
    edclft         : integer range 0 to 2  := 0;
    mdint_pol      : integer range 0 to 1  := 0;
    enable_mdint   : integer range 0 to 1  := 0;
    multicast      : integer range 0 to 1  := 0;
    edclsepahbg    : integer range 0 to 1  := 0;
    ramdebug       : integer range 0 to 2  := 0;
    mdiohold       : integer := 1;
    maxsize        : integer := 1500;
    gmiimode       : integer range 0 to 1 := 0
    );
  port(
    rst            : in  std_ulogic;
    clk            : in  std_ulogic;
    msti           : in nasti_master_in_type;
    msto           : out nasti_master_out_type;
    mstcfg         : out nasti_master_config_type;
    msto2          : out nasti_master_out_type;
    mstcfg2        : out nasti_master_config_type;
    slvi           : in nasti_slave_in_type;
    slvo           : out nasti_slave_out_type;
    slvcfg         : out nasti_slave_config_type;
    ethi           : in eth_in_type;
    etho           : out eth_out_type;
    irq            : out  std_logic
  );
  end component;

end package;

package body grethpkg is

  function mirror(din : in std_logic_vector)
                        return std_logic_vector is
    variable do : std_logic_vector(din'range);
  begin
    for i in 0 to din'length-1 loop
      do(din'high-i) := din(i+din'low);
    end loop;
    return do;
  end function;

  function crc32_4(d   : in std_logic_vector(3 downto 0);
                   crc : in std_logic_vector(31 downto 0))
                         return std_logic_vector is
    variable ncrc : std_logic_vector(31 downto 0);
    variable tc   : std_logic_vector(3 downto 0);
  begin
    tc(0) := d(0) xor crc(31); tc(1) := d(1) xor crc(30);
    tc(2) := d(2) xor crc(29); tc(3) := d(3) xor crc(28);
    ncrc(31) := crc(27);
    ncrc(30) := crc(26);
    ncrc(29) := tc(0) xor crc(25);
    ncrc(28) := tc(1) xor crc(24);
    ncrc(27) := tc(2) xor crc(23);
    ncrc(26) := tc(0) xor tc(3) xor crc(22);
    ncrc(25) := tc(0) xor tc(1) xor crc(21);
    ncrc(24) := tc(1) xor tc(2) xor crc(20);
    ncrc(23) := tc(2) xor tc(3) xor crc(19);
    ncrc(22) := tc(3) xor crc(18);
    ncrc(21) := crc(17);
    ncrc(20) := crc(16);
    ncrc(19) := tc(0) xor crc(15);
    ncrc(18) := tc(1) xor crc(14);
    ncrc(17) := tc(2) xor crc(13);
    ncrc(16) := tc(3) xor crc(12);
    ncrc(15) := tc(0) xor crc(11);
    ncrc(14) := tc(0) xor tc(1) xor crc(10);
    ncrc(13) := tc(0) xor tc(1) xor tc(2) xor crc(9);
    ncrc(12) := tc(1) xor tc(2) xor tc(3) xor crc(8);
    ncrc(11) := tc(0) xor tc(2) xor tc(3) xor crc(7);
    ncrc(10) := tc(0) xor tc(1) xor tc(3) xor crc(6);
    ncrc(9)  := tc(1) xor tc(2) xor crc(5);
    ncrc(8)  := tc(0) xor tc(2) xor tc(3) xor crc(4);
    ncrc(7)  := tc(0) xor tc(1) xor tc(3) xor crc(3);
    ncrc(6)  := tc(1) xor tc(2) xor crc(2);
    ncrc(5)  := tc(0) xor tc(2) xor tc(3) xor crc(1);
    ncrc(4)  := tc(0) xor tc(1) xor tc(3) xor crc(0);
    ncrc(3)  := tc(0) xor tc(1) xor tc(2);
    ncrc(2)  := tc(1) xor tc(2) xor tc(3);
    ncrc(1)  := tc(2) xor tc(3);
    ncrc(0)  := tc(3);
    return ncrc;
  end function;

  --16-bit one's complement adder
  function crc16(d1   : in std_logic_vector(15 downto 0);
                 d2   : in std_logic_vector(15 downto 0))
                        return std_logic_vector is
    variable vd1  : std_logic_vector(16 downto 0);
    variable vd2  : std_logic_vector(16 downto 0);
    variable sum  : std_logic_vector(16 downto 0);
  begin
    vd1 := '0' & d1; vd2 := '0' & d2;
    sum := vd1 + vd2;
    sum(15 downto 0) := sum(15 downto 0) + sum(16);
    return sum(15 downto 0);
  end function;

  --16-bit one's complement adder for ip/tcp checksum detection
  function crc16_2(d1   : in std_logic_vector(15 downto 0);
                   d2   : in std_logic_vector(25 downto 0))
                          return std_logic_vector is
    variable vd1  : std_logic_vector(25 downto 0);
    variable vd2  : std_logic_vector(25 downto 0);
    variable sum  : std_logic_vector(25 downto 0);
  begin
    vd1 := "0000000000" & d1; vd2 := d2;
    sum := vd1 + vd2;
    return sum;
  end function;

  function validlen(len   : in std_logic_vector(10 downto 0);
                    bcnt  : in std_logic_vector(10 downto 0);
                    usesz : in std_ulogic)
                            return std_ulogic is
    variable valid : std_ulogic;
  begin
    valid := '1';
    if usesz = '1' then
      if len > minpload then
        if bcnt /= len then
          valid := '0';
        end if;
      else
        if bcnt /= minpload then
          valid := '0';
        end if;
      end if;
    end if;
    return valid;
  end function;

  function setburstlength(fifosize : in integer) return integer is
  begin
    if fifosize <= 64 then
      return fifosize/2;
    else
      return 32;
    end if;
  end function;

  function getfifosize(edcl, fifosize, ebufsize : in integer) return integer is
  begin
    if (edcl /= 0) and (ebufsize > fifosize) then
      return ebufsize;
    else
      return fifosize;
    end if;
  end function;

  function calccrc(d   : in std_logic_vector(3 downto 0);
                   crc : in std_logic_vector(31 downto 0))
                         return std_logic_vector is
    variable ncrc : std_logic_vector(31 downto 0);
    variable tc   : std_logic_vector(3 downto 0);
  begin
    tc(0) := d(0) xor crc(31); tc(1) := d(1) xor crc(30);
    tc(2) := d(2) xor crc(29); tc(3) := d(3) xor crc(28);
    ncrc(31) := crc(27);
    ncrc(30) := crc(26);
    ncrc(29) := tc(0) xor crc(25);
    ncrc(28) := tc(1) xor crc(24);
    ncrc(27) := tc(2) xor crc(23);
    ncrc(26) := tc(0) xor tc(3) xor crc(22);
    ncrc(25) := tc(0) xor tc(1) xor crc(21);
    ncrc(24) := tc(1) xor tc(2) xor crc(20);
    ncrc(23) := tc(2) xor tc(3) xor crc(19);
    ncrc(22) := tc(3) xor crc(18);
    ncrc(21) := crc(17);
    ncrc(20) := crc(16);
    ncrc(19) := tc(0) xor crc(15);
    ncrc(18) := tc(1) xor crc(14);
    ncrc(17) := tc(2) xor crc(13);
    ncrc(16) := tc(3) xor crc(12);
    ncrc(15) := tc(0) xor crc(11);
    ncrc(14) := tc(0) xor tc(1) xor crc(10);
    ncrc(13) := tc(0) xor tc(1) xor tc(2) xor crc(9);
    ncrc(12) := tc(1) xor tc(2) xor tc(3) xor crc(8);
    ncrc(11) := tc(0) xor tc(2) xor tc(3) xor crc(7);
    ncrc(10) := tc(0) xor tc(1) xor tc(3) xor crc(6);
    ncrc(9)  := tc(1) xor tc(2) xor crc(5);
    ncrc(8)  := tc(0) xor tc(2) xor tc(3) xor crc(4);
    ncrc(7)  := tc(0) xor tc(1) xor tc(3) xor crc(3);
    ncrc(6)  := tc(1) xor tc(2) xor crc(2);
    ncrc(5)  := tc(0) xor tc(2) xor tc(3) xor crc(1);
    ncrc(4)  := tc(0) xor tc(1) xor tc(3) xor crc(0);
    ncrc(3)  := tc(0) xor tc(1) xor tc(2);
    ncrc(2)  := tc(1) xor tc(2) xor tc(3);
    ncrc(1)  := tc(2) xor tc(3);
    ncrc(0)  := tc(3);
    return ncrc;
  end function;


  --function calccrc_8(data : in std_logic_vector( 7 downto 0);
  --                   crc  : in std_logic_vector(31 downto 0))
  --                         return std_logic_vector is
  --  variable ncrc : std_logic_vector(31 downto 0);
  --  variable d    : std_logic_vector(7 downto 0);
  --begin
  --  d(7) := data(0); d(6) := data(1); d(5) := data(2); d(4) := data(3);
  --  d(3) := data(4); d(2) := data(5); d(1) := data(6); d(0) := data(7);
  --  ncrc(0) := d(6) xor d(0) xor crc(24) xor crc(30);
  --  ncrc(1) := d(7) xor d(6) xor d(1) xor d(0) xor crc(24) xor crc(25) xor crc(30) xor crc(31);
  --  ncrc(2) := d(7) xor d(6) xor d(2) xor d(1) xor d(0) xor crc(24) xor crc(25) xor crc(26) xor crc(30) xor crc(31);
  --  ncrc(3) := d(7) xor d(3) xor d(2) xor d(1) xor crc(25) xor crc(26) xor crc(27) xor crc(31);
  --  ncrc(4) := d(6) xor d(4) xor d(3) xor d(2) xor d(0) xor crc(24) xor crc(26) xor crc(27) xor crc(28) xor crc(30);
  --  ncrc(5) := d(7) xor d(6) xor d(5) xor d(4) xor d(3) xor d(1) xor d(0) xor crc(24) xor crc(25) xor crc(27) xor crc(28) xor crc(29) xor crc(30) xor crc(31);
  --  ncrc(6) := d(7) xor d(6) xor d(5) xor d(4) xor d(2) xor d(1) xor crc(25) xor crc(26) xor crc(28) xor crc(29) xor crc(30) xor crc(31);
  --  ncrc(7) := d(7) xor d(5) xor d(3) xor d(2) xor d(0) xor crc(24) xor crc(26) xor crc(27) xor crc(29) xor crc(31);
  --  ncrc(8) := d(4) xor d(3) xor d(1) xor d(0) xor crc(0) xor crc(24) xor crc(25) xor crc(27) xor crc(28);
  --  ncrc(9) := d(5) xor d(4) xor d(2) xor d(1) xor crc(1) xor crc(25) xor crc(26) xor crc(28) xor crc(29);
  --  ncrc(10) := d(5) xor d(3) xor d(2) xor d(0) xor crc(2) xor crc(24) xor crc(26) xor crc(27) xor crc(29);
  --  ncrc(11) := d(4) xor d(3) xor d(1) xor d(0) xor crc(3) xor crc(24) xor crc(25) xor crc(27) xor crc(28);
  --  ncrc(12) := d(6) xor d(5) xor d(4) xor d(2) xor d(1) xor d(0) xor crc(4) xor crc(24) xor crc(25) xor crc(26) xor crc(28) xor crc(29) xor crc(30);
  --  ncrc(13) := d(7) xor d(6) xor d(5) xor d(3) xor d(2) xor d(1) xor crc(5) xor crc(25) xor crc(26) xor crc(27) xor crc(29) xor crc(30) xor crc(31);
  --  ncrc(14) := d(7) xor d(6) xor d(4) xor d(3) xor d(2) xor crc(6) xor crc(26) xor crc(27) xor crc(28) xor crc(30) xor crc(31);
  --  ncrc(15) := d(7) xor d(5) xor d(4) xor d(3) xor crc(7) xor crc(27) xor crc(28) xor crc(29) xor crc(31);
  --  ncrc(16) := d(5) xor d(4) xor d(0) xor crc(8) xor crc(24) xor crc(28) xor crc(29);
  --  ncrc(17) := d(6) xor d(5) xor d(1) xor crc(9) xor crc(25) xor crc(29) xor crc(30);
  --  ncrc(18) := d(7) xor d(6) xor d(2) xor crc(10) xor crc(26) xor crc(30) xor crc(31);
  --  ncrc(19) := d(7) xor d(3) xor crc(11) xor crc(27) xor crc(31);
  --  ncrc(20) := d(4) xor crc(12) xor crc(28);
  --  ncrc(21) := d(5) xor crc(13) xor crc(29);
  --  ncrc(22) := d(0) xor crc(14) xor crc(24);
  --  ncrc(23) := d(6) xor d(1) xor d(0) xor crc(15) xor crc(24) xor crc(25) xor crc(30);
  --  ncrc(24) := d(7) xor d(2) xor d(1) xor crc(16) xor crc(25) xor crc(26) xor crc(31);
  --  ncrc(25) := d(3) xor d(2) xor crc(17) xor crc(26) xor crc(27);
  --  ncrc(26) := d(6) xor d(4) xor d(3) xor d(0) xor crc(18) xor crc(24) xor crc(27) xor crc(28) xor crc(30);
  --  ncrc(27) := d(7) xor d(5) xor d(4) xor d(1) xor crc(19) xor crc(25) xor crc(28) xor crc(29) xor crc(31);
  --  ncrc(28) := d(6) xor d(5) xor d(2) xor crc(20) xor crc(26) xor crc(29) xor crc(30);
  --  ncrc(29) := d(7) xor d(6) xor d(3) xor crc(21) xor crc(27) xor crc(30) xor crc(31);
  --  ncrc(30) := d(7) xor d(4) xor crc(22) xor crc(28) xor crc(31);
  --  ncrc(31) := d(5) xor crc(23) xor crc(29);
  --  return ncrc;
  --end function;

  --16-bit one's complement adder
  function crcadder(d1   : in std_logic_vector(15 downto 0);
                    d2   : in std_logic_vector(17 downto 0))
                         return std_logic_vector is
    variable vd1  : std_logic_vector(17 downto 0);
    variable vd2  : std_logic_vector(17 downto 0);
    variable sum  : std_logic_vector(17 downto 0);
  begin
    vd1 := "00" & d1; vd2 := d2;
    sum := vd1 + vd2;
    return sum;
  end function;

end package body;

