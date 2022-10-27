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
library ethlib;
use ethlib.types_eth.all;

entity grethaxi is
  generic(
    async_reset    : boolean := false;
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
    ipaddrl        : integer := 16#0135#;
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
    msti           : in axi4_master_in_type;
    msto           : out axi4_master_out_type;
    mstcfg         : out axi4_master_config_type;
    msto2          : out axi4_master_out_type;
    mstcfg2        : out axi4_master_config_type;
    slvi           : in axi4_slave_in_type;
    slvo           : out axi4_slave_out_type;
    slvcfg         : out axi4_slave_config_type;
    ethi           : in eth_in_type;
    etho           : out eth_out_type;
    irq            : out  std_logic
  );
end entity;
  
architecture arch_grethaxi of grethaxi is

  constant bufsize : std_logic_vector(2 downto 0) :=
                       conv_std_logic_vector(log2(edclbufsz), 3);
                       
  constant xslvconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ETHMAC
  );

  constant xmstconfig : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => GAISLER_ETH_MAC_MASTER
  );

  constant xmstconfig2 : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => GAISLER_ETH_EDCL_MASTER
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1)
       of std_logic_vector(15 downto 0);

  type registers is record
      ctrl : eth_control_type;
      raddr : local_addr_array_type;
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

  signal wb_dev_rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re    : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we    : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);

