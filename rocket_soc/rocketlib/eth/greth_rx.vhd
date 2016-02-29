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
-- Entity: 	greth_rx 
-- File:	greth_rx.vhd
-- Author:	Marko Isomaki 
-- Description:	Ethernet receiver
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.grethpkg.all;

entity greth_rx is
  generic(
    nsync          : integer range 1 to 2 := 2;
    rmii           : integer range 0 to 1 := 0;
    multicast      : integer range 0 to 1 := 0;
    maxsize        : integer := 1500;
    gmiimode       : integer range 0 to 1 := 0
    );
  port(
    rst            : in  std_ulogic;
    clk            : in  std_ulogic;
    rxi            : in  host_rx_type;
    rxo            : out rx_host_type
  );           
end entity;

architecture rtl of greth_rx is
--  constant maxsize   : integer := 1518;
  constant maxsizerx : unsigned(15 downto 0) :=
    to_unsigned(maxsize + 18, 16);
  constant minsize   : integer := 64;
    
  --receiver types
  type rx_state_type is (idle, wait_sfd, data1, data2, errorst, report_status,
                         wait_report, check_crc, discard_packet);
  
  type rx_reg_type is record
    er              : std_ulogic;
    en              : std_ulogic;
    rxd             : std_logic_vector(3 downto 0);
    rxdp            : std_logic_vector(3 downto 0);
    crc             : std_logic_vector(31 downto 0);
    sync_start      : std_ulogic;
    gotframe        : std_ulogic;
    start           : std_ulogic;
    write           : std_ulogic; 
    done            : std_ulogic; 
    odd_nibble      : std_ulogic;
    lentype         : std_logic_vector(15 downto 0);
    ltfound         : std_ulogic;
    byte_count      : std_logic_vector(10 downto 0);
    data            : std_logic_vector(31 downto 0);
    dataout         : std_logic_vector(31 downto 0);
    rx_state        : rx_state_type;
    status          : std_logic_vector(3 downto 0);
    write_ack       : std_logic_vector(nsync-1 downto 0);
    done_ack        : std_logic_vector(nsync downto 0);
    rxen            : std_logic_vector(1 downto 0);
    got4b           : std_ulogic;
    mcasthash       : std_logic_vector(5 downto 0);
    hashlock        : std_ulogic;

    --rmii
    enold           : std_ulogic;
    act             : std_ulogic;
    dv              : std_ulogic;
    cnt             : std_logic_vector(3 downto 0);
    rxd2            : std_logic_vector(1 downto 0);
    speed           : std_logic_vector(1 downto 0);
    zero            : std_ulogic;
  end record;

  --receiver signals
  signal r, rin     : rx_reg_type;
  signal rxrst      : std_ulogic;
  signal vcc        : std_ulogic;


