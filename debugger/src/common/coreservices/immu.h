/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      HC08 MMU interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IMMU_H__
#define __DEBUGGER_COMMON_CORESERVICES_IMMU_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_MMU = "IMMU";

class IMMU : public IFace {
 public:
    IMMU() : IFace(IFACE_MMU) {}

    virtual uint32_t get_ppage() = 0;
    virtual void set_ppage(uint8_t v) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IMMU_H__