begin

  slv0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => rst,
    i_xcfg => xslvconfig, 
    i_xslvi => slvi,
    o_xslvo => slvo,
    i_ready => '1',
    i_rdata => wb_dev_rdata,
    o_re => w_bus_re,
    o_r32 => open,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );
  
  comb : process(r, ethi, omac_rdbgdata, omac_status, rst, w_bus_re,
                 wb_bus_raddr, wb_bus_waddr, w_bus_we, wb_bus_wstrb, wb_bus_wdata) is
      variable v        : registers;
      variable vcmd     : eth_command_type;
      variable waddr : std_logic_vector(15 downto 0);
      variable vrdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
      variable wdata32 : std_logic_vector(31 downto 0);
      variable val : std_logic_vector(8*CFG_ALIGN_BYTES-1 downto 0);
  begin

    v := r;
    vcmd := eth_command_none;
   
    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       v.raddr(n) := wb_bus_raddr(n)(17 downto log2(CFG_ALIGN_BYTES));
       val := (others => '0');
       
       if (ramdebug = 0) or (r.raddr(n)(15 downto 14) = "00") then 
         case r.raddr(n)(3 downto 0) is
         when "0000" => --ctrl reg
           if ramdebug /= 0 then
             val(13) := r.ctrl.ramdebugen;
           end if;
           if (edcl /= 0) then
             val(31) := '1';
             val(30 downto 28) := bufsize;
             val(14) := r.ctrl.edcldis;
             val(12) := r.ctrl.disableduplex;
           end if;
           if enable_mdint = 1 then
             val(26) := '1';
             val(10) := r.ctrl.pstatirqen;
           end if;
           if multicast = 1 then
             val(25) := '1';
             val(11) := r.ctrl.mcasten;
           end if;
           if rmii = 1 then
           val(7) := omac_status.speed;
           end if;
           val(6) := omac_status.reset;
           val(5) := r.ctrl.prom;
           val(4) := omac_status.full_duplex;
           val(3) := r.ctrl.rx_irqen;
           val(2) := r.ctrl.tx_irqen;
           val(1) := omac_status.rxen;
           val(0) := omac_status.txen; 
         when "0001" => --status/int source reg
           val(9) := not (omac_status.edcltx_idle or omac_status.edclrx_idle);
           if enable_mdint = 1 then
             val(8) := omac_status.phystat;
           end if;
           val(7) := omac_status.invaddr;
           val(6) := omac_status.toosmall;
           val(5) := omac_status.txahberr;
           val(4) := omac_status.rxahberr;
           val(3) := omac_status.tx_int;
           val(2) := omac_status.rx_int;
           val(1) := omac_status.tx_err;
           val(0) := omac_status.rx_err; 
         when "0010" => --mac addr lsb
           val := r.ctrl.mac_addr(31 downto 0); 
         when "0011" => --mac addr msb/mdio address
           val(15 downto 0) := r.ctrl.mac_addr(47 downto 32);
         when "0100" => --mdio ctrl/status
           val(31 downto 16) := omac_status.mdio.cmd.data;
           val(15 downto 11) := r.ctrl.mdio_phyadr;
           val(10 downto 6) :=  omac_status.mdio.cmd.regadr;  
           val(3) := omac_status.mdio.busy;
           val(2) := omac_status.mdio.linkfail;
           val(1) := omac_status.mdio.cmd.read;
           val(0) := omac_status.mdio.cmd.write; 
         when "0101" => --tx descriptor 
           val(31 downto 10) := r.ctrl.txdesc;
           val(9 downto 3)   := omac_status.txdsel;
         when "0110" => --rx descriptor
           val(31 downto 10) := r.ctrl.rxdesc;
           val(9 downto 3)   := omac_status.rxdsel;
         when "0111" => --edcl ip
           if (edcl /= 0) then
              val := r.ctrl.edclip;
           end if;
         when "1000" =>
           if multicast = 1 then
             val := r.ctrl.hash(63 downto 32);
           end if;
         when "1001" =>
           if multicast = 1 then
             val := r.ctrl.hash(31 downto 0);
           end if;
         when "1010" =>
           if edcl /= 0 then
             val(15 downto 0) := r.ctrl.emacaddr(47 downto 32);
           end if;
         when "1011" =>
           if edcl /= 0 then
             val := r.ctrl.emacaddr(31 downto 0);
           end if;
         when others => null; 
         end case;
       elsif r.raddr(n)(15 downto 14) = "01" then
           if ramdebug /= 0 then
             vcmd.dbg_access_id := DBG_ACCESS_TX_BUFFER;
             vcmd.dbg_rd_ena    := r.ctrl.ramdebugen;
             vcmd.dbg_addr      := r.raddr(n)(13 downto 0);
             val                := omac_rdbgdata;
           end if;
       elsif r.raddr(n)(15 downto 14) = "10" then
           if ramdebug /= 0 then
             vcmd.dbg_access_id := DBG_ACCESS_RX_BUFFER;
             vcmd.dbg_rd_ena    := r.ctrl.ramdebugen;
             vcmd.dbg_addr      := r.raddr(n)(13 downto 0);
             val                := omac_rdbgdata;
           end if;
       elsif r.raddr(n)(15 downto 14) = "11" then 
           if (ramdebug = 2) and (edcl /= 0) then
             vcmd.dbg_access_id := DBG_ACCESS_EDCL_BUFFER;
             vcmd.dbg_rd_ena    := r.ctrl.ramdebugen;
             vcmd.dbg_addr      := r.raddr(n)(13 downto 0);
             val                := omac_rdbgdata;
           end if;
       end if;

       vrdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := val;
    end loop;


    if w_bus_we = '1' then
      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr := wb_bus_waddr(n)(17 downto 2);
         wdata32 := wb_bus_wdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n);

          if wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n) /= "0000" then
            if (ramdebug = 0) or (waddr(15 downto 14) = "00") then 
             case waddr(3 downto 0) is
             when "0000" => --ctrl reg
               if ramdebug /= 0 then
                 v.ctrl.ramdebugen := wdata32(13);
               end if;
               if edcl /= 0 then
                 v.ctrl.edcldis  := wdata32(14);
                 v.ctrl.disableduplex := wdata32(12);
               end if;
               if multicast = 1 then
                 v.ctrl.mcasten := wdata32(11);
               end if;
               if enable_mdint = 1 then
                 v.ctrl.pstatirqen  := wdata32(10);
               end if;
               if rmii = 1 then
                 vcmd.set_speed       := wdata32(7);  
                 vcmd.clr_speed       := not wdata32(7);  
               end if;
               vcmd.set_reset       := wdata32(6);
               vcmd.clr_reset       := not wdata32(6);
               v.ctrl.prom          := wdata32(5); 
               vcmd.set_full_duplex := wdata32(4);
               vcmd.clr_full_duplex := not wdata32(4);
               v.ctrl.rx_irqen           := wdata32(3);
               v.ctrl.tx_irqen           := wdata32(2);
               vcmd.set_rxena     := wdata32(1);
               vcmd.clr_rxena     := not wdata32(1);
               vcmd.set_txena     := wdata32(0);
               vcmd.clr_txena     := not wdata32(0);
             when "0001" => --status/int source reg
               if enable_mdint = 1 then
                 vcmd.clr_status_phystat := wdata32(8);
               end if;
               vcmd.clr_status_invaddr  := wdata32(7);
               vcmd.clr_status_toosmall := wdata32(6);
               vcmd.clr_status_txahberr := wdata32(5);
               vcmd.clr_status_rxahberr := wdata32(4);
               vcmd.clr_status_tx_int := wdata32(3);
               vcmd.clr_status_rx_int := wdata32(2);
               vcmd.clr_status_tx_err := wdata32(1);
               vcmd.clr_status_rx_err := wdata32(0);
             when "0010" => --mac addr lsb
               v.ctrl.mac_addr(31 downto 0)  := wdata32(31 downto 0);
             when "0011" => --mac addr msb
               v.ctrl.mac_addr(47 downto 32) := wdata32(15 downto 0);
             when "0100" => --mdio ctrl/status
               if enable_mdio = 1 then
                 vcmd.mdio_cmd.valid := not omac_status.mdio.busy;
                 if omac_status.mdio.busy = '0' then
                   v.ctrl.mdio_phyadr := wdata32(15 downto 11);
                 end if;
                 vcmd.mdio_cmd.data   := wdata32(31 downto 16);
                 vcmd.mdio_cmd.regadr := wdata32(10 downto 6);
                 vcmd.mdio_cmd.read   := wdata32(1);
                 vcmd.mdio_cmd.write  := wdata32(0);
               end if;
             when "0101" => --tx descriptor 
               vcmd.set_txdsel := '1';
               vcmd.txdsel := wdata32(9 downto 3);
               v.ctrl.txdesc := wdata32(31 downto 10);
             when "0110" => --rx descriptor
               vcmd.set_rxdsel := '1';
               vcmd.rxdsel := wdata32(9 downto 3);
               v.ctrl.rxdesc := wdata32(31 downto 10);
             when "0111" => --edcl ip
               if (edcl /= 0) then
                 v.ctrl.edclip := wdata32;
               end if;
             when "1000" => --hash msb
               if multicast = 1 then
                 v.ctrl.hash(63 downto 32) := wdata32;
               end if;
             when "1001" => --hash lsb
               if multicast = 1 then
                 v.ctrl.hash(31 downto 0) := wdata32;
               end if;
             when "1010" =>
               if edcl /= 0 then
                 v.ctrl.emacaddr(47 downto 32) := wdata32(15 downto 0);
               end if;
             when "1011" =>
               if edcl /= 0 then
                 v.ctrl.emacaddr(31 downto 0) := wdata32;
               end if;
             when others => null; 
             end case;
           elsif waddr(15 downto 14) = "01" then
             if ramdebug /= 0 then
               vcmd.dbg_access_id := DBG_ACCESS_TX_BUFFER;
               vcmd.dbg_wr_ena    := r.ctrl.ramdebugen;
               vcmd.dbg_addr      := waddr(13 downto 0);
               vcmd.dbg_wdata     := wdata32;
             end if;
           elsif waddr(15 downto 14) = "10" then  
             if ramdebug /= 0 then
               vcmd.dbg_access_id := DBG_ACCESS_RX_BUFFER;
               vcmd.dbg_wr_ena    := r.ctrl.ramdebugen;
               vcmd.dbg_addr      := waddr(13 downto 0);
               vcmd.dbg_wdata     := wdata32;
             end if;
           elsif waddr(15 downto 14) = "11" then 
             if (ramdebug = 2) and (edcl /= 0) then
               vcmd.dbg_access_id := DBG_ACCESS_EDCL_BUFFER;
               vcmd.dbg_wr_ena    := r.ctrl.ramdebugen;
               vcmd.dbg_addr      := waddr(13 downto 0);
               vcmd.dbg_wdata     := wdata32;
             end if;
           end if;
         end if;
      end loop;
    end if;
   
   
------------------------------------------------------------------------------
-- RESET ----------------------------------------------------------------------
-------------------------------------------------------------------------------
    if rst = '0' then
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
      v.raddr(0) := (others => '0');
      v.raddr(1) := (others => '0');
    end if;

   
   rin <= v;
   imac_cmd <= vcmd;
   wb_dev_rdata <= vrdata;
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
  axi0 : eth_axi_mst port map (
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
    axi1 : eth_axi_mst port map (
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
      msto2	<= axi4_master_out_none;
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

