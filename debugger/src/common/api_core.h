/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API methods declaration.
 */
/**
 * @page dbg_overview Overview
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
 *      porting procedure (Zephyr OS for an example) so that 
 *      simulation doesn't require any hardware and allows to develop SW and HW
 *      simultaneously.
 *
 */

/**
 * @defgroup dbg_prj_structure_g Project structure
 * @ingroup debugger_group
 * @page dbg_prj_structure Project structure
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
 */

 /**
 * @defgroup dbg_connect_g Debug session
 * @ingroup debugger_group
 * @page dbg_connect Debug session
 *
 * @section dbg_connect_1 Plugins interaction
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
 * GUI plugin uses QT-libraries and interacts with the core library using the
 * text console input interface. GUI generates the same text commands 
 * that are available in debugger console for any who's using this debugger.
 * That's why any presented in GUI widgets information can be achieved
 * in console mode.
 *
 * @section dbg_connect_2 Start Debugger
 *
 * We provide several targets that can run software (bootloader, firmware
 * or user specific application) without any source code modifications:
 * 
 * Start Configuration            | Description
 * -------------------------------|-----------------
 * $ ./_run_functional_sim.sh[bat]| Functional RISC-V Full System Model
 * $ ./_run_systemc_sim.sh[bat]   | Use SystemC Precise Model of RIVER CPU
 * $ ./_run_fpga_gui.sh[bat]      | FPGA board. Default port 'COM3', TAP IP = 192.168.0.51
 *
 * To run debugger with the real FPGA target connected via Ethernet do:
 * @code
 *     # cd rocket_soc/debugger/win32build/debug
 *     # _run_functional_sim.bat
 * @endcode
 *
 * The result should look like on the picture below:
 * 
 * <img src="pics/dbg_gui_start.png" alt="debugger 1-st look"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_gui_start.png}} @endlatexonly
 *
 * @par Example of the debug session
 * Switch ON all User LEDs on board:
 * @code
 *      riscv# help                     -- Print full list of commands
 *      riscv# csr MCPUID               -- Read supported ISA extensions
 *      riscv# read 0xfffff000 20       -- Read 20 bytes from PNP module
 *      riscv# write 0x80000000 4 0xff  -- Write into GPIO new LED value
 *      riscv# loadelf helloworld       -- Load elf-file to board RAM and run
 * @endcode
 *          
 * Console mode view
 *
 * <img src="pics/dbg_testhw.png" alt="HW debug example"> 
 * @latexonly {\includegraphics{pics/dbg_testhw.png}} @endlatexonly
 *
 * @section dbg_connect_3 Debug Zephyr OS kernel with symbols
 *
 * Build Zephyr kernel from scratch using our patches enabling 64-bits RISC-V
 * architecture support:
 * @code
 *     $ mkdir zephyr_160
 *     $ cd zephyr_160
 *     $ git clone https://gerrit.zephyrproject.org/r/zephyr
 *     $ cd zephyr
 *     $ git checkout tags/v1.6.0
 *     $ cp ../../riscv_vhdl/zephyr/v1.6.0-riscv64-base.diff .
 *     $ cp ../../riscv_vhdl/zephyr/v1.6.0-riscv64-exten.diff .
 *     $ git apply v1.6.0-riscv64-base.diff
 *     $ git apply v1.6.0-riscv64-exten.diff
 * @endcode
 *
 * Then build elf-file:
 * @code
 *    $ export ZEPHYR_BASE=/home/zephyr_160/zephyr
 *    $ cd zephyr/samples/shell
 *    $ make ARCH=riscv64 CROSS_COMPILE=/home/your_path/gnu-toolchain-rv64ima/bin/riscv64-unknown-elf- BOARD=riscv_gnss 2>&1
 * @endcode
 *
 * Load debug symbols from elf-file without target reprogramming (or with):
 * @code
 *    riscv# loadelf zephyr.elf
 *    riscv# loadelf zephyr.elf nocode
 * @endcode
 *
 * <img src="pics/dbg_gui_symb.png" alt="debugger symbols"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_gui_symb.png}} @endlatexonly
 * 
 * Now becomes available the following features:
 * - Stack trace with function names
 * - Function names in Disassembler including additional information for
 *   branch and jump instructions in column \c 'comment'.
 * - Symbol Browser with filter.
 * - Opening Disassembler and Memory Viewer widgets in a new window by name.
 *
 * Debugger provides additional features that could simplify software
 * development: 
 * - Clock Per Instruction (CPI) hardware measure
 * - Bus utilization information 
 * - Others. List of a new features is constantly increasing.
 * 
 * <img src="pics/dbg_fpga_gui1.png" alt="debugger FPGA+GUI"> 
 * @latexonly {\includegraphics[scale=0.8]{pics/dbg_fpga_gui1.png}} @endlatexonly
 * 
 */

 /**
 * @defgroup dbg_troubles_g Troubleshooting
 * @ingroup debugger_group
 * @page dbg_troubles Troubleshooting
 *
 * @subpage dbg_trouble_1
 *
 * @subpage dbg_trouble_2
 *
 * @subpage dbg_trouble_3
 *
 */

