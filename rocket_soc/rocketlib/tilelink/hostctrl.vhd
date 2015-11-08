-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Host IO interface controller.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_tile.all;

entity HostController is
port 
 (
    clk   : in  std_logic;
    nrst  : in  std_logic;
    i     : in hostctrl_in_type;
    o     : out hostctrl_out_type
  );
end; 
 
architecture arch_HostController of HostController is


  type registers is record
    host_reset : std_logic_vector(1 downto 0);
  end record;


signal r, rin : registers;


begin

  comblogic : process(i, r)
    variable v : registers;
  begin

    v := r;

    v.host_reset := r.host_reset(1) & '0';
   
    rin <= v;
  end process;


   o.host.reset <= r.host_reset(1);
   o.host.id <= '0';
   o.host.csr_req_valid <= '0';
   o.host.csr_req_bits_rw <= '0';
   o.host.csr_req_bits_addr <= (others => '0');
   o.host.csr_req_bits_data <= (others => '0');
   o.host.csr_resp_ready <= '0';
   o.host.ipi_req_ready <= '0';
   o.host.ipi_rep_valid <= '0';
   o.host.ipi_rep_bits <= '0';


  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.host_reset <= (others => '1');
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
