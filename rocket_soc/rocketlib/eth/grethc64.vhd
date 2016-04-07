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
-----------------------------------------------------------------------------
-- Entity:  grethc
-- File:  grethc.vhd
-- Author:  Marko Isomaki 
-- Description: Ethernet Media Access Controller with Ethernet Debug
--              Communication Link
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;
library rocketlib;
use rocketlib.grethpkg.all;

entity grethc64 is
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
end entity;
  
architecture rtl of grethc64 is
  procedure sel_op_mode(
    capbil : in std_logic_vector(4 downto 0);
    speed  : out std_ulogic;
    duplex : out std_ulogic) is
    variable vspeed  : std_ulogic;
    variable vduplex : std_ulogic;
  begin
    vspeed := '0'; vduplex := '0';
    vspeed := capbil(4) or capbil(3) or capbil(2);

    vduplex := (vspeed and capbil(3)) or ((not vspeed) and capbil(1));
    speed := vspeed;
    duplex := vduplex;
  end procedure;
  
  --host constants
  constant fabits          : integer := log2(fifosize);
  constant burstlength     : integer := setburstlength(fifosize);
  constant burstbits       : integer := log2(burstlength);
  constant ctrlopcode      : std_logic_vector(15 downto 0) := X"8808"; 
  constant broadcast       : std_logic_vector(47 downto 0) := X"FFFFFFFFFFFF";
--  constant maxsizetx       : integer := 1514;
  constant index           : integer := log2(edclbufsz);
  constant receiveOK       : std_logic_vector(3 downto 0) := "0000";
  constant frameCheckError : std_logic_vector(3 downto 0) := "0100";
  constant alignmentError  : std_logic_vector(3 downto 0) := "0001";
  constant frameTooLong    : std_logic_vector(3 downto 0) := "0010";
  constant overrun         : std_logic_vector(3 downto 0) := "1000";
  constant minpload        : std_logic_vector(10 downto 0) :=
                             conv_std_logic_vector(60, 11);
 
  --mdio constants
  constant divisor : std_logic_vector(7 downto 0) :=
    conv_std_logic_vector(mdcscaler, 8);

  --receiver constants
  constant maxsizerx : unsigned(15 downto 0) :=
    to_unsigned(maxsize + 18 - 4, 16);

  --tranceiver constants
  constant maxsizetx : unsigned(15 downto 0) :=
    to_unsigned(maxsize + 18 - 4, 16);

  --edcl constants
  type szvct is array (0 to 6) of integer;
  constant ebuf : szvct := (64, 128, 128, 256, 256, 256, 256);
  constant blbits : szvct := (6, 7, 7, 8, 8, 8, 8);
  constant winsz : szvct := (4, 4, 8, 8, 16, 32, 64);
  constant macaddrt : std_logic_vector(47 downto 0) :=
    conv_std_logic_vector(macaddrh, 24) & conv_std_logic_vector(macaddrl, 24);
  constant bpbits : integer := blbits(log2(edclbufsz));
  constant wsz : integer := winsz(log2(edclbufsz));
  constant bselbits : integer := log2(wsz);
  constant eabits: integer := log2(edclbufsz) + 8;
  constant ebufmax : std_logic_vector(bpbits-1 downto 0) := (others => '1');
  constant bufsize : std_logic_vector(2 downto 0) :=
                       conv_std_logic_vector(log2(edclbufsz), 3);
  constant ebufsize : integer := ebuf(log2(edclbufsz));
  constant txfifosize : integer := getfifosize(edcl, fifosize, ebufsize);
  constant txfabits : integer := log2(txfifosize);
  constant txfifosizev : std_logic_vector(txfabits downto 0) :=
    conv_std_logic_vector(txfifosize, txfabits+1);

  constant rxburstlen      : std_logic_vector(fabits downto 0) :=
    conv_std_logic_vector(burstlength, fabits+1);
  constant txburstlen      : std_logic_vector(txfabits downto 0) :=
    conv_std_logic_vector(burstlength, txfabits+1);
  
  type edclrstate_type is (idle, wrda, wrdsa, wrsa, wrtype, ip, ipdata,
                           oplength, arp, iplength, ipcrc, arpop, udp, spill);

  type duplexstate_type is (start, waitop, nextop, selmode, done);
      
  --host types
  type txd_state_type is (idle, read_desc, check_desc, req, fill_fifo, 
                          check_result, write_result, readhdr, start, wrbus1,
                          etdone, getlen, ahberror, fill_fifo2, wrbus2);
  type rxd_state_type is (idle, read_desc, check_desc, read_req, read_fifo,
                          discard, write_status, write_status2);

  --mdio types
  type mdio_state_type is (idle, preamble, startst, op, op2, phyadr, regadr,
                           ta, ta2, ta3, data, dataend);


  type fifo_access_in_type is record
    renable   : std_ulogic;    
    raddress  : std_logic_vector(fabits-1 downto 0);
    write     : std_ulogic;
    waddress  : std_logic_vector(fabits-1 downto 0);
    datain    : std_logic_vector(31 downto 0);
  end record;

  type fifo_access_out_type is record
    data      : std_logic_vector(31 downto 0);
  end record;

  type tx_fifo_access_in_type is record
    renable   : std_ulogic;    
    raddress  : std_logic_vector(txfabits-1 downto 0);
    write     : std_ulogic;
    waddress  : std_logic_vector(txfabits-1 downto 0);
    datain    : std_logic_vector(31 downto 0);
  end record;

  type tx_fifo_access_out_type is record
    data      : std_logic_vector(31 downto 0);
  end record;

  type edcl_ram_in_type is record
    renable   : std_ulogic;    
    raddress  : std_logic_vector(eabits-1 downto 0);
    writem    : std_ulogic;
    writel    : std_ulogic;
    waddressm : std_logic_vector(eabits-1 downto 0);
    waddressl : std_logic_vector(eabits-1 downto 0);
    datain    : std_logic_vector(31 downto 0);
  end record;

  type edcl_ram_out_type is record
    data      : std_logic_vector(31 downto 0);
  end record;

  type reg_type is record
    --user registers
    status      : eth_mac_status_type;
        
    --master tx interface
    tmsto           : eth_tx_ahb_in_type;
    tmsto2          : eth_tx_ahb_in_type;
    txdstate        : txd_state_type;
    txwrap          : std_ulogic;
    txden           : std_ulogic;
    txirq           : std_ulogic;
    txaddr          : std_logic_vector(31 downto 2);
    txlength        : std_logic_vector(10 downto 0);
    txburstcnt      : std_logic_vector(burstbits downto 0);
    tfwpnt          : std_logic_vector(txfabits-1 downto 0);
    tfrpnt          : std_logic_vector(txfabits-1 downto 0);
    tfcnt           : std_logic_vector(txfabits downto 0); 
    txcnt           : std_logic_vector(10 downto 0);
    txstart         : std_ulogic;
    txirqgen        : std_ulogic;
    txstatus        : std_logic_vector(1 downto 0);
    txvalid         : std_ulogic; 
    txdata          : std_logic_vector(31 downto 0);
    writeok         : std_ulogic;
    txread          : std_logic_vector(nsync-1 downto 0);
    txrestart       : std_logic_vector(nsync downto 0);
    txdone          : std_logic_vector(nsync downto 0);
    txstart_sync    : std_ulogic;
    txreadack       : std_ulogic;
    txdataav        : std_ulogic;
    txburstav       : std_ulogic;
          
    --master rx interface
    rxrenable       : std_ulogic;
    rmsto           : eth_rx_ahb_in_type;
    rxdstate        : rxd_state_type;
    rxstatus        : std_logic_vector(4 downto 0);
    rxaddr          : std_logic_vector(31 downto 2);
    rxlength        : std_logic_vector(10 downto 0);
    rxbytecount     : std_logic_vector(10 downto 0);
    rxwrap          : std_ulogic;
    rxirq           : std_ulogic;
    rfwpnt          : std_logic_vector(fabits-1 downto 0);
    rfrpnt          : std_logic_vector(fabits-1 downto 0);
    rfcnt           : std_logic_vector(fabits downto 0);
    rxcnt           : std_logic_vector(10 downto 0);
    rxdoneold       : std_ulogic;
    rxdoneack       : std_ulogic; 
    rxdone          : std_logic_vector(nsync-1 downto 0);
    rxstart         : std_logic_vector(nsync downto 0);
    rxwrite         : std_logic_vector(nsync-1 downto 0);
    rxwriteack      : std_ulogic; 
    rxburstcnt      : std_logic_vector(burstbits downto 0);
    addrok          : std_ulogic;
    addrdone        : std_ulogic;
    ctrlpkt         : std_ulogic;
    check           : std_ulogic;
    checkdata       : std_logic_vector(31 downto 0);
    usesizefield    : std_ulogic;
    rxden           : std_ulogic;
    gotframe        : std_ulogic;
    bcast           : std_ulogic;
    msbgood         : std_ulogic;
    rxburstav       : std_ulogic;
    hashlookup      : std_ulogic;
    mcast           : std_ulogic;
    mcastacc        : std_ulogic;
    
    --mdio
    mdccnt          : std_logic_vector(7 downto 0);
    mdioclk         : std_ulogic;
    mdioclkold      : std_logic_vector(mdiohold-1 downto 0);
    mdio_state      : mdio_state_type;
    mdioo           : std_ulogic;
    mdioi           : std_ulogic;
    mdioen          : std_ulogic;
    cnt             : std_logic_vector(4 downto 0);
    duplexstate     : duplexstate_type;
    init_busy       : std_ulogic;
    ext             : std_ulogic;
    extcap          : std_ulogic;
    regaddr         : std_logic_vector(4 downto 0);
    phywr           : std_ulogic;
    rstphy          : std_ulogic;
    capbil          : std_logic_vector(4 downto 0);
    rstaneg         : std_ulogic;
    mdint_sync      : std_logic_vector(2 downto 0);

    --edcl
    erenable        : std_ulogic;
    edclrstate      : edclrstate_type;
    edclactive      : std_ulogic;
    nak             : std_ulogic;
    ewr             : std_ulogic;
    write           : std_logic_vector(wsz-1 downto 0);
    seq             : std_logic_vector(13 downto 0);
    abufs           : std_logic_vector(bselbits downto 0);
    tpnt            : std_logic_vector(bselbits-1 downto 0);
    rpnt            : std_logic_vector(bselbits-1 downto 0);
    tcnt            : std_logic_vector(bpbits-1 downto 0);
    rcntm           : std_logic_vector(bpbits-1 downto 0);
    rcntl           : std_logic_vector(bpbits-1 downto 0);
    ipcrc           : std_logic_vector(17 downto 0);
    applength       : std_logic_vector(15 downto 0);
    oplen           : std_logic_vector(9 downto 0);
    udpsrc          : std_logic_vector(15 downto 0);
    ecnt            : std_logic_vector(3 downto 0);
    tarp            : std_ulogic;
    tnak            : std_ulogic;
    tedcl           : std_ulogic;
    edclbcast       : std_ulogic;
    edclsepahb      : std_ulogic;
  end record;

  --host signals
  signal arst             : std_ulogic;
  signal irst             : std_ulogic;
  signal vcc              : std_ulogic;

  signal txi              : host_tx_type;
  signal txo              : tx_host_type;

  signal rxi              : host_rx_type;
  signal rxo              : rx_host_type;

    --rx ahb fifo
  signal rxrenable      : std_ulogic;
  signal rxraddress     : std_logic_vector(10 downto 0);
  signal rxwrite        : std_ulogic;
  signal rxwdata        : std_logic_vector(31 downto 0);
  signal rxwaddress     : std_logic_vector(10 downto 0);
  signal rxrdata        : std_logic_vector(31 downto 0);    
    --tx ahb fifo  
  signal txrenable      : std_ulogic;
  signal txraddress     : std_logic_vector(10 downto 0);
  signal txwrite        : std_ulogic;
  signal txwdata        : std_logic_vector(31 downto 0);
  signal txwaddress     : std_logic_vector(10 downto 0);
  signal txrdata        : std_logic_vector(31 downto 0);    
    --edcl buf     
  signal erenable       : std_ulogic;
  signal eraddress      : std_logic_vector(15 downto 0);
  signal ewritem        : std_ulogic;
  signal ewritel        : std_ulogic;
  signal ewaddressm     : std_logic_vector(15 downto 0);
  signal ewaddressl     : std_logic_vector(15 downto 0);
  signal ewdata         : std_logic_vector(31 downto 0);
  signal erdata         : std_logic_vector(31 downto 0);

  signal r, rin           : reg_type;

