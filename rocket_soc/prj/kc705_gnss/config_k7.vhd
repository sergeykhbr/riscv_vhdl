-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	FPGA Kintex7 specific constants definition.
------------------------------------------------------------------------------
library techmap;
use techmap.gencomp.all;

package config_target is
-- Technology and synthesis options
  constant CFG_FABTECH : integer := kintex7;
  constant CFG_MEMTECH : integer := kintex7;
  constant CFG_PADTECH : integer := kintex7;
  constant CFG_JTAGTECH : integer := kintex7;
end;
