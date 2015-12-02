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
--! @warning There are bugs in L2 implementaiton by this reason we implemented
--!          this define that makes possible use Rocket without instantiation 
--!          L1toL2interconnect. Probably these bug will fixed in future.
constant CFG_COMMON_L1toL2_ENABLE : boolean := false;

--! hex file used in a case of inferred Boot ROM
constant CFG_SIM_BOOTROM_HEX : string := "E:/Projects/VHDLProjects/rocket/fw_images/bootimage.hex";

--! hex file used in a case of inferred FwImage ROM
constant CFG_SIM_FWIMAGE_HEX : string := "E:/Projects/VHDLProjects/rocket/fw_images/fwimage.hex";

--! @brief Disable/Enable usage of the 'gnsslib' library.
--! @details This 'gnsslib' is the property of the "GNSS Sensor ltd"
--!          (www.gnss-sensor.com) and it implements a lot of Navigation related
--!          peripheries, like: Multi-system GNSS Engine, Fast Search modules,
--!          Viterbi decoders, Self-test generators and so on.
constant CFG_GNSSLIB_ENABLE : boolean := false;

end;
