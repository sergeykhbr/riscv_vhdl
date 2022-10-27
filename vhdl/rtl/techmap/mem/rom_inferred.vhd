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
use ieee.numeric_std.ALL;
use IEEE.STD_LOGIC_TEXTIO.ALL;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

entity Rom_inferred is
  generic (
    abits : integer;
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in global_addr_array_type;
    data    : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  );
end;

architecture rtl of Rom_inferred is

constant ROM_LENGTH : integer := 2**(abits - log2(CFG_SYSBUS_DATA_BYTES));

type rom_block is array (0 to ROM_LENGTH-1) of std_logic_vector(31 downto 0);
type rom_type is array (0 to CFG_WORDS_ON_BUS-1) of rom_block;

type local_addr_arr is array (0 to CFG_WORDS_ON_BUS-1) of integer;

impure function init_rom(file_name : in string) return rom_type is
    file rom_file : text open read_mode is file_name;
    variable rom_line : line;
    variable temp_bv : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable temp_mem : rom_type;
begin
    for i in 0 to (ROM_LENGTH-1) loop
        readline(rom_file, rom_line);
        hread(rom_line, temp_bv);
        for n in 0 to (CFG_WORDS_ON_BUS-1) loop
          temp_mem(n)(i) := temp_bv((n+1)*32-1 downto 32*n);
        end loop;
    end loop;
    return temp_mem;
end function;

constant rom : rom_type := init_rom(hex_filename);

begin

  reg : process (clk) 
    variable t_adr : local_addr_arr;
  begin
    if rising_edge(clk) then 
        for n in 0 to CFG_WORDS_ON_BUS-1 loop
            t_adr(n) := conv_integer(address(n)(abits-1 downto log2(CFG_SYSBUS_DATA_BYTES)));
            data(32*(n+1)-1 downto 32*n) <= rom(n)(t_adr(n));
        end loop;
    end if;
  end process;

end;