begin
  vcc <= '1';

  rx_rst : eth_rstgen
  port map(rst, clk, vcc, rxrst, open);
  
  rx : process(rxrst, r, rxi) is
    variable v         : rx_reg_type;
    variable index     : integer range 0 to 3;
    variable crc_en    : std_ulogic;
    variable write_req : std_ulogic;
    variable write_ack : std_ulogic;
    variable done_ack  : std_ulogic;

    variable er        : std_ulogic;
    variable dv        : std_ulogic;
    variable act       : std_ulogic;
    variable rxd       : std_logic_vector(3 downto 0);
  begin
    v := r; v.rxd := rxi.rxd(3 downto 0);
    if rmii = 0 then
      v.en := rxi.rx_dv;
    else
      v.en := rxi.rx_crs;
    end if;
    v.er := rxi.rx_er; write_req := '0'; crc_en := '0';
    index := conv_integer(r.byte_count(1 downto 0));
    
    --synchronization
    v.rxen(1) := r.rxen(0); v.rxen(0) := rxi.enable;
   
    v.write_ack(0) := rxi.writeack;
    v.done_ack(0)  := rxi.doneack; 
    
    if nsync = 2 then
      v.write_ack(1) := r.write_ack(0);
      v.done_ack(1)  := r.done_ack(0);
    end if;

    write_ack := not (r.write xor r.write_ack(nsync-1));
    done_ack  := not (r.done xor r.done_ack(nsync-1));

    --rmii/mii
    if rmii = 0 then
      er := r.er; dv := r.en; act := r.en; rxd := r.rxd;
    else
      --sync 
      v.speed(1) := r.speed(0); v.speed(0) := rxi.speed;
      
      rxd := r.rxd(1 downto 0) & r.rxd2;

      if r.cnt = "0000" then
        v.cnt := "1001";
      else
        v.cnt := r.cnt - 1;
      end if;

      if v.cnt = "0000" then
        v.zero := '1';
      else
        v.zero := '0';
      end if;
    
      act := r.act; er := '0';
      
      if r.speed(1) = '0' then
        if r.zero = '1' then
          v.enold := r.en; 
          dv := r.en and r.dv;
          v.dv := r.act and not r.dv;
          if r.dv = '0' then
            v.rxd2 := r.rxd(1 downto 0);
          end if;
          if (r.enold or r.en) = '0' then
            v.act := '0';
          end if;
        else
          dv := '0';
        end if;
      else
        v.enold := r.en; 
        dv := r.en and r.dv;
        v.dv := r.act and not r.dv;
        v.rxd2 := r.rxd(1 downto 0);
        if (r.enold or r.en) = '0' then
          v.act := '0';
        end if;
      end if;
    end if;

    if (r.en and not r.act) = '1' then
      if (rxd = "0101") and (r.speed(1) or
         (not r.speed(1) and r.zero)) = '1' then
        v.act := '1'; v.dv := '0'; v.rxdp := rxd;
      end if;
    end if;

    if (dv = '1') then
      v.rxdp := rxd;
    end if;

    if multicast = 1 then
      if (r.byte_count(2 downto 0) = "110") and (r.hashlock = '0') then
        v.mcasthash := r.crc(5 downto 0); v.hashlock := '1';
      end if;
    end if;
    
    --fsm
    case r.rx_state is
    when idle =>
      v.gotframe := '0'; v.status := (others => '0'); v.got4b := '0';
      v.byte_count := (others => '0'); v.odd_nibble := '0';
      v.ltfound := '0';
      if multicast = 1 then
        v.hashlock := '0';
      end if;
      if (dv and r.rxen(1)) = '1' then
        if (rxd = "1101") and (r.rxdp = "0101") then
          v.rx_state := data1; v.sync_start := not r.sync_start;
        end if;
        v.start := '0'; v.crc := (others => '1');
        if er = '1' then v.status(2) := '1'; end if;
      elsif dv = '1' then
        v.rx_state := discard_packet;
      end if;
    when discard_packet =>
      if act = '0' then v.rx_state := idle; end if; 
    when data1 =>
      if (act and dv) = '1' then 
        crc_en := '1';
        v.odd_nibble := not r.odd_nibble; v.rx_state := data2;
        case index is
        when 0 => v.data(27 downto 24) := rxd;
        when 1 => v.data(19 downto 16) := rxd;
        when 2 => v.data(11 downto 8)  := rxd;
        when 3 => v.data(3 downto 0)   := rxd; 
        end case;
      elsif act = '0' then
        v.rx_state := check_crc;
      end if;
      if (r.byte_count(1 downto 0) = "00" and (r.start and act and dv) = '1') then
        write_req := '1';
      end if;
      if er = '1' then v.status(2) := '1'; end if;
      if conv_integer(r.byte_count) > maxsizerx then
        v.rx_state := errorst; v.status(1) := '1';
        v.byte_count := r.byte_count - 4;
      end if;
      v.got4b := v.byte_count(2) or r.got4b;
    when data2 =>
      if (act and dv) = '1' then
        crc_en := '1';
        v.odd_nibble := not r.odd_nibble; v.rx_state := data1;
        v.byte_count := r.byte_count + 1; v.start := '1';
        case index is
        when 0 => v.data(31 downto 28) := rxd;
        when 1 => v.data(23 downto 20) := rxd;
        when 2 => v.data(15 downto 12) := rxd;
        when 3 => v.data(7 downto 4)   := rxd;
        end case;
      elsif act = '0' then
        v.rx_state := check_crc;
      end if;
      if er = '1' then v.status(2) := '1'; end if;
      v.got4b := v.byte_count(2) or r.got4b;
    when check_crc =>
      if r.crc /= X"C704DD7B" then
        if r.odd_nibble = '1' then v.status(0) := '1';
        else v.status(2) := '1'; end if;
      end if;
      if write_ack = '1' then
        if r.got4b = '1' then
          v.byte_count := r.byte_count - 4;
        else
          v.byte_count := (others => '0');
        end if;
        v.rx_state := report_status; 
        if conv_integer(r.byte_count) < minsize then
          v.rx_state := wait_report; v.done := not r.done;
        end if;
      end if; 
    when errorst =>
      if act = '0' then
        v.rx_state := wait_report; v.done := not r.done;
        v.gotframe := '1';
      end if;
    when report_status =>
      v.done := not r.done; v.rx_state := wait_report;
      v.gotframe := '1';
    when wait_report =>
      if done_ack = '1' then
        if act = '1' then
          v.rx_state := discard_packet;
        else
          v.rx_state := idle;
        end if;
      end if;    
    when others => null;
    end case;

    --write to fifo
    if write_req = '1' then
      if (r.status(3) or not write_ack) = '1' then
        v.status(3) := '1';
      else
        v.dataout := r.data; v.write := not r.write; 
      end if;
      if (r.byte_count(4 downto 2) = "100") and (r.ltfound = '0') then
        v.lentype := r.data(31 downto 16) + 14; v.ltfound := '1';
      end if;
    end if;

    if write_ack = '1' then 
      if rxi.writeok = '0' then v.status(3) := '1'; end if;
    end if;

    --crc generation
    if crc_en = '1' then
      v.crc := calccrc(rxd, r.crc); 
    end if;
    
    if rxrst = '0' then
      v.rx_state  := idle; v.write := '0'; v.done := '0'; v.sync_start := '0';
      v.done_ack  := (others => '0'); 
      v.gotframe  := '0'; v.write_ack := (others => '0');
      v.dv := '0'; v.cnt := (others => '0'); v.zero := '0';
      v.byte_count := (others => '0'); v.lentype := (others => '0');
      v.status := (others => '0'); v.got4b := '0'; v.odd_nibble := '0';
      v.ltfound := '0';
      v.mcasthash := (others => '0');
      v.dataout := (others => '0');
      if multicast = 1 then
        v.hashlock := '0';
      end if;

    end if;
    if rmii = 0 then
        v.cnt := (others => '0'); v.zero := '0';
    end if;

    rin            <= v;
    rxo.dataout    <= r.dataout;
    rxo.start      <= r.sync_start;
    rxo.done       <= r.done;
    rxo.write      <= r.write;
    rxo.status     <= r.status;
    rxo.gotframe   <= r.gotframe;
    rxo.byte_count <= r.byte_count;
    rxo.lentype    <= r.lentype;
    rxo.mcasthash  <= r.mcasthash;
    
  end process;

  gmiimode0 : if gmiimode = 0 generate
     rxregs0 : process(clk) is
     begin
       if rising_edge(clk) then
         r <= rin;
       end if;
     end process;
  end generate;

  gmiimode1 : if gmiimode = 1 generate
     rxregs1 : process(clk) is
     begin
       if rising_edge(clk) then
         if (rxi.rx_en = '1' or rxrst = '0') then r <= rin; end if;
       end if;
     end process;
  end generate;


end architecture;

