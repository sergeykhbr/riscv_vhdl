-----------------------------------------------------------------------------
-- @file
-- @author  Sergey Khabarov
-- @brief   ROM storage with the boot image (4KB default = 16 x 256)
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
use techmap.allmem.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;


entity nasti_bootrom is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
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
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS)
  );

  type registers is record
    bank_axi : nasti_slave_bank_type;
  end record;



signal r, rin : registers;

signal raddr0 : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
signal rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

begin

  comblogic : process(i, r, rdata)
    variable v : registers;
  begin

    v := r;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);
    raddr0 <= v.bank_axi.raddr(0)(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
    
    o <= functionAxi4Output(r.bank_axi, rdata);

    rin <= v;
  end process;

  cfg  <= xconfig;
  
  tech0 : BootRom_tech generic map (
    memtech => memtech
  ) port map (
    clk     => clk,
    address => raddr0,
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