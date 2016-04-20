/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API methods declaration.
 */
/**
 * @page dbg_link SW Debugger (C++)
 * 
 * @par Overview
 *      This debugger was specially developed as a software utility to interact
 *      with our SOC implementation in \c rocket_soc repository. The main 
 *      purpose was to provide convinient way to develop and debug our 
 *      Satellite Navigation firmware that can not be debugged by any other 
 *      tool provided RISC-V community (for now).
 *      Debugger provides base functionality such as read/write memory 
 *      and CSR registers, reloading FW image and reset target. Also we are
 *      developing own version of the CPU simulator (analog of \c spike) 
 *      and Full SOC simulators. These extensions for the debugger simplify
 *      porting procedure of the Operating System (Zephyr project) so that 
 *      simulation doesn't require any hardware and allow to see CPU registers
 *      on each step and logging them into file.
 *
 * @section dbg_1 Project structure
 *
 * General idea of the project is to develop one \c Core library 
 * providing API methods for registering \c classes, \c services, \c attributes
 * and methods to interact with them. Each extension plugin registers one or 
 * several class services performing some usefull work. All plugins are 
 * built as an independent libraries that are opening by \Core library
 * at initialization stage with the call of method <b>plugin_init()</b>.
 * All Core API methods start with \c RISCV_... prefix:
 * @code
 *   void RISCV_register_class(IFace *icls);
 *
 *   IFace *RISCV_create_service(IFace *iclass, const char *name, 
 *                               AttributeType *args);
 *
 *   IFace *RISCV_get_service(const char *name);
 *   ...
 * @endcode
 *
 * Configuration of the debugger and plugins is fully described in JSON
 * formatted configuraiton file <b>config.json</b>. This file stores all
 * instantiated services names, attributes values and interconnect among
 * plugins. This configuration can be saved to/load from file at any
 * time. By default command \c exit will save current debugger state into
 * file (including full command history). 
 *
 * @note You can manually add/change new CSR registers names and indexes
 *       by modifying this config file without changing source code. If you
 *       accidently corrupt the JSON format you can return default setting
 *       by starting debugger with \c -nocfg key.
 * 
 * @par Folders description
 *    -# \b libdgb64g - Core library (so/dll) that provides standard API 
 * methods defined in file \c api_core.h.
 *    -# \b appdbg64g - Executable (exe) file implements functionality of 
 *                      the console debugger.
 *    -# \a Plugins:
 *      -# \b simple_plugin - Simple plugin (so/dll library) just for
 *                      demonstration of the integration with debugger.
 *      -# \b cpu_fnc_plugin - Functional model of the RISC-V CPU 
 *                     (so/dll library).
 *      -# \b socsim_plugin - Functional models of the peripheries 
 *                     and assembled board (so/dll library). This plugin
 *                     registers several classes: \c UART, \c GPIO, \c SRAM,
 *                     \c ROMs and etc.
 * 
 * @section dbg_target Debug Target
 *
 * We provide two targets that can run your software (bootloader, firmware
 * or user application) without modifications:
 *     -# Platform simulator. Use it if you haven't ML605 or KC705 FPGA boards
 * that are used in our developemnt porjects or just for fun.
 *     -# FPGA boards. We provide bit-files for ML605 (Virtex6) and KC705
 * (Kintex7) boards that can be used without synthesis procedure.
 *
 * @subsection dbg_sim Debug SOC simulator
 *      To run simulation board that reproduce behaviour of the full system
 *      (GPIO, UART, GNSS engine etc) it is enough to start debugger executable 
 *      file with key <b> -sim </b>.
 *
 * @verbatim bin/release/appdbg64g.exe -sim  @endverbatim
 *
 * In this case common structure will be look as follow:
 *
 * <img src="pics/dbg_sim.png" alt="sim debug"> 
 *
 * If you'll get the error messages that image files not found
 *
 * <img src="pics/dbg_err1.png" alt="File not found"> 
 * 
 * To fix this problem do the following steps:
 *     -# Close debugger console using \c exit command.
 *     -# Open <em>config.json</em> file in any editor.
 *     -# Find strings that specify these paths and correct them. Simulator 
 * uses the same images as VHDL platform for ROMs intialization. You can find
 * them in <em>'rocket_soc/fw_images'</em> directory. After that you should
 * see something like follow:
 *
 * <img src="pics/dbg_simout1.png" alt="Simulator output"> 
 *
 * Debug your target. All commands that are available for Real Hardware
 * absolutely valid for the Simulation. Debugger doesn't see any difference
 * between these two targets.
 *
 * @note We redirect all output streams of the simulated platform into
 *      debugger console but we are going to implement independent GUI for
 *      simulated platform with its own UART/GPIO or Ethernet outputs
 *      and serial console window.
 *
 * @subsection dbg_hw Debug Real Hardware
 *
   Before you start pass through the following checklist:
 *    -# You should properly @link eth_link setup network connection @endlink
 * and see FPGA board in ARP-table.
 *    -# If you've changed default FPGA IP address:
 *          -# Open <em>config.json</em>
 *          -# Change value <b>['BoardIP','192.168.0.51']</b> on your one.
 *    -# Run debugger
 *
 * @verbatim bin/release/appdbg64g.exe @endverbatim
 *
 * In this case connection scheme with FPGA board looks as follow:
 *
 * <img src="pics/dbg_fpga.png" alt="fpga debug"> 
 *
 * Example of debugging session (Switch ON all User LEDs on board):
 * @code
 *      riscv# csr MCPUID               -- Read supported ISA extensions
 *      riscv# read 0xfffff000 20       -- Read 20 bytes from PNP module
 *      riscv# csr MRESET 1             -- Raise CPU reset signal
 *      riscv# write 0x80000000 4 0xff  -- Write into GPIO new LED value
 *      riscv# loadelf helloworld       -- Load elf-file to board RAM and run
 * @endcode
 *          
 * Debugger console view
 *
 * <img src="pics/dbg_testhw.png" alt="HW debug example"> 
 *
 * FPGA serial console output:
 *
 * <img src="pics/dbg_testhw2.png" alt="Serial output"> 
 */

