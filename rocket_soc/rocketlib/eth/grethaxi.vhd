-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Implementation of the grethaxi device.
--! @details    This is Ethernet MAC device with the AMBA AXI inteface
--!             and EDCL debugging functionality.
------------------------------------------------------------------------------
--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! Ethernet specific declarations.
library rocketlib;
use rocketlib.grethpkg.all;

entity grethaxi is
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
end entity;
  
architecture arch_grethaxi of grethaxi is

  constant bufsize : std_logic_vector(2 downto 0) :=
                       conv_std_logic_vector(log2(edclbufsz), 3);
                       
  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 4;

  constant xslvconfig : nasti_slave_config_type := (
     xindex => xslvindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ETHMAC,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );

  constant xmstconfig : nasti_master_config_type := (
     xindex => xmstindex,
     vid => VENDOR_GNSSSENSOR,
     did => GAISLER_ETH_MAC_MASTER,
     descrtype => PNP_CFG_TYPE_MASTER,
     descrsize => PNP_CFG_MASTER_DESCR_BYTES
  );

  constant xmstconfig2 : nasti_master_config_type := (
     xindex => xmstindex2,
     vid => VENDOR_GNSSSENSOR,
     did => GAISLER_ETH_EDCL_MASTER,
     descrtype => PNP_CFG_TYPE_MASTER,
     descrsize => PNP_CFG_MASTER_DESCR_BYTES
  );

  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of std_logic_vector(15 downto 0);

  type registers is record
      bank_slv : nasti_slave_bank_type;
      ctrl     : eth_control_type;
  end record;

  signal r, rin         : registers;
  signal imac_cmd       : eth_command_type;
  signal omac_status    : eth_mac_status_type;
  signal omac_rdbgdata  : std_logic_vector(31 downto 0);
  
  signal omac_tmsto          : eth_tx_ahb_in_type;
  signal imac_tmsti          : eth_tx_ahb_out_type;

  signal omac_tmsto2         : eth_tx_ahb_in_type;
  signal imac_tmsti2         : eth_tx_ahb_out_type;

  signal omac_rmsto          : eth_rx_ahb_in_type;
  signal imac_rmsti          : eth_rx_ahb_out_type;
  
  procedure procedureReadReg(addr      : in std_logic_vector(15 downto 0);
                             dbg_rdata : in std_logic_vector(31 downto 0);
                             ir         : in eth_control_type;
                             macstat   : in eth_mac_status_type;
                             ovcmd     : out eth_command_type;
                             ordata  : out std_logic_vector(31 downto 0)) is
  begin
   ordata := (others => '0');
   if (ramdebug = 0) or (addr(15 downto 14) = "00") then 
     case addr(3 downto 0) is
     when "0000" => --ctrl reg
       if ramdebug /= 0 then
         ordata(13) := ir.ramdebugen;
       end if;
       if (edcl /= 0) then
         ordata(31) := '1';
         ordata(30 downto 28) := bufsize;
         ordata(14) := ir.edcldis;
         ordata(12) := ir.disableduplex;
       end if;
       if enable_mdint = 1 then
         ordata(26) := '1';
         ordata(10) := ir.pstatirqen;
       end if;
       if multicast = 1 then
         ordata(25) := '1';
         ordata(11) := ir.mcasten;
       end if;
       if rmii = 1 then
       ordata(7) := macstat.speed;
       end if;
       ordata(6) := macstat.reset;
       ordata(5) := ir.prom;
       ordata(4) := macstat.full_duplex;
       ordata(3) := ir.rx_irqen;
       ordata(2) := ir.tx_irqen;
       ordata(1) := macstat.rxen;
       ordata(0) := macstat.txen; 
     when "0001" => --status/int source reg
       ordata(9) := not (macstat.edcltx_idle or macstat.edclrx_idle);
       if enable_mdint = 1 then
         ordata(8) := macstat.phystat;
       end if;
       ordata(7) := macstat.invaddr;
       ordata(6) := macstat.toosmall;
       ordata(5) := macstat.txahberr;
       ordata(4) := macstat.rxahberr;
       ordata(3) := macstat.tx_int;
       ordata(2) := macstat.rx_int;
       ordata(1) := macstat.tx_err;
       ordata(0) := macstat.rx_err; 
     when "0010" => --mac addr msb/mdio address
       ordata(15 downto 0) := ir.mac_addr(47 downto 32);
     when "0011" => --mac addr lsb
       ordata := ir.mac_addr(31 downto 0); 
     when "0100" => --mdio ctrl/status
       ordata(31 downto 16) := macstat.mdio.cmd.data;
       ordata(15 downto 11) := ir.mdio_phyadr;
       ordata(10 downto 6) :=  macstat.mdio.cmd.regadr;  
       ordata(3) := macstat.mdio.busy;
       ordata(2) := macstat.mdio.linkfail;
       ordata(1) := macstat.mdio.cmd.read;
       ordata(0) := macstat.mdio.cmd.write; 
     when "0101" => --tx descriptor 
       ordata(31 downto 10) := ir.txdesc;
       ordata(9 downto 3)   := macstat.txdsel;
     when "0110" => --rx descriptor
       ordata(31 downto 10) := ir.rxdesc;
       ordata(9 downto 3)   := macstat.rxdsel;
     when "0111" => --edcl ip
       if (edcl /= 0) then
          ordata := ir.edclip;
       end if;
     when "1000" =>
       if multicast = 1 then
         ordata := ir.hash(63 downto 32);
       end if;
     when "1001" =>
       if multicast = 1 then
         ordata := ir.hash(31 downto 0);
       end if;
     when "1010" =>
       if edcl /= 0 then
         ordata(15 downto 0) := ir.emacaddr(47 downto 32);
       end if;
     when "1011" =>
       if edcl /= 0 then
         ordata := ir.emacaddr(31 downto 0);
       end if;
     when others => null; 
     end case;
   elsif addr(15 downto 14) = "01" then
       if ramdebug /= 0 then
         ovcmd.dbg_access_id := DBG_ACCESS_TX_BUFFER;
         ovcmd.dbg_rd_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ordata            := dbg_rdata;
       end if;
   elsif addr(15 downto 14) = "10" then
       if ramdebug /= 0 then
         ovcmd.dbg_access_id := DBG_ACCESS_RX_BUFFER;
         ovcmd.dbg_rd_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ordata              := dbg_rdata;
       end if;
   elsif addr(15 downto 14) = "11" then 
       if (ramdebug = 2) and (edcl /= 0) then
         ovcmd.dbg_access_id := DBG_ACCESS_EDCL_BUFFER;
         ovcmd.dbg_rd_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ordata              := dbg_rdata;
       end if;
   end if;
  end procedure procedureReadReg;

  procedure procedureWriteReg(addr      : in std_logic_vector(15 downto 0);
                              strobs    : in std_logic_vector(3 downto 0);
                              wdata     : in std_logic_vector(31 downto 0);
                              ir         : in eth_control_type;
                              macstat   : in eth_mac_status_type;
                              ovcmd     : out eth_command_type;
                              ov        : out eth_control_type) is
  begin
    if strobs /= "0000" then
      if (ramdebug = 0) or (addr(15 downto 14) = "00") then 
       case addr(3 downto 0) is
       when "0000" => --ctrl reg
         if ramdebug /= 0 then
           ov.ramdebugen := wdata(13);
         end if;
         if edcl /= 0 then
           ov.edcldis  := wdata(14);
           ov.disableduplex := wdata(12);
         end if;
         if multicast = 1 then
           ov.mcasten := wdata(11);
         end if;
         if enable_mdint = 1 then
           ov.pstatirqen  := wdata(10);
         end if;
         if rmii = 1 then
           ovcmd.set_speed       := wdata(7);  
           ovcmd.clr_speed       := not wdata(7);  
         end if;
         ovcmd.set_reset       := wdata(6);
         ovcmd.clr_reset       := not wdata(6);
         ov.prom               := wdata(5); 
         ovcmd.set_full_duplex := wdata(4);
         ovcmd.clr_full_duplex := not wdata(4);
         ov.rx_irqen           := wdata(3);
         ov.tx_irqen           := wdata(2);
         ovcmd.set_rxena     := wdata(1);
         ovcmd.clr_rxena     := not wdata(1);
         ovcmd.set_txena     := wdata(0);
         ovcmd.clr_txena     := not wdata(0);
       when "0001" => --status/int source reg
         if enable_mdint = 1 then
           ovcmd.clr_status_phystat := wdata(8);
         end if;
         ovcmd.clr_status_invaddr  := wdata(7);
         ovcmd.clr_status_toosmall := wdata(6);
         ovcmd.clr_status_txahberr := wdata(5);
         ovcmd.clr_status_rxahberr := wdata(4);
         ovcmd.clr_status_tx_int := wdata(3);
         ovcmd.clr_status_rx_int := wdata(2);
         ovcmd.clr_status_tx_err := wdata(1);
         ovcmd.clr_status_rx_err := wdata(0);
       when "0010" => --mac addr msb
         ov.mac_addr(47 downto 32) := wdata(15 downto 0);
       when "0011" => --mac addr lsb
         ov.mac_addr(31 downto 0)  := wdata(31 downto 0);
       when "0100" => --mdio ctrl/status
         if enable_mdio = 1 then
           ovcmd.mdio_cmd.valid := not macstat.mdio.busy;
           if macstat.mdio.busy = '0' then
             ov.mdio_phyadr := wdata(15 downto 11);
           end if;
           ovcmd.mdio_cmd.data   := wdata(31 downto 16);
           ovcmd.mdio_cmd.regadr := wdata(10 downto 6);
           ovcmd.mdio_cmd.read   := wdata(1);
           ovcmd.mdio_cmd.write  := wdata(0);
         end if;
       when "0101" => --tx descriptor 
         ovcmd.set_txdsel := '1';
         ovcmd.txdsel := wdata(9 downto 3);
         ov.txdesc := wdata(31 downto 10);
       when "0110" => --rx descriptor
         ovcmd.set_rxdsel := '1';
         ovcmd.rxdsel := wdata(9 downto 3);
         ov.rxdesc := wdata(31 downto 10);
       when "0111" => --edcl ip
         if (edcl /= 0) then
           ov.edclip := wdata;
         end if;
       when "1000" => --hash msb
         if multicast = 1 then
           ov.hash(63 downto 32) := wdata;
         end if;
       when "1001" => --hash lsb
         if multicast = 1 then
           ov.hash(31 downto 0) := wdata;
         end if;
       when "1010" =>
         if edcl /= 0 then
           ov.emacaddr(47 downto 32) := wdata(15 downto 0);
         end if;
       when "1011" =>
         if edcl /= 0 then
           ov.emacaddr(31 downto 0) := wdata;
         end if;
       when others => null; 
       end case;
     elsif addr(15 downto 14) = "01" then
       if ramdebug /= 0 then
         ovcmd.dbg_access_id := DBG_ACCESS_TX_BUFFER;
         ovcmd.dbg_wr_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ovcmd.dbg_wdata     := wdata;
       end if;
     elsif addr(15 downto 14) = "10" then  
       if ramdebug /= 0 then
         ovcmd.dbg_access_id := DBG_ACCESS_RX_BUFFER;
         ovcmd.dbg_wr_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ovcmd.dbg_wdata     := wdata;
       end if;
     elsif addr(15 downto 14) = "11" then 
       if (ramdebug = 2) and (edcl /= 0) then
         ovcmd.dbg_access_id := DBG_ACCESS_EDCL_BUFFER;
         ovcmd.dbg_wr_ena    := ir.ramdebugen;
         ovcmd.dbg_addr      := addr(13 downto 0);
         ovcmd.dbg_wdata     := wdata;
       end if;
     end if;
   end if;
  end procedure procedureWriteReg;

