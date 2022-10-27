-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Definition of the gencomp package.
--! @details   This file defines constants that are used to enable/disable
--!            target dependable modules.
--!            This file inherits values from the \e grlib library that
--!            that are published under GPL license. All unused values may
--!            freely removed or reassigned on others values.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;

--! @brief   Technologies names definition
--! @details This package must be built first in a case of manual compilation
--!          order (\e ModelSim).
package gencomp is

--! @brief   Total number of the known technologies.
--! @details These values was inherited from the \e grlib library.
constant NTECH : integer := 53;

--! Prototype of the data type for mapping name on certain index.
type tech_ability_type is array (0 to NTECH) of integer;

--! @name  Techologies names.
--! @brief Set of the predefined technology names.
--! @{
constant inferred    : integer := 0;  --! Behaviour simulation target.
constant virtex      : integer := 1;  --! Not implemented.
constant virtex2     : integer := 2;  --! Not implemented.
constant memvirage   : integer := 3;  --! Not implemented.
constant axcel       : integer := 4;  --! Not implemented.
constant proasic     : integer := 5;  --! Not implemented.
constant atc18s      : integer := 6;  --! Not implemented.
constant altera      : integer := 7;  --! Not implemented.
constant umc         : integer := 8;  --! Not implemented.
constant rhumc       : integer := 9;  --! Not implemented.
constant apa3        : integer := 10; --! Not implemented.
constant spartan3    : integer := 11; --! Not implemented.
constant ihp25       : integer := 12; --! Not implemented.
constant rhlib18t    : integer := 13; --! Not implemented.
constant virtex4     : integer := 14; --! Not implemented.
constant lattice     : integer := 15; --! Not implemented.
constant ut25        : integer := 16; --! Not implemented.
constant spartan3e   : integer := 17; --! Not implemented.
constant peregrine   : integer := 18; --! Not implemented.
constant memartisan  : integer := 19; --! Not implemented.
constant virtex5     : integer := 20; --! Not implemented.
constant custom1     : integer := 21; --! Not implemented.
constant ihp25rh     : integer := 22; --! Not implemented.
constant stratix1    : integer := 23; --! Not implemented.
constant stratix2    : integer := 24; --! Not implemented.
constant eclipse     : integer := 25; --! Not implemented.
constant stratix3    : integer := 26; --! Not implemented.
constant cyclone3    : integer := 27; --! Not implemented.
constant memvirage90 : integer := 28; --! Not implemented.
constant tsmc90      : integer := 29; --! Not implemented.
constant easic90     : integer := 30; --! Not implemented.
constant atc18rha    : integer := 31; --! Not implemented.
constant smic013     : integer := 32; --! Not implemented.
constant tm65gpl     : integer := 33; --! Not implemented.
constant axdsp       : integer := 34; --! Not implemented.
constant spartan6    : integer := 35; --! Supported. Use files with the '_s6' suffix.
constant virtex6     : integer := 36; --! Supported. Use files with the '_v6' suffix.
constant actfus      : integer := 37; --! Not implemented.
constant stratix4    : integer := 38; --! Not implemented.
constant st65lp      : integer := 39; --! Not implemented.
constant st65gp      : integer := 40; --! Not implemented.
constant easic45     : integer := 41; --! Not implemented.
constant cmos9sf     : integer := 42; --! Not implemented.
constant apa3e       : integer := 43; --! Not implemented.
constant apa3l       : integer := 44; --! Not implemented.
constant ut130       : integer := 45; --! Not implemented.
constant ut90        : integer := 46; --! Not implemented.
constant gf65        : integer := 47; --! Not implemented.
constant virtex7     : integer := 48; --! Not implemented.
constant kintex7     : integer := 49; --! Supported. Use files with the '_k7' suffix.
constant artix7      : integer := 50; --! Not implemented.
constant zynq7000    : integer := 51; --! Not implemented.
constant rhlib13t    : integer := 52; --! Not implemented.
constant mikron180   : integer := 53; --! Mikron 180nm. Use files with the '_micron180' suffix.

--! @}

--! @name    FPGAs technologies group.
--! @details It is convinient sometimes to implement one module for a group of
--!          technologies, this array specifies FPGA group.
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

