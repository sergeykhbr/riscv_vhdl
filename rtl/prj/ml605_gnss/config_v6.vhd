-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief	FPGA Virtex6 specific constants definition.
------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library techmap;
use techmap.gencomp.all;

package config_target is
-- Technology and synthesis options
  constant CFG_FABTECH : integer := virtex6;
  constant CFG_MEMTECH : integer := virtex6;
  constant CFG_PADTECH : integer := virtex6;
  constant CFG_JTAGTECH : integer := virtex6;

  constant CFG_TOPDIR : string := "../../../";

  --! @brief   Disable/Enable River CPU instance.
  --! @details When enabled platform will instantiate processor named as
  --!          "RIVER" entirely written on VHDL. 
  --!          Otherwise "Rocket" will be used (developed by Berkley
  --!          team).
  --! @warning DSU available only for \e "RIVER" processor.
  constant CFG_COMMON_RIVER_CPU_ENABLE : boolean := true;

  --! @brief   Dual-core configuration enabling
  --! @details This config parameter used only with CPU River
  constant CFG_COMMON_DUAL_CORE_ENABLE : boolean := false;

  --! @brief   HEX-image for the initialization of the Boot ROM.
  --! @details This file is used by \e inferred ROM implementation.
  constant CFG_SIM_BOOTROM_HEX : string := 
              CFG_TOPDIR & "examples/boot/linuxbuild/bin/bootimage.hex";

  --! @brief   HEX-image for the initialization of the FwImage ROM.
  --! @details This file is used by \e inferred ROM implementation.
  constant CFG_SIM_FWIMAGE_HEX : string := 
                CFG_TOPDIR & "examples/fasttask/makefiles/bin/gnssfw.hex";
                

  --! @brief Hardware SoC Identificator.
  --!
  --! @details Read Only unique platform identificator that could be
  --!          read by firmware from the Plug'n'Play support module.
  constant CFG_HW_ID : std_logic_vector(31 downto 0) := X"20190524";

  --! @brief Enabling Ethernet MAC interface.
  --! @details By default MAC module enables support of the debug feature EDCL.
  constant CFG_ETHERNET_ENABLE : boolean := true;

  --! @brief Enable/Disable Debug Unit 
  constant CFG_DSU_ENABLE : boolean := true;

end;
