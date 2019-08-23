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
--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

entity Rom_tech is
generic (
    memtech : integer := 0;
    abits : integer;
    sim_hexfile : string
);
port (
    clk       : in std_logic;
    address   : in global_addr_array_type;
    data      : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
);
end;

architecture rtl of Rom_tech is

  component Rom_inferred is
  generic (
    abits : integer;
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in global_addr_array_type;
    data    : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  );
  end component;

begin

  genrom0 : if memtech = inferred or is_fpga(memtech) /= 0 generate
      infer0 : Rom_inferred  generic map (abits, sim_hexfile)
               port map (clk, address, data);
  end generate;

end; 


