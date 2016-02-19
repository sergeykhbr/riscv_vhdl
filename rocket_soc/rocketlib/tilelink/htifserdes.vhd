-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Implementation of the 'htifserdes' module.
--! @details    Used only for system with enabled L2-cache.
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_rocket.all;

--! @brief   Uncore messages serializer/deserializer.
--! @details There is implemented logic of conversion of the 128-bit 
--!          'Uncore' messages on chunks of HTIF_WIDTH bits size (default 16
--!          bits). Message is formed using HostIO signal request.
entity htif_serdes is
generic (
     core_idx : integer := 0
);
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

--! @name    Uncore message IDs specific for HostIO interface.
--! @brief   Used See htif.scala, class Htif, line about 105.
--! @{

--! Reading from physical memory. Not used in this SOC.
constant HTIF_CMD_READ_MEMORY : std_logic_vector(3 downto 0) := X"0";
--! Writting into physical memory. Not used in this SOC.
constant HTIF_CMD_WRITE_MEMORY : std_logic_vector(3 downto 0) := X"1";
--! Reading Control Register (CSR).
constant HTIF_CMD_READ_CONTROL_REG : std_logic_vector(3 downto 0) := X"2";
--! Writting Control Register (CSR).
constant HTIF_CMD_WRITE_CONTROL_REG : std_logic_vector(3 downto 0) := X"3";
--! Handshake success.
constant HTIF_CMD_ACK : std_logic_vector(3 downto 0) := X"4";
--! Handshake error.
constant HTIF_CMD_NACK : std_logic_vector(3 downto 0) := X"5";
--! @}

constant HTIF_DATA_EMPTY : 
	std_logic_vector(HTIF_WIDTH-1 downto 0) := (others => '1');

type state_type is (wait_cmd, transmit, response, resp_ready);

type registers is record
  state    : state_type;
  muxCnt   : integer range 0 to 128/HTIF_WIDTH + 1;
  cmd      : std_logic_vector(127 downto 0);
  seqno    : std_logic_vector(7 downto 0);
end record;

signal r, rin: registers;

function functionMakeMessage( r : registers;
                        hst : host_out_type)
return std_logic_vector is
   variable ret :  std_logic_vector(127 downto 0);
begin
   ret(127 downto 64) := hst.csr_req_bits_data;
   ret(63 downto 44) := conv_std_logic_vector(core_idx, 20);
   ret(43 downto 36) := X"00";
   ret(35 downto 24) := hst.csr_req_bits_addr;
   ret(23 downto 16) := r.seqno;
   ret(15 downto 4)  := X"001"; -- total number of words
   if hst.csr_req_bits_rw = '1' then
      ret(3 downto 0)   := HTIF_CMD_WRITE_CONTROL_REG;
   else
      ret(3 downto 0)   := HTIF_CMD_READ_CONTROL_REG;
   end if;
   return ret;
end;


begin

  comblogic : process(hostoi, srdi, r)
    variable v : registers;
    variable vo : host_in_type;
  begin
    v := r;
    vo.grant := (others => '0');
    vo.csr_req_ready := '0';
    vo.csr_resp_valid := '0';
    vo.csr_resp_bits := (others => '0');
    vo.debug_stats_csr := '0';

    case r.state is
    when wait_cmd =>
       vo.csr_req_ready := '1';
       if hostoi.csr_req_valid = '1' then
             v.state := transmit;
             v.seqno := r.seqno + 1;
             v.cmd := functionMakeMessage(r, hostoi);
       end if;
       srdo.valid <= '0';
       srdo.ready <= '0';
       srdo.bits  <= (others => '0');
    when transmit =>
      --! Multiplexer of the command into HTIF bus
       if srdi.ready = '1' then
          v.muxCnt := r.muxCnt + 1;
       end if;
       v.cmd :=  HTIF_DATA_EMPTY & r.cmd(127 downto HTIF_WIDTH);
       if r.muxCnt = (128/HTIF_WIDTH) - 1 then
          v.state := response;
          v.muxCnt := 0;
       end if;
       srdo.valid <= '1';
       srdo.ready <= '0';
       srdo.bits  <= r.cmd(HTIF_WIDTH-1 downto 0);
    when response =>
       if srdi.valid = '1' then
            v.muxCnt := r.muxCnt + 1;
            v.cmd := srdi.bits & r.cmd(127 downto HTIF_WIDTH);
            if r.muxCnt = (128/HTIF_WIDTH) - 1 then
                v.state := resp_ready;
            end if;
       end if;
       srdo.valid <= '0';
       srdo.ready <= '1';
       srdo.bits  <= (others => '0');
    when resp_ready =>
       if hostoi.csr_resp_ready = '1' then
          v.state := wait_cmd;
       end if;
       srdo.valid <= '0';
       srdo.ready <= '0';
       srdo.bits  <= (others => '0');
       vo.csr_resp_valid := '1';
       vo.csr_resp_bits := r.cmd(127 downto 64);
    when others =>
    end case;

    rin <= v;
    hostio <= vo;

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
