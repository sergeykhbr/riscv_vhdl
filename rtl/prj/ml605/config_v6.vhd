-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	FPGA Virtex6 specific constants definition.
------------------------------------------------------------------------------
library techmap;
use techmap.gencomp.all;

package config_target is
-- Technology and synthesis options
  constant CFG_FABTECH : integer := virtex6;
  constant CFG_MEMTECH : integer := virtex6;
  constant CFG_PADTECH : integer := virtex6;
  constant CFG_JTAGTECH : integer := virtex6;
end;
