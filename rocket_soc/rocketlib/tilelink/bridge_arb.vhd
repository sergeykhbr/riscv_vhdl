-----------------------------------------------------------------------------
--! @file
--! @brief   Cached/Uncached requests arbiter.
--! @author  Sergey Khabarov
--! @details This file implements multiplexing the tile request into common
--!          NASTI bus..
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity TileBridgeArbiter is
  port (
    i_cached    : in  nasti_slave_in_type;
    i_uncached  : in  nasti_slave_in_type;
    o           : out nasti_slave_in_type
  );
end; 
 
architecture arch_TileBridgeArbiter of TileBridgeArbiter is
begin

  comblogic : process(i_cached, i_uncached)
    variable t : nasti_slave_in_type;
  begin
 
    t := i_uncached;

    if i_cached.aw_valid = '1' then
        t.aw_valid := i_cached.aw_valid;
        t.aw_bits := i_cached.aw_bits;
        t.aw_id := i_cached.aw_id;
        t.aw_user := i_cached.aw_user;
    end if;

    if i_cached.w_valid = '1' then
        t.w_valid := i_cached.w_valid;
        t.w_data := i_cached.w_data;
        t.w_last := i_cached.w_last;
        t.w_strb := i_cached.w_strb;
        t.w_user := i_cached.w_user;
    end if;

    if i_cached.ar_valid = '1' then
        t.ar_valid := i_cached.ar_valid;
        t.ar_bits := i_cached.ar_bits;
        t.ar_id := i_cached.ar_id;
        t.ar_user := i_cached.ar_user;
    end if;

    t.b_ready := i_cached.b_ready or i_uncached.b_ready;
    t.r_ready := i_cached.r_ready or i_uncached.r_ready;
   
    o <= t;
  end process;

end;
