/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Clock interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>
#include "iclklistener.h"

namespace debugger {

static const char *const IFACE_CLOCK = "IClock";

class IClock : public IFace {
 public:
    IClock() : IFace(IFACE_CLOCK) {}

    virtual uint64_t getStepCounter() = 0;

    /** Executed instruction counter.
     *
     * One executed instruction = 1 step for functional simulation.
     * And it can be more than 1 for precise SystemC model if enabled
     * GENERATE_CORE_TRACE.
     */
    virtual void registerStepCallback(IClockListener *cb, uint64_t t) = 0;

    virtual double getFreqHz() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__
