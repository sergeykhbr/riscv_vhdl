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
library rocketlib;
use rocketlib.grethpkg.all;

entity eth_axi_mst is
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
end entity;

architecture rtl of eth_axi_mst is
  constant STATE_IDLE : integer := 0;
  constant STATE_W    : integer := STATE_IDLE+1;
  constant STATE_R    : integer := STATE_W+1;
  constant STATE_B    : integer := STATE_R+1;

  type reg_type is record
    state    : integer range 0 to STATE_B;
    addr     : std_logic_vector(31 downto 0);
    len      : integer;
    rx_tx    : std_logic;
  end record;

  signal r, rin : reg_type;
begin
  comb : process(rst, r, tmsti, rmsti,  aximi) is
  variable v       : reg_type;
  variable tretry  : std_ulogic;
  variable rretry  : std_ulogic;
  variable rready  : std_ulogic;
  variable tready  : std_ulogic;
  variable rerror  : std_ulogic;
  variable terror  : std_ulogic;
  variable tgrant  : std_ulogic;
  variable rgrant  : std_ulogic;
  variable vmsto   : nasti_master_out_type;
  variable rdata_lsb : std_logic_vector(31 downto 0);
  variable wdata_lsb : std_logic_vector(31 downto 0);
  begin
    v := r;
    vmsto := nasti_master_out_none;
    rready := '0';
    tready := '0';
    tretry := '0';
    rretry := '0';
    rerror := '0';
    terror := '0';
    tgrant := '0';
    rgrant := '0';

    vmsto.ar_user       := '0';
    vmsto.ar_id         := conv_std_logic_vector(xindex, CFG_ROCKET_ID_BITS);
    vmsto.ar_bits.size  := "010"; -- 4 bytes
    vmsto.ar_bits.burst := NASTI_BURST_INCR;
    vmsto.aw_user       := '0';
    vmsto.aw_id         := conv_std_logic_vector(xindex, CFG_ROCKET_ID_BITS);
    vmsto.aw_bits.size  := "010"; -- 4 bytes
    vmsto.aw_bits.burst := NASTI_BURST_INCR;

    case r.state is
    when STATE_IDLE =>
        if rmsti.req = '1' then
            v.rx_tx := '0';
            v.addr := rmsti.addr;
            vmsto.ar_valid      := not rmsti.write;
            vmsto.aw_valid      := rmsti.write;
            if rmsti.write = '1' then
                vmsto.aw_bits.addr  := rmsti.addr(31 downto 4) & "0000";
                v.len  := conv_integer(rmsti.burst_bytes(10 downto 2)) - 1;
                vmsto.aw_bits.len := conv_std_logic_vector(v.len, 8);
                if (aximi.grant(xindex) and aximi.aw_ready) = '1' then
                    rgrant := '1';
                    v.state := STATE_W;
                end if;
            else
                vmsto.ar_bits.addr  := rmsti.addr;
                v.len  := conv_integer(rmsti.burst_bytes(10 downto 2)) - 1;
                vmsto.ar_bits.len := conv_std_logic_vector(v.len, 8);
                if (aximi.grant(xindex) and aximi.ar_ready) = '1' then
                    rgrant := '1';
                    v.state := STATE_R;
                end if;
            end if;
        elsif tmsti.req = '1' then
            v.rx_tx := '1';
            v.addr := tmsti.addr;
            vmsto.ar_valid      := not tmsti.write;
            vmsto.aw_valid      := tmsti.write;
            if tmsti.write = '1' then
                vmsto.aw_bits.addr  := tmsti.addr(31 downto 4) & "0000";
                v.len  := conv_integer(tmsti.burst_bytes(10 downto 2)) - 1;
                vmsto.aw_bits.len := conv_std_logic_vector(v.len, 8);
                if (aximi.grant(xindex) and aximi.aw_ready) = '1' then
                    tgrant := '1';
                    v.state := STATE_W;
                end if;
            else
                vmsto.ar_bits.addr  := tmsti.addr;
                v.len  := conv_integer(tmsti.burst_bytes(10 downto 2)) - 1;
                vmsto.ar_bits.len := conv_std_logic_vector(v.len, 8);
                if (aximi.grant(xindex) and aximi.ar_ready) = '1' then
                    tgrant := '1';
                    v.state := STATE_R;
                end if;
            end if;
        end if;

    when STATE_W =>
        vmsto.w_valid := '1';
        case r.addr(3 downto 2) is
        when "00" => vmsto.w_strb := X"000f";
        when "01" => vmsto.w_strb := X"00f0";
        when "10" => vmsto.w_strb := X"0f00";
        when "11" => vmsto.w_strb := X"f000";
        when others =>
        end case;
        if r.rx_tx = '0' then
            wdata_lsb := rmsti.data(7 downto 0) & rmsti.data(15 downto 8)
                       & rmsti.data(23 downto 16) & rmsti.data(31 downto 24);
        else
            wdata_lsb := tmsti.data(7 downto 0) & tmsti.data(15 downto 8)
                       & tmsti.data(23 downto 16) & tmsti.data(31 downto 24);
        end if;
        vmsto.w_data := wdata_lsb & wdata_lsb & wdata_lsb & wdata_lsb;
        
        if aximi.w_ready = '1' then
            tready := r.rx_tx;
            rready := not r.rx_tx;
            if r.len = 0 then
                v.state := STATE_B;
            else 
                tgrant := r.rx_tx;
                rgrant := not r.rx_tx;
                v.len := r.len - 1;
                -- Incremented on slave side
                --v.addr = r.addr + 4;
            end if;
        end if;

    when STATE_R =>
        vmsto.r_ready := '1';
        if aximi.r_valid = '1' then
            if aximi.r_resp = NASTI_RESP_OKAY then
                tready := r.rx_tx;
                rready := not r.rx_tx;
            else
                terror := r.rx_tx;
                rerror := not r.rx_tx;
            end if;

            if r.len = 0 then
                v.state := state_idle;
            else
                tgrant := r.rx_tx;
                rgrant := not r.rx_tx;
                v.len := r.len - 1;
            end if;
        end if;

    when STATE_B =>
        vmsto.b_ready := '1';
        if aximi.b_valid = '1' then
            v.state := STATE_IDLE;
        end if;
    when others =>
    end case;

    if rst = '0' then
      v.state := STATE_IDLE;
      v.addr := (others => '0');
      v.len := 0;
      v.rx_tx := '0';
    end if;

    -- Pre-fix for SPARC byte order.
    -- It is better to fix in MAC itselfm but for now it will be here.
    rdata_lsb := aximi.r_data(7 downto 0) & aximi.r_data(15 downto 8)
               & aximi.r_data(23 downto 16) & aximi.r_data(31 downto 24);
    
    rin <= v;
    aximo <= vmsto;

    tmsto.error    <= terror;
    tmsto.retry    <= tretry;
    tmsto.ready    <= tready;
    tmsto.grant    <= tgrant;
    tmsto.data     <= rdata_lsb;

    rmsto.error    <= rerror;
    rmsto.retry    <= rretry;
    rmsto.ready    <= rready;
    rmsto.grant    <= rgrant;
    rmsto.data     <= rdata_lsb;
  end process;

  regs : process(clk)
  begin
    if rising_edge(clk) then r <= rin; end if;
  end process; 
 
end architecture; 

