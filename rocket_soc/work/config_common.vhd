-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Definition of the config_common package.
--! @details    This file defines constants and the system paramters that are
--!    	        valid for any ASIC, FPGA and Simulation projects.
------------------------------------------------------------------------------

--! @defgroup config_common_group SoC configuration constants
--! @ingroup generic_group
--! @details Target independible constants that are the same for FPGA, ASIC 
--!          and behaviour simulation.
--! @{
--!

--! Standard library
library IEEE;
--! Standard signal definitions
use IEEE.STD_LOGIC_1164.ALL;

--! Technology definition library
library techmap;
--! Generic IDs constants import
use techmap.gencomp.all;

--! @brief   Techology independent configuration settings.
--! @details This file defines configuration that are valid for all supported
--!          targets: behaviour simulation, FPGAs or ASICs.
package config_common is

--! @brief   Disable/Enable River CPU instance.
--! @details When enabled platform will instantiate processor named as
--!          "RIVER" entirely written on VHDL. 
--!          Otherwise "Rocket" will be used (developed by Berkley
--!          team).
--! @warning DSU available only for \e "RIVER" processor.
constant CFG_COMMON_RIVER_CPU_ENABLE : boolean := true;

--! @brief   HEX-image for the initialization of the Boot ROM.
--! @details This file is used by \e inferred ROM implementation.
constant CFG_SIM_BOOTROM_HEX : string := 
              "../../fw_images/bootimage.hex";

--! @brief   HEX-image for the initialization of the FwImage ROM.
--! @details This file is used by \e inferred ROM implementation.
constant CFG_SIM_FWIMAGE_HEX : string := 
                "../../fw_images/fwimage.hex";
                

--! @brief Hardware SoC Identificator.
--!
--! @details Read Only unique platform identificator that could be
--!          read by firmware from the Plug'n'Play support module.
constant CFG_HW_ID : std_logic_vector(31 downto 0) := X"20170311";

--! @brief Enabling Ethernet MAC interface.
--! @details By default MAC module enables support of the debug feature EDCL.
constant CFG_ETHERNET_ENABLE : boolean := true;

--! @brief Enable/Disable Debug Unit 
constant CFG_DSU_ENABLE : boolean := true;

end;

--! @}
--!
