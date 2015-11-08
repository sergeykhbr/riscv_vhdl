-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	ROM Image with the Firmware
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.ALL;
use IEEE.STD_LOGIC_TEXTIO.ALL;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity RomImage_inferred is
  generic (
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in  global_addr_array_type;
    data    : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
end;

architecture rtl of RomImage_inferred is

constant ROM_ADDR_WIDTH : integer := 16;
constant ROM_LENGTH : integer := 2**(ROM_ADDR_WIDTH-4);

type rom_block is array (0 to ROM_LENGTH-1) of std_logic_vector(7 downto 0);
type rom_type is array (0 to CFG_NASTI_DATA_BYTES-1) of rom_block;

type local_addr_arr is array (0 to CFG_NASTI_DATA_BYTES-1) of integer;

impure function init_rom(file_name : in string) return rom_type is
    file rom_file : text open read_mode is file_name;
    variable rom_line : line;
    variable temp_bv : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable temp_mem : rom_type;
begin
    for i in 0 to (ROM_LENGTH-1) loop
        readline(rom_file, rom_line);
        hread(rom_line, temp_bv);
        for n in 0 to (CFG_NASTI_DATA_BYTES-1) loop
          temp_mem(n)(i) := temp_bv((n+1)*8-1 downto 8*n);
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
        for n in 0 to (CFG_NASTI_DATA_BYTES-1) loop
            t_adr(n) := conv_integer(address(n)(ROM_ADDR_WIDTH-1 downto 4));
            data((n+1)*8-1 downto 8*n) <= rom(n)(t_adr(n));
        end loop;
    end if;
  end process;


end;
