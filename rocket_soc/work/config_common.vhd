-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	     Target independent configuration file.
--! @details  	 This file defines constants and the system paramters that are
--  		          valid for any ASIC, FPGA and Simulation projects.
------------------------------------------------------------------------------
library techmap;
use techmap.gencomp.all;

package config_common is

--! @brief Disable/Enable L2-cache as part of Uncore module.
--! @details After removing MultiChannelTop layer from verilog this option
--!          became unavailable due the different data bits width 128 <=> 64
--!          bits. Probably we will fix it in a future.
constant CFG_COMMON_L1toL2_ENABLE : boolean := false;

end;