begin
  
  comb : process(r, ethi, slvi, omac_rdbgdata, omac_status, rst) is
      variable v        : registers;
      variable vcmd     : eth_command_type;
      variable raddr_reg : local_addr_array_type;
      variable waddr_reg : local_addr_array_type;
      variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
      variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
      variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
      variable val : std_logic_vector(8*ALIGNMENT_BYTES-1 downto 0);
  begin

    v := r;
    vcmd := eth_command_none;
    
    procedureAxi4(slvi, xslvconfig, r.bank_slv, v.bank_slv);

    for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
       raddr_reg(n) := r.bank_slv.raddr(ALIGNMENT_BYTES*n)(17 downto log2(ALIGNMENT_BYTES));
       val := (others => '0');
       
       procedureReadReg(raddr_reg(n), 
                        omac_rdbgdata, 
                        r.ctrl,
                        omac_status,
                        vcmd,
                        val);
       rdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n) := val;
    end loop;


    if slvi.w_valid = '1' and 
       r.bank_slv.wstate = wtrans and 
       r.bank_slv.wresp = NASTI_RESP_OKAY then

      wdata := slvi.w_data;
      wstrb := slvi.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
         waddr_reg(n) := r.bank_slv.waddr(ALIGNMENT_BYTES*n)(17 downto 2);

         procedureWriteReg(waddr_reg(n), 
                           wstrb(ALIGNMENT_BYTES*(n+1)-1 downto ALIGNMENT_BYTES*n), 
                           wdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n),
                           r.ctrl,
                           omac_status,
                           vcmd,
                           v.ctrl);
      end loop;
    end if;
   
    slvo <= functionAxi4Output(r.bank_slv, rdata);

   
