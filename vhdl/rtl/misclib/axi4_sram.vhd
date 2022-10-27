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
    cfg  : out axi4_slave_config_type;
    i    : in  axi4_slave_in_type;
    o    : out axi4_slave_out_type
  );
end; 
 
architecture arch_axi4_sram of axi4_sram is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_SRAM
  );

  type ram_in_type is record
    raddr : global_addr_array_type;
    re    : std_logic;
    waddr : global_addr_array_type;
    we    : std_logic;
    wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  end record;

signal rdata_mux : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
signal rami : ram_in_type;

begin

  cfg  <= xconfig;

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i,
    o_xslvo => o,
    i_ready => '1',
    i_rdata => rdata_mux,
    o_re => rami.re,
    o_r32 => open,
    o_radr => rami.raddr,
    o_wadr => rami.waddr,
    o_we => rami.we,
    o_wstrb => rami.wstrb,
    o_wdata => rami.wdata
  );

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

end;
