------------------------------------------------------------------------------
--  This file is a part of the GRLIB VHDL IP LIBRARY
--  Copyright (C) 2003 - 2008, Gaisler Research
--  Copyright (C) 2008 - 2014, Aeroflex Gaisler
--  Copyright (C) 2015 - 2016, Cobham Gaisler
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
-----------------------------------------------------------------------------
-- Entity: 	eth_rstgen
-- File:	eth_rstgen.vhd
-- Author:	Jiri Gaisler - Gaisler Research
-- Description:	Reset generation with glitch filter
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity eth_rstgen is
  generic (acthigh : integer := 0);
  port (
    rstin     : in  std_ulogic;
    clk       : in  std_ulogic;
    clklock   : in  std_ulogic;
    rstout    : out std_ulogic;
    rstoutraw : out std_ulogic
  );
end;

architecture rtl of eth_rstgen is
signal r : std_logic_vector(4 downto 0);
signal rst : std_ulogic;

attribute equivalent_register_removal : string;
attribute keep                        : string;

attribute equivalent_register_removal of r : signal is "no";
attribute equivalent_register_removal of rst : signal is "no";

attribute keep of r : signal is "true";
attribute keep of rst : signal is "true";

begin

  rst <= not rstin when acthigh = 1 else rstin;
  rstoutraw <= rst;

  reg1 : process (clk, rst) begin
    if rising_edge(clk) then 
      r <= r(3 downto 0) & clklock; 
      rstout <= r(4) and r(3) and r(2);
    end if;
    if rst = '0' then r <= "00000"; rstout <= '0'; end if;
  end process;

end;

