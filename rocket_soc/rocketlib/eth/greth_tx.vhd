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
-- Entity: 	greth_tx
-- File:	greth_tx.vhd
-- Author:	Marko Isomaki 
-- Description:	Ethernet transmitter
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.grethpkg.all;

entity greth_tx is
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
end entity;

architecture rtl of greth_tx is
  function mirror2(din : in std_logic_vector(3 downto 0))
                        return std_logic_vector is
    variable do : std_logic_vector(3 downto 0);
  begin
    do(3) := din(0); do(2) := din(1);
    do(1) := din(2); do(0) := din(3);
    return do;
  end function; 
  
  function init_ifg(
    ifg_gap : in integer;
    rmii    : in integer)
              return integer is
  begin
    if rmii = 0 then
      return log2(ifg_gap);
    else
      return log2(ifg_gap*20);
    end if;
  end function;

  constant maxattempts : std_logic_vector(4 downto 0) :=
    conv_std_logic_vector(attempt_limit, 5);

  --transmitter constants
  constant ifg_bits : integer := init_ifg(ifg_gap, rmii);
  constant ifg_p1 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector((ifg_gap)/3, ifg_bits);
  constant ifg_p2 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector((ifg_gap*2)/3, ifg_bits);
  constant ifg_p1_r100 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector((ifg_gap*2)/3, ifg_bits);
  constant ifg_p2_r100 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector(rmii*(ifg_gap*4)/3, ifg_bits);
  constant ifg_p1_r10 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector(rmii*(ifg_gap*20)/3, ifg_bits);
  constant ifg_p2_r10 : std_logic_vector(ifg_bits-1 downto 0) :=
    conv_std_logic_vector(rmii*(ifg_gap*40)/3, ifg_bits); 

  function ifg_sel(
    rmii  : in integer;
    p1    : in integer;
    speed : in std_ulogic)
            return std_logic_vector is
  begin
    if p1 = 1 then
      if rmii = 0 then
        return ifg_p1;
      else
        if speed = '1' then
          return ifg_p1_r100;
        else
          return ifg_p1_r10;
        end if;
      end if;
    else
      if rmii = 0 then
        return ifg_p2;
      else
        if speed = '1' then
          return ifg_p2_r100;
        else
          return ifg_p2_r10;
        end if;
      end if;
    end if;
  end function;
  
  --transmitter types
  type tx_state_type is (idle, preamble, sfd, data1, data2, pad1, pad2, fcs,
    fcs2, finish, calc_backoff, wait_backoff, send_jam, send_jam2,
    check_attempts);
  type def_state_type is (monitor, def_on, ifg1, ifg2, frame_waitingst);
  
  type tx_reg_type is record
    --deference process
    def_state        : def_state_type; 
    ifg_cycls        : std_logic_vector(ifg_bits-1 downto 0);
    deferring        : std_ulogic; 
    was_transmitting : std_ulogic;
    
    --tx process
    main_state   : tx_state_type;
    transmitting : std_ulogic;
    tx_en        : std_ulogic;
    txd          : std_logic_vector(3 downto 0);
    cnt          : std_logic_vector(3 downto 0);
    icnt         : std_logic_vector(1 downto 0);
    crc          : std_logic_vector(31 downto 0);
    crc_en       : std_ulogic; 
    byte_count   : std_logic_vector(10 downto 0);
    slot_count   : std_logic_vector(6 downto 0);
    random       : std_logic_vector(9 downto 0);
    delay_val    : std_logic_vector(9 downto 0);
    retry_cnt    : std_logic_vector(4 downto 0);
    status       : std_logic_vector(1 downto 0); 
    data         : std_logic_vector(31 downto 0);
    
    --synchronization
    read         : std_ulogic;
    done         : std_ulogic;
    restart      : std_ulogic;
    start        : std_logic_vector(nsync downto 0);
    read_ack     : std_logic_vector(nsync-1 downto 0);
    crs          : std_logic_vector(1 downto 0);
    col          : std_logic_vector(1 downto 0);
    fullduplex   : std_logic_vector(1 downto 0);
    
    --rmii
    crs_act      : std_ulogic;
    crs_prev     : std_ulogic;
    speed        : std_logic_vector(1 downto 0);
    rcnt         : std_logic_vector(3 downto 0);
    switch       : std_ulogic;
    txd_msb      : std_logic_vector(1 downto 0);
    zero         : std_ulogic;
    rmii_crc_en  : std_ulogic;
  end record;

  --transmitter signals
  signal r, rin  : tx_reg_type;
  signal txrst   : std_ulogic;
  signal vcc     : std_ulogic;

