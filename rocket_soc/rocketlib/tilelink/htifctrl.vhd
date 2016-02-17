-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @details   Implementation of the Host Controller device. 
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_rocket.all;

--! @brief   HostIO (HTIF) bus controller. 
--! @todo    Add more flexible multi-sources logic.
entity htifctrl is
  port (
    clk    : in std_logic;
    nrst   : in std_logic;
    srcs   : in host_in_vector;
    htifoi : in host_out_type;
    htifio : out host_in_type
);
end; 
 
architecture arch_htifctrl of htifctrl is

  constant HTIF_ZERO : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0) := (others => '0');

  type reg_type is record
     idx : integer range 0 to CFG_HTIF_SRC_TOTAL-1;
     sel : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0);
  end record;

  signal rin, r : reg_type;
begin

  comblogic : process(srcs, htifoi)
    variable v : reg_type;
    variable req : std_logic;
    variable cur_htif : host_in_type;
  begin

    v := r;
    req := '0';
    cur_htif := host_in_none;

    for n in 0 to CFG_HTIF_SRC_TOTAL-1 loop
       if req = '0' and srcs(n).csr_req_valid = '1' and r.sel = HTIF_ZERO then
          v.idx    := n;
          v.sel(n) := '1';
          req := '1';
          cur_htif := srcs(n);
       end if;
    end loop;

   if r.sel /= HTIF_ZERO then
      cur_htif := srcs(r.idx);
       --! Free bus:
      if (htifoi.csr_resp_valid and srcs(r.idx).csr_resp_ready) = '1' then
         v.sel := HTIF_ZERO;
      end if;
   end if;

    rin <= v;
    htifio <= cur_htif;
  end process;

  reg0 : process(clk, nrst) begin
     if nrst = '0' then
        r.idx <= 0;
        r.sel <= (others =>'0');
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 

  end process;
end;
