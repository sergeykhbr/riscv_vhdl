/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API methods declaration.
 */
/**
 * @page sw_debugger_api_link SW Debugger API
 * 
 * @par Overview
 *      This debugger was specially developed as a software utility to interact
 *      with our SOC implementation in \c riscv_soc repository. The main
 *      purpose was to provide convinient way to develop and debug our 
 *      Satellite Navigation firmware that can not be debugged by any other 
 *      tool provided RISC-V community. Additionally, we would like to use
 *      the single unified application capable to work with Real and Simulated
 *      platforms without any modification of source code.
 *      Debugger provides base functionality such as: run control,
 *      read/write memory, registers and CSRs, breakpoints. It allows to
 *      reload FW image and reset target.
 *      Also we are developing own version of the CPU simulator
 *      (analog of \c spike) that can be extended with peripheries models to
 *      Full SOC simulator. These extensions for the debugger simplify
 *      porting procedure of the Operating System (Zephyr project) so that 
 *      simulation doesn't require any hardware and allow develop SW
 *      simultaneously with HW developing.
 *
 * @section dbg_1 C++ Project structure
 *
 * General idea of the project is to develop one \c Core library 
 * providing API methods for registering \c classes, \c services, \c attributes
 * and methods to interact with them. Each extension plugin registers one or 
 * several class services performing some usefull work. All plugins are 
 * built as an independent libraries that are opening by \c Core library
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
 * formatted configuration files <b>targets/target_name.json</b>.
 * These files store all instantiated services names, attributes values
 * and interconnect among plugins.
 * This configuration can be saved to/load from file at any
 * time. By default command \c exit will save current debugger state into
 * file (including full command history). 
 *
 * @note You can manually add/change new Registers/CSRs names and indexes
 *       by modifying this config file without changing source code.
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
 *      -# \b cpu_sysc_plugin - Precise SystemC model of RIVER CPU
 *                     (so/dll library).
 *      -# \b socsim_plugin - Functional models of the peripheries 
 *                     and assembled board (so/dll library). This plugin
 *                     registers several classes: \c UART, \c GPIO, \c SRAM,
 *                     \c ROMs and etc.
 * 
 * @section dbg_target Debug Target
 *
 * We provide several targets that can run your software (bootloader, firmware
 * or user application) without source code modifications:
 * 
 * Start Configuration            | Description
 * -------------------------------|-----------------
 * $ ./_run_functional_sim.sh[bat]| Functional RISC-V Full System Model
 * $ ./_run_systemc_sim.sh[bat]   | Use SystemC Precise Model of RIVER CPU
 * $ ./_run_fpga_gui.sh[bat]      | FPGA board. Default port 'COM3', TAP IP = 192.168.0.51
 *
 * To run debugger with real FPGA target connected via Ethernet do:
 * @verbatim
 *     # cd rocket_soc/debugger/win32build/debug
 *     # _run_functional_sim.bat
 * @endverbatim
 *
 * The result should look like on the picture below:
 * 
 * <img src="pics/dbg_fpga_gui1.png" alt="debugger FPGA+GUI"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_fpga_gui1.png}} @endlatexonly
 *
 * @subsection sw_debug_ss1 Plugins interaction structure
 *
 * Core library uses UDP protocol to communicate with all targets: FPGA or
 * simulators. The general structure is looking like on the following figure:
 *
 * <img src="pics/dbg_sim.png" alt="sim debug"> 
 * @latexonly {\includegraphics[scale=0.9]{pics/dbg_sim.png}} @endlatexonly
 *
 * or with real Hardware
 *
 * <img src="pics/dbg_fpga.png" alt="fpga debug"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_fpga.png}} @endlatexonly
 *
 * @section sw_debug_troubles Troubleshooting
 *
 * @subsection sw_problem_1 Image Files not found
 *
 * If you'll get the error messages that image files not found
 *
 * <img src="pics/dbg_err1.png" alt="File not found"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_err1.png}} @endlatexonly
 * 
 * To fix this problem do the following steps:
 *     -# Close debugger console using \c exit command.
 *     -# Open <em>config_file_name.json</em> file in any editor.
 *     -# Find strings that specify these paths and correct them. Simulator 
 * uses the same images as VHDL platform for ROMs intialization. You can find
 * them in <em>'rocket_soc/fw_images'</em> directory. After that you should
 * see something like follow:
 *
 * <img src="pics/dbg_simout1.png" alt="Simulator output"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_simout1.png}} @endlatexonly
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
 * @subsection sw_problem_2 Can't open COM3 when FPGA is used
 *    -# Open <em>fpga_gui.json</em>
 *    -# Change value <b>['ComPortName','COM3'],</b> on your one
 *       (for an example on \c ttyUSB0).
 *
 * @subsection sw_problem_3 EDCL: No response. Break read transaction
 * This erros means that host cannot locate board with specified IP address.
 * Before you continue pass through the following checklist:
 *    -# You should properly @link eth_link setup network connection @endlink
 * and see FPGA board in ARP-table.
 *    -# If you've changed default FPGA IP address:
 *          -# Open <em>fpga_gui.json</em>
 *          -# Change value <b>['BoardIP','192.168.0.51']</b> on your one.
 *    -# Run debugger
 *
 * Example of debugging session (Switch ON all User LEDs on board):
 * @code
 *      riscv# help                     -- Print full list of commands
 *      riscv# csr MCPUID               -- Read supported ISA extensions
 *      riscv# read 0xfffff000 20       -- Read 20 bytes from PNP module
 *      riscv# write 0x80000000 4 0xff  -- Write into GPIO new LED value
 *      riscv# loadelf helloworld       -- Load elf-file to board RAM and run
 * @endcode
 *          
 * Debugger console view
 *
 * <img src="pics/dbg_testhw.png" alt="HW debug example"> 
 * @latexonly {\includegraphics{pics/dbg_testhw.png}} @endlatexonly
 *
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
int RISCV_set_configuration(AttributeType *cfg);

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
 * @brief Triggersystem event (hap) from Service.
 */
void RISCV_trigger_hap(IFace *isrc, int type, const char *descr);

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

/**
 * @brief State of the core library.
 *
 * Core library is active while woudln't break by RISCV_break_simulation()
 */
int RISCV_is_active();

#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_CORE_H__
