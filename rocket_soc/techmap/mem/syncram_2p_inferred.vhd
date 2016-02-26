-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Synchronous 2-port ram, common clock
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;

entity syncram_2p_inferred is
  generic (
    abits : integer := 8;
    dbits : integer := 32;
    sepclk: integer := 0
  );
  port (
    rclk : in std_ulogic;
    wclk : in std_ulogic;
    rdaddress: in std_logic_vector (abits -1 downto 0);
    wraddress: in std_logic_vector (abits -1 downto 0);
    data: in std_logic_vector (dbits -1 downto 0);
    wren : in std_ulogic;
    q: out std_logic_vector (dbits -1 downto 0)
  );
end;

architecture arch_syncram_2p_inferred of syncram_2p_inferred is
  
  type dregtype is array (0 to 2**abits - 1) 
	of std_logic_vector(dbits -1 downto 0);

  --! This fuinction just to check with C++ reference model. Can be removed.
  impure function init_ram(file_name : in string) return dregtype is
    variable temp_mem : dregtype;
  begin
    for i in 0 to (2**abits - 1) loop
        if dbits = 32 then
           temp_mem(i) := X"00000000";--X"CCCCCCCC";
        else
           temp_mem(i) := X"0000";--X"CCCC";
        end if;
    end loop;
    return temp_mem;
  end function;

  signal rfd : dregtype := init_ram("");

begin

  wp : process(wclk)
  begin
    if rising_edge(wclk) then
      if wren = '1' then rfd(conv_integer(wraddress)) <= data; end if;
    end if;
  end process;

  oneclk : if sepclk = 0 generate
    rp : process(wclk) begin
      if rising_edge(wclk) then 
        q <= rfd(conv_integer(rdaddress)); 
      end if;
    end process;
  end generate;

  twoclk : if sepclk = 1 generate
    rp : process(rclk) begin
    if rising_edge(rclk) then q <= rfd(conv_integer(rdaddress)); end if;
    end process;
  end generate;

end;
