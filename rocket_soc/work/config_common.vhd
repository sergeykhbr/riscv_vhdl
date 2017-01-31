-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Definition of the config_common package.
--! @details    This file defines constants and the system paramters that are
--!    	        valid for any ASIC, FPGA and Simulation projects.
------------------------------------------------------------------------------
--!
--! @defgroup config_common_group SoC configuration constants
--! @ingroup generic_group
--! @details Target independible constants that are the same for FPGA, ASIC 
--!          and behaviour simulation.
--! @{
--!

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
                

--! @brief Disable/Enable usage of the <em><b>gnsslib library</b></em>.
--!
--! @details This \e  'gnsslib' is the property of the <em>"GNSS Sensor ltd"
--!          </em> (\link www.gnss-sensor.com \endlink) and it 
--!          implements a lot of Navigation related peripheries, like: 
--!          <ul>
--!              <li>RF front-end synthezators controller;</li>
--!              <li>Multi-system GNSS Engine;</li>
--!              <li>Fast Search modules;</li>
--!              <li>Viterbi decoders;</li>
--!              <li>Self-test generators and so on.</li>
--!          </ul>
--! @warning This define enables RF front-end clock as a source of ADC clock.
--! 
constant CFG_GNSSLIB_ENABLE : boolean := false;

--! @brief Enable GNSS Engine module.
constant CFG_GNSSLIB_GNSSENGINE_ENABLE : boolean := false;
--! @brief Enable Fast Search Engine for the GPS signals.
constant CFG_GNSSLIB_FSEGPS_ENABLE : boolean := false;

--! @brief Enabling Ethernet MAC interface.
--! @details By default MAC module enables support of the debug feature EDCL.
constant CFG_ETHERNET_ENABLE : boolean := true;

--! @brief Enable/Disable Debug Unit 
constant CFG_DSU_ENABLE : boolean := true;

--! @brief Remove BUFGMUX from project and use internaly generate ADC clock.
--! @details We have some difficulties with Vivado + Kintex7 constrains, so
--!          to make test-mode stable working we use this temporary config
--!          parameter that hardcodes 'test_mode' is always enabled
constant CFG_TESTMODE_ON : boolean := true;

end;

--! @}
--!
