/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_API_CORE_H__
#define __DEBUGGER_API_CORE_H__

#include <api_utils.h>
#include <iface.h>
#include <attribute.h>

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
void RISCV_get_configuration(AttributeType *cfg);

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
 * @brief Get interface of the specified device.
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
 * @brief Get interface of the specified device:port.
 * @details This method can be used in runtime to implement dynamic connection
 *          of different services and their ports
 * @code
 *     ...
 *     IMemoryOperation *imem = static_cast<IMemoryOperation *>
 *              (RISCV_get_service_port_iface("mem0", "port0",
 *                                        IFACE_MEMORY_OPERATION));
 *     ...
 * @endcode
 */
IFace *RISCV_get_service_port_iface(const char *servname,
                                    const char *portname,
                                    const char *facename);

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
