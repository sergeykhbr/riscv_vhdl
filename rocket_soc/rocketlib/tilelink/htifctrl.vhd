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
entity htifctrl is
  port (
    clk    : in std_logic;
    nrst   : in std_logic;
    srcsi  : in host_out_vector;
    srcso  : out host_out_type;
    htifii : in host_in_type;
    htifio : out host_in_type
);
end; 
 
architecture arch_htifctrl of htifctrl is

  constant HTIF_ZERO : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0) := (others => '0');

  type reg_type is record
     idx : integer range 0 to CFG_HTIF_SRC_TOTAL-1;
     srcsel : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0);
  end record;

  signal rin, r : reg_type;
begin

  comblogic : process(srcsi, htifii, r)
    variable v : reg_type;
    variable idx : integer range 0 to CFG_HTIF_SRC_TOTAL-1;
    variable srcsel : std_logic_vector(CFG_HTIF_SRC_TOTAL-1 downto 0);
    variable req : std_logic;
  begin

    v := r;
    req := '0';
    idx := 0;
    srcsel := r.srcsel;

    for n in 0 to CFG_HTIF_SRC_TOTAL-1 loop
       if req = '0' and srcsi(n).csr_req_valid = '1' then
          idx := n;
          req := '1';
       end if;
    end loop;

    if (srcsel = HTIF_ZERO) or (htifii.csr_resp_valid and srcsi(r.idx).csr_resp_ready) = '1' then
       srcsel(r.idx) := '0';
       srcsel(idx) := req;
       v.idx := idx;
    else
       idx := r.idx;
    end if;

    v.srcsel := srcsel;

    rin <= v;
    
    srcso <= srcsi(idx);
    htifio.grant           <= srcsel;
    htifio.csr_req_ready   <= htifii.csr_req_ready;
    htifio.csr_resp_valid  <= htifii.csr_resp_valid;
    htifio.csr_resp_bits   <= htifii.csr_resp_bits;
    htifio.debug_stats_csr <= htifii.debug_stats_csr;

  end process;

  reg0 : process(clk, nrst) begin
     if nrst = '0' then
        r.idx <= 0;
        r.srcsel <= (others =>'0');
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 

  end process;
end;