begin
   
  --reset generators for transmitter and receiver
  vcc <= '1';
  arst <= testrst when (scanen = 1) and (testen = '1') 
          else rst and not r.status.reset;
  irst <= rst and not r.status.reset;
     
  comb : process(rst, irst, ctrli, cmdi, r, rmsti, tmsti, txo, rxo, 
                 erdata, rxrdata, txrdata, mdio_i, phyrstaddr,
                 testen, testrst, edcladdr, mdint, tmsti2, edcldisable,
                 edclsepahb) is
    variable v             : reg_type;
    variable vpirq         : std_ulogic;
    variable vrdbgdata     : std_logic_vector(31 downto 0);
    variable txvalid       : std_ulogic;
    variable vtxfi         : tx_fifo_access_in_type; 
    variable vrxfi         : fifo_access_in_type;
    variable lengthav      : std_ulogic;
    variable txdone        : std_ulogic;
    variable txread        : std_ulogic;
    variable txrestart     : std_ulogic;
    variable rxstart       : std_ulogic;
    variable rxdone        : std_ulogic;
    variable vrxwrite      : std_ulogic;
    variable ovrunstop     : std_ulogic;
    --mdio
    variable mdioindex     : integer range 0 to 31;
    variable mclk          : std_ulogic;  --rising mdio clk edge
    variable nmclk         : std_ulogic;  --falling mdio clk edge
    variable mclkvec       : std_logic_vector(mdiohold downto 0);
    --edcl
    variable veri          : edcl_ram_in_type;
    variable swap          : std_ulogic;
    variable setmz         : std_ulogic;
    variable ipcrctmp      : std_logic_vector(15 downto 0);
    variable ipcrctmp2     : std_logic_vector(17 downto 0);
    variable vrxenable     : std_ulogic;
    variable crctmp        : std_ulogic;
    variable vecnt         : integer;
  begin 
    v := r; vrdbgdata := (others => '0'); vpirq := '0';
    v.check := '0'; lengthav := r.rxdoneold;-- or r.usesizefield;
    ovrunstop := '0'; vrxfi.raddress := v.rfrpnt;

    if edcl /= 0 then
      veri.renable := r.erenable;
      veri.datain := rxo.dataout;
      veri.writem := '0'; veri.writel := '0';
      veri.waddressm := r.rpnt & r.rcntm; veri.waddressl := r.rpnt & r.rcntl;
    end if;

    vtxfi.renable := '0';
    vtxfi.datain := tmsti.data; 
    vtxfi.raddress := r.tfrpnt; vtxfi.write := '0';
    vtxfi.waddress := r.tfwpnt; 

    vrxfi.datain := rxo.dataout; 
    vrxfi.write := '0'; vrxfi.waddress := r.rfwpnt;
    vrxfi.renable := r.rxrenable; vrxenable := r.status.rxen;

    --synchronization
    v.txdone(0)     := txo.done;
    v.txread(0)     := txo.read;
    v.txrestart(0)  := txo.restart;
    v.rxstart(0)    := rxo.start;
    v.rxdone(0)     := rxo.done;
    v.rxwrite(0)    := rxo.write;
    
    if nsync = 2 then
      v.txdone(1)     := r.txdone(0);
      v.txread(1)     := r.txread(0);
      v.txrestart(1)  := r.txrestart(0);
      v.rxstart(1)    := r.rxstart(0);
      v.rxdone(1)     := r.rxdone(0);
      v.rxwrite(1)    := r.rxwrite(0);
    end if;

    if enable_mdint = 1 then
      v.mdint_sync(0) := mdint;
      v.mdint_sync(1) := r.mdint_sync(0);
      v.mdint_sync(2) := r.mdint_sync(1);
    end if;

    txdone     := r.txdone(nsync)     xor r.txdone(nsync-1);
    txread     := r.txreadack         xor r.txread(nsync-1);
    txrestart  := r.txrestart(nsync)  xor r.txrestart(nsync-1);
    rxstart    := r.rxstart(nsync)    xor r.rxstart(nsync-1);
    rxdone     := r.rxdoneack         xor r.rxdone(nsync-1);
    vrxwrite   := r.rxwriteack        xor r.rxwrite(nsync-1);
   
    if txdone = '1' then
      v.txstatus := txo.status;
    end if;
        