------------------------------------------------------------------------------
-- RESET ----------------------------------------------------------------------
-------------------------------------------------------------------------------
    if rst = '0' then
      v.bank_slv := NASTI_SLAVE_BANK_RESET;
      v.ctrl.tx_irqen := '0';
      v.ctrl.rx_irqen := '0';
      v.ctrl.prom := '0';
      v.ctrl.pstatirqen := '0';
      v.ctrl.mcasten := '0';
      v.ctrl.ramdebugen := '0';
      if edcl = 3 then
        v.ctrl.edcldis  := ethi.edcldisable;
      elsif edcl /= 0 then
        v.ctrl.edcldis := '0';
      end if;
      v.ctrl.disableduplex := '0';
      if phyrstadr /= 32 then 
        v.ctrl.mdio_phyadr := conv_std_logic_vector(phyrstadr, 5);
      else
        v.ctrl.mdio_phyadr := ethi.phyrstaddr;
      end if;

      v.ctrl.mac_addr := (others => '0');
      
      v.ctrl.txdesc := (others => '0');
      v.ctrl.rxdesc := (others => '0');
      v.ctrl.hash := (others => '0');
      v.ctrl.edclip := conv_std_logic_vector(ipaddrh, 16) &
                      conv_std_logic_vector(ipaddrl, 16);
      v.ctrl.emacaddr := conv_std_logic_vector(macaddrh, 24) & 
                    conv_std_logic_vector(macaddrl, 24);
      if edcl > 1 then
        v.ctrl.edclip(3 downto 0) := ethi.edcladdr;
        v.ctrl.emacaddr(3 downto 0) := ethi.edcladdr;
      end if;
    end if;

   
   rin <= v;
   imac_cmd <= vcmd;
