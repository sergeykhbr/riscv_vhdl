/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Clock interface.
 */

#ifndef __DEBUGGER_PLUGIN_ICLOCK_H__
#define __DEBUGGER_PLUGIN_ICLOCK_H__

#include "iface.h"
#include <inttypes.h>
#include "iclklistener.h"

namespace debugger {

static const char *const IFACE_CLOCK = "IClock";

class IClock : public IFace {
public:
    IClock() : IFace(IFACE_CLOCK) {}

    virtual uint64_t getStepCounter() =0;

    virtual void registerStepCallback(IClockListener *cb, uint64_t t) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ICLOCK_H__
