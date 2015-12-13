--! 
--! @mainpage General configuration RISC-V System-on-Chip
--! 
--! This project provides SoC top-level template with the following features:
--!   <ul>
--!     <li>Pre-generated single-core \e "Rocket-chip" core (RISC-V).</li>
--!     <li>VHDL Bridge from TileLinks to AXI4 (NASTI) bus.</li>
--!     <li>Set of common peripheries to verify core: UART, GPIO (LEDs) etc.</li>
--!     <li>Configuration paramters to enable/disable additional functionality,
--!         like: <em><b>GNSS Engine</b>, <b>Viterbi decoder</b>, etc.</em></li>
--!   </ul>
--! 
--! <b>Common Top-level structure:</b>
--! <img src="pics/soc_top.png" alt="Top Level"> 
--! 
--! Information about GNSS (<em>Satellite Navigation Engine</em>) you can find at
--! @link www.gnss-sensor.com.

-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Definition of the config_common package.
--! @details    This file defines constants and the system paramters that are
--!    	        valid for any ASIC, FPGA and Simulation projects.
------------------------------------------------------------------------------
--! Technology definition library
library techmap;
use techmap.gencomp.all;

--! @brief   Techology independent configuration settings.
--! @details This file defines configuration that are valid for all supported
--!          targets: behaviour simulation, FPGAs or ASICs.
package config_common is

--! @brief   Disable/Enable L2-cache.
--! @details Disabling L2-cache is made by removing \e "Uncore" module
--!          from the SoC. Such configuration provides only Single-Core
--!          mode.
--! @warning There are bugs in the L2 implementation by this reason we've
--!          added this define to provide possibility of using "Rocket" 
--!          core without \e "Uncore" module.
--!          Probably these bugs will be fixed in the nearest future.
constant CFG_COMMON_L1toL2_ENABLE : boolean := false;

--! @brief   HEX-image for the initialization of the Boot ROM.
--! @details This file is used by \e inferred ROM implementation.
constant CFG_SIM_BOOTROM_HEX : string := 
              "E:/Projects/VHDLProjects/rocket/fw_images/bootimage.hex";

--! @brief   HEX-image for the initialization of the FwImage ROM.
--! @details This file is used by \e inferred ROM implementation.
constant CFG_SIM_FWIMAGE_HEX : string := 
                "E:/Projects/VHDLProjects/rocket/fw_images/fwimage.hex";

--! @brief Disable/Enable usage of the <em><b>gnsslib library</b></em>.
--! @details This \e  'gnsslib' is the property of the <em>"GNSS Sensor ltd"
--!          </em> (\link www.gnss-sensor.com \endlink) and it 
--!          implements a lot of Navigation related peripheries, like: 
--!          <ul>
--!              <li>Multi-system GNSS Engine;</li>
--!              <li>Fast Search modules;</li>
--!              <li>Viterbi decoders;</li>
--!              <li>Self-test generators and so on.</li>
--!          </ul>
constant CFG_GNSSLIB_ENABLE : boolean := false;

end;