-------------------------------------------------------------------------------
-- HOST INTERFACE -------------------------------------------------------------
-------------------------------------------------------------------------------
    --SLAVE INTERFACE
    if cmdi.set_speed = '1' then v.status.speed := '1';
    elsif cmdi.clr_speed = '1' then v.status.speed := '0';
    end if;
    if cmdi.set_reset = '1' then v.status.reset := '1';
    elsif cmdi.clr_reset = '1' then v.status.reset := '0';
    end if;
    if cmdi.set_full_duplex = '1' then v.status.full_duplex := '1';
    elsif cmdi.clr_full_duplex = '1' then v.status.full_duplex := '0';
    end if;
    if cmdi.set_rxena = '1' then v.status.rxen := '1';
    elsif cmdi.clr_rxena = '1' then v.status.rxen := '0';
    end if;
    if cmdi.set_txena = '1' then v.status.txen := '1';
    elsif cmdi.clr_txena = '1' then v.status.txen := '0';
    end if;

    if cmdi.clr_status_phystat = '1' then v.status.phystat  := '0'; end if;
    if cmdi.clr_status_invaddr = '1' then v.status.invaddr  := '0'; end if;
    if cmdi.clr_status_toosmall = '1' then v.status.toosmall := '0'; end if;
    if cmdi.clr_status_txahberr = '1' then v.status.txahberr := '0'; end if;
    if cmdi.clr_status_rxahberr = '1' then v.status.rxahberr := '0';  end if;
    if cmdi.clr_status_tx_int = '1' then v.status.tx_int := '0'; end if;
    if cmdi.clr_status_rx_int = '1' then v.status.rx_int := '0'; end if;
    if cmdi.clr_status_tx_err = '1' then v.status.tx_err := '0'; end if;
    if cmdi.clr_status_rx_err = '1' then v.status.rx_err := '0'; end if;

    if cmdi.mdio_cmd.valid = '1' then
       v.status.mdio.cmd.data   := cmdi.mdio_cmd.data;
       v.status.mdio.cmd.regadr := cmdi.mdio_cmd.regadr;
       v.status.mdio.cmd.read   := cmdi.mdio_cmd.read;
       v.status.mdio.cmd.write  := cmdi.mdio_cmd.write;
       v.status.mdio.busy       := cmdi.mdio_cmd.read or cmdi.mdio_cmd.write;
    end if;
    
    if cmdi.dbg_access_id = DBG_ACCESS_TX_BUFFER then
       vtxfi.write    := cmdi.dbg_wr_ena; 
       vtxfi.waddress := cmdi.dbg_addr(txfabits+1 downto 2);
       vtxfi.datain   := cmdi.dbg_wdata; 
       vtxfi.raddress := cmdi.dbg_addr(txfabits+1 downto 2);
       vtxfi.renable  := cmdi.dbg_rd_ena;
       vrdbgdata      := txrdata;
    end if;
    if cmdi.dbg_access_id = DBG_ACCESS_RX_BUFFER then
       vrxfi.write    := cmdi.dbg_wr_ena; 
       vrxfi.waddress := cmdi.dbg_addr(fabits+1 downto 2);
       vrxfi.datain   := cmdi.dbg_wdata; 
       vrxfi.raddress := cmdi.dbg_addr(fabits+1 downto 2);
       vrxfi.renable  := cmdi.dbg_rd_ena;
       vrdbgdata      := rxrdata;
    end if;
    if cmdi.dbg_access_id = DBG_ACCESS_EDCL_BUFFER then
       veri.writem    := cmdi.dbg_wr_ena; 
       veri.writel    := cmdi.dbg_wr_ena; 
       veri.waddressm := cmdi.dbg_addr(eabits+1 downto 2);
       veri.waddressl := cmdi.dbg_addr(eabits+1 downto 2);
       veri.datain    := cmdi.dbg_wdata; 
       veri.raddress  := cmdi.dbg_addr(eabits+1 downto 2);
       veri.renable   := cmdi.dbg_rd_ena;
       vrdbgdata      := erdata;
    end if;

      
   --PHY STATUS DETECTION
   if enable_mdint = 1 then
     if mdint_pol = 0 then
       if (r.mdint_sync(2) and not r.mdint_sync(1)) = '1' then
         v.status.phystat := '1';
         if ctrli.pstatirqen = '1' then
           vpirq := '1';
         end if;
       end if;
     else
       if (r.mdint_sync(1) and not r.mdint_sync(2)) = '1' then
         v.status.phystat := '1';
         if ctrli.pstatirqen = '1' then
           vpirq := '1';
         end if;
       end if;
     end if;
   end if;
      
   --MASTER INTERFACE

   v.txburstav := '0';
   if (txfifosizev - r.tfcnt) >= txburstlen then
     v.txburstav := '1'; 
   end if;

   if (conv_integer(r.abufs) /= 0) then
     v.status.edcltx_idle := '0';
   else
     v.status.edcltx_idle := '1';
   end if; 

   --tx dma fsm
   case r.txdstate is
   when idle =>
     v.txcnt := (others => '0'); v.txburstcnt := (others => '0');
     if (edcl /= 0) then
       v.tedcl := '0'; v.erenable := '0';
     end if;
     if (edcl /= 0) and (conv_integer(r.abufs) /= 0) and
        (ctrli.edcldis = '0') then
       v.erenable := '1'; v.status.edcltx_idle := '0';
       if r.erenable = '1' then
         v.txdstate := getlen; 
       end if;
       v.tcnt := conv_std_logic_vector(10, bpbits);
     elsif r.status.txen = '1' then
       v.txdstate := read_desc;
       v.tmsto.write := '0';  
       v.tmsto.addr := ctrli.txdesc & r.status.txdsel & "000";
       v.tmsto.req := '1';
       --! AXI_ENABLE: burst transaction size in bytes
       v.tmsto.burst_bytes := conv_std_logic_vector(8, 11);
     end if;
     if r.txirqgen = '1' then
       vpirq := '1'; v.txirqgen := '0';
     end if;
     if txrestart = '1' then
       v.txrestart(nsync) := r.txrestart(nsync-1);
       v.tfcnt := (others => '0'); v.tfrpnt := (others => '0');
       v.tfwpnt := (others => '0');
     end if;
   when read_desc =>
     v.tmsto.write := '0'; v.txstatus := (others => '0'); 
     v.tfwpnt := (others => '0'); v.tfrpnt := (others => '0');
     v.tfcnt := (others => '0');
     if tmsti.grant = '1' then
       v.txburstcnt := r.txburstcnt + 1; v.tmsto.addr := r.tmsto.addr + 4;
       if r.txburstcnt(0) = '1' then
         v.tmsto.req := '0';
       end if;
     end if;
     if tmsti.ready = '1' then
       v.txcnt := r.txcnt + 1; 
       case r.txcnt(1 downto 0) is
         when "00" =>
           v.txlength  := tmsti.data(10 downto 0);
           v.txden     := tmsti.data(11);
           v.txwrap    := tmsti.data(12);
           v.txirq     := tmsti.data(13);
           v.status.txen := tmsti.data(11);
         when "01" =>
           v.txaddr    := tmsti.data(31 downto 2);
           v.txdstate  := check_desc;
         when others => null;
       end case; 
     end if;
   when check_desc =>
     v.txstart := '0'; 
     v.txburstcnt := (others => '0'); 
     if r.txden = '1' then
       if (unsigned(r.txlength) > unsigned(maxsizetx)) or
                  (conv_integer(r.txlength) = 0) then
         v.txdstate := write_result;
         v.tmsto.req := '1';
         v.tmsto.write := '1';
         v.tmsto.addr := ctrli.txdesc & r.status.txdsel & "000";
         v.tmsto.data := (others => '0');
         --! AXI_ENABLE: length of transaction not defined so use simple DMA access
         v.tmsto.burst_bytes := conv_std_logic_vector(4,11);
       else
         v.txdstate := req;
         v.tmsto.addr := r.txaddr & "00"; 
         v.txcnt(10 downto 0) := r.txlength;
         --! AXI_ENABLE: length of transaction defined
         v.tmsto.burst_bytes := r.txlength;
       end if;
     else
       v.txdstate := idle;
     end if;
   when req =>
     if txrestart = '1' then
       v.txdstate := idle; v.txstart := '0'; 
       if (edcl /= 0) and (r.tedcl = '1') then
         v.txdstate := idle; 
       end if;
     elsif txdone = '1' then
       v.txdstate := check_result;
       v.tfcnt := (others => '0'); v.tfrpnt := (others => '0');
       v.tfwpnt := (others => '0');
       if (edcl /= 0) and (r.tedcl = '1') then
         v.txdstate := etdone;
       end if;
     elsif conv_integer(r.txcnt) = 0 then
       v.txdstate := check_result;
       if (edcl /= 0) and (r.tedcl = '1') then
         v.txdstate := etdone; v.txstart_sync := not r.txstart_sync;
       end if;
     elsif (r.txburstav = '1') or (r.tedcl = '1') then
       if (edclsepahbg = 0) or (edcl = 0) or
          (r.edclsepahb = '0') or (r.tedcl = '0') then 
         v.tmsto.req := '1'; v.txdstate := fill_fifo;
       else
         v.tmsto2.req := '1'; v.txdstate := fill_fifo2;
       end if;
     end if;
     v.txburstcnt := (others => '0');
   when fill_fifo =>
     v.txburstav := '0';
     if tmsti.grant = '1' then
       v.tmsto.addr := r.tmsto.addr + 4;
       if ((conv_integer(r.txcnt) <= 8) and (tmsti.ready = '1')) or
          ((conv_integer(r.txcnt) <= 4) and (tmsti.ready = '0')) then
         v.tmsto.req := '0'; 
       end if;
       v.txburstcnt := r.txburstcnt + 1;
       if (conv_integer(r.txburstcnt) = burstlength-1) then
         v.tmsto.req := '0';
       end if;
     end if;
     if (tmsti.ready = '1') or ((edcl /= 0) and (r.tedcl and tmsti.error) = '1') then
       v.tfwpnt := r.tfwpnt + 1; v.tfcnt := r.tfcnt + 1; vtxfi.write := '1';
       if r.tmsto.req = '0' then
         v.txdstate := req;
         if (r.txstart = '0') and not ((edcl /= 0) and (r.tedcl = '1')) then
           v.txstart := '1'; v.txstart_sync := not r.txstart_sync; 
         end if;
       end if;
       if conv_integer(r.txcnt) > 3 then
         v.txcnt := r.txcnt - 4;
       else
         v.txcnt := (others => '0');
       end if;
     end if;
   when fill_fifo2 =>
     if edclsepahbg = 1 then
       v.txburstav := '0';
       vtxfi.datain := tmsti2.data;
       if tmsti2.grant = '1' then
         v.tmsto2.addr := r.tmsto2.addr + 4;
         if ((conv_integer(r.txcnt) <= 8) and (tmsti2.ready = '1')) or
            ((conv_integer(r.txcnt) <= 4) and (tmsti2.ready = '0')) then
           v.tmsto2.req := '0'; 
         end if;
         v.txburstcnt := r.txburstcnt + 1;
         if (conv_integer(r.txburstcnt) = burstlength-1) then
           v.tmsto2.req := '0';
         end if;
       end if;
       if (tmsti2.ready = '1') or ((edcl /= 0) and (r.tedcl and tmsti2.error) = '1') then
         v.tfwpnt := r.tfwpnt + 1; v.tfcnt := r.tfcnt + 1; vtxfi.write := '1';
         if r.tmsto2.req = '0' then
           v.txdstate := req;
           if (r.txstart = '0') and not ((edcl /= 0) and (r.tedcl = '1')) then
             v.txstart := '1'; v.txstart_sync := not r.txstart_sync; 
           end if;
         end if;
         if conv_integer(r.txcnt) > 3 then
           v.txcnt := r.txcnt - 4;
         else
           v.txcnt := (others => '0');
         end if;
       end if;
     end if;
   when check_result =>
     if txdone = '1' then
       v.txdstate := write_result; v.tmsto.req := '1'; v.txstart := '0';
       v.tmsto.write := '1'; v.tmsto.addr := ctrli.txdesc & r.status.txdsel & "000";
       v.tmsto.data(31 downto 16) := (others => '0');
       v.tmsto.data(15 downto 14) := v.txstatus;
       v.tmsto.data(13 downto 0)  := (others => '0');
       v.txdone(nsync) := r.txdone(nsync-1);
     elsif txrestart = '1' then
       v.txdstate := idle; v.txstart := '0'; 
     end if;
   when write_result =>
     if tmsti.grant = '1' then
       v.tmsto.req := '0'; v.tmsto.addr := r.tmsto.addr + 4;
     end if;
     if tmsti.ready = '1' then
       v.txdstate := idle; 
       v.txirqgen := ctrli.tx_irqen and r.txirq; 
       if r.txwrap = '0' then v.status.txdsel := r.status.txdsel + 1;
       else v.status.txdsel := (others => '0'); end if;
       if conv_integer(r.txstatus) = 0 then v.status.tx_int := '1';
       else v.status.tx_err := '1'; end if;
     end if;
   when ahberror =>
     v.tfcnt := (others => '0'); v.tfwpnt := (others => '0');
     v.tfrpnt := (others => '0');
     v.status.txahberr := '1'; v.status.txen := '0';
     if not ((edcl /= 0) and (r.tedcl = '1')) then
       if r.txstart = '1' then
         if txdone = '1' then
           v.txdstate := idle; v.txdone(nsync) := r.txdone(nsync-1);
         end if;
       else
         v.txdstate := idle;
       end if;
     else
       v.txdstate := idle; 
       v.abufs := r.abufs - 1; v.tpnt := r.tpnt + 1;
     end if;
   when others =>
     null;
   end case;

   --tx fifo read
   v.txdataav := '0';
   if conv_integer(r.tfcnt) /= 0 then
     v.txdataav := '1';
   end if;
   if txread = '1' then
     v.txreadack := not r.txreadack;
     if r.txdataav = '1' then
       if conv_integer(r.tfcnt) < 2 then
         v.txdataav := '0';
       end if;
       v.txvalid := '1';
       v.tfcnt := v.tfcnt - 1; v.tfrpnt := r.tfrpnt + 1;
     else
       v.txvalid := '0';
     end if;
     v.txdata := txrdata;
   end if;

   v.rxburstav := '0';
   if r.rfcnt >= rxburstlen then
     v.rxburstav := '1'; 
   end if;

   if ramdebug = 0 then
     vtxfi.renable := v.txdataav;
   else
     vtxfi.renable := vtxfi.renable or v.txdataav;
   end if;

   --rx dma fsm
   case r.rxdstate is
   when idle =>
     v.rmsto.req := '0'; v.rmsto.write := '0'; v.addrok := '0';
     v.rxburstcnt := (others => '0'); v.addrdone := '0';
     v.rxcnt := (others => '0'); v.rxdoneold := '0';
     v.ctrlpkt := '0'; v.bcast := '0'; v.edclactive := '0';
     v.msbgood := '0'; v.rxrenable := '0';
     if multicast = 1 then
       v.mcast := '0'; v.mcastacc := '0';
     end if;
     if r.status.rxen = '1' then
       v.rxdstate := read_desc;
       v.rmsto.req := '1';
       v.rmsto.addr := ctrli.rxdesc & r.status.rxdsel & "000"; 
       --! AXI_ENABLE: burst transaction descriptor header size in bytes
       v.rmsto.burst_bytes := conv_std_logic_vector(8, 11);
     elsif rxstart = '1' then
       v.rxstart(nsync) := r.rxstart(nsync-1);
       v.rxdstate := discard;
     end if;
   when read_desc =>
     v.rxstatus := (others => '0');
     if rmsti.grant = '1' then
       v.rxburstcnt := r.rxburstcnt + 1; v.rmsto.addr := r.rmsto.addr + 4;
       if r.rxburstcnt(0) = '1' then
         v.rmsto.req := '0';
         --! AXI_ENABLE: don't use burst operation:
         v.rmsto.burst_bytes := conv_std_logic_vector(4,11);
       end if;
     end if;
     if rmsti.ready = '1' then
       v.rxcnt := r.rxcnt + 1;
       case r.rxcnt(1 downto 0) is
         when "00" =>
           v.status.rxen := rmsti.data(11);
           v.rxden     := rmsti.data(11);
           v.rxwrap    := rmsti.data(12);
           v.rxirq     := rmsti.data(13);
         when "01" =>
           v.rxaddr    := rmsti.data(31 downto 2);
           v.rxdstate  := check_desc;
           v.rxrenable := '1';
         when others =>
           null;
       end case; 
     end if;
     if rmsti.error = '1' then
       v.rmsto.req := '0'; v.rxdstate := idle;
       v.status.rxahberr := '1'; v.status.rxen := '0';
     end if;
   when check_desc =>
     v.rxcnt := (others => '0'); v.usesizefield := '0'; v.rmsto.write := '1'; 
     if r.rxden = '1' then
       if rxstart = '1' then
         v.rxdstate := read_req; v.rxstart(nsync) := r.rxstart(nsync-1);
       end if; 
     else
       v.rxdstate := idle;
     end if;
     v.rmsto.addr := r.rxaddr & "00";
   when read_req =>
     if r.edclactive = '1' then
       v.rxdstate := discard;
     elsif (r.rxdoneold and r.rxstatus(3)) = '1' then
       v.rxdstate := write_status; 
       v.rfcnt := (others => '0'); v.rfwpnt := (others => '0');
       v.rfrpnt := (others => '0'); v.writeok := '1';
       v.rxbytecount := (others => '0'); v.rxlength := (others => '0');
     elsif ((r.addrdone and not r.addrok) or r.ctrlpkt) = '1' then
       v.rxdstate := discard; v.status.invaddr := '1';
     elsif ((r.rxdoneold = '1') and r.rxcnt >= r.rxlength) then
       if r.gotframe = '1' then
         v.rxdstate := write_status;
       else
         v.rxdstate := discard; v.status.toosmall := '1';
       end if;
     elsif (r.rxburstav or r.rxdoneold) = '1' then
       v.rmsto.req := '1'; v.rxdstate := read_fifo;
       v.rfrpnt := r.rfrpnt + 1; v.rfcnt := r.rfcnt - 1;
     end if;
     v.rxburstcnt := (others => '0'); v.rmsto.data := rxrdata; 
   when read_fifo =>
     v.rxburstav := '0';
     if rmsti.grant = '1' then
       v.rmsto.addr := r.rmsto.addr + 4;
       if (lengthav = '1') then
         if ((conv_integer(r.rxcnt) >=
              (conv_integer(r.rxlength) - 8)) and (rmsti.ready = '1')) or
         ((conv_integer(r.rxcnt) >=
           (conv_integer(r.rxlength) - 4)) and (rmsti.ready = '0')) then
           v.rmsto.req := '0'; 
         end if;
       end if;
       v.rxburstcnt := r.rxburstcnt + 1;
       if (conv_integer(r.rxburstcnt) = burstlength-1) then
         v.rmsto.req := '0';
       end if;
     end if;
     if rmsti.ready = '1' then
       v.rmsto.data := rxrdata; 
       v.rxcnt := r.rxcnt + 4;  
       if r.rmsto.req = '0' then
         v.rxdstate := read_req; 
       else
         v.rfcnt := r.rfcnt - 1; v.rfrpnt := r.rfrpnt + 1;
       end if;
       v.check := '1'; v.checkdata := r.rmsto.data; 
     end if;
     if rmsti.error = '1' then
       v.rmsto.req := '0'; v.rxdstate := discard;
       v.rxcnt := r.rxcnt + 4;
       v.status.rxahberr := '1'; v.status.rxen := '0';
     end if;
   when write_status =>
     v.rmsto.req := '1'; v.rmsto.addr := ctrli.rxdesc & r.status.rxdsel & "000";
     v.rxdstate := write_status2;
     if multicast = 1 then
       v.rmsto.data := "00000" & r.mcastacc & "0000000" &
                        r.rxstatus & "000" & r.rxlength;
     else
       v.rmsto.data := "0000000000000" &
                        r.rxstatus & "000" & r.rxlength;
     end if;
   when write_status2 =>
     if rmsti.grant = '1' then
       v.rmsto.req := '0'; v.rmsto.addr := r.rmsto.addr + 4;
     end if;
     if rmsti.ready = '1' then
       if (r.rxstatus(4) or not r.rxstatus(3)) = '1' then
         v.rxdstate := discard;
       else
         v.rxdstate := idle;
       end if;
       if (ctrli.rx_irqen and r.rxirq) = '1' then
         vpirq := '1';
       end if;
       if conv_integer(r.rxstatus) = 0 then v.status.rx_int := '1';
       else v.status.rx_err := '1'; end if;
       if r.rxwrap = '1' then
         v.status.rxdsel := (others => '0');
       else
         v.status.rxdsel := r.status.rxdsel + 1;
       end if;
     end if;
     if rmsti.error = '1' then
       v.rmsto.req := '0'; v.rxdstate := idle;
       v.status.rxahberr := '1'; v.status.rxen := '0';
     end if;
   when discard =>
     if (r.rxdoneold = '0') then
       if conv_integer(r.rfcnt) /= 0 then
         v.rfrpnt := r.rfrpnt + 1; v.rfcnt := r.rfcnt - 1;
         v.rxcnt := r.rxcnt + 4;
       end if;
     else 
       if r.rxstatus(3) = '1' then
         v.rfcnt := (others => '0'); v.rfwpnt := (others => '0');
         v.rfrpnt := (others => '0'); v.writeok := '1';
         v.rxbytecount := (others => '0'); v.rxlength := (others => '0');
         v.rxdstate := idle;
       elsif (conv_integer(r.rxcnt) < conv_integer(r.rxbytecount)) then
         if conv_integer(r.rfcnt) /= 0 then
           v.rfrpnt := r.rfrpnt + 1; v.rfcnt := r.rfcnt - 1;
           v.rxcnt := r.rxcnt + 4;
         end if;    
       else
         v.rxdstate := idle; v.ctrlpkt := '0';
       end if;
     end if;
   when others =>
     null;
   end case;
   
   --rx address/type check
   if r.check = '1' and r.rxcnt(10 downto 5) = "000000" then 
     case r.rxcnt(4 downto 2) is
     when "001" =>
       if ctrli.prom = '1' then
         v.addrok := '1';
       end if;
       v.mcast := r.checkdata(24);
       if r.checkdata = broadcast(47 downto 16) then
         v.bcast := '1';
       end if;
       if r.checkdata = ctrli.mac_addr(47 downto 16) then
         v.msbgood := '1';
       end if;
     when "010" =>
       if r.checkdata(31 downto 16) = broadcast(15 downto 0) then
         if r.bcast = '1' then
           v.addrok := '1';
         end if;
       else
         v.bcast := '0';
       end if;
       if r.checkdata(31 downto 16) = ctrli.mac_addr(15 downto 0) then
         if r.msbgood = '1' then
           v.addrok := '1';
         end if;
       end if;
       if multicast = 1 then
         v.hashlookup := ctrli.hash(conv_integer(rxo.mcasthash));
       end if;
     when "011" =>
       if multicast = 1 then
         if (r.hashlookup and ctrli.mcasten and r.mcast) = '1' then
           v.addrok := '1';
           if r.bcast = '0' then
             v.mcastacc := '1';
           end if;
         end if;
       end if;
     when "100" =>
       if r.checkdata(31 downto 16) = ctrlopcode then v.ctrlpkt := '1'; end if;
       v.addrdone := '1';
     when others =>
       null;
     end case; 
   end if;
    
   --rx packet done
   if (rxdone and not rxstart) = '1' then
     v.gotframe := rxo.gotframe; v.rxbytecount := rxo.byte_count;
     v.rxstatus(3 downto 0) := rxo.status;
     if (unsigned(rxo.lentype) > maxsizerx) or (rxo.status /= "0000") then
       v.rxlength := rxo.byte_count;
     else
       v.rxlength := rxo.lentype(10 downto 0);
       if (rxo.lentype(10 downto 0) > minpload) and
          (rxo.lentype(10 downto 0) /= rxo.byte_count) then
         if rxo.status(2 downto 0) = "000" then
           v.rxstatus(4) := '1'; v.rxlength := rxo.byte_count;
           v.usesizefield := '0';
         end if;
       elsif (rxo.lentype(10 downto 0) <= minpload) and
             (rxo.byte_count /= minpload) then
         if rxo.status(2 downto 0) = "000" then
           v.rxstatus(4) := '1'; v.rxlength := rxo.byte_count;
           v.usesizefield := '0';
         end if;
       end if;
     end if;
     v.rxdoneold := '1';
     v.rxdoneack := not r.rxdoneack; 
   end if; 
     
   --rx fifo write
   if vrxwrite = '1' then
     v.rxwriteack := not r.rxwriteack;
     if (not r.rfcnt(fabits)) = '1' then 
       v.rfwpnt := r.rfwpnt + 1; v.rfcnt := v.rfcnt + 1; v.writeok := '1'; 
       vrxfi.write := '1'; 
     else
       v.writeok := '0'; 
     end if;
   end if;  

   --must be placed here because it uses variable
   if (ramdebug = 0) or (ctrli.ramdebugen = '0') then 
     vrxfi.raddress := v.rfrpnt;
   end if;

