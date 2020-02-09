-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      AXI Master device implementing DMA access.
--! @details    AMBA4 AXI Master interface module dedicated for the eth MAC.
------------------------------------------------------------------------------
--! Standard library
library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! Rocket-chip specific library
library ethlib;
use ethlib.types_eth.all;

entity eth_axi_mst is
  port(
    rst     : in  std_ulogic;
    clk     : in  std_ulogic;
    aximi   : in  axi4_master_in_type;
    aximo   : out axi4_master_out_type;
    tmsti   : in  eth_tx_ahb_in_type;
    tmsto   : out eth_tx_ahb_out_type;
    rmsti   : in  eth_rx_ahb_in_type;
    rmsto   : out eth_rx_ahb_out_type
  );
end entity;

architecture rtl of eth_axi_mst is
  constant STATE_IDLE   : integer := 0;
  constant STATE_W      : integer := STATE_IDLE+1;
  constant STATE_R_WAIT_RESP   : integer := STATE_W+1;
  constant STATE_R_WAIT_NEXT : integer := STATE_R_WAIT_RESP+1;
  constant STATE_B      : integer := STATE_R_WAIT_NEXT+1;
  
  constant Rx : integer := 0;
  constant Tx : integer := 1;

  type eth_in_type is record
    req     : std_ulogic;
    write   : std_ulogic;
    addr    : std_logic_vector(31 downto 0);
    data    : std_logic_vector(31 downto 0);
    burst_bytes : std_logic_vector(10 downto 0);
  end record;

  type eth_out_type is record
    grant    : std_ulogic;
    data     : std_logic_vector(31 downto 0);
    ready    : std_ulogic;
    error    : std_ulogic;
    retry    : std_ulogic;
  end record;

  type eth_out_vector is array (0 to 1) of eth_out_type;

  type reg_type is record
    state    : integer range 0 to STATE_B;
    len      : integer;
    x        : integer range 0 to 1;
    waddr2   : std_logic;
  end record;

  signal r, rin : reg_type;
