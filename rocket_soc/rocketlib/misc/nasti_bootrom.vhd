-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     ROM storage with the boot image (4KB default = 16 x 256)
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

library commonlib;
use commonlib.types_common.all;

--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;


entity nasti_bootrom is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    sim_hexfile : string
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_bootrom of nasti_bootrom is

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_BOOTROM,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );

  type registers is record
    bank_axi : nasti_slave_bank_type;
  end record;



signal r, rin : registers;

signal raddr_reg : global_addr_array_type;
signal rdata     : unaligned_data_array_type;

begin

  comblogic : process(i, r, rdata)
    variable v : registers;
    variable raddr_mux : global_addr_array_type;
    variable rdata_mux : unaligned_data_array_type;
  begin

    v := r;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);
   
    if v.bank_axi.raddr(0)(3 downto 2) = "00" then
       raddr_mux := v.bank_axi.raddr;
    elsif v.bank_axi.raddr(0)(3 downto 2) = "01" then
       raddr_mux(0) := v.bank_axi.raddr(3);
       raddr_mux(1) := v.bank_axi.raddr(0);
       raddr_mux(2) := v.bank_axi.raddr(1);
       raddr_mux(3) := v.bank_axi.raddr(2);
    elsif v.bank_axi.raddr(0)(3 downto 2) = "10" then
       raddr_mux(0) := v.bank_axi.raddr(2);
       raddr_mux(1) := v.bank_axi.raddr(3);
       raddr_mux(2) := v.bank_axi.raddr(0);
       raddr_mux(3) := v.bank_axi.raddr(1);
    else
       raddr_mux(0) := v.bank_axi.raddr(1);
       raddr_mux(1) := v.bank_axi.raddr(2);
       raddr_mux(2) := v.bank_axi.raddr(3);
       raddr_mux(3) := v.bank_axi.raddr(0);
    end if;


    if r.bank_axi.raddr(0)(3 downto 2) = "00" then
       rdata_mux := rdata;
    elsif r.bank_axi.raddr(0)(3 downto 2) = "01" then
       rdata_mux(0) := rdata(1);
       rdata_mux(1) := rdata(2);
       rdata_mux(2) := rdata(3);
       rdata_mux(3) := rdata(0);
    elsif r.bank_axi.raddr(0)(3 downto 2) = "10" then
       rdata_mux(0) := rdata(2);
       rdata_mux(1) := rdata(3);
       rdata_mux(2) := rdata(0);
       rdata_mux(3) := rdata(1);
    else
       rdata_mux(0) := rdata(3);
       rdata_mux(1) := rdata(0);
       rdata_mux(2) := rdata(1);
       rdata_mux(3) := rdata(2);
    end if;

    raddr_reg <= raddr_mux;

    o <= functionAxi4Output(r.bank_axi, rdata_mux);

    rin <= v;
  end process;

  cfg  <= xconfig;
  
  tech0 : BootRom_tech generic map (
    memtech     => memtech,
    sim_hexfile => sim_hexfile
  ) port map (
    clk     => clk,
    address => raddr_reg,
    data    => rdata
  );

  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;