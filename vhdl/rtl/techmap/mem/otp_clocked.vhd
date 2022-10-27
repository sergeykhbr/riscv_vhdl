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

entity otp_clocked is
  port (
    clk     : in  std_ulogic;
    we      : in  std_ulogic;
    re      : in  std_ulogic;
    address : in std_logic_vector(11 downto 0);
    wdata   : in std_logic_vector(15 downto 0);
    rdata   : out std_logic_vector(15 downto 0)
  );
end;

architecture arch_otp_clocked of otp_clocked is


constant SRAM_LENGTH : integer := 2**12;
constant FILE_IMAGE_LINES_TOTAL : integer := SRAM_LENGTH;
type ram_type is array (0 to SRAM_LENGTH-1) of std_logic_vector(15 downto 0);

impure function init_ram(file_name : in string) return ram_type is
    file ram_file : text open read_mode is file_name;
    variable ram_line : line;
    variable temp_bv : std_logic_vector(15 downto 0);
    variable temp_mem : ram_type;
begin
    for i in 0 to (FILE_IMAGE_LINES_TOTAL-1) loop
        readline(ram_file, ram_line);
        hread(ram_line, temp_bv);
        temp_mem(i) := temp_bv;
    end loop;
    return temp_mem;
end function;

--! @warning SIMULATION INITIALIZATION
signal ram : ram_type;-- := init_ram(init_file);

begin

  reg : process (clk, address, we, re, wdata, ram) begin
    if rising_edge(clk) then 
      if we = '1' then
        ram(conv_integer(address)) <= wdata;
      end if;
    end if;

    if wdata = X"FFFF" and re = '1' then
      rdata <= ram(conv_integer(address));
    else
      rdata <= X"CCCC";
    end if;

  end process;

 end;
