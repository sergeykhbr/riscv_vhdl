-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Implementation of the 'starter' module.
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

--! @brief   Hard-reset initialization module.
--! @details L2-cached system implementing Uncore module must be switched
--!          from resetting state that is done by this module.
--! param[in] HTIF   interface clock.
--! param[in] Reset  signal with the active LOW level.
--! param[in] i_host HostIO input signals.
--! param[out] o_host HostIO output signals.
entity starter is port 
(
    clk   : in std_logic;
    nrst  : in std_logic;
    i_host : in host_in_type;
    o_host : out host_out_type;
    o_init_ena : out std_logic
);
end;

architecture arch_starter of starter is

type state_type is (init_reset, init_cmd, wait_ready, wait_resp, disable);

type registers is record
  state    : state_type;
  init_ena : std_logic;
  cmdCnt   : integer range 0 to 3;
end record;

signal r, rin: registers;
begin

  comblogic : process(i_host, r)
    variable v : registers;
  begin
    v := r;
    
    case r.state is
      when init_reset =>
        v.state := init_cmd;
        o_host.reset <= '1';
        o_host.id    <= '0';
        o_host.csr_req_valid <= '0';
        o_host.csr_req_bits_rw <= '0';
        o_host.csr_req_bits_addr <= (others  => '0');
        o_host.csr_req_bits_data <= (others  => '0');
        o_host.csr_resp_ready <= '1';

      when init_cmd =>
        o_host.reset <= '0';
        --! Select CSR write command
        case r.cmdCnt is
          when 0 =>   
            -- PLL divide. One Tile at once.
            o_host.csr_req_valid <= '1';
            o_host.csr_req_bits_rw <= '1';
            o_host.csr_req_bits_addr <= X"03f";
            o_host.csr_req_bits_data <= X"0000000000020005";
          when 1 =>
            -- Set CSR29.
            o_host.csr_req_valid <= '1';
            o_host.csr_req_bits_rw <= '1';
            o_host.csr_req_bits_addr <= X"01d";
            o_host.csr_req_bits_data <= X"0000000000000001";
          when 2 =>
            -- Clear CSR29.
            o_host.csr_req_valid <= '1';
            o_host.csr_req_bits_rw <= '1';
            o_host.csr_req_bits_addr <= X"01d";
            o_host.csr_req_bits_data <= X"0000000000000000";
          when 3 =>
            -- Write MRESET
            o_host.csr_req_valid <= '1';
            o_host.csr_req_bits_rw <= '1';
            o_host.csr_req_bits_addr <= X"782";
            o_host.csr_req_bits_data <= X"0000000000000000";
          when others =>
            v.state := disable;
        end case;
        if i_host.csr_req_ready = '0' then
            v.state := wait_ready;
        else
            v.state := wait_resp;
        end if;
      when wait_ready =>
           if i_host.csr_req_ready = '1' then
               v.state := wait_resp;
               o_host.csr_req_valid <= '0';
           end if;
      when wait_resp =>
        if i_host.csr_resp_valid = '1' then
            v.cmdCnt    := r.cmdCnt + 1;
            if r.cmdCnt = 3 then
                v.state := disable;
                v.init_ena := '0';
            else
                v.state := init_cmd;
            end if;
        end if;
      when others =>
    end case;

    rin <= v;
  end process;
  
  o_init_ena  <= r.init_ena;


  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.state <= init_reset;
       r.init_ena <= '1';
       r.cmdCnt <= 0;
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
