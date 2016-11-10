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
#include "attribute.h"

namespace debugger {

static const char *const IFACE_CLOCK = "IClock";

class IClock : public IFace {
public:
    IClock() : IFace(IFACE_CLOCK) {}

    virtual uint64_t getStepCounter() =0;

    /** Executed instruction counter. One executed instruction = 1 step. */
    virtual void registerStepCallback(IClockListener *cb, uint64_t t) =0;

    virtual uint64_t getClockCounter() =0;

    /** For Functional models one clock always = one step. */
    virtual void registerClockCallback(IClockListener *cb, uint64_t t) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ICLOCK_H__
