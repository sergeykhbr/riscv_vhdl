
library ieee;
use ieee.std_logic_1164.all;

package gencomp is

-- technologies and libraries

constant NTECH : integer := 53;
type tech_ability_type is array (0 to NTECH) of integer;

constant inferred    : integer := 0;
constant virtex      : integer := 1;
constant virtex2     : integer := 2;
constant memvirage   : integer := 3;
constant axcel       : integer := 4;
constant proasic     : integer := 5;
constant atc18s      : integer := 6;
constant altera      : integer := 7;
constant umc         : integer := 8;
constant rhumc       : integer := 9;
constant apa3        : integer := 10;
constant spartan3    : integer := 11;
constant ihp25       : integer := 12;
constant rhlib18t    : integer := 13;
constant virtex4     : integer := 14;
constant lattice     : integer := 15;
constant ut25        : integer := 16;
constant spartan3e   : integer := 17;
constant peregrine   : integer := 18;
constant memartisan  : integer := 19;
constant virtex5     : integer := 20;
constant custom1     : integer := 21;
constant ihp25rh     : integer := 22;
constant stratix1    : integer := 23;
constant stratix2    : integer := 24;
constant eclipse     : integer := 25;
constant stratix3    : integer := 26;
constant cyclone3    : integer := 27;
constant memvirage90 : integer := 28;
constant tsmc90      : integer := 29;
constant easic90     : integer := 30;
constant atc18rha    : integer := 31;
constant smic013     : integer := 32;
constant tm65gpl     : integer := 33;
constant axdsp       : integer := 34;
constant spartan6    : integer := 35;
constant virtex6     : integer := 36;
constant actfus      : integer := 37;
constant stratix4    : integer := 38;
constant st65lp      : integer := 39;
constant st65gp      : integer := 40;
constant easic45     : integer := 41;
constant cmos9sf     : integer := 42;
constant apa3e       : integer := 43;
constant apa3l       : integer := 44;
constant ut130       : integer := 45;
constant ut90        : integer := 46;
constant gf65        : integer := 47;
constant virtex7     : integer := 48;
constant kintex7     : integer := 49;
constant artix7      : integer := 50;
constant zynq7000    : integer := 51;
constant rhlib13t    : integer := 52;
constant micron180   : integer := 53;

constant DEFMEMTECH  : integer := inferred;
constant DEFPADTECH  : integer := inferred;
constant DEFFABTECH  : integer := inferred;

constant is_fpga : tech_ability_type :=
	(inferred => 1, virtex => 1, virtex2 => 1, axcel => 1,
	 proasic => 1, altera => 1, apa3 => 1, spartan3 => 1,
         virtex4 => 1, lattice => 1, spartan3e => 1, virtex5 => 1,
	 stratix1 => 1, stratix2 => 1, eclipse => 1,
	 stratix3 => 1, cyclone3 => 1, axdsp => 1,
	 spartan6 => 1, virtex6 => 1, actfus => 1,
	 stratix4 => 1, apa3e => 1, apa3l => 1, virtex7 => 1, kintex7 => 1,
	 artix7 => 1, zynq7000 => 1,
	 others => 0);

end;