-------------------------------------------------------------------------------
-- MDIO INTERFACE -------------------------------------------------------------
-------------------------------------------------------------------------------
   --mdio commands
   if enable_mdio = 1 then
     mclkvec := r.mdioclkold & r.mdioclk;
     mclk := mclkvec(mdiohold-1) and not mclkvec(mdiohold);
     nmclk := mclkvec(1) and not mclkvec(0);
     v.mdioclkold := mclkvec(mdiohold-1 downto 0);
     if r.mdccnt = "00000000" then
       v.mdccnt := divisor;
       v.mdioclk := not r.mdioclk;
     else
       v.mdccnt := r.mdccnt - 1;
     end if;
     mdioindex := conv_integer(r.cnt); v.mdioi := mdio_i;
     case r.mdio_state is
       when idle =>
         if (enable_mdio = 1) and (edcl = 0) and (r.status.reset = '1') then
           v.mdio_state := idle; v.status.mdio.cmd.read := '0';
           v.status.mdio.cmd.write := '0'; v.status.mdio.busy := '0';
           v.status.mdio.cmd.data := (others => '0');
           v.status.mdio.cmd.regadr := (others => '0');
           v.status.reset := '0';
           if OEPOL = 0 then v.mdioen := '1'; else v.mdioen := '0'; end if;
         end if;
         if mclk = '1' then
           v.cnt := (others => '0');
           if r.status.mdio.busy = '1' then
             v.status.mdio.linkfail := '0'; 
             if r.status.mdio.cmd.read = '1' then
               v.status.mdio.cmd.write := '0'; 
             end if;
             v.mdio_state := preamble; v.mdioo := '1';
             if OEPOL = 0 then v.mdioen := '0'; else v.mdioen := '1'; end if;
           end if;
         end if;
       when preamble =>
         if mclk = '1' then
           v.cnt := r.cnt + 1; 
           if r.cnt = "11111" then
             v.mdioo := '0'; v.mdio_state := startst; 
           end if;
         end if;
       when startst =>
         if mclk = '1' then
           v.mdioo := '1'; v.mdio_state := op; v.cnt := (others => '0');
         end if;
       when op =>
         if mclk = '1' then
           v.mdio_state := op2;    
           if r.status.mdio.cmd.read = '1' then v.mdioo := '1';
           else v.mdioo := '0'; end if;
         end if;
       when op2 =>
         if mclk = '1' then
           v.mdioo := not r.mdioo; v.mdio_state := phyadr;
           v.cnt := (others => '0');
         end if;
       when phyadr =>
         if mclk = '1' then
           v.cnt := r.cnt + 1;
           case mdioindex is
           when 0 => v.mdioo := ctrli.mdio_phyadr(4);
           when 1 => v.mdioo := ctrli.mdio_phyadr(3);
           when 2 => v.mdioo := ctrli.mdio_phyadr(2);
           when 3 => v.mdioo := ctrli.mdio_phyadr(1);
           when 4 => v.mdioo := ctrli.mdio_phyadr(0);
                     v.mdio_state := regadr; v.cnt := (others => '0');
           when others => null;
           end case;
         end if;
       when regadr =>
         if mclk = '1' then
           v.cnt := r.cnt + 1;
           case mdioindex is
           when 0 => v.mdioo := r.status.mdio.cmd.regadr(4);
           when 1 => v.mdioo := r.status.mdio.cmd.regadr(3);
           when 2 => v.mdioo := r.status.mdio.cmd.regadr(2);
           when 3 => v.mdioo := r.status.mdio.cmd.regadr(1);
           when 4 => v.mdioo := r.status.mdio.cmd.regadr(0);
                     v.mdio_state := ta; v.cnt := (others => '0');
           when others => null;
           end case;
         end if;
       when ta =>
         if mclk = '1' then
           v.mdio_state := ta2;
           if r.status.mdio.cmd.read = '1' then 
             if OEPOL = 0 then v.mdioen := '1'; else v.mdioen := '0'; end if;
           else v.mdioo := '1'; end if;
         end if;
       when ta2 =>
         if mclk = '1' then
           v.cnt := "01111"; v.mdio_state := ta3; 
           if r.status.mdio.cmd.write = '1' then v.mdioo := '0'; v.mdio_state := data; end if;
         end if;
       when ta3 =>
         if mclk = '1' then
           v.mdio_state := data;
         end if;
         if nmclk = '1' then
           if r.mdioi /= '0' then
             v.status.mdio.linkfail := '1';
           end if;
         end if;
       when data =>
         if mclk = '1' then
           v.cnt := r.cnt - 1;
           if r.cnt = "00000" then
             v.mdio_state := dataend;
           end if;
           if r.status.mdio.cmd.read = '0' then
             v.mdioo := r.status.mdio.cmd.data(mdioindex);
           end if;
         end if;
         if nmclk = '1' then
           if r.status.mdio.cmd.read = '1' then
             v.status.mdio.cmd.data(mdioindex) := r.mdioi; 
           end if;
         end if;
       when dataend =>
         if mclk = '1' then
           if (rmii = 1) or (edcl /= 0) then
             v.init_busy := '0';
             if (r.duplexstate = done or ctrli.edcldis = '1' or ctrli.disableduplex = '1') then
               v.status.mdio.busy := '0';
             end if;
           else
             v.status.mdio.busy := '0'; 
           end if;
           v.status.mdio.cmd.read := '0'; 
           v.status.mdio.cmd.write := '0'; v.mdio_state := idle;
           if OEPOL = 0 then v.mdioen := '1'; else v.mdioen := '0'; end if;
         end if;
       when others =>
         null;
     end case;
   end if;

