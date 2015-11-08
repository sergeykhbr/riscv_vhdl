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
library rocketlib;
use rocketlib.types_nasti.all;

entity BootRom_inferred is
  port (
    clk     : in  std_ulogic;
    address : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
    data    : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
end;

architecture rtl of BootRom_inferred is

constant ROM_ADDR_WIDTH : integer := 13;
constant ROM_LENGTH : integer := 2**(ROM_ADDR_WIDTH-4);

type rom_type is array (0 to ROM_LENGTH-1) of std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

impure function init_rom(file_name : in string) return rom_type is
    file rom_file : text open read_mode is file_name;
    variable rom_line : line;
    variable temp_bv : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable temp_mem : rom_type;
begin
    for i in 0 to (ROM_LENGTH-1) loop
        readline(rom_file, rom_line);
        hread(rom_line, temp_bv);
        temp_mem(i) := temp_bv;
    end loop;
    return temp_mem;
end function;

constant rom : rom_type := init_rom("E:/Projects/VHDLProjects/rocket/fw_images/bootimage.hex");

begin

  reg : process (clk, address) begin
    if rising_edge(clk) then 
        data <= rom(conv_integer(address));
    end if;
  end process;

end;