begin
  comb : process(rst, r, tmsti, rmsti,  aximi) is
  variable v       : reg_type;
  variable xmsti : eth_in_type;
  variable xmsto : eth_out_vector;
  variable vaximo   : axi4_master_out_type;
  variable rdata_lsb : std_logic_vector(31 downto 0);
  variable wdata_lsb : std_logic_vector(31 downto 0);
  begin
    v := r;

    
    vaximo := axi4_master_out_none;
    vaximo.ar_user       := (others => '0');
    vaximo.ar_id         := conv_std_logic_vector(0, CFG_SYSBUS_ID_BITS);
    vaximo.ar_bits.size  := "010"; -- 4 bytes
    vaximo.ar_bits.burst := AXI_BURST_INCR;
    vaximo.aw_user       := (others => '0');
    vaximo.aw_id         := conv_std_logic_vector(0, CFG_SYSBUS_ID_BITS);
    vaximo.aw_bits.size  := "010"; -- 4 bytes
    vaximo.aw_bits.burst := AXI_BURST_INCR;
    
    xmsto := (others => ('0', rdata_lsb, '0', '0', '0'));

    if r.x = Rx then
      xmsti.req := rmsti.req;
      xmsti.write := rmsti.write;
      xmsti.addr := rmsti.addr;
      xmsti.data := rmsti.data;
      xmsti.burst_bytes := rmsti.burst_bytes;
    else
      xmsti.req := tmsti.req;
      xmsti.write := tmsti.write;
      xmsti.addr := tmsti.addr;
      xmsti.data := tmsti.data;
      xmsti.burst_bytes := tmsti.burst_bytes;
    end if;

    -- Pre-fix for SPARC byte order.
    -- It is better to fix in MAC itselfm but for now it will be here.
    wdata_lsb := xmsti.data(7 downto 0) & xmsti.data(15 downto 8)
               & xmsti.data(23 downto 16) & xmsti.data(31 downto 24);
    rdata_lsb := aximi.r_data(7 downto 0) & aximi.r_data(15 downto 8)
               & aximi.r_data(23 downto 16) & aximi.r_data(31 downto 24);

    case r.state is
    when STATE_IDLE =>
        if rmsti.req = '1' then
            v.x := Rx;
            vaximo.ar_valid      := not rmsti.write;
            vaximo.aw_valid      := rmsti.write;
            if rmsti.write = '1' then
                vaximo.aw_bits.addr  := rmsti.addr(31 downto 3) & "000";
                v.waddr2 := rmsti.addr(2);
                v.len  := conv_integer(rmsti.burst_bytes(10 downto 2)) - 1;
                vaximo.aw_bits.len := conv_std_logic_vector(v.len, 8);
                if aximi.aw_ready = '1' then
                    xmsto(Rx).grant := '1';
                    v.state := STATE_W;
                end if;
            else
                vaximo.ar_bits.addr  := rmsti.addr;
                v.len  := conv_integer(rmsti.burst_bytes(10 downto 2)) - 1;
                vaximo.ar_bits.len := conv_std_logic_vector(v.len, 8);
                if aximi.ar_ready = '1' then
                    xmsto(Rx).grant := '1';
                    v.state := STATE_R_WAIT_RESP;
                end if;
            end if;
        elsif tmsti.req = '1' then
            v.x := Tx;
            vaximo.ar_valid      := not tmsti.write;
            vaximo.aw_valid      := tmsti.write;
            if tmsti.write = '1' then
                vaximo.aw_bits.addr  := tmsti.addr(31 downto 3) & "000";
                v.waddr2 := tmsti.addr(2);
                v.len  := conv_integer(tmsti.burst_bytes(10 downto 2)) - 1;
                vaximo.aw_bits.len := conv_std_logic_vector(v.len, 8);
                if aximi.aw_ready = '1' then
                    xmsto(Tx).grant := '1';
                    v.state := STATE_W;
                end if;
            else
                vaximo.ar_bits.addr  := tmsti.addr;
                v.len  := conv_integer(tmsti.burst_bytes(10 downto 2)) - 1;
                vaximo.ar_bits.len := conv_std_logic_vector(v.len, 8);
                if aximi.ar_ready = '1' then
                    xmsto(Tx).grant := '1';
                    v.state := STATE_R_WAIT_RESP;
                end if;
            end if;
        end if;
        
      
    when STATE_R_WAIT_RESP =>
        vaximo.r_ready := '1';
        if aximi.r_valid = '1' then
            xmsto(r.x).ready := '1';
            if aximi.r_last = '1' then
                v.state := STATE_IDLE;
            else
                if xmsti.req = '1' then
                    xmsto(r.x).grant := '1';
                else
                    v.state := STATE_R_WAIT_NEXT;
                end if;
            end if;
        end if;

    when STATE_R_WAIT_NEXT =>
        if xmsti.req = '1' then
            xmsto(r.x).grant := '1';
            v.state := STATE_R_WAIT_RESP;
        end if;

    when STATE_W =>
        vaximo.w_valid := '1';
        case r.waddr2 is
        when '0' => vaximo.w_strb := X"0f";
        when '1' => vaximo.w_strb := X"f0";
        when others =>
        end case;
        vaximo.w_data := wdata_lsb & wdata_lsb;
        
        if aximi.w_ready = '1' then
            xmsto(r.x).ready := '1';
            if r.len = 0 then
                v.state := STATE_B;
                vaximo.w_last := '1';
            else 
                xmsto(r.x).grant := '1';
                v.len := r.len - 1;
                -- Address will be incremented on slave side
                --v.waddr2 := not r.waddr2;
            end if;
        end if;

    when STATE_B =>
        vaximo.w_last := '0';
        vaximo.b_ready := '1';
        if aximi.b_valid = '1' then
            v.state := STATE_IDLE;
        end if;
    when others =>
    end case;

    if rst = '0' then
      v.state := STATE_IDLE;
      v.waddr2 := '0';
      v.len := 0;
      v.x := Rx;
    end if;

    
    rin <= v;
    aximo <= vaximo;

    tmsto.grant   <= xmsto(Tx).grant;
    tmsto.data    <= xmsto(Tx).data;
    tmsto.ready   <= xmsto(Tx).ready;
    tmsto.error   <= xmsto(Tx).error;
    tmsto.retry   <= xmsto(Tx).retry;

    rmsto.grant   <= xmsto(Rx).grant;
    rmsto.data    <= xmsto(Rx).data;
    rmsto.ready   <= xmsto(Rx).ready;
    rmsto.error   <= xmsto(Rx).error;
    rmsto.retry   <= xmsto(Rx).retry;
  end process;

  regs : process(clk)
  begin
    if rising_edge(clk) then r <= rin; end if;
  end process; 
 
end architecture; 

