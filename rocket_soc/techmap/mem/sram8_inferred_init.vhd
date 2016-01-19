----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      8-bits memory block with the generic data size parameter.
--! @details    This module absolutely similar to the 'inferred' implementation
--!             but it support initialization of the SRAM.
--!             This feature is very useful during RTL simulation so that
--!             current FW supports skipping of the copying FwImage state.
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.ALL;
use IEEE.STD_LOGIC_TEXTIO.ALL;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;

entity sram8_inferred_init is
  generic (
    abits     : integer := 12;
    byte_idx  : integer := 0;
    init_file : string
  );
  port (
    clk     : in  std_ulogic;
    address : in  std_logic_vector(abits-1 downto 0);
    rdata   : out std_logic_vector(7 downto 0);
    we      : in  std_logic;
    wdata   : in  std_logic_vector(7 downto 0)
  );
end;

architecture arch_sram8_inferred_init of sram8_inferred_init is

constant FILE_IMAGE_LINES_TOTAL : integer := 16384;

constant SRAM_LENGTH : integer := 2**abits;
type ram_type is array (0 to SRAM_LENGTH-1) of std_logic_vector(7 downto 0);

impure function init_ram(file_name : in string) return ram_type is
    file ram_file : text open read_mode is file_name;
    variable ram_line : line;
    variable temp_bv : std_logic_vector(127 downto 0);
    variable temp_mem : ram_type;
begin
    for i in 0 to (FILE_IMAGE_LINES_TOTAL-1) loop
        readline(ram_file, ram_line);
        hread(ram_line, temp_bv);
        temp_mem(i) := temp_bv((byte_idx+1)*8-1 downto 8*byte_idx);
    end loop;
    return temp_mem;
end function;

--! @warning SIMULATION INITIALIZATION
signal ram : ram_type := init_ram(init_file);
signal adr : std_logic_vector(abits-1 downto 0);

begin

  reg : process (clk, address, wdata) begin
    if rising_edge(clk) then 
      if we = '1' then
        ram(conv_integer(address)) <= wdata;
      end if;
      adr <= address;
    end if;
  end process;

  rdata <= ram(conv_integer(adr));
end;
