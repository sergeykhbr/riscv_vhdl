--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

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


entity axi4_sram is
  generic (
    memtech  : integer := inferred;
    async_reset : boolean := false;
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
 
architecture arch_axi4_sram of axi4_sram is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_SRAM
  );

  constant wstrb_zero : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0) := (others => '0');

  type registers is record
    bank_axi : nasti_slave_bank_type;
  end record;
  
  type ram_in_type is record
    raddr : global_addr_array_type;
    waddr : global_addr_array_type;
    we    : std_logic;
    wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  end record;

signal r, rin : registers;

signal rdata_mux : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
signal rami : ram_in_type;

begin

  comblogic : process(nrst, i, r, rdata_mux)
    variable v : registers;
    variable vrami : ram_in_type;
    variable vslvo : nasti_slave_out_type;
  begin

    v := r;

    procedureAxi4toMem(
      i      => i,
      cfg    => xconfig,
      i_bank => r.bank_axi,
      o_bank => v.bank_axi,
      o_radr => vrami.raddr,
      o_wadr => vrami.waddr,
      o_wstrb => vrami.wstrb,
      o_wdata => vrami.wdata
    );

    vrami.we := '0';
    if vrami.wstrb /= wstrb_zero then
        vrami.we := '1';
    end if;

    procedureMemToAxi4(
       i_dualport => '0',
       i_rready => '1',
       i_rdata => rdata_mux,
       i_bank => r.bank_axi,
       i_slvi => i,
       o_slvo => vslvo
    );

    if not async_reset and nrst = '0' then
       v.bank_axi := NASTI_SLAVE_BANK_RESET;
    end if;
    
    rami <= vrami;
    rin <= v;
    o <= vslvo;
  end process;

  cfg  <= xconfig;
  
  tech0 : srambytes_tech generic map (
    memtech   => memtech,
    abits     => abits,
    init_file => init_file -- only for 'inferred'
  ) port map (
    clk     => clk,
    raddr   => rami.raddr,
    rdata   => rdata_mux,
    waddr   => rami.waddr,
    we      => rami.we,
    wstrb   => rami.wstrb,
    wdata   => rami.wdata
  );

  -- registers:
  regs : process(clk, nrst)
  begin 
     if async_reset and nrst = '0' then
       r.bank_axi <= NASTI_SLAVE_BANK_RESET;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