-------------------------------------------------------------------------------
-- EDCL -----------------------------------------------------------------------
-------------------------------------------------------------------------------
   if (edcl /= 0) then
     if (ramdebug /= 2) or (ctrli.ramdebugen = '0') then
       veri.renable := r.erenable; veri.writem := '0'; veri.writel := '0';
       veri.waddressm := r.rpnt & r.rcntm; veri.waddressl := r.rpnt & r.rcntl;
       vrxenable := '1';  
     end if;

     swap := '0'; vecnt := conv_integer(r.ecnt); setmz := '0';
     
     if vrxwrite = '1' then
       if ctrli.edcldis = '0' then
         v.rxwriteack := not r.rxwriteack;
       end if;
     end if;

     --edcl receiver 
     case r.edclrstate is
       when idle =>
         v.edclbcast := '0'; v.status.edclrx_idle := '1';
         if (ramdebug /= 2) or (ctrli.ramdebugen = '0') then 
           if (rxstart and not ctrli.edcldis) = '1' then
             v.edclrstate := wrda; v.edclactive := '0'; v.status.edclrx_idle := '0';
             v.rcntm := conv_std_logic_vector(2, bpbits);
             v.rcntl := conv_std_logic_vector(1, bpbits);
           end if;
         end if;
       when wrda =>
         if vrxwrite = '1' then
           v.edclrstate := wrdsa;
           veri.writem := '1'; veri.writel := '1';
           swap := '1';
           v.rcntm := r.rcntm - 2; v.rcntl := r.rcntl + 1;
           if (ctrli.emacaddr(47 downto 16) /= rxo.dataout) and
                        (X"FFFFFFFF" /= rxo.dataout) then
             v.edclrstate := spill;
           elsif (X"FFFFFFFF" = rxo.dataout) then
              v.edclbcast := '1'; 
           end if;
           if conv_integer(r.abufs) = wsz then
             v.edclrstate := spill;
           end if;
         end if;
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when wrdsa =>
         if vrxwrite = '1' then
           v.edclrstate := wrsa; swap := '1';
           veri.writem := '1'; veri.writel := '1';
           v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl - 2;
           if (ctrli.emacaddr(15 downto 0) /= rxo.dataout(31 downto 16)) and
                           (X"FFFF" /= rxo.dataout(31 downto 16)) then
             v.edclrstate := spill; 
           elsif (X"FFFF" = rxo.dataout(31 downto 16)) then
             v.edclbcast := r.edclbcast; 
           end if;
         end if;
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when wrsa =>
         if vrxwrite = '1' then
           veri.writem := '1'; veri.writel := '1';
           v.edclrstate := wrtype; swap := '1';
           v.rcntm := r.rcntm + 2; v.rcntl := r.rcntl + 3;
         end if;
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when wrtype =>
         if vrxwrite = '1' then
           veri.writem := '1'; veri.writel := '1';
           v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
           if X"0800" = rxo.dataout(31 downto 16) and (r.edclbcast = '0') then
             v.edclrstate := ip;
           elsif X"0806" = rxo.dataout(31 downto 16) and (r.edclbcast = '1') then
             v.edclrstate := arp;
           else
             v.edclrstate := spill;
           end if;
         end if;
         v.ecnt := (others => '0'); v.ipcrc := (others => '0');
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when ip =>
         if vrxwrite = '1' then
           v.ecnt := r.ecnt + 1;
           veri.writem := '1'; veri.writel := '1';
           case vecnt is
             when 0 =>
               v.ipcrc :=
               crcadder(not rxo.dataout(31 downto 16), r.ipcrc);
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
             when 1 =>
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 2;
             when 2 =>
               v.ipcrc :=
               crcadder(not rxo.dataout(31 downto 16), r.ipcrc);
               v.rcntm := r.rcntm + 2; v.rcntl := r.rcntl - 1;
             when 3 =>
               v.rcntm := r.rcntm - 1; v.rcntl := r.rcntl + 2;
             when 4 =>
               v.udpsrc := rxo.dataout(15 downto 0);
               v.rcntm := r.rcntm + 2; v.rcntl := r.rcntl + 1; 
             when 5 =>
               setmz := '1';
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1; 
             when 6 =>
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1; 
             when 7 =>
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
               if (rxo.dataout(31 downto 18) = r.seq) then
                 v.nak := '0'; 
               else
                 v.nak := '1'; 
                 veri.datain(31 downto 18) := r.seq;
               end if;
               veri.datain(17) := v.nak; v.ewr := rxo.dataout(17);
               if (rxo.dataout(17) or v.nak) = '1' then
                 veri.datain(16 downto 7) := (others => '0');
               end if;
               v.oplen := rxo.dataout(16 downto 7);
               v.applength := "000000" & veri.datain(16 downto 7);
               v.ipcrc :=
               crcadder(v.applength + 38, r.ipcrc);
               v.write(conv_integer(r.rpnt)) := rxo.dataout(17);
             when 8 =>
               ipcrctmp := (others => '0');
               ipcrctmp(1 downto 0) := r.ipcrc(17 downto 16);
               ipcrctmp2 := "00" & r.ipcrc(15 downto 0);
               v.ipcrc :=
               crcadder(ipcrctmp, ipcrctmp2);
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
               v.edclrstate := ipdata;
             when others =>
               null;
           end case;
         end if;
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when ipdata =>
         if (vrxwrite and r.ewr and not r.nak) = '1' and
                               (r.rcntm /= ebufmax) then
           veri.writem := '1'; veri.writel := '1';
           v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
         end if;
         if rxdone = '1' then
           v.edclrstate := ipcrc; v.rcntm := conv_std_logic_vector(6, bpbits);
           ipcrctmp := (others => '0');
           ipcrctmp(1 downto 0) := r.ipcrc(17 downto 16);
           ipcrctmp2 := "00" & r.ipcrc(15 downto 0);
           v.ipcrc := crcadder(ipcrctmp, ipcrctmp2);
           if conv_integer(v.rxstatus(3 downto 0)) /= 0 then
             v.edclrstate := idle;
           end if;
         end if;
       when ipcrc =>
         veri.writem := '1'; veri.datain(31 downto 16) := not r.ipcrc(15 downto 0);
         v.edclrstate := udp; v.rcntm := conv_std_logic_vector(9, bpbits);
         v.rcntl := conv_std_logic_vector(9, bpbits);
       when udp =>
         veri.writem := '1'; veri.writel := '1';
         v.edclrstate := iplength;
         veri.datain(31 downto 16) := r.udpsrc;
         veri.datain(15 downto 0) := r.applength + 18;
         v.rcntm := conv_std_logic_vector(4, bpbits);
       when iplength =>
         veri.writem := '1';
         veri.datain(31 downto 16) := r.applength + 38;
         v.edclrstate := oplength;
         v.rcntm := conv_std_logic_vector(10, bpbits);
         v.rcntl := conv_std_logic_vector(10, bpbits);
       when oplength =>
         if rxstart = '0' then
           v.abufs := r.abufs + 1; v.rpnt := r.rpnt + 1;
           veri.writel := '1'; veri.writem := '1';
         end if;
         if r.nak = '0' then
           v.seq := r.seq + 1;
         end if;
         v.edclrstate := idle;
         veri.datain(31 downto 0) := (others => '0');
         veri.datain(15 downto 0) := "00000" & r.nak & r.oplen;
       when arp =>
         if vrxwrite = '1' then
           v.ecnt := r.ecnt + 1;
           veri.writem := '1'; veri.writel := '1';
           case vecnt is
             when 0 =>
               v.rcntm := r.rcntm + 4; 
             when 1 =>
               swap := '1'; veri.writel := '0'; 
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 4;
             when 2 =>
               swap := '1';
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1;
             when 3 =>
               swap := '1';
               v.rcntm := r.rcntm - 4; v.rcntl := r.rcntl - 4;
             when 4 =>
               veri.datain := ctrli.emacaddr(31 downto 16) & ctrli.emacaddr(47 downto 32);
               v.rcntm := r.rcntm + 1; v.rcntl := r.rcntl + 1; 
             when 5 =>
               v.rcntl := r.rcntl + 1;
               veri.datain(31 downto 16) := rxo.dataout(15 downto 0);
               veri.datain(15 downto 0) := ctrli.emacaddr(15 downto 0);
               if rxo.dataout(15 downto 0) /= ctrli.edclip(31 downto 16) then
                 v.edclrstate := spill;
               end if;
             when 6 =>
               swap := '1'; veri.writem := '0'; 
               v.rcntm := conv_std_logic_vector(5, bpbits);
               v.rcntl := conv_std_logic_vector(1, bpbits);
               if rxo.dataout(31 downto 16) /= ctrli.edclip(15 downto 0) then
                 v.edclrstate := spill;
               else
                 v.edclactive := '1'; 
               end if;
             when 7 =>
               veri.writem := '0';
               veri.datain(15 downto 0) := ctrli.emacaddr(47 downto 32);
               v.rcntl := r.rcntl + 1;
               v.rcntm := conv_std_logic_vector(2, bpbits);
             when 8 =>
               v.edclrstate := arpop;
               veri.datain := ctrli.emacaddr(31 downto 0);
               v.rcntm := conv_std_logic_vector(5, bpbits);
             when others =>
               null;
           end case;
         end if;
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
       when arpop =>
         veri.writem := '1'; veri.datain(31 downto 16) := X"0002";
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
           if conv_integer(v.rxstatus) = 0 and (rxo.gotframe = '1') then
             v.abufs := r.abufs + 1; v.rpnt := r.rpnt + 1;
           end if;
         end if;
       when spill =>
         if (rxdone and not rxstart) = '1' then
           v.edclrstate := idle;
         end if;
     end case;
          
     --edcl transmitter
     case r.txdstate is
       when getlen =>
         v.tcnt := r.tcnt + 1;
         if conv_integer(r.tcnt) = 10 then
           v.txlength := '0' & erdata(9 downto 0);
           v.tnak := erdata(10);
           v.txcnt := v.txlength;
           if (r.write(conv_integer(r.tpnt)) or v.tnak) = '1' then
             v.txlength := (others => '0');
           end if;
         end if;
         if conv_integer(r.tcnt) = 11 then
           v.txdstate := readhdr;
           v.tcnt := (others => '0');
         end if;
       when readhdr =>
         v.tcnt := r.tcnt + 1; vtxfi.write := '1';
         v.tfwpnt := r.tfwpnt + 1; v.tfcnt := v.tfcnt + 1; 
         vtxfi.datain := erdata;
         if conv_integer(r.tcnt) = 12 then
           v.txaddr := erdata(31 downto 2);
         end if;
         if conv_integer(r.tcnt) = 3 then
           if erdata(31 downto 16) = X"0806" then
             v.tarp := '1'; v.txlength := conv_std_logic_vector(42, 11);
           else
             v.tarp := '0'; v.txlength := r.txlength + 52;
           end if;
         end if;
         if r.tarp = '0' then
           if conv_integer(r.tcnt) = 12 then
             v.txdstate := start;
           end if;
         else
           if conv_integer(r.tcnt) = 10 then
             v.txdstate := start;
           end if;
         end if;
         if (txrestart or txdone) = '1' then
           v.txdstate := etdone;
         end if;
       when start =>
         v.tmsto.addr := r.txaddr & "00"; 
         v.tmsto.write := r.write(conv_integer(r.tpnt));
         -- AXI_ENABLE: EDCL burst length decoded from payload
         v.tmsto.burst_bytes := r.txcnt;
         if (edclsepahbg /= 0) and (edcl /= 0) then
           v.tmsto2.addr := r.txaddr & "00"; 
           v.tmsto2.write := r.write(conv_integer(r.tpnt));
           -- AXI_ENABLE: EDCL burst length decoded from payload
           v.tmsto2.burst_bytes := r.txcnt;
         end if;
         if (conv_integer(r.txcnt) = 0) or (r.tarp or r.tnak) = '1' then
           v.txdstate := etdone;
           v.txstart_sync := not r.txstart_sync;
           v.tmsto.req := '0';
           if (edclsepahbg /= 0) and (edcl /= 0) then
             v.tmsto2.req := '0';
           end if;
         elsif r.write(conv_integer(r.tpnt)) = '0' then
           v.txdstate := req; v.tedcl := '1';
         else
           v.txstart_sync := not r.txstart_sync;
           v.tedcl := '1';
           v.tcnt := r.tcnt + 1;
           if (edclsepahbg = 0) or (edcl = 0) or (r.edclsepahb = '0') then
             v.tmsto.req := '1'; v.tmsto.data := erdata;
             v.txdstate := wrbus1; 
           else
             v.tmsto2.req := '1'; v.tmsto2.data := erdata;
             v.txdstate := wrbus2; 
           end if;
         end if;
         if (txrestart or txdone) = '1' then
           v.txdstate := etdone;
         end if;
       when wrbus1 =>
         if tmsti.grant = '1' then
           v.tmsto.addr := r.tmsto.addr + 4;
           if ((conv_integer(r.txcnt) <= 4) and (tmsti.ready = '0')) or
              ((conv_integer(r.txcnt) <= 8) and (tmsti.ready = '1')) then
             v.tmsto.req := '0';
           end if;
         end if;
         if (tmsti.ready or tmsti.error) = '1' then
           v.tmsto.data := erdata; v.tcnt := r.tcnt + 1;
           v.txcnt := r.txcnt - 4;
           if r.tmsto.req = '0' then
             v.txdstate := etdone;
           end if;
         end if;
         if tmsti.retry = '1' then
           v.tmsto.addr := r.tmsto.addr - 4; v.tmsto.req := '1';
         end if;
       when wrbus2 =>
         if tmsti2.grant = '1' then
           v.tmsto2.addr := r.tmsto2.addr + 4;
           if ((conv_integer(r.txcnt) <= 4) and (tmsti2.ready = '0')) or
              ((conv_integer(r.txcnt) <= 8) and (tmsti2.ready = '1')) then
             v.tmsto2.req := '0';
           end if;
         end if;
         if (tmsti2.ready or tmsti2.error) = '1' then
           v.tmsto2.data := erdata; v.tcnt := r.tcnt + 1;
           v.txcnt := r.txcnt - 4;
           if r.tmsto2.req = '0' then
             v.txdstate := etdone;
           end if;
         end if;
         if tmsti2.retry = '1' then
           v.tmsto2.addr := r.tmsto2.addr - 4; v.tmsto2.req := '1';
         end if;
       when etdone =>
         if txdone = '1' then
           v.txdstate := idle; v.txdone(nsync) := r.txdone(nsync-1);
           v.abufs := v.abufs - 1; v.tpnt := r.tpnt + 1;
           v.tfcnt := (others => '0'); v.tfrpnt := (others => '0');
           v.tfwpnt := (others => '0');
         elsif txrestart = '1' then
           v.txdstate := idle;
         end if;
       when others =>
         null;
     end case;

     if swap = '1' then
       veri.datain(31 downto 16) := rxo.dataout(15 downto 0); 
       veri.datain(15 downto 0)  := rxo.dataout(31 downto 16);
     end if;
     if setmz = '1' then
       veri.datain(31 downto 16) := (others => '0');
     end if;
     if (ramdebug /= 2) or (edcl = 0) or (cmdi.dbg_rd_ena = '0') then
       veri.raddress := r.tpnt & v.tcnt;
     end if;
   end if;

   --edcl duplex mode read
   if (rmii = 1) or (edcl /= 0) then
     --edcl, gbit link mode check
     case r.duplexstate is
       when start =>
         if (ctrli.edcldis = '0' and ctrli.disableduplex = '0') then
           v.status.mdio.cmd.regadr := r.regaddr; v.init_busy := '1';
           v.status.mdio.busy := '1'; v.duplexstate := waitop;
           if (r.phywr or r.rstphy) = '1' then
             v.status.mdio.cmd.write := '1'; 
           else
             v.status.mdio.cmd.read := '1';
           end if;
           if r.rstphy = '1' then
             v.status.mdio.cmd.data := X"9000";
           end if;
         end if;
       when waitop =>
         if r.init_busy = '0' then
           if r.status.mdio.linkfail = '1' then
             v.duplexstate := start; 
           elsif r.rstphy = '1' then
             v.duplexstate := start; v.rstphy := '0';
           else
             v.duplexstate := nextop;
           end if;
         end if;
       when nextop =>
         case r.regaddr is
           when "00000" =>
             if r.status.mdio.cmd.data(15) = '1' then --rst not finished
               v.duplexstate := start; 
             elsif (r.phywr and not r.rstaneg) = '1' then --forced to 10 Mbit HD
               v.duplexstate := selmode;
             elsif r.status.mdio.cmd.data(12) = '0' then --no auto neg
               v.duplexstate := start; v.phywr := '1';
               v.status.mdio.cmd.data := (others => '0');
             else
               v.duplexstate := start; v.regaddr := "00001";
             end if;
             if r.rstaneg = '1' then
               v.phywr := '0';
             end if;
             if ctrli.disableduplex = '1' then
               v.duplexstate := done; v.status.mdio.busy := '0';
             end if;
           when "00001" =>
             v.ext := r.status.mdio.cmd.data(8); --extended status register
             v.extcap := r.status.mdio.cmd.data(1); --extended register capabilities
             v.duplexstate := start;
             if r.status.mdio.cmd.data(0) = '0' then
               --no extended register capabilites, unable to read aneg config
               --forcing 10 Mbit
               v.duplexstate := start; v.phywr := '1';
               v.status.mdio.cmd.data := (others => '0');
               v.regaddr := (others => '0');
             elsif (r.status.mdio.cmd.data(8) and not r.rstaneg) = '1' then
               --phy gbit capable, disable gbit
               v.regaddr := "01001"; 
             elsif r.status.mdio.cmd.data(5) = '1' then --auto neg completed
               v.regaddr := "00100";
             end if;
             if ctrli.disableduplex = '1' then
               v.duplexstate := done; v.status.mdio.busy := '0';
             end if;
           when "00100" =>
             v.duplexstate := start; v.regaddr := "00101";
             v.capbil(4 downto 0) := r.status.mdio.cmd.data(9 downto 5);
           when "00101" =>
             v.duplexstate := selmode;
             v.capbil(4 downto 0) :=
             r.capbil(4 downto 0) and r.status.mdio.cmd.data(9 downto 5);
           when "01001" =>
             if r.phywr = '0' then
               v.duplexstate := start; v.phywr := '1';
               v.status.mdio.cmd.data(9 downto 8) := (others => '0');
             else
               v.regaddr := "00000";
               v.duplexstate := start; v.phywr := '1';
               v.status.mdio.cmd.data := X"3300"; v.rstaneg := '1';
             end if;
           when others =>
             null;
         end case;
       when selmode =>
         v.duplexstate := done; v.status.mdio.busy := '0';
         if r.phywr = '1' then
           v.status.full_duplex := '0'; v.status.speed := '0';
         else
           sel_op_mode(r.capbil, v.status.speed, v.status.full_duplex);
         end if;
       when done =>
         null;
     end case;

     -- MDIO Disable
     if ctrli.edcldis = '1' or ctrli.disableduplex = '1' then
        if  v.duplexstate /= start then
          v.duplexstate := start;
          v.status.mdio.cmd.regadr := (others => '0');
          v.status.mdio.busy := '0';
          v.init_busy := '0';
          v.status.mdio.cmd.write := '0';
          v.status.mdio.cmd.read := '0';
          v.status.mdio.cmd.data := X"0000";
        end if;
     end if;

   end if;

   --transmitter retry
   if tmsti.retry = '1' then
     v.tmsto.req := '1'; v.tmsto.addr := r.tmsto.addr - 4;
     v.txburstcnt := r.txburstcnt - 1;
   end if;

   --transmitter AHB error
   if tmsti.error = '1' and (not ((edcl /= 0) and (r.tedcl = '1'))) then
     v.tmsto.req := '0'; v.txdstate := ahberror;
   end if;

   if (edclsepahbg /= 0) and (edcl /= 0) then
     --transmitter retry
     if tmsti2.retry = '1' then
       v.tmsto2.req := '1'; v.tmsto2.addr := r.tmsto2.addr - 4;
       v.txburstcnt := r.txburstcnt - 1;
     end if;

     --transmitter AHB error
     if tmsti2.error = '1' and (not ((edcl /= 0) and (r.tedcl = '1'))) then
       v.tmsto2.req := '0'; v.txdstate := ahberror;
     end if;
   end if;
    
   --receiver retry
   if rmsti.retry = '1' then
     v.rmsto.req := '1'; v.rmsto.addr := r.rmsto.addr - 4;
     v.rxburstcnt := r.rxburstcnt - 1;
   end if;