/**
 * @defgroup dbg_trouble_1_g Image Files not found
 * @ingroup dbg_troubles_g
 * @page dbg_trouble_1 Image Files not found
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
 * absolutely valid for the Simulation. Users shouldn't see any difference
 * between these targets this is our purpose.
 */

 /**
 * @defgroup dbg_trouble_2_g Can't open COM3 when FPGA is used
 * @ingroup dbg_troubles_g
 * @page dbg_trouble_2 Can't open COM3 when FPGA is used
 *
 *    -# Open <em>fpga_gui.json</em>
 *    -# Change value <b>['ComPortName','COM3'],</b> on your one
 *       (for an example on \c ttyUSB0).
 *
 */

 /**
 * @defgroup dbg_trouble_3_g EDCL: No response. Break read transaction
 * @ingroup dbg_troubles_g
 * @page dbg_trouble_3 EDCL: No response. Break read transaction
 *
 * This error means that host cannot locate board with specified IP address.
 * Before you continue pass through the following checklist:
 *    -# You should properly @link eth_link setup network connection @endlink
 * and see FPGA board in ARP-table.
 *    -# If you've changed default FPGA IP address:
 *          -# Open <em>_run_fpga_gui.bat (*.sh)</em>
 *          -# Change value <b>['BoardIP','192.168.0.51']</b> on your one.
 *    -# Run debugger
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
 * @defgroup dbg_core_api_g Core API methods
 * @ingroup debugger_group
 * @details Core methods that allow create, modify and delete base library
 *          objects such as: Attributes, Classes, Services and Interfaces
 * @{
 */

/**
 * @brief Library initialization.
 * @details This method must be called before any other from this library.
 */
int RISCV_init();

/**
 * @brief Destroy and cleanup all dynamically allocated objects.
 * @details This method allows gracefully close library by stopping all running
 *          threads and free allocated resources.
 */
void RISCV_cleanup();

/** 
 * @brief Set core library configuration.
 * @details Configuration specify all instantiated services and interconnect
 *          among them.
 * @param [in] cfg Configuration attribute.
 */
int RISCV_set_configuration(AttributeType *cfg);

/** 
 * @brief Read library configuration.
 * @details This method allows serialize library state and save configuration
 *          into the file in JSON format. Afterward configuration can be
 *          restored.
 */
const char *RISCV_get_configuration();

/** 
 * @brief Get current core configuration.
 * @details JSON configuration string implements special section \c 'Global'
 *          that contains parameters not related to any specific service or
 *          class.
 */
const AttributeType *RISCV_get_global_settings();

/**
 * @brief Registration of the class in the library kernel.
 * @details Registering interface pointer will be put into kernel list of
 *          classes. Any plugin can add its own class interfaces.
 * @param [in] icls Pointer on new class interface.
 */
void RISCV_register_class(IFace *icls);

/**
 * @brief Registration of the system event (hap) listener.
 * @details Haps are used to synchronized different threads by a specific
 *          events in a system. Now there's used such haps as: 
 *             - ConfigDone
 *             - Breakpoint
 */
void RISCV_register_hap(IFace *ihap);

/**
 * @brief Trigger system event (hap) from Service.
 * @details This method allows to call all registered listeneres of a specific
 *          event from running Service.
 */
void RISCV_trigger_hap(IFace *isrc, int type, const char *descr);

/**
 * @brief Get registred class interface by its name.
 * @details This method generally used to create instances of a specific
 *          service.
 */
IFace *RISCV_get_class(const char *name);

/**
 * @brief Create service of the specified class.
 * @details This method creates intstance of Service and assignes all
 *          registered attributes to its initial values.
 */
IFace *RISCV_create_service(IFace *iclass, const char *name, 
                                        AttributeType *args);

/**
 * @brief Get IService interface by its name.
 * @details This method is used for interaction of different services in a
 *          system.
 */
IFace *RISCV_get_service(const char *name);

/**
 * @brief Get interface of the specified ervice.
 * @details This method can be used in runtime to implement dynamic connection
 *          of different services
 * @code
 *     ...
 *     IUdp *iudp1 = static_cast<IUdp *>
 *               (RISCV_get_service_iface("udpboard", IFACE_UDP));
 *     ...
 * @endcode
 */
IFace *RISCV_get_service_iface(const char *servname, const char *facename);

/**
 * @brief Get list of services implementing specific interface.
 * @details This method can return list of services of different classes
 *          and implementing different functionality.
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
 * @details This method gracefully stops all threads and allows to avoid
 *          simulation hanging on library closing stage.
 */
void RISCV_break_simulation();

/**
 * @brief Run main loop in main thread
 */
void RISCV_dispatcher_start();

/**
 * @brief callback from the main thread function prototype
 */
typedef void (*timer_callback_type)(void *args);

/**
 * @brief Register timer's callback in main loop
 */
void RISCV_register_timer(int msec, int single_shot,
                          timer_callback_type cb, void *args);

/**
 * @brief Unregister timer's callback from main loop
 */
void RISCV_unregister_timer(timer_callback_type cb);


#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_CORE_H__