begin
  vcc <= '1';
  
  tx_rst : eth_rstgen
  port map(rst, clk, vcc, txrst, open);

  tx : process(txrst, r, txi) is
    variable collision     : std_ulogic; 
    variable frame_waiting : std_ulogic;
    variable index         : integer range 0 to 7;
    variable start         : std_ulogic;
    variable read_ack      : std_ulogic;
    variable v             : tx_reg_type;

    variable crs           : std_ulogic;
    variable col           : std_ulogic;
    variable tx_done       : std_ulogic;
  begin
    v := r; frame_waiting := '0'; tx_done := '0'; v.rmii_crc_en := '0';
    
    --synchronization
    v.col(1) := r.col(0); v.col(0) := txi.rx_col;
    v.crs(1) := r.crs(0); v.crs(0) := txi.rx_crs;
    v.fullduplex(0) := txi.full_duplex;
    v.fullduplex(1) := r.fullduplex(0);

    v.start(0)       := txi.start;
    v.read_ack(0)    := txi.readack;
     
    if nsync = 2 then
      v.start(1)       := r.start(0);
      v.read_ack(1)    := r.read_ack(0);
    end if;

    start       := r.start(nsync) xor r.start(nsync-1);
    read_ack    := not (r.read    xor r.read_ack(nsync-1));

    --crc generation
    if (r.crc_en = '1') and ((rmii = 0) or (r.rmii_crc_en = '1')) then
      v.crc := calccrc(r.txd, r.crc);
    end if;

    --rmii
    if rmii = 0 then
      col := r.col(1); crs := r.crs(1);
      tx_done := '1';
    else
      v.crs_prev := r.crs(1);
      if (r.crs(0) and not r.crs_act) = '1' then
        v.crs_act := '1';
      end if;
      if (r.crs(1) or r.crs(0)) = '0' then
        v.crs_act := '0';
      end if;
      crs := r.crs(1) and not ((not r.crs_prev) and r.crs_act);
      col := crs and r.tx_en;

      v.speed(1) := r.speed(0); v.speed(0) := txi.speed;

      if r.tx_en = '1' then
        v.rcnt := r.rcnt - 1;
        if r.speed(1) = '1' then
          v.switch := not r.switch;
          if r.switch = '1' then
            tx_done := '1'; v.rmii_crc_en := '1';
          end if;
          if r.switch = '0' then
            v.txd(1 downto 0) := r.txd_msb;
          end if;  
        else
          v.zero := '0';
          if r.rcnt = "0001" then
            v.zero := '1'; 
          end if;
          if r.zero = '1' then
            v.switch := not r.switch;
            v.rcnt := "1001";
            if r.switch = '0' then
              v.txd(1 downto 0) := r.txd_msb;
            end if;
          end if;
          if (r.switch and r.zero) = '1' then
            tx_done := '1'; v.rmii_crc_en := '1';
          end if;
        end if;
      end if;
    end if;

    collision := col and not r.fullduplex(1); 
       
    --main fsm
    case r.main_state is
    when idle =>
      v.transmitting := '0'; 
      if rmii = 1 then
        v.rcnt := "1001"; v.switch := '0';
      end if;
      if (start and not r.deferring) = '1' then
        v.main_state := preamble; v.transmitting := '1'; v.tx_en := '1';
	v.byte_count := (others => '1'); v.status := (others => '0');
	v.read := not r.read; v.start(nsync) := r.start(nsync-1);
      elsif start = '1' then
	frame_waiting := '1'; 
      end if;
      v.txd := "0101"; v.cnt := "1110";
    when preamble =>
      if tx_done = '1' then
        v.cnt := r.cnt - 1; 
        if r.cnt = "0000" then
          v.txd := "1101"; v.main_state := sfd; 
        end if;
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when sfd =>
      if tx_done = '1' then
        v.main_state := data1; v.icnt := (others => '0'); v.crc_en := '1';
        v.crc := (others => '1'); v.byte_count := (others => '0');
        v.txd := txi.data(27 downto 24);
        if (read_ack and txi.valid) = '0' then
          v.status(0) := '1'; v.main_state := finish; v.tx_en := '0';  
        else
          v.data := txi.data; v.read := not r.read;
        end if;
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when data1 =>
      index := conv_integer(r.icnt);
      if tx_done = '1' then
        v.byte_count := r.byte_count + 1;
        v.main_state := data2; v.icnt := r.icnt + 1;
        case index is
        when 0 => v.txd := r.data(31 downto 28);
        when 1 => v.txd := r.data(23 downto 20);
        when 2 => v.txd := r.data(15 downto 12);
        when 3 => v.txd := r.data(7 downto 4);
        when others => null;
        end case;
        if v.byte_count = txi.len then
          v.tx_en := '1';
          if conv_integer(v.byte_count) >= 60 then
            v.main_state := fcs; v.cnt := (others => '0'); 
          else
            v.main_state := pad1; 
          end if;
        elsif index = 3 then
          if (read_ack and txi.valid) = '0' then
            v.status(0) := '1'; v.main_state := finish; v.tx_en := '0';  
          else
            v.data := txi.data; v.read := not r.read;
          end if;
        end if;
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when data2 =>
      index := conv_integer(r.icnt); 
      if tx_done = '1' then
        v.main_state := data1;
        case index is
        when 0 => v.txd := r.data(27 downto 24);
        when 1 => v.txd := r.data(19 downto 16);
        when 2 => v.txd := r.data(11 downto 8);
        when 3 => v.txd := r.data(3 downto 0);
        when others => null;
        end case;
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when pad1 =>
      if tx_done = '1' then
        v.main_state := pad2; 
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when pad2 =>
      if tx_done = '1' then
        v.byte_count := r.byte_count + 1; 
        if conv_integer(v.byte_count) = 60 then
          v.main_state := fcs; v.cnt := (others => '0');
        else
          v.main_state := pad1;
        end if; 
        if collision = '1' then v.main_state := send_jam; end if;
      end if;
    when fcs =>
      if tx_done = '1' then
        v.cnt := r.cnt + 1; v.crc_en := '0'; index := conv_integer(r.cnt);
        case index is
        when 0 => v.txd := mirror2(not v.crc(31 downto 28));
        when 1 => v.txd := mirror2(not r.crc(27 downto 24));
        when 2 => v.txd := mirror2(not r.crc(23 downto 20));
        when 3 => v.txd := mirror2(not r.crc(19 downto 16));
        when 4 => v.txd := mirror2(not r.crc(15 downto 12));
        when 5 => v.txd := mirror2(not r.crc(11 downto 8));
        when 6 => v.txd := mirror2(not r.crc(7 downto 4));
        when 7 => v.txd := mirror2(not r.crc(3 downto 0));
                  v.main_state := fcs2;
        when others => null;
        end case;
      end if;
    when fcs2 =>
      if tx_done = '1' then
        v.main_state := finish; v.tx_en := '0';
      end if;
    when finish =>
      v.tx_en := '0'; v.transmitting := '0'; v.main_state := idle;
      v.retry_cnt := (others => '0'); v.done := not r.done;
    when send_jam =>
      if tx_done = '1' then
        v.cnt := "0110"; v.main_state := send_jam2; v.crc_en := '0';
      end if;  
    when send_jam2 =>
      if tx_done = '1' then
        v.cnt := r.cnt - 1;
        if r.cnt = "0000" then
          v.main_state := check_attempts; v.retry_cnt := r.retry_cnt + 1;
          v.tx_en := '0';
        end if;
      end if;
    when check_attempts =>
      v.transmitting := '0';
      if r.retry_cnt = maxattempts then
        v.main_state := finish; v.status(1) := '1';  
      else
        v.main_state := calc_backoff; v.restart := not r.restart;
      end if;
      v.tx_en := '0';
    when calc_backoff =>
      v.delay_val := (others => '0');
      for i in 1 to backoff_limit-1 loop
	if i < conv_integer(r.retry_cnt)+1 then
	  v.delay_val(i) := r.random(i);
	end if; 
      end loop;
      v.main_state := wait_backoff; v.slot_count := (others => '1'); 
    when wait_backoff =>
      if conv_integer(r.delay_val) = 0 then
	v.main_state := idle; 
      end if;
      v.slot_count := r.slot_count - 1;
      if conv_integer(r.slot_count) = 0 then
	v.slot_count := (others => '1'); v.delay_val := r.delay_val - 1; 
      end if;
    when others =>
      v.main_state := idle;
    end case;
    
    --random values; 
    v.random := r.random(8 downto 0) & (not (r.random(2) xor r.random(9)));
   
    --deference
    case r.def_state is
    when monitor =>
      v.was_transmitting := '0'; 
      if ( (crs and not r.fullduplex(1)) or
	   (r.transmitting and r.fullduplex(1)) ) = '1' then
	v.deferring := '1'; v.def_state := def_on;
	v.was_transmitting := r.transmitting; 
      end if;
    when def_on =>
      v.was_transmitting := r.was_transmitting or r.transmitting; 
      if r.fullduplex(1) = '1' then
	if r.transmitting = '0' then v.def_state := ifg1; end if;
        v.ifg_cycls := ifg_sel(rmii, 1, r.speed(1));
      else
	if (r.transmitting or crs) = '0' then
	  v.def_state := ifg1; v.ifg_cycls := ifg_sel(rmii, 1, r.speed(1));
	end if; 
      end if; 
    when ifg1 =>
      v.ifg_cycls := r.ifg_cycls - 1;
      if r.ifg_cycls = zero32(ifg_bits-1 downto 0) then
        v.def_state := ifg2;
        v.ifg_cycls := ifg_sel(rmii, 0, r.speed(1));
      elsif (crs and not r.fullduplex(1)) = '1' then
        v.ifg_cycls := ifg_sel(rmii, 1, r.speed(1));
      end if; 
    when ifg2 =>
      v.ifg_cycls := r.ifg_cycls - 1;
      if r.ifg_cycls = zero32(ifg_bits-1 downto 0) then
	v.deferring := '0'; 
	if (r.fullduplex(1) or not frame_waiting) = '1' then
	  v.def_state := monitor;
        elsif frame_waiting = '1' then
	  v.def_state := frame_waitingst;
	end if; 
      end if;
    when frame_waitingst =>
       if frame_waiting = '0' then v.def_state := monitor; end if; 
    when others => v.def_state := monitor; 
    end case;

    if rmii = 1 then
      v.txd_msb := v.txd(3 downto 2);
    end if;
        
    if txrst = '0' then
      v.main_state := idle; v.random := (others => '0');
      v.def_state := monitor; v.deferring := '0'; v.tx_en := '0'; 
      v.done := '0'; v.restart := '0'; v.read := '0';
      v.start := (others => '0'); v.read_ack := (others => '0');
      v.icnt := (others => '0'); v.delay_val := (others => '0');
      v.ifg_cycls := (others => '0');
      v.crs_act := '0'; 
      v.slot_count := (others => '1'); 
      v.retry_cnt := (others => '0');
      v.cnt := (others => '0');
    end if;

    rin                      <= v;
    txo.tx_er                <= '0';
    txo.tx_en                <= r.tx_en;
    txo.txd                  <= r.txd;
    txo.done                 <= r.done;
    txo.read                 <= r.read;
    txo.restart              <= r.restart;
    txo.status               <= r.status;
  end process;


  gmiimode0 : if gmiimode = 0 generate
    txregs0 : process(clk) is
    begin
      if rising_edge(clk) then 
        r <= rin;
        if txrst = '0' then
          r.icnt <= (others => '0'); r.delay_val <= (others => '0');
          r.cnt <= (others => '0');
        else
          r.icnt <= rin.icnt; r.delay_val <= rin.delay_val;
          r.cnt <= rin.cnt;
        end if;
      end if;
    end process;
  end generate;

  gmiimode1 : if gmiimode = 1 generate
    txregs0 : process(clk) is
    begin
      if rising_edge(clk) then 
        if (txi.datavalid = '1' or txrst = '0')  then r <= rin; end if;
        if txrst = '0' then
          r.icnt <= (others => '0'); r.delay_val <= (others => '0');
          r.cnt <= (others => '0');
        else
          if txi.datavalid = '1' then
            r.icnt <= rin.icnt; r.delay_val <= rin.delay_val;
            r.cnt <= rin.cnt;
          end if;
        end if;
      end if;
    end process;
  end generate;

end architecture;

