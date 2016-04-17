--! 
--! @mainpage RISC-V System-on-Chip VHDL IP library
--! 
--! @par Overview
--! The IP Library is an integrated set of reusable IP cores, designed for 
--! system-on-chip (SOC) development. The IP cores are centered around a 
--! common on-chip AMBA AXI system bus, and use a coherent method for 
--! simulation and synthesis. This library is vendor independent, with support
--! for different CAD tools and target technologies. Inherited from gaisler 
--! GRLIB library plug&play method was further developed and used to configure 
--! and connect the IP cores without the need to modify any global resources.
--!
--! @par Library organization
--! This repository is organized around VHDL libraries, where each major IP 
--! is assigned a unique library name. Using separate libraries avoids name 
--! clashes between IP cores and hides unnecessary implementation details 
--! from the end user. 
--!
--! @par Satellite Navigation support
--! Hardware part of the satellite navigation functionality is fully 
--! implemented inside of the <i>gnsslib</i> library. This library is the 
--! commercial product of GNSS Sensor limited and in this shared repository 
--! you can find only: modules declaration, configuration parameters and 
--! stub modules that provide enough functionality to use SOC as
--! general purpose processor system based on RISC-V architecture.
--! Netlists of the real GNSS IPs either as RF front-end for the FPGA
--! development boards could be acquires via special request.
--!
--! @par Common Top-level structure
--! <img src="pics/soc_top.png" alt="Top Level"> 
--! 
--! @par Features
--!   <ul>
--!     <li>Pre-generated single-core \e "Rocket-chip" core (RISC-V).
--!         This is 64-bits processor with I/D caches, MMU, branch predictor,
--!         128-bits width data bus, FPU (if enabled) and etc.</li>
--!     <li>VHDL Bridge from TileLinks to AXI4 (NASTI) bus.</li>
--!     <li>Set of common peripheries: UART, GPIO (LEDs), Interrupt controller
--!         etc.</li>
--!     <li>@link dbg_link Debugging @endlink via @link eth_link Ethernet 
--!         @endlink using EDCL capability of the MAC. This
--!         capability allows to redirect UDP requests directly on system bus
--!         and allows to use external debugger from the Reset Vector.</li>
--!     <li>Debug Support Unit (DSU) provides access to the internal registers
--!        of the CPUs using HostIO interface.</li>
--!     <li>Templates for the AXI slaves and master devices with DMA access</li>
--!     <li>Configuration parameters to enable/disable additional functionality,
--!         like: <em><b>GNSS Engine</b>, <b>Viterbi decoder</b>, etc.</em></li>
--!   </ul>
--! 
--! @par Top-level simulation
--! Use file <b>work/tb/rocket_soc.vhd</b> to run simulation scenario. You can
--! get the following time diagram after simulation of 2 ms interval.
--!
--! <img src="pics/soc_sim.png" alt="Simulating top"> 
--!
--! @note Provided Firmware can detect RTL simulation target (see 
--!       <i>fw/boot/src/main.c</i> line 35) and can speed-up simulation
--!       by removing some delay and changing some parameters (UART speed for
--!       example).
--!
--! @par Running on FPGA
--! Supported FPGA:
--! <ul>
--!    <li>ML605 with Virtex6 FPGA using ISE 14.7 (default).</li>
--!    <li>KC705 with Kintex7 FPGA using Vivado 2015.4.</li>
--! </ul>
--! @warning <em><b>Switch ON DIP[0] (i_int_clkrf) to enable test mode because
--!          you most probably doesn't have RF front-end. Otherwise there 
--!          wouldn't be generated interrupts and, as result, no UART 
--!          output.</b></em>.
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
--!              <li>Multi-system GNSS Engine;</li>
--!              <li>Fast Search modules;</li>
--!              <li>Viterbi decoders;</li>
--!              <li>Self-test generators and so on.</li>
--!          </ul>
--! @warning Disabling this define will tourn off source of the  msec 
--!          interrupts that makes impossible default FW execution.
--!          Source of such interrupts is the gnssengine module (stub 
--!          or full functional device).
constant CFG_GNSSLIB_ENABLE : boolean := true;

--! @brief Enable Fast Search Engine for the GPS signals.
constant CFG_GNSSLIB_FSEGPS_ENABLE : integer := 0;

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
