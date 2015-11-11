-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Rocket Cores hard-reset initialization module
--! @details    Everytime after hard reset Rocket core is in resetting
--!             state. Module Uncore::HTIF implements writting into 
--!             MRESET CSR-register (0x784) and not allow to start CPU
--!             execution. This resetting cycle is ongoing upto external
--!             write 0-value into this MRESET register.
-----------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library rocketlib;
use rocketlib.types_rocket.all;

entity Starter is port 
(
    clk   : in std_logic;
    nrst  : in std_logic;
    i     : in starter_in_type;
    o     : out starter_out_type
);
end;

architecture Starter_rtl of Starter is

--! data, addr(core=-1), seqno=1, words=1, cmd=HTIF_CMD_WRITE_CONTROL_REG
constant CSR_WR_PLLDIVIDER : std_logic_vector(127 downto 0) := 
    X"0000000000020005" & (X"fffff0003f" & X"01" & X"001" & X"3");

--! data, addr(core=0),  seqno=2, words=1, cmd=HTIF_CMD_WRITE_CONTROL_REG
constant CSR_WR1_CSR29 : std_logic_vector(127 downto 0) := 
    X"0000000000000001" & (X"000000001d" & X"02" & X"001" & X"3"); 

--! data, addr(core=0),  seqno=3, words=1, cmd=HTIF_CMD_WRITE_CONTROL_REG
constant CSR_WR0_CSR29 : std_logic_vector(127 downto 0) := 
    X"0000000000000000" & (X"000000001d" & X"03" & X"001" & X"3");

--! data, addr(core=0),  seqno=4, words=1, cmd=HTIF_CMD_WRITE_CONTROL_REG
constant CSR_WR0_MRESET : std_logic_vector(127 downto 0) := 
    X"0000000000000000" & (X"0000000782" & X"04" & X"001" & X"3");


type registers is record
  in_valid : std_logic;
  muxCnt   : integer range 0 to 9;
  cmdCnt   : integer range 0 to 3;
  --! Multiplexed data
  data     : std_logic_vector(HTIF_WIDTH-1 downto 0);
end record;

signal r, rin: registers;
begin

  comblogic : process(i, r)
  variable v : registers;
    variable wCmd : std_logic_vector(127 downto 0);
  begin
    v := r;

    --! Select CSR write command
    case r.cmdCnt is
      when 0 =>   wCmd := CSR_WR_PLLDIVIDER;
      when 1 =>   wCmd := CSR_WR1_CSR29;
      when 2 =>   wCmd := CSR_WR0_CSR29;
      when 3 =>   wCmd := CSR_WR0_MRESET;
      when others => wCmd := (others =>'0');
    end case;

    --! Multiplexer of the command into HTIF bus
    if i.in_ready = '1' then
      v.muxCnt := r.muxCnt + 1;

      case r.muxCnt is
        when 0 =>  v.data := wCmd(15 downto  0); v.in_valid := '1'; 
        when 1 =>  v.data := wCmd(31 downto  16);
        when 2 =>  v.data := wCmd(47 downto  32);
        when 3 =>  v.data := wCmd(63 downto  48);
        when 4 =>  v.data := wCmd(79 downto  64);
        when 5 =>  v.data := wCmd(95 downto  80);
        when 6 =>  v.data := wCmd(111 downto  96);
        when 7 =>  v.data := wCmd(127 downto  112);
        when 8 =>  
          v.data  := X"face";
          v.in_valid := '0'; 
          if r.cmdCnt = 3 then
            v.muxCnt := 8;
          else
            v.cmdCnt    := r.cmdCnt + 1;
            v.muxCnt := 0;
          end if;
        when others =>
      end case;
    end if;

    rin <= v;
  end process;

  o.in_valid  <= r.in_valid;
  o.out_ready <= '1';
  o.in_bits   <= r.data;
  o.exit_t    <= (others =>'0');


  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.in_valid <= '0';
       r.muxCnt <= 0;
       r.cmdCnt <= 0;
       r.data <= (others => '0');
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
