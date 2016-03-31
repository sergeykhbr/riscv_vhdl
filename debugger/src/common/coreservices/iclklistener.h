/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Clock listener interface.
 */

#ifndef __DEBUGGER_PLUGIN_ICLOCK_LISTENER_H__
#define __DEBUGGER_PLUGIN_ICLOCK_LISTENER_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_CLOCK_LISTENER = "IClockListener";

class IClockListener : public IFace {
public:
    IClockListener() : IFace(IFACE_CLOCK_LISTENER) {}

    virtual void stepCallback(uint64_t t) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ICLOCK_LISTENER_H__