------------------------------------------------------------------------------
-- RESET ----------------------------------------------------------------------
-------------------------------------------------------------------------------
    if irst = '0' then
      v.txdstate := idle; v.rxdstate := idle; v.rfrpnt := (others => '0');
      v.rfwpnt := (others => '0'); 
      v.rfcnt := (others => '0'); 
      v.status.txen := '0';
      v.status.tx_int := '0';
      v.status.rx_int := '0';
      v.status.tx_err := '0';
      v.status.rx_err := '0';
      v.status.txahberr := '0';
      v.status.rxahberr := '0';
      v.txirqgen := '0'; v.status.rxen := '0';
      v.status.txdsel := (others => '0'); v.txstart_sync := '0';
      v.txread := (others => '0'); v.txrestart := (others => '0');
      v.txdone := (others => '0'); v.txreadack := '0'; 
      v.status.rxdsel := (others => '0'); v.rxdone := (others => '0');
      v.rxdoneold := '0'; v.rxdoneack := '0'; v.rxwriteack := '0';
      v.rxstart := (others => '0'); v.rxwrite := (others => '0');
      v.status.invaddr := '0'; v.status.toosmall := '0';
      v.status.full_duplex := '0'; v.writeok := '1';
      if (enable_mdio = 0) or (edcl /= 0) then
        v.status.reset := '0';
      end if;
      if enable_mdint = 1 then
        v.status.phystat := '0';
      end if;
      if (edcl /= 0) then
        v.tpnt := (others => '0'); v.rpnt := (others => '0');
        v.tcnt := (others => '0'); v.edclactive := '0';
        v.tarp := '0'; v.abufs := (others => '0');
        v.edclrstate := idle;
      end if;
      if (rmii = 1) then
        v.status.speed := '1';
      else
        v.status.speed := '1';
      end if;
    end if;

    if edcl = 0 then
      v.edclrstate := idle; v.edclactive := '0'; v.nak := '0'; v.ewr := '0';
      v.write := (others => '0'); v.seq := (others => '0'); v.abufs := (others => '0');
      v.tpnt := (others => '0'); v.rpnt := (others => '0'); v.tcnt := (others => '0');
      v.rcntm := (others => '0'); v.rcntl := (others => '0'); v.ipcrc := (others => '0');
      v.applength := (others => '0'); v.oplen := (others => '0');
      v.udpsrc := (others => '0'); v.ecnt := (others => '0'); v.tarp := '0';
      v.tnak := '0'; v.tedcl := '0'; v.edclbcast := '0';
    end if;

    --some parts of edcl are only affected by hw reset
    if rst = '0' then
      v.duplexstate := start; v.regaddr := (others => '0');
      v.phywr := '0'; v.rstphy := '1'; v.rstaneg := '0';
      v.seq := (others => '0');
      v.mdioo := '0';
      if (enable_mdio = 1) then
        v.mdccnt := divisor; v.mdioclk := '0';
      end if;
      v.status.reset := '0';
      if (enable_mdio = 1) then
        v.mdio_state := idle; v.status.mdio.cmd.read := '0';
        v.status.mdio.cmd.valid := '0'; 
        v.status.mdio.cmd.write := '0'; v.status.mdio.busy := '0';
        v.status.mdio.cmd.data := (others => '0');
        v.status.mdio.cmd.regadr := (others => '0');
        v.status.reset := '0'; v.status.mdio.linkfail := '1';
        if OEPOL = 0 then v.mdioen := '1'; else v.mdioen := '0'; end if;
        v.cnt := (others => '0');
      end if;
      if edclsepahbg /= 0 then
        v.edclsepahb := edclsepahb;
      end if;
     v.txcnt := (others => '0'); v.txburstcnt := (others => '0');
     v.tedcl := '0'; v.erenable := '0';
     v.addrok := '0';
     v.rxburstcnt := (others => '0'); v.addrdone := '0';
     v.rxcnt := (others => '0'); v.rxdoneold := '0';
     v.ctrlpkt := '0'; v.bcast := '0'; v.edclactive := '0';
     v.msbgood := '0'; v.rxrenable := '0';
     if multicast = 1 then
       v.mcast := '0'; v.mcastacc := '0';
     end if;
     v.tnak := '0'; v.tedcl := '0'; v.edclbcast := '0';
     v.gotframe := '0';
     v.rxbytecount := (others => '0'); v.rxlength := (others => '0');
     v.txburstav := '0'; v.txdataav := '0';
     v.txstatus := (others => '0'); v.txstart := '0'; 
     v.tfcnt := (others => '0'); v.tfrpnt := (others => '0');
     v.tfwpnt := (others => '0'); v.txaddr := (others => '0');
     v.txdata := (others => '0');
     v.txvalid := '0';
     v.txlength := (others => '0');
     v.cnt := (others => '0');
     v.rxaddr := (others => '0');
     v.rxstatus := (others => '0');
     v.rxwrap := '0'; v.rxden := '0';
     
     v.rmsto.req := '0';
     v.rmsto.write := '0';
     v.rmsto.addr := (others => '0');
     v.rmsto.data := (others => '0');
     v.tmsto.req := '0';
     v.tmsto.write := '0';
     v.tmsto.addr := (others => '0');
     v.tmsto.data := (others => '0');
     v.tmsto2.req := '0';
     v.tmsto2.write := '0';
     v.tmsto2.addr := (others => '0');
     v.tmsto2.data := (others => '0');
     v.nak := '0'; v.ewr := '0';
     v.write := (others => '0');
     v.applength := (others => '0');
     v.oplen := (others => '0');
     v.udpsrc := (others => '0'); v.ecnt := (others => '0');
     v.rcntm := (others => '0'); v.rcntl := (others => '0');
     v.txwrap := '0';
     v.txden := '0';
     v.txirq := '0';
     v.rxirq := '0';
    end if;