end process;

 slvcfg <= xslvconfig;
 mstcfg <= xmstconfig;
 mstcfg2 <= xmstconfig2;
  
 eth64 : grethc64 generic map (
    memtech        => memtech,
    ifg_gap        => ifg_gap,
    attempt_limit  => attempt_limit,
    backoff_limit  => backoff_limit,
    mdcscaler      => mdcscaler,
    enable_mdio    => enable_mdio,
    fifosize       => fifosize,
    nsync          => nsync,
    edcl           => edcl,
    edclbufsz      => edclbufsz,
    macaddrh       => macaddrh,
    macaddrl       => macaddrl,
    ipaddrh        => ipaddrh,
    ipaddrl        => ipaddrl,
    phyrstadr      => phyrstadr,
    rmii           => rmii,
    oepol          => oepol,
    scanen         => scanen,
    mdint_pol      => mdint_pol,
    enable_mdint   => enable_mdint,
    multicast      => multicast,
    edclsepahbg    => edclsepahbg,
    ramdebug       => ramdebug,
    mdiohold       => mdiohold,
    maxsize        => maxsize,
    gmiimode       => gmiimode
  ) port map (
    rst            => rst,
    clk            => clk,
    ctrli          => r.ctrl,
    cmdi           => imac_cmd,
    statuso        => omac_status,
    rdbgdatao      => omac_rdbgdata,
    --irq
    irq            => irq,
    --ethernet input signals
    rmii_clk       => ethi.rmii_clk,
    tx_clk         => ethi.tx_clk,
    rx_clk         => ethi.rx_clk,
    tx_dv          => ethi.tx_dv,
    rxd            => ethi.rxd,
    rx_dv          => ethi.rx_dv,
    rx_er          => ethi.rx_er,
    rx_col         => ethi.rx_col,
    rx_en          => ethi.rx_en,
    rx_crs         => ethi.rx_crs,
    mdio_i         => ethi.mdio_i,
    phyrstaddr     => ethi.phyrstaddr,
    mdint          => ethi.mdint,
    --ethernet output signals
    reset          => etho.reset,
    txd            => etho.txd,
    tx_en          => etho.tx_en,
    tx_er          => etho.tx_er,
    mdc            => etho.mdc,
    mdio_o         => etho.mdio_o,
    mdio_oe        => etho.mdio_oe,
    testrst        => '0',
    testen         => '0',
    testoen        => '0',
    edcladdr       => ethi.edcladdr,
    edclsepahb     => ethi.edclsepahb,
    edcldisable    => ethi.edcldisable,
    speed          => etho.speed,
    tmsto          => omac_tmsto,
    tmsti          => imac_tmsti,
    tmsto2         => omac_tmsto2,
    tmsti2         => imac_tmsti2,
    rmsto          => omac_rmsto,
    rmsti          => imac_rmsti
  );

  etho.tx_clk <= '0';
  etho.gbit <= '0';

  --! AXI Master interface providing DMA access
  axi0 : eth_axi_mst generic map (
     xindex => xmstindex
  ) port map (
     rst, 
     clk, 
     msti, 
     msto, 
     omac_tmsto, 
     imac_tmsti, 
     omac_rmsto, 
     imac_rmsti
  );

  edclmst_on : if edclsepahbg = 1 generate
    axi1 : eth_axi_mst generic map (
        xindex => xmstindex2
      ) port map (
        rst, 
        clk,
        msti,
        msto2,
        omac_tmsto2,
        imac_tmsti2,
        eth_rx_in_none,
        open
      );
  end generate;
  edclmst_off : if edclsepahbg = 0 generate
      msto2	<= nasti_master_out_none;
      imac_tmsti2.grant <= '0';
      imac_tmsti2.data <= (others => '0');
      imac_tmsti2.ready <= '0';
      imac_tmsti2.error <= '0';
      imac_tmsti2.retry <= '0';
  end generate;


  regs : process(clk) is
  begin
    if rising_edge(clk) then r <= rin; end if;
  end process;
  
end architecture;

