-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	Target independent configuration file.
--! @details  	This file defines constants and the system paramters that are
--  		valid for any ASIC, FPGA and Simulation projects.
------------------------------------------------------------------------------
library techmap;
use techmap.gencomp.all;

package config_common is

constant CFG_COMMON_L1toL2_ENABLE : boolean := false;

end;
