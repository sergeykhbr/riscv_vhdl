-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Internal SRAM module with the byte access
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

library commonlib;
use commonlib.types_common.all;

--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;


entity nasti_sram is
  generic (
    memtech  : integer := inferred;
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    abits    : integer := 17;
    init_file : string := "" -- only for inferred
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_sram of nasti_sram is

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_SRAM,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );

  type registers is record
    bank_axi : nasti_slave_bank_type;
  end record;

signal r, rin : registers;

signal raddr_reg : global_addr_array_type;
signal rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

signal waddr_reg : global_addr_array_type;
signal we        : std_logic;
signal wstrb     : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
signal wdata     : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

begin

  comblogic : process(i, r, rdata)
    variable v : registers;
    variable wr_ena : std_logic;
  begin

    v := r;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
       raddr_reg(n) <= v.bank_axi.raddr(n);
    end loop;

    wr_ena := '0';
    if (i.w_valid = '1' and r.bank_axi.wstate = wtrans 
        and r.bank_axi.wresp = NASTI_RESP_OKAY) then
      wr_ena := '1';
      wdata <= i.w_data;
      wstrb <= i.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES-1 loop
         waddr_reg(n) <= r.bank_axi.waddr(n);
      end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata);
    we <= wr_ena;

    rin <= v;
  end process;

  cfg  <= xconfig;
  
  tech0 : srambytes_tech generic map (
    memtech   => memtech,
    abits     => abits,
    init_file => init_file -- only for 'inferred'
  ) port map (
    clk     => clk,
    raddr   => raddr_reg,
    rdata   => rdata,
    waddr   => waddr_reg,
    we      => we,
    wstrb   => wstrb,
    wdata   => wdata
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
