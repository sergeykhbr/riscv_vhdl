-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      HostIO Interface request serializer
--! @details    Used only for system with enabled L2-cache.
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_rocket.all;

entity htif_serdes is
port (
    clk   : in std_logic;
    nrst  : in std_logic;
    hostoi : in host_out_type;
    hostio : out host_in_type;
    srdi  : in htif_serdes_in_type;
    srdo  : out htif_serdes_out_type
);
end;

architecture arch_htif_serdes of htif_serdes is

constant CORE_IDX : std_logic_vector(15 downto 0) := X"0000";
type state_type is (wait_cmd, transmit);

type registers is record
  state    : state_type;
  muxCnt   : integer range 0 to 9;
  --! Multiplexed data
  cmd      : std_logic_vector(127 downto 0);
  seqno    : std_logic_vector(7 downto 0);
end record;

signal r, rin: registers;
begin

  comblogic : process(hostoi, srdi, r)
    variable v : registers;
    variable data     : std_logic_vector(HTIF_WIDTH-1 downto 0);
  begin
    v := r;

    case r.state is
    when wait_cmd =>
       srdo.valid <= '0';
       if hostoi.csr_req_valid = '1' then
             v.seqno := r.seqno + 1;
             v.cmd := hostoi.csr_req_bits_data 
                     & CORE_IDX & X"0000" 
                     & hostoi.csr_req_bits_addr
                     & r.seqno
                     & X"01"
                     & X"3";
       end if;
    when transmit =>
      --! Multiplexer of the command into HTIF bus
      srdo.valid <= '1';
      if srdi.ready = '1' then
        v.muxCnt := r.muxCnt + 1;

        case r.muxCnt is
          when 0 =>  data := v.cmd(15 downto  0);
          when 1 =>  data := v.cmd(31 downto  16);
          when 2 =>  data := v.cmd(47 downto  32);
          when 3 =>  data := v.cmd(63 downto  48);
          when 4 =>  data := v.cmd(79 downto  64);
          when 5 =>  data := v.cmd(95 downto  80);
          when 6 =>  data := v.cmd(111 downto  96);
          when 7 =>  data := v.cmd(127 downto  112);
          when 8 =>  v.state := wait_cmd;
          when others =>
        end case;
      end if;  
    end case;

    rin <= v;

    srdo.ready <= '1';
    srdo.bits <= data;
  end process;



  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.state <= wait_cmd;
       r.muxCnt <= 0;
       r.cmd <= (others => '0');
       r.seqno <= X"01";
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
