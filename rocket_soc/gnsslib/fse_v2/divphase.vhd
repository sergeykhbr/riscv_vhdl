-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Integer divider to get 6-bits phase counter
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity Divphase is
port (
    i : in dphs_in_type;
    o : out dphs_out_type
);
end;

architecture rtl of Divphase is

type dif_type is array (0 to 5) of std_logic_vector(37 downto 0);
type resid_type is array (1 to 5) of std_logic_vector(37 downto 0);
signal bit : std_logic_vector(5 downto 0);
begin

  comb : process (i)
  variable dif : dif_type;
  variable resid : resid_type;
  begin

    dif(5) := i.carr_acc - ('0' & i.carr_th & "00000");
    if dif(5)(37) = '0' then
      resid(5) := dif(5);
      bit(5) <= '1';
    else
      resid(5) := i.carr_acc;
      bit(5) <= '0';
    end if;

    dif(4) := resid(5) - ("00" & i.carr_th & "0000");
    if dif(4)(37) = '0' then
      resid(4) := dif(4);
      bit(4) <= '1';
    else
      resid(4) := resid(5);
      bit(4) <= '0';
    end if;

    dif(3) := resid(4) - ("000" & i.carr_th & "000");
    if dif(3)(37) = '0' then
      resid(3) := dif(3);
      bit(3) <= '1';
    else
      resid(3) := resid(4);
      bit(3) <= '0';
    end if;

    dif(2) := resid(3) - ("0000" & i.carr_th & "00");
    if dif(2)(37) = '0' then
      resid(2) := dif(2);
      bit(2) <= '1';
    else
      resid(2) := resid(3);
      bit(2) <= '0';
    end if;

    dif(1) := resid(2) - ("00000" & i.carr_th & "0");
    if dif(1)(37) = '0' then
      resid(1) := dif(1);
      bit(1) <= '1';
    else
      resid(1) := resid(2);
      bit(1) <= '0';
    end if;

    dif(0) := resid(1) - ("000000" & i.carr_th);
    if dif(0)(37) = '0' then
      bit(0) <= '1';
    else
      bit(0) <= '0';
    end if;
  end process;

  o.phase_cnt <= bit;

end; 


