/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API methods declaration.
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
void RISCV_set_configuration(const char *config);

/** 
 * @brief Get current core configuration.
 */
const char *RISCV_get_configuration();

/**
 * @brief Registration of the class in the library kernel.
 */
void RISCV_register_class(IFace *icls);

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

/// @todo add attributes/save/restore

#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_CORE_H__
