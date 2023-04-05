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

#include "api_types.h"
#include <iface.h>
#include <attribute.h>
#include <stdarg.h>

namespace debugger {

#define LOG_ERROR     1
#define LOG_IMPORTANT 2   // can be used for the autotest messages
#define LOG_INFO      3
#define LOG_DEBUG     4

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
 * @brief Remove class from the library kernel.
 * @details If the class with the defined name will be found in the list of
            of registered classes it will be removed and all plugins loose their
            capability to create service of this class.
 * @param [in] clsname String name of the class to remove.
 */
void RISCV_unregister_class(const char *clsname);

/**
 * @brief Registration of the service instance in the library kernel.
 * @details Registering interface pointer will be put into kernel list of
 *          services. Services coudl be registered without class registration.
 * @param [in] isrv Pointer on new service interface.
 */
void RISCV_register_service(IFace *isrv);

/**
 * @brief Remove service instance from the library kernel.
 * @details If the service with the defined name will be found in the list of
            of registered objects it will be removed.
 * @param [in] srvname String name of the service instance to remove.
 */
void RISCV_unregister_service(const char *srvname);

/**
 * @brief Registration of the system event (hap) listener.
 * @details Haps are used to synchronized different threads by a specific
 *          events in a system. Now there's used such haps as: 
 *             - ConfigDone
 *             - Breakpoint
 */
void RISCV_register_hap(IFace *ihap);

/**
 * @brief Remove the system event (hap) listener from the core list of haps.
 */
void RISCV_unregister_hap(IFace *ihap);

/**
 * @brief Trigger system event (hap) from Service.
 * @details This method allows to call all registered listeneres of a specific
 *          event from running Service.
 */
void RISCV_trigger_hap(int type, uint64_t param, const char *descr);

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
 * @brief Get list of interfaces implemented by services or service's ports.
 */
void RISCV_get_iface_list(const char *iname, AttributeType *list);

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

/** Generate unique string. */
void RISCV_generate_name(AttributeType *name);

/** Redirect output to specified console. */
void RISCV_add_default_output(void *iout);
void RISCV_remove_default_output(void *iout);

/** Provide access to step counter shown in all messages. */
void RISCV_set_default_clock(void *iclk);

/** 
 * @brief Write output to logfile.
 * @return 0 on success, 1 when failed
 */
int RISCV_enable_log(const char *filename);

void RISCV_disable_log();

/** Format output to string. */
int RISCV_sprintf(char *s, size_t len, const char *fmt, ...);

/** Format output to the default stream. */
int RISCV_printf(void *iface, int level, const char *fmt, ...);

/** Format input data */
int RISCV_sscanf(const char *s, const char *fmt, ...);

/** Output always */
#define RISCV_printf0(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), 0, fmt, __VA_ARGS__)

/** Output with the maximal logging level */
#define RISCV_error(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_ERROR, "%s:%d " fmt, \
                 __FILE__, __LINE__, __VA_ARGS__)

/** Output with the information logging level */
#define RISCV_important(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_IMPORTANT, fmt, __VA_ARGS__)

/** Output with the information logging level */
#define RISCV_info(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_INFO, fmt, __VA_ARGS__)

/** Output with the lower logging level */
#define RISCV_debug(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_DEBUG, fmt, __VA_ARGS__)

/** Suspend thread on certain number of milliseconds */
void RISCV_sleep_ms(int ms);

/** Get current time in milliseconds. */
uint64_t RISCV_get_time_ms();

/** Get process ID. */
int RISCV_get_pid();

/** Memory barrier */
void RISCV_memory_barrier();

void RISCV_thread_create(void *data);
uint64_t RISCV_thread_id();

int RISCV_mutex_init(mutex_def *mutex);
int RISCV_mutex_lock(mutex_def *mutex);
int RISCV_mutex_unlock(mutex_def *mutex);
int RISCV_mutex_destroy(mutex_def *mutex);
void RISCV_thread_join(thread_def th, int ms);

void RISCV_event_create(event_def *ev, const char *name);
void RISCV_event_close(event_def *ev);
void RISCV_event_set(event_def *ev);
int RISCV_event_is_set(event_def *ev);
void RISCV_event_clear(event_def *ev);
void RISCV_event_wait(event_def *ev);
int RISCV_event_wait_ms(event_def *ev, int ms);

sharemem_def RISCV_memshare_create(const char *name, int sz);
void* RISCV_memshare_map(sharemem_def h, int sz);
void RISCV_memshare_unmap(void *buf, int sz);
void RISCV_memshare_delete(sharemem_def h);

/** Memory allocator/de-allocator */
void *RISCV_malloc(uint64_t sz);
void RISCV_free(void *p);

/** Get absolute directory where core library is placed. */
int RISCV_get_core_folder(char *out, int sz);
int RISCV_get_core_folderw(wchar_t* out, int sz);

/** Set $(pwd) directory equals to executable location */
void RISCV_set_current_dir();

/** Execute shell command */
int RISCV_system(const char *cmd);

/** Reading configuration from JSON formatted file. */
int RISCV_read_json_file(const char *filename, void *outattr);

/** Write configuration string to JSON formatted file. */
void RISCV_write_json_file(const char *filename, const char *s);

#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_CORE_H__
