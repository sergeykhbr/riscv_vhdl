/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      PLL interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IPLL_H__
#define __DEBUGGER_COMMON_CORESERVICES_IPLL_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_PLL = "IPLL";

class IPLL : public IFace {
 public:
    IPLL() : IFace(IFACE_PLL) {}

    virtual uint64_t getBusClockHz() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IPLL_H__