-------------------------------------------------------------------------------
-- SIGNAL ASSIGNMENTS ---------------------------------------------------------
-------------------------------------------------------------------------------
    rin           <= v;
    rdbgdatao     <= vrdbgdata;
    irq           <= vpirq;

    --rx ahb fifo
    rxrenable                         <= vrxfi.renable;
    rxraddress(10 downto fabits)      <= (others => '0');
    rxraddress(fabits-1 downto 0)     <= vrxfi.raddress;
    rxwrite                           <= vrxfi.write;  
    rxwdata                           <= vrxfi.datain;
    rxwaddress(10 downto fabits)      <= (others => '0');
    rxwaddress(fabits-1 downto 0)     <= vrxfi.waddress;

    --tx ahb fifo  
    txrenable                         <= vtxfi.renable;
    txraddress(10 downto txfabits)    <= (others => '0');
    txraddress(txfabits-1 downto 0)   <= vtxfi.raddress;
    txwrite                           <= vtxfi.write;
    txwdata                           <= vtxfi.datain;
    txwaddress(10 downto txfabits)    <= (others => '0');
    txwaddress(txfabits-1 downto 0)   <= vtxfi.waddress;

    --edcl buf     
    erenable                          <= veri.renable;
    eraddress(15 downto eabits)       <= (others => '0');
    eraddress(eabits-1 downto 0)      <= veri.raddress;
    ewritem                           <= veri.writem;
    ewritel                           <= veri.writel;
    ewaddressm(15 downto eabits)      <= (others => '0');
    ewaddressm(eabits-1 downto 0)     <= veri.waddressm(eabits-1 downto 0);
    ewaddressl(15 downto eabits)      <= (others => '0');
    ewaddressl(eabits-1 downto 0)     <= veri.waddressl(eabits-1 downto 0);
    ewdata                            <= veri.datain;

    rxi.enable    <= vrxenable;
  end process;
  
  statuso <= r.status;

  rxi.writeack    <= r.rxwriteack;
  rxi.doneack     <= r.rxdoneack;
  rxi.speed       <= r.status.speed;
  rxi.writeok     <= r.writeok;
  rxi.rxd         <= rxd;
  rxi.rx_dv       <= rx_dv;
  rxi.rx_crs      <= rx_crs;
  rxi.rx_er       <= rx_er;
  rxi.rx_en       <= rx_en;
  
  txi.rx_col      <= rx_col;
  txi.rx_crs      <= rx_crs;
  txi.full_duplex <= r.status.full_duplex;
  txi.start       <= r.txstart_sync;
  txi.readack     <= r.txreadack;
  txi.speed       <= r.status.speed;
  txi.data        <= r.txdata;
  txi.valid       <= r.txvalid;
  txi.len         <= r.txlength;
  txi.datavalid   <= tx_dv;
  
  mdc             <= r.mdioclk;
  mdio_o          <= r.mdioo;
  mdio_oe         <= testoen when (scanen/=0 and testen/='0') else r.mdioen;
  tmsto           <= r.tmsto;
  rmsto           <= r.rmsto;
  tmsto2          <= r.tmsto2;

  txd             <= txo.txd;
  tx_en           <= txo.tx_en;
  tx_er           <= txo.tx_er;

  speed           <= r.status.speed;

  reset     <= irst;

  regs : process(clk) is
  begin
    if rising_edge(clk) then r <= rin; end if;
  end process;
