----------------------------------------------------------------------------
--  INFORMATION:  http://www.GNSS-sensor.com
--  PROPERTY:     GNSS Sensor Ltd
--  E-MAIL:       sergey.khabarov@gnss-sensor.com
--  DESCRIPTION:  This file contains copy of the firmware image
------------------------------------------------------------------------------
--  WARNING:      
------------------------------------------------------------------------------
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

entity BootRom_inferred is
  generic (
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in global_addr_array_type;
    data    : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
end;

architecture rtl of BootRom_inferred is

constant ROM_ADDR_WIDTH : integer := 13;
constant ROM_LENGTH : integer := 2**(ROM_ADDR_WIDTH - log2(CFG_NASTI_DATA_BYTES));

type rom_block is array (0 to ROM_LENGTH-1) of std_logic_vector(31 downto 0);
type rom_type is array (0 to CFG_WORDS_ON_BUS-1) of rom_block;

type local_addr_arr is array (0 to CFG_WORDS_ON_BUS-1) of integer;

impure function init_rom(file_name : in string) return rom_type is
    file rom_file : text open read_mode is file_name;
    variable rom_line : line;
    variable temp_bv : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
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
            t_adr(n) := conv_integer(address(n)(ROM_ADDR_WIDTH-1 downto log2(CFG_NASTI_DATA_BYTES)));
            data(32*(n+1)-1 downto 32*n) <= rom(n)(t_adr(n));
        end loop;
    end if;
  end process;

end;