#ifndef __DEBUGGER_API_CORE_H__
#define __DEBUGGER_API_CORE_H__

#include "api_utils.h"
#include "iface.h"
#include "attribute.h"

namespace debugger {

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Library initialization with the defined configuration.
 * @param [in] config Configuration attribute.
 */
int RISCV_init();

/**
 * @param Destroy and cleanup all dynamically allocated objects,
 */
void RISCV_cleanup();

/** 
 * @brief Set core library configuration.
 * @details Configuration specify all instantiated services and interconnect
 *          among them.
 */
void RISCV_set_configuration(AttributeType *cfg);

/** 
 * @brief Get current core configuration.
 */
const char *RISCV_get_configuration();

/** 
 * @brief Get current core configuration.
 */
const AttributeType *RISCV_get_global_settings();

/**
 * @brief Registration of the class in the library kernel.
 */
void RISCV_register_class(IFace *icls);

/**
 * @brief Registration of the system event (hap) listener.
 */
void RISCV_register_hap(IFace *ihap);

/**
 * @brief Get registred class interface by its name.
 */
IFace *RISCV_get_class(const char *name);

/**
 * @brief Create service of the specified class.
 */
IFace *RISCV_create_service(IFace *iclass, const char *name, 
                                        AttributeType *args);

/**
 * @brief Get IService interface by its name.
 */
IFace *RISCV_get_service(const char *name);

/**
 * @brief Get interface of the specified service.
 */
IFace *RISCV_get_service_iface(const char *servname, const char *facename);

/// @todo add attributes/save/restore

/**
 * @brief Get list of services implementing specific interface.
 */
void RISCV_get_services_with_iface(const char *iname, AttributeType *list);


/**
 * @brief Get list of all clock generators.
 * @details Clock generator must implement IClock (and usually IThread)
 *          interfaces. CPU is a most general clock generator.
 */
void RISCV_get_clock_services(AttributeType *list);


/**
 * @brief Break all threads that could be run by different services.
 */
void RISCV_break_simulation();

#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_CORE_H__