-------------------------------------------------------------------------------
-- TRANSMITTER-----------------------------------------------------------------
-------------------------------------------------------------------------------
  tx_rmii0 : if rmii = 0 generate
    tx0: greth_tx
      generic map(
        ifg_gap        => ifg_gap,
        attempt_limit  => attempt_limit,
        backoff_limit  => backoff_limit,
        nsync          => nsync,
        rmii           => rmii,
        gmiimode       => gmiimode
        )
      port map(
        rst            => arst,
        clk            => tx_clk,
        txi            => txi,
        txo            => txo);
  end generate;

  tx_rmii1 : if rmii = 1 generate
    tx0: greth_tx
      generic map(
        ifg_gap        => ifg_gap,
        attempt_limit  => attempt_limit,
        backoff_limit  => backoff_limit,
        nsync          => nsync,
        rmii           => rmii,
        gmiimode       => gmiimode
        )
      port map(
        rst            => arst,
        clk            => rmii_clk,
        txi            => txi,
        txo            => txo);
  end generate;
  
-------------------------------------------------------------------------------
-- RECEIVER -------------------------------------------------------------------
-------------------------------------------------------------------------------
  rx_rmii0 : if rmii = 0 generate
    rx0 : greth_rx
      generic map(
        nsync     => nsync,
        rmii      => rmii,
        multicast => multicast,
        maxsize   => maxsize,
        gmiimode  => gmiimode
        )
      port map(
        rst   => arst,
        clk   => rx_clk,
        rxi   => rxi,  
        rxo   => rxo);
  end generate;

  rx_rmii1 : if rmii = 1 generate
    rx0 : greth_rx
      generic map(
        nsync     => nsync,
        rmii      => rmii,
        multicast => multicast,
        maxsize   => maxsize,
        gmiimode  => gmiimode)
      port map(
        rst   => arst,
        clk   => rmii_clk,
        rxi   => rxi,  
        rxo   => rxo);
  end generate;

  --! Tx FIFO
  tx_fifo0 : syncram_2p_tech generic map (
    tech => memtech, 
    abits => txfabits,
    dbits => 32,
    sepclk => 0
  ) port map (
    clk, 
    txrenable, 
    txraddress(txfabits-1 downto 0), 
    txrdata, 
    clk,
    txwrite, 
    txwaddress(txfabits-1 downto 0), 
    txwdata
  );

  --! Rx FIFO
  rx_fifo0 : syncram_2p_tech generic map (
    tech => memtech, 
    abits => fabits,
    dbits => 32, 
    sepclk => 0
  ) port map (
    clk, 
    rxrenable, 
    rxraddress(fabits-1 downto 0), 
    rxrdata, 
    clk,
    rxwrite,
    rxwaddress(fabits-1 downto 0), 
    rxwdata
  );

  --! EDCL buffer ram
  edclramnft : if (edcl /= 0) generate
    r0 : syncram_2p_tech generic map (
       memtech, 
       eabits, 
       16
    ) port map (
       clk, 
       erenable, 
       eraddress(eabits-1 downto 0), 
       erdata(31 downto 16), 
       clk,
       ewritem, 
       ewaddressm(eabits-1 downto 0), 
       ewdata(31 downto 16)
    ); 
    r1 : syncram_2p_tech generic map (
       memtech, 
       eabits, 
       16
    ) port map(
       clk, 
       erenable, 
       eraddress(eabits-1 downto 0), 
       erdata(15 downto 0), 
       clk,
       ewritel, 
       ewaddressl(eabits-1 downto 0), 
       ewdata(15 downto 0)
    ); 
  end generate;

  
end architecture;

