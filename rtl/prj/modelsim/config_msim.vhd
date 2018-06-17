-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2018 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	     ModelSim specific constants definition.
------------------------------------------------------------------------------
library techmap;
use techmap.gencomp.all;

package config_target is
-- Technology and synthesis options
  constant CFG_FABTECH : integer := inferred;
  constant CFG_MEMTECH : integer := inferred;
  constant CFG_PADTECH : integer := inferred;
  constant CFG_JTAGTECH : integer := inferred;

  constant CFG_TOPDIR : string := "../../../";

end;
